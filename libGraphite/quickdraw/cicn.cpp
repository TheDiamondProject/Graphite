//
// Created by Tom Hancocks on 25/03/2020.
//

#include "libGraphite/quickdraw/cicn.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include <tuple>

// MARK: - Constructor

graphite::qd::cicn::cicn(std::shared_ptr<graphite::data::data> data, int64_t id, std::string name)
        : m_id(id), m_name(name)
{
    data::reader reader(data);
    parse(reader);
}

auto graphite::qd::cicn::load_resource(int64_t id) -> std::shared_ptr<graphite::qd::cicn>
{
    if (auto res = graphite::rsrc::manager::shared_manager().find("cicn", id).lock()) {
        return std::make_shared<graphite::qd::cicn>(res->data(), id, res->name());
    }
    return nullptr;
}


// MARK: - Accessors

auto graphite::qd::cicn::surface() const -> std::weak_ptr<graphite::qd::surface>
{
    return m_surface;
}

// MARK: - Parser

auto graphite::qd::cicn::parse(graphite::data::reader& reader) -> void
{
    m_pixmap = graphite::qd::pixmap(reader.read_data(qd::pixmap::length));
    m_mask_base_addr = reader.read_long();
    m_mask_row_bytes = reader.read_short();
    m_mask_bounds = qd::rect::read(reader);
    m_bmap_base_addr = reader.read_long();
    m_bmap_row_bytes = reader.read_short();
    m_bmap_bounds = qd::rect::read(reader);

    reader.move(4);

    auto mask_data_size = m_mask_row_bytes * m_mask_bounds.height();
    auto bmap_data_size = m_bmap_row_bytes * m_bmap_bounds.height();
    auto pmap_data_size = m_pixmap.row_bytes() * m_pixmap.bounds().height();

    auto mask_data = reader.read_data(mask_data_size);
    auto bmap_data = reader.read_data(bmap_data_size);
    m_clut = qd::clut(reader);
    auto pmap_data = reader.read_data(pmap_data_size);

    // Now that all information has been extracted from the resource, proceed and attempt to render it.
    m_surface = std::make_shared<graphite::qd::surface>(m_pixmap.bounds().width(), m_pixmap.bounds().height());

    if (m_pixmap.cmp_size() == 1 && m_pixmap.cmp_count() == 1) {

        for (auto y = 0; y < m_pixmap.bounds().height(); ++y) {
            auto y_offset = (y * m_pixmap.row_bytes());
            auto mask_y_offset = (y * m_mask_row_bytes);

            for (auto x = 0; x < m_pixmap.bounds().width(); ++x) {
                auto byte_offset = 7 - (x % 8);

                auto byte = pmap_data->at(y_offset + (x / 8));
                auto mask = mask_data->at(mask_y_offset + (x / 8));
                auto v = (byte >> byte_offset) & 0x1;

                if ((mask >> byte_offset) & 0x1) {
                    m_surface->set(x, y, m_clut.get(v));
                }
            }
        }

    }
    else if (m_pixmap.cmp_size() == 1 && m_pixmap.cmp_count() == 2) {

        for (auto y = 0; y < m_pixmap.bounds().height(); ++y) {
            auto y_offset = (y * m_pixmap.row_bytes());
            auto mask_y_offset = (y * m_mask_row_bytes);

            for (auto x = 0; x < m_pixmap.bounds().width(); ++x) {
                auto byte_offset = (3 - (x % 4)) << 1;
                auto mask_offset = (7 - (x % 8));

                auto byte = pmap_data->at(y_offset + (x / 4));
                auto mask = mask_data->at(mask_y_offset + (x / 8));
                auto v = (byte >> byte_offset) & 0x3;

                if ((mask >> mask_offset) & 0x1) {
                    m_surface->set(x, y, m_clut.get(v));
                }
            }
        }

    }
    else if (m_pixmap.cmp_size() == 1 && m_pixmap.cmp_count() == 4) {

        for (auto y = 0; y < m_pixmap.bounds().height(); ++y) {
            auto y_offset = (y * m_pixmap.row_bytes());
            auto mask_y_offset = (y * m_mask_row_bytes);

            for (auto x = 0; x < m_pixmap.bounds().width(); ++x) {
                auto byte_offset = (1 - (x % 2)) << 2;
                auto mask_offset = (7 - (x % 8));

                auto byte = pmap_data->at(y_offset + (x / 2));
                auto mask = mask_data->at(mask_y_offset + (x / 8));
                auto v = (byte >> byte_offset) & 0xF;

                if ((mask >> mask_offset) & 0x1) {
                    m_surface->set(x, y, m_clut.get(v));
                }
            }
        }

    }
    else if (m_pixmap.cmp_size() == 1 && m_pixmap.cmp_count() == 8) {

        for (auto y = 0; y < m_pixmap.bounds().height(); ++y) {
            auto y_offset = (y * m_pixmap.row_bytes());
            auto mask_y_offset = (y * m_mask_row_bytes);

            for (auto x = 0; x < m_pixmap.bounds().width(); ++x) {
                auto mask_offset = (7 - (x % 8));

                auto byte = static_cast<uint8_t>(pmap_data->at(y_offset + x));
                auto mask = mask_data->at(mask_y_offset + (x / 8));

                if ((mask >> mask_offset) & 0x1) {
                    m_surface->set(x, y, m_clut.get(byte));
                }
            }
        }

    }
    else {
        throw std::runtime_error("Currently unsupported cicn configuration: cmp_size=" +
                                 std::to_string(m_pixmap.cmp_size()) +
                                 ", cmp_count=" + std::to_string(m_pixmap.cmp_count()));
    }
}

// MARK: - Encoder

auto graphite::qd::cicn::data() -> std::shared_ptr<graphite::data::data>
{
    auto data = std::shared_ptr<graphite::data::data>();
    auto writer = graphite::data::writer(data);
    auto width = m_surface->size().width();
    auto height = m_surface->size().height();

    // Rebuild the Color Table for the surface. To do this we want to create an empty table, and populate it.
    m_clut = qd::clut();
    std::vector<uint16_t> color_values;
    std::vector<bool> mask_values;
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            auto color = m_surface->at(x, y);
            mask_values.emplace_back((color.alpha_component() & 0x80) != 0);
            color_values.emplace_back(m_clut.set(color));
        }
    }

    // Determine what component configuration we need.
    m_pixmap = qd::pixmap();
    graphite::data::writer mask_data(std::make_shared<graphite::data::data>());
    graphite::data::writer bmap_data(std::make_shared<graphite::data::data>());
    graphite::data::writer pmap_data(std::make_shared<graphite::data::data>());
    m_mask_row_bytes = m_bmap_row_bytes = width >> 3;

    bmap_data.write_byte(0, m_bmap_row_bytes * height);

    // Construct the mask data for the image.
    uint8_t scratch = 0;
    for (auto n = 0; n < mask_values.size(); ++n) {
        // We need to write the scratch byte every 8th bit that is visited, and clear it.
        auto bit_offset = n % 8;
        if (bit_offset == 0 && n != 0) {
            mask_data.write_byte(scratch);
            scratch = 0;
        }
        uint8_t value = mask_values[n] ? 1 : 0;
        value <<= (7 - bit_offset);
        scratch |= value;
    }

    if (m_clut.size() >= 256) {
        throw std::runtime_error("Implementation does not currently handle more than 256 colors in a CICN");
    }
    else if (m_clut.size() >= 16) {
        m_pixmap.set_cmp_size(1);
        m_pixmap.set_cmp_count(8);
        m_pixmap.set_row_bytes(m_surface->size().width());

        for (auto n = 0; n < color_values.size(); ++n) {
            pmap_data.write_byte(static_cast<uint8_t>(color_values[n] & 0xFF));
        }
    }
    else if (m_clut.size() >= 4) {
        m_pixmap.set_cmp_size(1);
        m_pixmap.set_cmp_count(4);
        m_pixmap.set_row_bytes(m_surface->size().width() >> 1);

        scratch = 0;
        for (auto n = 0; n < color_values.size(); ++n) {
            auto bit_offset = n % 2;
            if (bit_offset == 0 && n != 0) {
                pmap_data.write_byte(scratch);
                scratch = 0;
            }
            uint8_t value = static_cast<uint8_t>(color_values[n] & 0xF);
            value <<= (4 - (bit_offset * 4));
            scratch |= value;
        }
    }
    else if (m_clut.size() >= 2) {
        m_pixmap.set_cmp_size(1);
        m_pixmap.set_cmp_count(2);
        m_pixmap.set_row_bytes(m_surface->size().width() >> 2);

        scratch = 0;
        for (auto n = 0; n < color_values.size(); ++n) {
            auto bit_offset = n % 4;
            if (bit_offset == 0 && n != 0) {
                pmap_data.write_byte(scratch);
                scratch = 0;
            }
            uint8_t value = static_cast<uint8_t>(color_values[n] & 0x3);
            value <<= (6 - (bit_offset * 2));
            scratch |= value;
        }
    }
    else {
        m_pixmap.set_cmp_size(1);
        m_pixmap.set_cmp_count(1);
        m_pixmap.set_row_bytes(m_surface->size().width() >> 3);

        scratch = 0;
        for (auto n = 0; n < color_values.size(); ++n) {
            auto bit_offset = n % 8;
            if (bit_offset == 0 && n != 0) {
                pmap_data.write_byte(scratch);
                scratch = 0;
            }
            uint8_t value = static_cast<uint8_t>(color_values[n] & 0x1);
            value <<= (7 - bit_offset);
            scratch |= value;
        }
    }

    // Write out the image data for the cicn.
    writer.write_data(mask_data.data());
    writer.write_data(bmap_data.data());
    m_clut.write(writer);
    writer.write_data(pmap_data.data());

    return data;
}
