// Copyright (c) 2022 Tom Hancocks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "libGraphite/quickdraw/support/pixmap.hpp"

// MARK: - Construction

graphite::quickdraw::pixmap::pixmap(const rect<std::int16_t>& frame)
    : m_bounds(frame),
      m_row_bytes(frame.size.width * constants::color_width)
{
}

graphite::quickdraw::pixmap::pixmap(const data::block &data, rsrc::resource::identifier id, const std::string &name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::quickdraw::pixmap::pixmap(data::reader &reader)
{
    decode(reader);
}

// MARK: - Accessors

auto graphite::quickdraw::pixmap::bounds() const -> rect<std::int16_t>
{
    return m_bounds;
}

auto graphite::quickdraw::pixmap::row_bytes() const -> std::int16_t
{
    return m_row_bytes;
}

auto graphite::quickdraw::pixmap::pack_type() const -> std::int16_t
{
    return m_pack_type;
}

auto graphite::quickdraw::pixmap::pack_size() const -> std::int16_t
{
    return m_pack_size;
}

auto graphite::quickdraw::pixmap::pixel_type() const -> std::int16_t
{
    return m_pixel_type;
}

auto graphite::quickdraw::pixmap::pixel_size() const -> std::int16_t
{
    return m_pixel_size;
}

auto graphite::quickdraw::pixmap::component_count() const -> std::int16_t
{
    return m_component_count;
}

auto graphite::quickdraw::pixmap::component_size() const -> std::int16_t
{
    return m_component_size;
}

auto graphite::quickdraw::pixmap::pixel_format() const -> enum pixel_format
{
    return m_pixel_format;
}

auto graphite::quickdraw::pixmap::pm_table() const -> std::uint32_t
{
    return m_pm_table;
}

auto graphite::quickdraw::pixmap::set_bounds(const rect<std::int16_t>& rect) -> void
{
    m_bounds = rect;
}

auto graphite::quickdraw::pixmap::set_row_bytes(std::int16_t row_bytes) -> void
{
    m_row_bytes = row_bytes;
}

auto graphite::quickdraw::pixmap::set_pack_type(std::int16_t pack_type) -> void
{
    m_pack_type = pack_type;
}

auto graphite::quickdraw::pixmap::set_pack_size(std::int16_t pack_size) -> void
{
    m_pack_size = pack_size;
}

auto graphite::quickdraw::pixmap::set_pixel_type(std::int16_t pixel_type) -> void
{
    m_pixel_type = pixel_type;
}

auto graphite::quickdraw::pixmap::set_pixel_size(std::int16_t pixel_size) -> void
{
    m_pixel_size = pixel_size;
}

auto graphite::quickdraw::pixmap::set_component_count(std::int16_t component_count) -> void
{
    m_component_count = component_count;
}

auto graphite::quickdraw::pixmap::set_component_size(std::int16_t component_size) -> void
{
    m_component_size = component_size;
}

auto graphite::quickdraw::pixmap::set_pixel_format(enum pixel_format format) -> void
{
    m_pixel_format = format;
}

auto graphite::quickdraw::pixmap::set_pm_table(std::uint32_t table) -> void
{
    m_pm_table = table;
}

// MARK: - Calculations

auto graphite::quickdraw::pixmap::total_component_width() const -> std::size_t
{
    return m_component_size * m_component_count;
}

// MARK: - Draw Configuration

auto graphite::quickdraw::pixmap::basic_draw_configuration() const -> struct draw_configuration
{
    struct draw_configuration cfg;
    cfg.pixmap.base_address = m_base_address; // Check that this is actually the correct value here?
    cfg.pixmap.row_bytes = m_row_bytes;
    cfg.pixmap.bounds = m_bounds;
    return std::move(cfg);
}

auto graphite::quickdraw::pixmap::draw_configuration::aspect::expected_data_size() const -> std::size_t
{
    return row_bytes * bounds.size.height;
}

// MARK: - Encoding / Decoding

auto graphite::quickdraw::pixmap::decode(data::reader &reader) -> void
{
    m_base_address = reader.read_long();
    m_row_bytes = static_cast<std::int16_t>(static_cast<std::uint16_t>(reader.read_signed_short()) & 0x7FFF);
    m_bounds = reader.read<rect<std::int16_t>>();
    m_pm_version = reader.read_signed_short();
    m_pack_type = reader.read_signed_short();
    m_pack_size = reader.read_signed_long();
    m_dpi = reader.read<size<double>>();
    m_pixel_type = reader.read_signed_short();
    m_pixel_size = reader.read_signed_short();
    m_component_count = reader.read_signed_short();
    m_component_size = reader.read_signed_short();
    m_pixel_format = reader.read_enum<enum pixel_format>();
    m_pm_table = reader.read_long();
    m_pm_extension = reader.read_long();
}

auto graphite::quickdraw::pixmap::encode(data::writer &writer) -> void
{
    writer.write_long(m_base_address);
    writer.write_short(0x8000 | m_row_bytes);
    writer.write<rect<std::int16_t>>(m_bounds);
    writer.write_signed_short(m_pm_version);
    writer.write_signed_short(m_pack_type);
    writer.write_signed_long(m_pack_size);
    writer.write<size<double>>(m_dpi);
    writer.write_signed_short(m_pixel_type);
    writer.write_signed_short(m_pixel_size);
    writer.write_signed_short(m_component_count);
    writer.write_signed_short(m_component_size);
    writer.write_enum(m_pixel_format);
    writer.write_long(m_pm_table);
    writer.write_long(m_pm_extension);
}

// MARK: - Surface / Image

auto graphite::quickdraw::pixmap::build_pixel_data(const data::block& color_data, std::uint16_t pixel_size) -> data::block
{
    data::block pixel_map_data;
    data::writer pmap(&pixel_map_data);
    data::reader colors(&color_data);

    m_pixel_size = static_cast<std::int16_t>(pixel_size);
    m_component_count = 1;

    if (pixel_size == 8) {
        m_row_bytes = m_bounds.size.width;
        while (!colors.eof()) {
            pmap.write_byte(colors.read_short() & 0xFF);
        }
    }
    else {
        auto width = m_bounds.size.width;
        auto mod = 8 / pixel_size;
        auto mask = (1 << pixel_size) - 1;
        auto diff = 8 - pixel_size;
        m_row_bytes = (width - 1) / mod + 1;

        for (std::int16_t y = 0; y < m_bounds.size.height; ++y) {
            std::uint8_t scratch = 0;
            for (std::int16_t x = 0; x < width; ++x) {
                auto bit_offset = x % mod;
                if (bit_offset == 0 && x != 0) {
                    pmap.write_byte(scratch);
                    scratch = 0;
                }

                auto value = static_cast<std::uint8_t>(colors.read_short() & mask);
                value <<= (diff - (bit_offset * pixel_size));
                scratch |= value;
            }
            pmap.write_byte(scratch);
        }
    }

    return std::move(pixel_map_data);
}

auto graphite::quickdraw::pixmap::build_surface(surface &surface, const data::block &pixel_data, const clut &clut, const rect<std::int16_t> &destination) -> void
{
    if (pixel_data.size() < destination.size.height * m_row_bytes) {
        throw std::runtime_error("Insufficient data to build surface from PixMap");
    }
    auto pixel_size = m_component_count * m_component_size;

    if (pixel_size == 8) {
        for (std::int16_t y = 0; y < destination.size.height; ++y) {
            auto y_offset = (y * m_row_bytes);
            for (std::int16_t x = 0; x < destination.size.width; ++x) {
                auto byte = pixel_data.get<std::uint8_t>(y_offset + x);
                surface.set(destination.origin.x + x, destination.origin.y + y, clut.at(byte));
            }
        }
    }
    else {
        auto mod = 8 / pixel_size;
        auto mask = (1 << pixel_size) - 1;
        auto diff = 8 - pixel_size;

        for (std::int16_t y = 0; y < destination.size.height; ++y) {
            auto y_offset = (y * m_row_bytes);
            for (std::int16_t x = 0; x < destination.size.width; ++x) {
                auto byte = pixel_data.get<std::uint8_t>(y_offset + (x / mod));
                auto byte_offset = diff - ((x % mod) * pixel_size);
                auto v = (byte >> byte_offset) & mask;
                surface.set(destination.origin.x + x, destination.origin.y + y, clut.at(v));
            }
        }
    }
}

auto graphite::quickdraw::pixmap::draw(quickdraw::surface& surface) -> void
{
    if (m_component_size == 1 && m_component_count == 1) {

    }
    else if ((m_component_size = 1 && m_component_count == 2) || (m_component_size == 2 && m_component_count == 1)) {

    }
    else if ((m_component_size == 1 && m_component_count == 4) || (m_component_size == 4 || m_component_count == 1)) {

    }
    else if ((m_component_size == 1 && m_component_count == 8) || (m_component_size == 8 || m_component_count == 1)) {

    }
    else {
        throw std::runtime_error("Currently unsupported pixel map configuration: "
                                 "cmp_size=" + std::to_string(m_component_size)
                                 + ", cmp_count=" + std::to_string(m_component_count));
    }
}