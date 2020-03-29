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

std::shared_ptr<graphite::qd::cicn> graphite::qd::cicn::load_resource(int64_t id)
{
    if (auto res = graphite::rsrc::manager::shared_manager().find("cicn", id).lock()) {
        return std::make_shared<graphite::qd::cicn>(res->data(), id, res->name());
    }
    return nullptr;
}


// MARK: - Accessors

std::weak_ptr<graphite::qd::surface> graphite::qd::cicn::surface() const
{
    return m_surface;
}

// MARK: - Parser

void graphite::qd::cicn::parse(graphite::data::reader& reader)
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