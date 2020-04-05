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

graphite::qd::pixmap::pixmap(qd::rect frame)
    : m_base_address(0x000000ff),
      m_row_bytes(frame.width() * 4),
      m_bounds(frame),
      m_pm_version(0),
      m_pack_type(4),
      m_pack_size(0),
      m_h_res(0.001098632812),
      m_v_res(0.001098632812),
      m_pixel_type(16),
      m_pixel_size(32),
      m_cmp_count(3),
      m_cmp_size(8),
      m_pixel_format(unknown),
      m_pm_table(0),
      m_pm_extension(0)
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
    m_pack_size = px_reader.read_signed_long();
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

// MARK: -

void graphite::qd::pixmap::write(graphite::data::writer& writer)
{
    writer.write_long(m_base_address);
    writer.write_short(0x8000 | m_row_bytes);
    m_bounds.write(writer, rect::qd);
    writer.write_short(m_pm_version);
    writer.write_short(m_pack_type);
    writer.write_long(m_pack_size);
    writer.write_short(static_cast<uint32_t>(m_h_res * (1 << 16)));
    writer.write_short(0);
    writer.write_short(static_cast<uint32_t>(m_v_res * (1 << 16)));
    writer.write_short(0);
    writer.write_short(m_cmp_count);
    writer.write_short(m_cmp_size);
    writer.write_long(static_cast<uint32_t>(m_pixel_format));
    writer.write_long(m_pm_table);
    writer.write_long(m_pm_extension);
    writer.write_long(0); // Reserved.
}