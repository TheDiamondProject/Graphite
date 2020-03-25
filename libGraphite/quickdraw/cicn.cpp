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

std::weak_ptr<graphite::qd::surface> graphite::qd::cicn::image_surface() const
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

    auto mask_data_size = m_mask_row_bytes * m_mask_bounds.height();
    auto bmap_data_size = m_bmap_row_bytes * m_bmap_bounds.height();
    auto pmap_data_size = m_pixmap.row_bytes() * m_pixmap.bounds().height();

    auto mask_data = reader.read_data(mask_data_size);
    auto bmap_data = reader.read_data(bmap_data_size);
    m_clut = qd::clut(reader);
    auto pmap_data = reader.read_data(pmap_data_size);
}