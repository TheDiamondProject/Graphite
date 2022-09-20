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
    auto grid_width = std::min(constants::rle_grid_width, m_frame_count);
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
    data::writer writer;
    encode(writer);
    return std::move(*const_cast<data::block *>(writer.data()));
}

// MARK: - Operations

auto graphite::spriteworld::rleX::frame_rect(std::uint32_t frame) const -> quickdraw::rect<std::int16_t>
{
    return {
        quickdraw::point<std::int16_t>(
            static_cast<std::int16_t>(frame % constants::rle_grid_width) * m_frame_size.width,
            static_cast<std::int16_t>(frame / constants::rle_grid_width) * m_frame_size.height
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
    reader.change_byte_order(data::byte_order::lsb);

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
    auto grid_width = std::min(constants::rle_grid_width, m_frame_count);
    m_grid_size = quickdraw::size<std::int16_t>(grid_width, std::ceil(m_frame_count / static_cast<double>(grid_width)));

    // Create the surface in which all frames will be drawn to, and other working variables required to parse and
    // decode the RLE data correctly.
    m_surface = quickdraw::surface(m_grid_size.width * m_frame_size.width, m_grid_size.height * m_frame_size.height, quickdraw::colors::clear());

    rleX::opcode opcode = opcode::eof;
    std::int32_t current_frame = 0;
    std::uint32_t count = 0;
    std::uint32_t current_offset = 0;
    std::uint32_t right_bound = 0;
    std::uint32_t pitch = 0;

    bool completed_last_frame = false;
    auto frame = frame_rect(0);

    union quickdraw::ycbcr yuv {
        .components.y = 0,
        .components.cb = 128,
        .components.cr = 128,
        .components.alpha = 255
    };

    // NOTE: This is a very _hot_ code path and thus we need to squeeze out as much performance as possible.
    // Build a lookup table of opcodes, in order to reduce the number of comparisons.
    /* quickdraw::surface&, data::reader&, union quickdraw::ycbcr&, std::int32_t&, std::uint32_t&, quickdraw::rect<std::int16_t>& */
    std::function<auto()->void> opcode_lut[9] = {
        /* EOF */
        /* 0 */ [&] {
            if (++current_frame >= m_frame_count) {
                completed_last_frame = true;
                return;
            }
            opcode_lut[8]();
        },

        /* set components */
        /* 1 */ [&] { yuv.components.y = reader.read_byte(); },
        /* 2 */ [&] { yuv.components.cr = reader.read_byte(); },
        /* 3 */ [&] { yuv.components.cb = reader.read_byte(); },
        /* 4 */ [&] { yuv.components.alpha = reader.read_byte(); },

        /* advance */
        /* 5 */ [&] { count = reader.read_long(); opcode_lut[7](); },
        /* 6 */ [&] {
            count = static_cast<std::uint8_t>(opcode) & ~static_cast<std::uint8_t>(rleX::opcode::short_advance);
            opcode_lut[7]();
        },

        /* fill operation */
        /* 7 */ [&] {
            auto rgb = quickdraw::rgb(yuv);
            for (auto i = 0; i < count; ++i) {
                m_surface.set(current_offset, rgb);
                if (++current_offset >= right_bound) {
                    current_offset += pitch;
                    right_bound = current_offset + frame.size.width;
                }
            }
        },
        /* 8 */ [&] {
            frame = frame_rect(current_frame);
            current_offset = (frame.origin.y * m_surface.size().width) + frame.origin.x;
            right_bound = current_offset + frame.size.width;
            pitch = m_surface.size().width - frame.size.width;
        },
    };

    opcode_lut[8]();

    while (!reader.eof() && !completed_last_frame) {
        opcode = reader.read_enum<rleX::opcode>();
        if (static_cast<std::uint8_t>(opcode) & static_cast<std::uint8_t>(rleX::opcode::short_advance)) {
            opcode_lut[6]();
            continue;
        }
        opcode_lut[static_cast<int>(opcode)]();
    }
}

// MARK: - Encoding

auto graphite::spriteworld::rleX::encode(data::writer &writer) -> void
{
    writer.change_byte_order(data::byte_order::lsb);

    // Write out the header
    m_frame_size.encode(writer, quickdraw::coding_type::macintosh);
    writer.write_short(m_bpp);
    writer.write_short(m_palette_id);
    writer.write_short(m_frame_count);

    // Reserved fields.
    writer.write_short(0, 3);

    // Write out the RLE frames
    for (auto f = 0; f < m_frame_count; ++f) {
        auto frame = frame_rect(f);

        union quickdraw::ycbcr yuv {
            .components.y = 0,
            .components.cb = 128,
            .components.cr = 128,
            .components.alpha = 255
        };

        std::uint32_t count = 0;

        for (std::int16_t y = 0; y < frame.size.height; ++y) {
            for (std::int16_t x = 0; x < frame.size.width; ++x) {
                auto next_yuv = quickdraw::ycbcr(m_surface.at(frame.origin.x + x, frame.origin.y + y));

                if (next_yuv.value != yuv.value) {
                    if (count > 0) {
                        if (count < 127) {
                            auto opcode = static_cast<std::uint8_t>(opcode::short_advance) | count;
                            writer.write_byte(opcode);
                        } else {
                            writer.write_enum(opcode::advance);
                            writer.write_long(count);
                        }
                        count = 0;
                    }

                    if (next_yuv.components.y != yuv.components.y) {
                        writer.write_enum(rleX::opcode::set_luma);
                        writer.write_byte(next_yuv.components.y);
                    }

                    if (next_yuv.components.cr != yuv.components.cr) {
                        writer.write_enum(rleX::opcode::set_cr);
                        writer.write_byte(next_yuv.components.cr);
                    }

                    if (next_yuv.components.cb != yuv.components.cb) {
                        writer.write_enum(rleX::opcode::set_cb);
                        writer.write_byte(next_yuv.components.cb);
                    }

                    if (next_yuv.components.alpha != yuv.components.alpha) {
                        writer.write_enum(rleX::opcode::set_alpha);
                        writer.write_byte(next_yuv.components.alpha);
                    }

                    yuv.value = next_yuv.value;
                }

                count++;
            }
        }

        if (count < 127) {
            auto opcode = static_cast<std::uint8_t>(opcode::short_advance) | count;
            writer.write_byte(opcode);
        }
        else {
            writer.write_enum(opcode::advance);
            writer.write_long(count);
        }

        writer.write_enum(opcode::eof);
    }
}