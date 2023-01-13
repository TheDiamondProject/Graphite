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

#include "libGraphite/spriteworld/rleX.hpp"
#include <cmath>

// MARK: - Constants

namespace graphite::spriteworld::constants
{
    static constexpr std::uint16_t rle_grid_width = 6;
    static constexpr std::size_t advance = 2;
}

// MARK: - Construction

graphite::spriteworld::rleX::rleX(data::reader& reader)
{
    decode(reader);
}

graphite::spriteworld::rleX::rleX(const quickdraw::size<std::int16_t> &size, std::uint16_t frame_count)
    : m_id(0), m_name(type_code()), m_frame_size(size), m_frame_count(frame_count), m_bpp(32), m_palette_id(0)
{
    // Determine what the grid will be. We need to round up to the next whole number and have blank tiles
    // if the frame count is not divisible by the grid width constant.
    auto dim = static_cast<std::uint16_t>(std::ceil(std::sqrt(m_frame_count)));
    auto grid_width = std::min(dim, m_frame_count);
    m_grid_size = quickdraw::size<std::int16_t>(grid_width, std::ceil(m_frame_count / static_cast<double>(grid_width)));

    // Create the surface
    m_surface = quickdraw::surface(m_grid_size.width * m_frame_size.width, m_grid_size.height * m_frame_size.height);
}

graphite::spriteworld::rleX::rleX(const data::block& data, rsrc::resource::identifier id, const std::string& name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

// MARK: - Accessors

auto graphite::spriteworld::rleX::surface() -> quickdraw::surface&
{
    return m_surface;
}

auto graphite::spriteworld::rleX::frames() const -> std::vector<quickdraw::rect<std::int16_t>>
{
    return m_frames;
}

auto graphite::spriteworld::rleX::frame_count() const -> std::size_t
{
    return m_frame_count;
}

auto graphite::spriteworld::rleX::data() -> data::block
{
    data::writer writer(data::byte_order::msb);
    encode(writer);
    return std::move(*const_cast<data::block *>(writer.data()));
}

// MARK: - Operations

auto graphite::spriteworld::rleX::frame_rect(std::uint32_t frame) const -> quickdraw::rect<std::int16_t>
{
    return {
        quickdraw::point<std::int16_t>(
            static_cast<std::int16_t>(frame % m_grid_size.width) * m_frame_size.width,
            static_cast<std::int16_t>(frame / m_grid_size.width) * m_frame_size.height
        ),
        m_frame_size
    };
}

auto graphite::spriteworld::rleX::frame_surface(std::uint32_t frame) const -> quickdraw::surface
{
    quickdraw::surface surface(m_frame_size);
    quickdraw::rect<std::int16_t> src_rect(frame_rect(frame));

    // Extract the frame area of the origin surface
    for (std::int16_t y = 0; y < src_rect.size.height; ++y) {
        for (std::int16_t x = 0; x < src_rect.size.width; ++x) {
            surface.set(x, y, m_surface.at(x + src_rect.origin.x, y + src_rect.origin.y));
        }
    }

    return std::move(surface);
}

auto graphite::spriteworld::rleX::write_frame(std::uint32_t frame, const quickdraw::surface &surface) -> void
{
    quickdraw::rect<std::int16_t> dst_rect = frame_rect(frame);
    quickdraw::size<std::int16_t> src_size = surface.size();

    if (src_size.width != m_frame_size.width || src_size.height != m_frame_size.height) {
        throw std::runtime_error("Incorrect frame dimensions " + std::to_string(src_size.width) + "x" + std::to_string(src_size.height) +
                                 ", expected " + std::to_string(m_frame_size.width) + "x" + std::to_string(m_frame_size.height));
    }

    // Copy from the source surface into the destination frame
    for (std::int16_t y = 0; y < src_size.height; ++y) {
        for (std::int16_t x = 0; x < src_size.width; ++x) {
            m_surface.set(x + dst_rect.origin.x, y + dst_rect.origin.y, surface.at(x, y));
        }
    }
}

auto graphite::spriteworld::rleX::surface_offset(std::int32_t frame, std::int32_t line) -> std::uint64_t
{
    quickdraw::point<std::int16_t> fo(frame % constants::rle_grid_width, frame / constants::rle_grid_width);
    quickdraw::point<std::int16_t> p(fo.x * m_frame_size.width, (fo.y * m_frame_size.height) + line);
    return static_cast<uint64_t>(p.y * m_surface.size().width + p.x);
}

// MARK: - Decoding

auto graphite::spriteworld::rleX::decode(data::reader &reader) -> void
{
    // Read the header of the RLE information. This will tell us what we need to do in order to actually
    // decode the frame.
    m_frame_size = quickdraw::size<std::int16_t>::read(reader, quickdraw::coding_type::macintosh);
    m_bpp = reader.read_short();
    m_palette_id = reader.read_short();
    m_frame_count = reader.read_short();
    reader.move(6);

    // Ensure that the RLE has a BPP of 32. This is the only format that we support currently.
    if (m_bpp != 32) {
        throw std::runtime_error("Incorrect color depth for rlëX resource: " + std::to_string(m_id) + ", " + m_name);
    }

    // Determine what the grid will be. We need to round up to the next whole and have blank tiles if the frame count
    // is not divisible by the grid width constant.
    auto dim = static_cast<std::uint16_t>(std::ceil(std::sqrt(m_frame_count)));
    auto grid_width = std::min(dim, m_frame_count);
    m_grid_size = quickdraw::size<std::int16_t>(grid_width, std::ceil(m_frame_count / static_cast<double>(grid_width)));

    // Create the surface in which all frames will be drawn to, and other working variables required to parse and
    // decode the RLE data correctly.
    m_surface = quickdraw::surface(dim * m_frame_size.width, dim * m_frame_size.height, quickdraw::colors::clear());

    quickdraw::color *offset;
    quickdraw::color *right_bound;
    quickdraw::color *frame_bound;
    std::uint32_t pitch = m_surface.size().width - m_frame_size.width;

    auto rect = frame_rect(0);
    auto raw = m_surface.raw().get<quickdraw::color *>();

    
    for (auto frame = 0; frame < m_frame_count; ++frame) {
        rect = this->frame_rect(frame);
        offset = raw + (rect.origin.y * m_surface.size().width) + rect.origin.x;
        right_bound = offset + m_frame_size.width;
        frame_bound = right_bound + m_frame_size.height * m_surface.size().width;
        auto data_size = reader.read_long();
        auto data = reader.read_data(data_size).get<std::uint8_t *>();
        // The 8-byte end marker means we don't need to worry about reading past the end
        auto data_end = data + data_size - 8;
        
        // Decompress the QOI data
        quickdraw::color index[64] = {0};
        auto px = quickdraw::colors::black();
        while (data < data_end) {
            auto op = *data++;
            
            if (op == opcode::rgb) {
                px.components.red   = *data++;
                px.components.green = *data++;
                px.components.blue  = *data++;
                index[hash_color(px)] = px;
            }
            else if (op == opcode::rgba) {
                px = *(quickdraw::color *)data;
                data += 4;
                index[hash_color(px)] = px;
            }
            else if ((op & 0xC0) == opcode::index) {
                px = index[op & 0x3F];
            }
            else if ((op & 0xC0) == opcode::diff) {
                px.components.red   += ((op >> 4) & 0x03) - 2;
                px.components.green += ((op >> 2) & 0x03) - 2;
                px.components.blue  += ( op       & 0x03) - 2;
                index[hash_color(px)] = px;
            }
            else if ((op & 0xC0) == opcode::luma) {
                auto op2 = *data++;
                auto dg = (op & 0x3F) - 32;
                px.components.red   += dg - 8 + ((op2 >> 4) & 0x0F);
                px.components.green += dg;
                px.components.blue  += dg - 8 + ( op2       & 0x0F);
                index[hash_color(px)] = px;
            }
            else if ((op & 0xC0) == opcode::run) {
                auto run = (op & 0x3F) + 1;
                while (run > 0) {
                    if (offset == right_bound) {
                        if (offset == frame_bound) {
                            throw std::runtime_error("Invalid data for rlëX resource: " + std::to_string(m_id) + ", " + m_name);
                        }
                        offset += pitch;
                        right_bound = offset + m_frame_size.width;
                    }
                    auto limit = std::min(offset + run, right_bound);
                    run -= limit - offset;
                    // We don't need to write zero pixels
                    if (px.value != 0) {
                        while (offset < limit) {
                            *offset++ = px;
                        }
                    } else {
                        offset = limit;
                    }
                }
                continue;
            }
            
            if (offset == right_bound) {
                if (offset == frame_bound) {
                    throw std::runtime_error("Invalid data for rlëX resource: " + std::to_string(m_id) + ", " + m_name);
                }
                offset += pitch;
                right_bound = offset + m_frame_size.width;
            }
            *offset++ = px;
        }
    }
}

// MARK: - Encoding

auto graphite::spriteworld::rleX::encode(data::writer &writer) -> void
{
    // Write out the header
    m_frame_size.encode(writer, quickdraw::coding_type::macintosh);
    writer.write_short(m_bpp);
    writer.write_short(m_palette_id);
    writer.write_short(m_frame_count);

    // Reserved fields.
    writer.write_short(0, 3);

    // Write out the RLE frames
    for (auto f = 0; f < m_frame_count; ++f) {
        auto pos = writer.position();
        writer.write_long(0);
        auto size = compress(frame_surface(f), writer);
        writer.set_position(pos);
        writer.write_long(size);
        writer.move(size);
    }
}

auto graphite::spriteworld::rleX::hash_color(const quickdraw::color color) -> std::uint8_t
{
    return (color.components.red   * 3 +
            color.components.green * 5 +
            color.components.blue  * 7 +
            color.components.alpha * 11) % 64;
}

auto graphite::spriteworld::rleX::compress(const quickdraw::surface& frame, data::writer &writer) -> std::size_t
{
    // Compress data using QOI format (https://qoiformat.org)
    const auto pos = writer.position();
    auto offset = 0;
    auto end = frame.size().width * frame.size().height;
    // Trim trailing zero pixels, leaving no more than one
    // This is technically invalid but would still be handled by most decoders
    if (frame.at(end - 1).value == 0) {
        while (end > 1 && frame.at(end - 2).value == 0) {
            end--;
        }
    }
    
    // Index of previously seen colors
    quickdraw::color index[64] = {0};
    // Starting color is always black
    auto px = quickdraw::colors::black();

    while (offset < end) {
        auto run = 0;
        while (run < 62 && offset < end && frame.at(offset) == px) {
            run++;
            offset++;
        }
        if (run > 0) {
            writer.write_byte(opcode::run | run-1);
            continue;
        }
        
        auto prev = px;
        px = frame.at(offset++);
        
        auto hash = hash_color(px);
        if (index[hash] == px) {
            writer.write_byte(opcode::index | hash);
            continue;
        }
        index[hash] = px;
        
        if (px.components.alpha == prev.components.alpha) {
            int8_t dr = px.components.red   - prev.components.red;
            int8_t dg = px.components.green - prev.components.green;
            int8_t db = px.components.blue  - prev.components.blue;
            int8_t dg_r = dr - dg;
            int8_t dg_b = db - dg;
            
            if (
                dr > -3 && dr < 2 &&
                dg > -3 && dg < 2 &&
                db > -3 && db < 2
            ) {
                writer.write_byte(opcode::diff | (dr + 2) << 4 | (dg + 2) << 2 | (db + 2));
            }
            else if (
                dg_r >  -9 && dg_r <  8 &&
                dg   > -33 && dg   < 32 &&
                dg_b >  -9 && dg_b <  8
            ) {
                writer.write_byte(opcode::luma    | (dg   + 32));
                writer.write_byte((dg_r + 8) << 4 | (dg_b +  8));
            }
            else {
                writer.write_byte(opcode::rgb);
                writer.write_byte(px.components.red);
                writer.write_byte(px.components.green);
                writer.write_byte(px.components.blue);
            }
        }
        else {
            writer.write_byte(opcode::rgba);
            writer.write_byte(px.components.red);
            writer.write_byte(px.components.green);
            writer.write_byte(px.components.blue);
            writer.write_byte(px.components.alpha);
        }
    }
    
    // 8-byte end marker
    writer.write_quad(1);
    return writer.position() - pos;
}
