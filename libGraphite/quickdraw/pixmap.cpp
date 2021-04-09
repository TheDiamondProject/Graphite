//
// Created by Tom Hancocks on 19/03/2020.
//

#include "libGraphite/quickdraw/pixmap.hpp"
#include "libGraphite/quickdraw/clut.hpp"
#include "libGraphite/quickdraw/internal/surface.hpp"
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
    data::reader px_reader(std::move(px_data));

    // Read each of the member fields for the pixmap.
    m_base_address = px_reader.read_long();
    m_row_bytes = static_cast<int16_t>(static_cast<uint16_t>(px_reader.read_signed_short()) & 0x7FFFU);
    m_bounds = graphite::qd::rect::read(px_reader, qd::rect::qd);
    m_pm_version = px_reader.read_signed_short();
    m_pack_type = px_reader.read_signed_short();
    m_pack_size = px_reader.read_signed_long();
    m_h_res = static_cast<double>(px_reader.read_signed_long() / static_cast<double>(1U << 16UL));
    m_v_res = static_cast<double>(px_reader.read_signed_long() / static_cast<double>(1U << 16UL));
    m_pixel_type = px_reader.read_signed_short();
    m_pixel_size = px_reader.read_signed_short();
    m_cmp_count = px_reader.read_signed_short();
    m_cmp_size = px_reader.read_signed_short();
    m_pixel_format = static_cast<graphite::qd::pixel_format>(px_reader.read_long());
    m_pm_table = px_reader.read_long();
    m_pm_extension = px_reader.read_long();
}

// MARK: - Accessors

auto graphite::qd::pixmap::bounds() const -> graphite::qd::rect
{
    return m_bounds;
}

auto graphite::qd::pixmap::set_bounds(const graphite::qd::rect& rect) -> void
{
    m_bounds = rect;
}

auto graphite::qd::pixmap::row_bytes() const -> int16_t
{
    return m_row_bytes;
}

auto graphite::qd::pixmap::set_row_bytes(const int16_t& row_bytes) -> void
{
    m_row_bytes = row_bytes;
}

auto graphite::qd::pixmap::pack_type() const -> int16_t
{
    return m_pack_type;
}

auto graphite::qd::pixmap::set_pack_type(const int16_t& pack_type) -> void
{
    m_pack_type = pack_type;
}

auto graphite::qd::pixmap::pack_size() const -> int16_t
{
    return m_pack_size;
}

auto graphite::qd::pixmap::set_pack_size(const int16_t& pack_size) -> void
{
    m_pack_size = pack_size;
}

auto graphite::qd::pixmap::pixel_type() const -> int16_t
{
    return m_pixel_type;
}

auto graphite::qd::pixmap::set_pixel_type(const int16_t& pixel_type) -> void
{
    m_pixel_type = pixel_type;
}

auto graphite::qd::pixmap::pixel_size() const -> int16_t
{
    return m_pixel_size;
}

auto graphite::qd::pixmap::set_pixel_size(const int16_t& pixel_size) -> void
{
    m_pixel_size = pixel_size;
}

auto graphite::qd::pixmap::cmp_count() const -> int16_t
{
    return m_cmp_count;
}

auto graphite::qd::pixmap::set_cmp_count(const int16_t& cmp_count) -> void
{
    m_cmp_count = cmp_count;
}

auto graphite::qd::pixmap::cmp_size() const -> int16_t
{
    return m_cmp_size;
}

auto graphite::qd::pixmap::set_cmp_size(const int16_t& cmp_size) -> void
{
    m_cmp_size = cmp_size;
}

auto graphite::qd::pixmap::pixel_format() const -> enum graphite::qd::pixel_format
{
    return m_pixel_format;
}

auto graphite::qd::pixmap::pm_table() const -> uint32_t
{
    return m_pm_table;
}

auto graphite::qd::pixmap::set_pm_table(const uint32_t& pm_table) -> void
{
    m_pm_table = pm_table;
}

// MARK: -

auto graphite::qd::pixmap::build_surface(
    std::shared_ptr<graphite::qd::surface> surface,
    const std::vector<uint8_t>& pixel_data,
    const qd::clut& clut,
    qd::rect destination) -> void
{
    if (pixel_data.size() < destination.height() * m_row_bytes) {
        throw std::runtime_error("Insufficent data to build surface from pixmap.");
    }
    auto pixel_size = m_cmp_size * m_cmp_count;
    
    if (pixel_size == 8) {
        for (auto y = 0; y < destination.height(); ++y) {
            auto y_offset = (y * m_row_bytes);
            for (auto x = 0; x < destination.width(); ++x) {
                auto byte = pixel_data[y_offset + x];
                surface->set(destination.x() + x, destination.y() + y, clut.get(byte));
            }
        }
    }
    else {
        auto mod = 8 / pixel_size;
        auto mask = (1 << pixel_size) - 1;
        auto diff = 8 - pixel_size;

        for (auto y = 0; y < destination.height(); ++y) {
            auto y_offset = (y * m_row_bytes);
            for (auto x = 0; x < destination.width(); ++x) {
                auto byte = pixel_data[y_offset + (x / mod)];
                auto byte_offset = diff - ((x % mod) * pixel_size);
                auto v = (byte >> byte_offset) & mask;
                surface->set(destination.x() + x, destination.y() + y, clut.get(v));
            }
        }
    }
}

// MARK: -

auto graphite::qd::pixmap::build_pixel_data(const std::vector<uint16_t>& color_values, uint16_t pixel_size) -> std::shared_ptr<graphite::data::data>
{
    graphite::data::writer pmap_data(std::make_shared<graphite::data::data>());
    m_pixel_size = m_cmp_size = pixel_size;
    m_cmp_count = 1;

    if (pixel_size == 8) {
        m_row_bytes = m_bounds.width();
        for (auto color_value : color_values) {
            pmap_data.write_byte(static_cast<uint8_t>(color_value & 0xFF));
        }
    }
    else {
        auto width = m_bounds.width();
        auto mod = 8 / pixel_size;
        m_row_bytes = (width - 1) / mod + 1;
        auto mask = (1 << pixel_size) - 1;
        auto diff = 8 - pixel_size;

        for (auto y = 0; y < m_bounds.height(); ++y) {
            uint8_t scratch = 0;
            for (auto x = 0; x < width; ++x) {
                auto bit_offset = x % mod;
                if (bit_offset == 0 && x != 0) {
                    pmap_data.write_byte(scratch);
                    scratch = 0;
                }
                auto n = y * width + x;
                auto value = static_cast<uint8_t>(color_values[n] & mask);
                value <<= (diff - (bit_offset * pixel_size));
                scratch |= value;
            }
            pmap_data.write_byte(scratch);
        }
    }

    return pmap_data.data();
}

auto graphite::qd::pixmap::write(graphite::data::writer& writer) -> void
{
    writer.write_long(m_base_address);
    writer.write_short(0x8000 | m_row_bytes);
    m_bounds.write(writer, rect::qd);
    writer.write_signed_short(m_pm_version);
    writer.write_signed_short(m_pack_type);
    writer.write_signed_long(m_pack_size);
    writer.write_signed_long(static_cast<int32_t>(m_h_res * (1 << 16)));
    writer.write_signed_long(static_cast<int32_t>(m_v_res * (1 << 16)));
    writer.write_signed_short(m_pixel_type);
    writer.write_signed_short(m_pixel_size);
    writer.write_signed_short(m_cmp_count);
    writer.write_signed_short(m_cmp_size);
    writer.write_signed_long(static_cast<int32_t>(m_pixel_format));
    writer.write_long(m_pm_table);
    writer.write_signed_long(m_pm_extension);
}
