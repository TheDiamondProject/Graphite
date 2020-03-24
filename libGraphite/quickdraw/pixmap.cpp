//
// Created by Tom Hancocks on 19/03/2020.
//

#include "libGraphite/quickdraw/pixmap.hpp"
#include "libGraphite/data/reader.hpp"

// MARK: - Constructors

graphite::qd::pixmap::pixmap()
    : m_bounds(0, 0, 100, 100)
{

}

graphite::qd::pixmap::pixmap(std::shared_ptr<data::data> px_data)
    : m_bounds(0, 0, 0, 0)
{
    // Setup a new data reader for the pixmap
    data::reader px_reader(px_data);

    // Read each of the member fields for the pixmap.
    m_base_address = px_reader.read_long();
    m_row_bytes = px_reader.read_signed_short() & 0x7FFF;
    m_bounds = graphite::qd::rect::read(px_reader, qd::rect::qd);
    m_pm_version = px_reader.read_signed_short();
    m_pack_type = px_reader.read_signed_short();
    m_pack_size = px_reader.read_signed_short();
    m_h_res = static_cast<double>(px_reader.read_signed_long() / static_cast<double>(1 << 16));
    m_v_res = static_cast<double>(px_reader.read_signed_long() / static_cast<double>(1 << 16));
    m_pixel_type = px_reader.read_signed_short();
    m_pixel_size = px_reader.read_signed_short();
    m_cmp_count = px_reader.read_signed_short();
    m_cmp_size = px_reader.read_signed_short();
    m_pixel_format = static_cast<graphite::qd::pixel_format>(px_reader.read_long());
    m_pm_table = px_reader.read_long();
    m_pm_extension = px_reader.read_long();
}

// MARK: - Accessors

graphite::qd::rect graphite::qd::pixmap::bounds() const
{
    return m_bounds;
}

int16_t graphite::qd::pixmap::row_bytes() const
{
    return m_row_bytes;
}

int16_t graphite::qd::pixmap::pack_type() const
{
    return m_pack_type;
}

int16_t graphite::qd::pixmap::pack_size() const
{
    return m_pack_size;
}

int16_t graphite::qd::pixmap::pixel_type() const
{
    return m_pixel_type;
}

int16_t graphite::qd::pixmap::pixel_size() const
{
    return m_pixel_size;
}

int16_t graphite::qd::pixmap::cmp_count() const
{
    return m_cmp_count;
}

int16_t graphite::qd::pixmap::cmp_size() const
{
    return m_cmp_size;
}

enum graphite::qd::pixel_format graphite::qd::pixmap::pixel_format() const
{
    return m_pixel_format;
}