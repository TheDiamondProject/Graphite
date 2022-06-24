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

#include "libGraphite/quickdraw/format/rle.hpp"
#include <cmath>

// MARK: - Constants

namespace graphite::quickdraw::constants
{
    static constexpr std::uint16_t rle_grid_width = 6;
    static constexpr std::size_t advance = 2;
}

// MARK: - Construction

graphite::quickdraw::rle::rle(const data::block &data, rsrc::resource::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::quickdraw::rle::rle(data::reader &reader)
{
    decode(reader);
}

graphite::quickdraw::rle::rle(const size<std::int16_t> &size, std::uint16_t frame_count)
    : m_id(0), m_name("RLE"), m_frame_size(size), m_frame_count(frame_count), m_bpp(16), m_palette_id(0)
{
    // Determine what the grid will be. We need to round up to the next whole number and have blank tiles
    // if the frame count is not divisible by the grid width constant.
    auto grid_width = std::min(constants::rle_grid_width, frame_count);
    m_grid_size = quickdraw::size<std::int16_t>(grid_width, std::ceil(m_frame_count / static_cast<double>(grid_width)));

    // Create the surface
    m_surface = quickdraw::surface(m_grid_size.width * m_frame_size.width, m_grid_size.height * m_frame_size.height);
}

// MARK: - Accessors

auto graphite::quickdraw::rle::surface() const -> const quickdraw::surface&
{
    return m_surface;
}

auto graphite::quickdraw::rle::frames() const -> std::vector<rect<std::int16_t>>
{
    return m_frames;
}

auto graphite::quickdraw::rle::frame_count() const -> std::size_t
{
    return m_frame_count;
}

auto graphite::quickdraw::rle::data() -> data::block
{
    data::writer writer;
    encode(writer);
    return std::move(*const_cast<data::block *>(writer.data()));
}

// MARK: - Operations

auto graphite::quickdraw::rle::frame_rect(std::uint32_t frame) const -> rect <std::int16_t>
{
    return {
        point<std::int16_t>(
            static_cast<std::int16_t>(frame % constants::rle_grid_width) * m_frame_size.width,
            static_cast<std::int16_t>(frame / constants::rle_grid_width) * m_frame_size.height
        ),
        m_frame_size
    };
}

auto graphite::quickdraw::rle::frame_surface(std::uint32_t frame) const -> quickdraw::surface
{
    quickdraw::surface surface(m_frame_size);
    rect<std::int16_t> src_rect(frame_rect(frame));

    // Extract the frame area of the origin surface
    for (std::int16_t y = 0; y < src_rect.size.height; ++y) {
        for (std::int16_t x = 0; x < src_rect.size.width; ++x) {
            surface.set(x, y, m_surface.at(x + src_rect.origin.x, y + src_rect.origin.y));
        }
    }

    return std::move(surface);
}

auto graphite::quickdraw::rle::write_frame(std::uint32_t frame, const quickdraw::surface &surface) -> void
{
    rect<std::int16_t> dst_rect = frame_rect(frame);
    size<std::int16_t> src_size = surface.size();

    if (src_size.width != m_frame_size.width || src_size.height != m_frame_size.height) {
        throw std::runtime_error("Incorrect frame dimensions " + std::to_string(src_size.width) + "x" + std::to_string(src_size.height) +
                                 ", expected " + std::to_string(m_frame_size.width) + "x" + std::to_string(m_frame_size.height));
    }

    // Copy from the source surfac e into the destination frame
    for (std::int16_t y = 0; y < dst_rect.size.height; ++y) {
        for (std::int16_t x = 0; x < dst_rect.size.width; ++x) {
            m_surface.set(x + dst_rect.origin.x, y + dst_rect.origin.y, surface.at(x, y));
        }
    }
}

auto graphite::quickdraw::rle::write_pixel(std::uint16_t pixel, std::uint8_t mask, std::uint64_t offset) -> void
{
    m_surface.set(offset, rgb(pixel));
}

auto graphite::quickdraw::rle::write_pixel(std::uint32_t pixel, std::uint8_t mask, std::uint64_t offset, enum pixel_type type) -> void
{
    switch (type) {
        case pixel_type::type1: {
            m_surface.set(offset, rgb(pixel >> 16));
        }
        case pixel_type::type2: {
            m_surface.set(offset, rgb(pixel & 0xFFFF));
        }
    }
}

auto graphite::quickdraw::rle::surface_offset(std::uint32_t frame, std::uint64_t offset) -> std::uint64_t
{
    quickdraw::point<std::int16_t> fo(frame % constants::rle_grid_width, frame / constants::rle_grid_width);
    quickdraw::point<std::int16_t> p(fo.x * m_frame_size.width, (fo.y * m_frame_size.height) + offset);
    return static_cast<uint64_t>(p.y * m_surface.size().width + p.x);
}

// MARK: - Decoding

auto graphite::quickdraw::rle::decode(data::reader &reader) -> void
{
    // Read the header of the RLE information. This will tell us what we need to do in order to actually
    // decode the frames.
    m_frame_size = size<std::int16_t>::read(reader, coding_type::macintosh);
    m_bpp = reader.read_short();
    m_palette_id = reader.read_short();
    m_frame_count = reader.read_short();
    reader.move(6);

    // Ensure that the RLE has a BPP of 16. This is the only format that we support currently.
    if (m_bpp != 16) {
        throw std::runtime_error("Incorrect color depth for rlëD resource: " + std::to_string(m_id) + ", " + m_name);
    }

    // Determine what the grid will be. We need to round up to the next whole and have blank tiles if the frame count
    // is not divisible by the grid width constant.
    auto grid_width = std::min(constants::rle_grid_width, m_frame_count);
    m_grid_size = quickdraw::size<std::int16_t>(grid_width, std::ceil(m_frame_count / static_cast<double>(grid_width)));

    // Create the surface in which all frame will be draw to, and other working variables required to parse and decode
    // the RLE data correctly.
    m_surface = quickdraw::surface(m_grid_size.width * m_frame_size.width, m_grid_size.height * m_frame_size.height);
    rle::opcode opcode = opcode::eof;
    std::uint64_t position = 0;
    std::uint32_t row_start = 0;
    std::int32_t current_line = -1;
    std::uint64_t current_offset = 0;
    std::int32_t count = 0;
    std::uint16_t pixel = 0;
    std::int32_t current_frame = 0;
    std::uint32_t pixel_run = 0;

    while (!reader.eof()) {
        if ((row_start != 0) && ((position - row_start) & 0x03)) {
            position += 4 - ((position - row_start) & 0x03);
            reader.move(4 - (count & 0x03));
        }

        opcode = reader.read_enum<rle::opcode>();
        count = reader.read_triple();

        switch (opcode) {
            case opcode::eof: {
                // Check that we're not erroneously an EOF.
                if (current_line != m_frame_size.height - 1) {
                    throw std::runtime_error("Incorrect number of scanlines in rlëD resource: " + std::to_string(m_id) + ", " + m_name);
                }

                // Have we finished decoding the last frame in the data?
                if (++current_frame >= m_frame_count) {
                    goto COMPLETED_LAST_FRAME;
                }

                // Prepare for the next frame
                current_line = -1;
                break;
            }

            case opcode::line_start: {
                current_offset = surface_offset(current_frame, ++current_line);
                row_start = reader.position();
                break;
            }

            case opcode::pixel_data: {
                for (auto i = 0; i < count; i += 2) {
                    pixel = reader.read_short();
                    write_pixel(pixel, 0xFF, current_offset);
                    ++current_offset;
                }

                if (count & 0x03) {
                    reader.move(4 - (count & 0x03));
                }

                break;
            }

            case opcode::pixel_run: {
                pixel_run = reader.read_long();
                for (auto i = 0; i < count; i += 4) {
                    write_pixel(pixel_run, 0xFF, current_offset, pixel_type::type1);
                    ++current_offset;
                    if (i + 2 < count) {
                        write_pixel(pixel_run, 0xFF, current_offset, pixel_type::type2);
                        ++current_offset;
                    }
                }
                break;
            }

            case opcode::transparent_run: {
                current_offset += count >> 1;
                break;
            }
        }
    }

    COMPLETED_LAST_FRAME:
    return;
}

// MARK: - Encoding

auto graphite::quickdraw::rle::encode(data::writer &writer) -> void
{
    // Write out the header
    m_frame_size.encode(writer, coding_type::macintosh);
    writer.write_short(m_bpp);
    writer.write_short(m_palette_id);
    writer.write_short(m_frame_count);

    // Reserved fields
    writer.write_short(0, 3);

    // Write out the RLE frames
    for (auto f = 0; f < m_frame_count; ++f) {
        auto frame = frame_rect(f);

        for (std::int16_t y = 0; y < frame.size.height; ++y) {
            auto line_start_pos = writer.position();
            writer.write_long(0);

            rle::opcode run_state = opcode::line_start;
            auto run_start_pos = line_start_pos + 4;
            auto run_count = 0;

            for (std::int16_t x = 0; x < frame.size.width; ++x) {
                auto pixel = m_surface.at(frame.origin.x + x, frame.origin.y + y);

                if (pixel.components.alpha == 0) {
                    if (run_state == opcode::line_start) {
                        // Start of a transparent run
                        run_start_pos = writer.position();
                        writer.write_long(0);
                        run_state = opcode::transparent_run;
                        run_count = constants::advance;
                    }
                    else if (run_state == opcode::transparent_run) {
                        // Continue transparent run
                        run_count += constants::advance;
                    }
                    else {
                        // End of pixel run, start of transparent run.
                        auto run_end_pos = writer.position();
                        writer.set_position(run_start_pos);
                        writer.write_enum(opcode::pixel_data);
                        writer.write_triple(run_count);
                        writer.set_position(run_end_pos);

                        // Pad to nearest 4-byte boundary
                        if (run_count & 0x3) {
                            writer.move(4 - (run_count & 0x3));
                        }

                        // Start transparent run
                        run_start_pos = writer.position();
                        writer.write_long(0);
                        run_state = opcode::transparent_run;
                        run_count = constants::advance;
                    }
                }
                else {
                    if (run_state == opcode::line_start) {
                        // Start of pixel run
                        run_start_pos = writer.position();
                        writer.write_long(0);
                        run_state = opcode::pixel_data;
                        run_count = constants::advance;
                    }
                    else if (run_state == opcode::transparent_run) {
                        // End of transparent run, start of pixel run
                        writer.move(-4);
                        writer.write_enum(opcode::transparent_run);
                        writer.write_triple(run_count);

                        // Start pixel run
                        run_start_pos = writer.position();
                        writer.write_long(0);
                        run_state = opcode::pixel_data;
                        run_count = constants::advance;
                    }
                    else {
                        // Continue pixel run
                        run_count += constants::advance;
                    }

                    // Write the pixel
                    writer.write_short(
                        (pixel.components.blue >> 3) |
                        ((pixel.components.green >> 3) << 5) |
                        ((pixel.components.red >> 3) << 10)
                    );
                }
            }

            // Terminate the current opcode
            if (run_state == opcode::pixel_data) {
                auto run_end_pos = writer.position();
                writer.set_position(run_start_pos);
                writer.write_enum(opcode::pixel_data);
                writer.write_triple(run_count);

                // Pad to the nearest 4-byte boundary
                if ((run_end_pos - line_start_pos) & 0x3) {
                    writer.move(4 - ((run_end_pos - line_start_pos) & 0x3));
                }
            }
            else if (run_state == opcode::transparent_run) {
                // Erase the transparent run opcode placeholder -- remaining data is assumed transparent
                writer.set_position(run_start_pos);
            }

            // Write out the opcode and the size at the start of the line
            auto line_end_pos = writer.position();
            writer.set_position(line_start_pos);
            writer.write_enum(opcode::line_start);
            writer.write_triple(line_end_pos - line_start_pos - 4);
            writer.set_position(line_end_pos);
        }

        writer.write_enum(opcode::eof);
        writer.write_triple(0);
    }
}