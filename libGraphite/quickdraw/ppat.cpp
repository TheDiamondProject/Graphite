//
// Created by Tom Hancocks on 17/07/2020.
//

#include "libGraphite/quickdraw/ppat.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include <tuple>
#include <stdexcept>

// MARK: - Constructor

graphite::qd::ppat::ppat(std::shared_ptr<graphite::data::data> data, int64_t id, std::string name)
    : m_id(id), m_name(std::move(name))
{
    data::reader reader(std::move(data));
    parse(reader);
}

graphite::qd::ppat::ppat(std::shared_ptr<qd::surface> surface)
    : m_surface(std::move(surface))
{

}

auto graphite::qd::ppat::load_resource(int64_t id) -> std::shared_ptr<graphite::qd::ppat>
{
    if (auto res = graphite::rsrc::manager::shared_manager().find("ppat", id).lock()) {
        return std::make_shared<graphite::qd::ppat>(res->data(), id, res->name());
    }
    return nullptr;
}


// MARK: - Accessors

auto graphite::qd::ppat::surface() const -> std::weak_ptr<graphite::qd::surface>
{
    return m_surface;
}

// MARK: - Parser

auto graphite::qd::ppat::parse(graphite::data::reader& reader) -> void
{
    m_pat_type = reader.read_short();
    if (m_pat_type != 1) {
        throw std::runtime_error("Currently unsupported ppat configuration: pat_type=" +
                                 std::to_string(m_pat_type));
    }

    m_pmap_base_addr = reader.read_long();
    m_pat_base_addr = reader.read_long();

    reader.set_position(m_pmap_base_addr);
    m_pixmap = graphite::qd::pixmap(reader.read_data(qd::pixmap::length));

    reader.set_position(m_pat_base_addr);
    auto pmap_data_size = m_pixmap.row_bytes() * m_pixmap.bounds().height();
    auto pmap_data = reader.read_bytes(pmap_data_size);

    reader.set_position(m_pixmap.pm_table());
    m_clut = qd::clut(reader);

    // Now that all information has been extracted from the resource, proceed and attempt to render it.
    m_surface = std::make_shared<graphite::qd::surface>(m_pixmap.bounds().width(), m_pixmap.bounds().height());

    m_pixmap.build_surface(m_surface, std::vector<uint8_t>(pmap_data.begin(), pmap_data.end()), m_clut);
}

// MARK: - Encoder

auto graphite::qd::ppat::data() -> std::shared_ptr<graphite::data::data>
{
    auto data = std::make_shared<graphite::data::data>();
    auto writer = graphite::data::writer(data);
    auto width = m_surface->size().width();
    auto height = m_surface->size().height();

    // TODO: This is a brute force method of bringing down the color depth/number of colors required,
    // for a ppat image. It doesn't optimise for image quality at all, and should be replaced at somepoint.
    std::vector<uint16_t> color_values;
    uint8_t pass = 0;
    do {
        if (pass++ > 0) {
            for (auto y = 0; y < height; ++y) {
                for (auto x = 0; x < width; ++x) {
                    auto color = m_surface->at(x, y);
                    m_surface->set(x, y, qd::color(
                            color.red_component() & ~(1 << pass),
                            color.green_component() & ~(1 << pass),
                            color.blue_component() & ~(1 << pass),
                            color.alpha_component()
                    ));
                }
            }
        }

        // Rebuild the Color Table for the surface. To do this we want to create an empty table, and populate it.
        m_clut = qd::clut();
        color_values.clear();
        for (auto y = 0; y < height; ++y) {
            for (auto x = 0; x < width; ++x) {
                auto color = m_surface->at(x, y);
                color_values.emplace_back(m_clut.set(color));
            }
        }
    } while(m_clut.size() >= 256);


    // Determine what component configuration we need.
    m_pixmap = qd::pixmap();
    m_pixmap.set_bounds(qd::rect(point::zero(), m_surface->size()));
    std::shared_ptr<graphite::data::data> pmap_data;
    
    if (m_clut.size() > 256) {
        throw std::runtime_error("Implementation does not currently handle more than 256 colors in a PPAT");
    }
    else if (m_clut.size() > 16) {
        pmap_data = m_pixmap.build_pixel_data(color_values, 8);
    }
    else if (m_clut.size() > 4) {
        pmap_data = m_pixmap.build_pixel_data(color_values, 4);
    }
    else if (m_clut.size() > 2) {
        pmap_data = m_pixmap.build_pixel_data(color_values, 2);
    }
    else {
        pmap_data = m_pixmap.build_pixel_data(color_values, 1);
    }

    // Calculate some offsets
    m_pat_type = 1;
    m_pmap_base_addr = 28;
    m_pat_base_addr = m_pmap_base_addr + 50;
    m_pixmap.set_pm_table(m_pat_base_addr + pmap_data->size());

    // Write out the image data for the ppat.
    writer.write_short(m_pat_type);
    writer.write_long(m_pmap_base_addr);
    writer.write_long(m_pat_base_addr);
    writer.write_long(0);
    writer.write_short(0);
    writer.write_long(0);
    writer.write_quad(0);
    m_pixmap.write(writer);
    writer.write_data(pmap_data);
    m_clut.write(writer);

    return data;
}
