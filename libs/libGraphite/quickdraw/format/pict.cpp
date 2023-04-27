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

#include "libGraphite/quickdraw/format/pict.hpp"
#include "libGraphite/quickdraw/support/pixmap.hpp"
#include "libGraphite/quickdraw/format/clut.hpp"
#include "libGraphite/compression/packbits.hpp"
#include "libGraphite/quicktime/image_description.hpp"

// MARK: - Constants

namespace graphite::quickdraw::constants
{
    static constexpr std::uint16_t pict_v1_magic = 0x1101;
    static constexpr std::uint32_t pict_v2_magic = 0x001102ff;
}

// MARK: - Construction

graphite::quickdraw::pict::pict(const data::block &data, rsrc::resource::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::quickdraw::pict::pict(data::reader &reader)
{
    decode(reader);
}

graphite::quickdraw::pict::pict(quickdraw::surface &surface)
    : m_surface(std::move(surface)), m_dpi(72, 72)
{
    m_frame.size = m_surface.size();
}

// MARK: - Accessors

auto graphite::quickdraw::pict::surface() -> quickdraw::surface &
{
    return m_surface;
}

auto graphite::quickdraw::pict::data() -> data::block
{
    data::writer writer;
    encode(writer);
    return std::move(*const_cast<data::block *>(writer.data()));
}

// MARK: - Decoding

auto graphite::quickdraw::pict::decode(data::reader &reader) -> void
{
    reader.move(2);

    m_frame = reader.read<rect<std::int16_t>>();
    bool v1 = false;

    if (reader.read_short(0, data::reader::mode::peek) == constants::pict_v1_magic) {
        reader.move(2);
        v1 = true;
    }
    else {
        if (reader.read_long() != constants::pict_v2_magic) {
            throw std::runtime_error("Invalid PICT resource. Incorrect header: " + std::to_string(m_id) + ", " + m_name);
        }

        // The very first thing we should find is an extended header opcode. Read this
        // outside of the main opcode loop as it should only appear once.
        if (reader.read_enum<enum opcode>() != opcode::ext_header) {
            throw std::runtime_error("Expected to find PICT Extended Header.");
        }

        if ((reader.read_long() >> 16) != 0xFFFE) {
            // Standard header variant
            auto rect = reader.read<quickdraw::rect<double>>();
            m_dpi.x = m_frame.size.width / rect.size.width;
            m_dpi.y = m_frame.size.height / rect.size.height;
        }
        else {
            // Extended header variant
            reader.move(sizeof(std::uint32_t) * 2);
            auto rect = reader.read<quickdraw::rect<std::int16_t>>();
            m_dpi.x = static_cast<double>(m_frame.size.width) / rect.size.width;
            m_dpi.y = static_cast<double>(m_frame.size.height) / rect.size.height;

            // This isn't strictly correct, but it allows us to decode some images which would otherwise
            // fail due to mismatched frame sizes. QuickDraw would normally scale such images down to fit the frame
            // but here we just expand the frame.
            // TODO: Make this a setting, so that we can handle scaling
            m_frame.size = rect.size;
        }

        if (m_dpi.x <= 0 || m_dpi.y <= 0) {
            throw std::runtime_error("Invalid PICT resource. Content aspect ratio is not valid: " + std::to_string(m_id) + ", " + m_name);
        }

        reader.move(4);
    }

    // Begin parsing PICT opcodes
    auto opcode = opcode::eof;
    quickdraw::rect<std::int16_t> clipping_rect;
    m_size = 0;
    m_surface = quickdraw::surface(m_frame.size);

    while (!reader.eof()) {
        if (v1) {
            opcode = static_cast<enum opcode>(reader.read_byte());
        }
        else {
            reader.move(reader.position() % sizeof(std::uint16_t));
            opcode = reader.read_enum<enum opcode>();
        }

        if (opcode == opcode::eof) {
            break;
        }

        switch (opcode) {
            case opcode::clip_region: {
                clipping_rect = read_region(reader);
                break;
            }
            case opcode::origin: {
                m_frame.origin = point<std::int16_t>::read(reader, coding_type::macintosh);
                break;
            }
            case opcode::bits_rect: {
                read_indirect_bits_rect(reader, false, false);
                break;
            }
            case opcode::bits_region: {
                read_indirect_bits_rect(reader, false, true);
                break;
            }
            case opcode::pack_bits_rect: {
                read_indirect_bits_rect(reader, true, false);
                break;
            }
            case opcode::pack_bits_region: {
                read_indirect_bits_rect(reader, true, true);
                break;
            }
            case opcode::direct_bits_rect: {
                read_direct_bits_rect(reader, false);
                break;
            }
            case opcode::direct_bits_region: {
                read_direct_bits_rect(reader, true);
                break;
            }
            case opcode::long_comment: {
                read_long_comment(reader);
                break;
            }
            case opcode::short_comment: {
                read_short_comment(reader);
                break;
            }
            case opcode::short_line_from:
            case opcode::pen_mode: {
                reader.move(2);
                break;
            }
            case opcode::line_from:
            case opcode::pen_size: {
                reader.move(4);
                break;
            }
            case opcode::short_line:
            case opcode::rgb_fg_color:
            case opcode::rgb_bg_color:
            case opcode::hilite_color:
            case opcode::op_color: {
                reader.move(6);
                break;
            }
            case opcode::pen_pattern:
            case opcode::fill_pattern:
            case opcode::line:
            case opcode::frame_rect:
            case opcode::paint_rect:
            case opcode::erase_rect:
            case opcode::invert_rect:
            case opcode::fill_rect:
            case opcode::frame_same_rect:
            case opcode::paint_same_rect:
            case opcode::erase_same_rect:
            case opcode::invert_same_rect:
            case opcode::fill_same_rect: {
                reader.move(8);
                break;
            }
            case opcode::frame_region:
            case opcode::paint_region:
            case opcode::erase_region:
            case opcode::invert_region:
            case opcode::fill_region: {
                read_region(reader);
                break;
            }
            case opcode::nop:
            case opcode::eof:
            case opcode::ext_header:
            case opcode::hilite_mode:
            case opcode::def_hilite: {
                break;
            }
            case opcode::compressed_quicktime: {
                // Compressed QuickTime data is often followed by drawing routines telling you that you need QuickTime
                // to decode the image. We should skip these for now and just return after a successful decode.
                read_compressed_quicktime(reader);
                break;
            }
            case opcode::uncompressed_quicktime: {
                read_uncompressed_quicktime(reader);
                break;
            }
            default: {
                throw std::runtime_error("Encountered an incompatible PICT: " + std::to_string(m_id) + ", " + m_name);
            }
        }
    }
}

// MARK: - Encoding

auto graphite::quickdraw::pict::encode(data::writer &writer) -> void
{
    write_header(writer);
    write_def_hilite(writer);
    write_clip_region(writer);
    write_direct_bits_rect(writer);

    // Make sure we're word aligned and put out the end of picture opcode.
    auto align_adjust = writer.position() % sizeof(std::uint16_t);
    for (auto n = 0; n < align_adjust; ++n) {
        writer.write_byte(0);
    }
    writer.write_enum<enum opcode>(opcode::eof);
}

// MARK: - Reading Functions

auto graphite::quickdraw::pict::read_region(data::reader &reader) const -> rect<std::int16_t>
{
    auto size = reader.read_short();
    auto rect = reader.read<quickdraw::rect<std::int16_t>>();

    rect.origin.x /= m_dpi.x;
    rect.origin.y /= m_dpi.y;
    rect.size.width /= m_dpi.x;
    rect.size.height /= m_dpi.y;

    reader.move(size - 10);

    return rect;
}

auto graphite::quickdraw::pict::read_long_comment(data::reader &reader) const -> void
{
    reader.move(2);
    reader.move(reader.read_short());
}

auto graphite::quickdraw::pict::read_short_comment(data::reader &reader) const -> void
{
    reader.move(2);
}

auto graphite::quickdraw::pict::read_indirect_bits_rect(data::reader &reader, bool packed, bool region) -> void
{
    quickdraw::pixmap pm;
    quickdraw::clut color_table;

    // Determine if we're dealing with a PixMap or an old style BitMap.
    bool is_pixmap = reader.read_short(0, data::reader::mode::peek) & 0x8000;
    if (is_pixmap) {
        // The PixMap base address is omitted here, step back when reading.
        pm = reader.read<quickdraw::pixmap>(-4);
        color_table = reader.read<quickdraw::clut>();
    }
    else {
        // Old style Bitmap
        pm.set_pack_type(pixmap::argb);
        pm.set_component_count(1);
        pm.set_component_size(1);
        pm.set_row_bytes(reader.read_short());
        pm.set_bounds(reader.read<rect<std::int16_t>>());

        // Monochrome color table
        color_table.set(colors::white());
        color_table.set(colors::black());
    }

    m_format = pm.pixel_size();

    // Read the source and destination bounds
    auto source_rect = reader.read<rect<std::int16_t>>();
    auto destination_rect = reader.read<rect<std::int16_t>>();

    auto transfer_mode = reader.read_short();
    if (region) {
        read_region(reader);
    }

    // Setup pixel buffer for raw values
    data::block raw_data;
    auto row_bytes = pm.row_bytes();
    auto width = pm.bounds().size.width;
    auto height = pm.bounds().size.height;

    if (packed) {
        data::writer raw;
        for (std::int16_t scanline = 0; scanline < height; ++scanline) {
            auto data = reader.read_compressed_data<compression::packbits<8>>(row_bytes > 250 ? reader.read_short() : reader.read_byte());
            raw.write_data(&data);
        }
        raw_data = std::move(*raw.data());
    }
    else {
        raw_data = reader.read_data(row_bytes * height);
    }

    destination_rect.origin.x -= m_frame.origin.x;
    destination_rect.origin.y -= m_frame.origin.y;
    pm.build_surface(m_surface, raw_data, color_table, destination_rect);
    m_size += width * height;
}

auto graphite::quickdraw::pict::read_direct_bits_rect(data::reader &reader, bool region) -> void
{
    auto pm = reader.read<quickdraw::pixmap>();
    m_format = pm.pixel_size() == 16 ? 16 : pm.component_size() * pm.component_count();

    // Read the source and destination bounds
    auto source_rect = reader.read<rect<std::int16_t>>();
    auto destination_rect = reader.read<rect<std::int16_t>>();
    auto transfer_mode = reader.read_short();

    if (region) {
        read_region(reader);
    }

    auto pack_type = pm.pack_type();
    auto component_count = pm.component_count();
    auto color_model = pm.pixel_format();
    auto row_bytes = pm.row_bytes();
    auto bounds = pm.bounds();


    // Calculate the bounds of the pixels we need to copy to the surface, clipping to fit if necessary
    auto copy_x = destination_rect.origin.x - m_frame.origin.x;
    auto copy_y = destination_rect.origin.y - m_frame.origin.y;
    auto copy_w = std::min(destination_rect.size.width, static_cast<std::int16_t>(m_frame.size.width - copy_x));
    auto copy_h = std::min(destination_rect.size.height, static_cast<std::int16_t>(m_frame.size.height - copy_y));

    // When row bytes < 8, data is never packed. Raw format will instead match the pixel size, either 16-bit words or
    // 32-bit argb.
    auto packed = row_bytes >= 8 && pack_type >= pixmap::packbits_word;
    if (row_bytes < 8 && pack_type != pixmap::packbits_word) {
        pack_type = pixmap::argb;
    }
    else if (pack_type == pixmap::none || pack_type == pixmap::rgb) {
        // Row bytes is always width * 4, but alpha is omitted here so make sure we only read width * 3.
        row_bytes = bounds.size.width * 3;
    }

    data::block raw_data;
    for (auto y = 0; y < copy_h; ++y) {
        if (packed) {
            auto pack_bytes_count = row_bytes > 250 ? reader.read_short() : reader.read_byte();
            if (pack_type == pixmap::packbits_word) {
                raw_data = std::move(reader.read_compressed_data<compression::packbits<16>>(pack_bytes_count));
            }
            else {
                raw_data = std::move(reader.read_compressed_data<compression::packbits<8>>(pack_bytes_count));
            }
        }
        else {
            raw_data = std::move(reader.read_data(row_bytes));
        }

        for (auto x = 0; x < copy_w; ++x) {
            switch (pack_type) {
                case pixmap::none:
                case pixmap::rgb: {
                    m_surface.set(x + copy_x, y + copy_y, rgb(
                        raw_data.get<std::uint8_t>(3 * x),
                        raw_data.get<std::uint8_t>(3 * x + 1),
                        raw_data.get<std::uint8_t>(3 * x + 2)
                    ));
                    break;
                }
                case pixmap::argb: {
                    m_surface.set(x + copy_x, y + copy_y, rgb(
                        raw_data.get<std::uint8_t>(4 * x + 1),
                        raw_data.get<std::uint8_t>(4 * x + 2),
                        raw_data.get<std::uint8_t>(4 * x + 3)
                    ));
                    break;
                }
                case pixmap::packbits_word: {
                    m_surface.set(x + copy_x, y + copy_y, rgb(
                        (raw_data.get<std::uint8_t>(2 * x) << 8) | (raw_data.get<std::uint8_t>(2 * x + 1))
                    ));
                    break;
                }
                case pixmap::packbits_component: {
                    if (component_count == 3) {
                        m_surface.set(x + copy_x, y + copy_y, rgb(
                            raw_data.get<std::uint8_t>(x),
                            raw_data.get<std::uint8_t>(bounds.size.width + x),
                            raw_data.get<std::uint8_t>(2 * bounds.size.width + x)
                        ));
                    }
                    else if (component_count == 4) {
                        m_surface.set(x + copy_x, y + copy_y, rgb(
                            raw_data.get<std::uint8_t>(bounds.size.width + x),
                            raw_data.get<std::uint8_t>(2 * bounds.size.width + x),
                            raw_data.get<std::uint8_t>(3 * bounds.size.width + x)
                        ));
                    }
                    break;
                }
            }
        }
    }
}

auto graphite::quickdraw::pict::read_compressed_quicktime(data::reader &reader) -> void
{
    auto length = reader.read_long();
    reader.move(38);
    auto matte_size = reader.read_long();
    auto matte_rect = reader.read<rect<std::int16_t>>();
    reader.move(2);
    auto resource_rect = reader.read<rect<std::int16_t>>();
    reader.move(4);
    auto mask_size = reader.read_long();

    if (matte_size > 0) {
        auto matte = quicktime::image_description(reader);
    }

    if (mask_size > 0) {
        reader.move(mask_size);
    }

    auto image_description = quicktime::image_description(reader);
    m_surface = image_description.surface();
    m_format = static_cast<std::uint32_t>(image_description.compressor());
}

auto graphite::quickdraw::pict::read_uncompressed_quicktime(data::reader &reader) -> void
{
    auto length = reader.read_long();
    reader.move(38);
    auto matte_size = reader.read_long();
    auto matte_rect = reader.read<rect<std::int16_t>>();

    if (matte_size > 0) {
        read_image_description(reader);
    }
}

auto graphite::quickdraw::pict::read_image_description(data::reader &reader) -> void
{
    auto length = reader.read_long();
    if (length != 86) {
        throw std::runtime_error("Invalid QuickTime image description in PICT: " + std::to_string(m_id) + ", " + m_name);
    }

    auto compressor = reader.read_long();
    reader.move(24);
    auto size = reader.read<quickdraw::size<std::int16_t>>();
    reader.move(8);
    auto data_size = reader.read_long();
    reader.move(34);
    auto depth = reader.read_short();
    if (depth > 32) {
        depth -= 32; // Grayscale
    }

    auto color_table = reader.read<quickdraw::clut>();

    if (compressor == 'rle ') {
        // RLE is often garbage or redundant, skip over it and hope we find other image data later.
        reader.move(data_size);
        return;
    }

    switch (compressor) {
        default: {
            std::string compressor_name;
            compressor_name.push_back(compressor >> 24);
            compressor_name.push_back(compressor >> 16);
            compressor_name.push_back(compressor >> 8);
            compressor_name.push_back(compressor);

            throw std::runtime_error("Unsupported QuickTime compressor '" + compressor_name + "' at offset " + std::to_string(reader.position())
                                     + " in PICT: " + std::to_string(m_id) + ", " + m_name);
        }
    }
}

// MARK: - Writing Functions

auto graphite::quickdraw::pict::write_header(data::writer &writer) -> void
{
    // Write the size of zero. This seems to be fine.
    writer.write_short(0);

    // Set the image frame
    writer.write<rect<std::int16_t>>(m_frame);

    // We're only dealing with PICT version 2 currently.
    writer.write_long(constants::pict_v2_magic);
    writer.write_enum<enum opcode>(opcode::ext_header);
    writer.write_long(0xFFFE0000);

    // Image resolution (72dpi
    writer.write_short(static_cast<std::int16_t>(m_dpi.x));
    writer.write_short(0);
    writer.write_short(static_cast<std::int16_t>(m_dpi.y));
    writer.write_short(0);

    // Optimal source frame. (Identical to the image frame)
    writer.write<rect<std::int16_t>>(m_frame);

    // Reserved
    writer.write_long(0);
}

auto graphite::quickdraw::pict::write_def_hilite(data::writer &writer) -> void
{
    writer.write_enum<enum opcode>(opcode::def_hilite);
}

auto graphite::quickdraw::pict::write_clip_region(data::writer &writer) -> void
{
    writer.write_enum<enum opcode>(opcode::clip_region);
    writer.write_short(10);
    writer.write<rect<std::int16_t>>(m_frame);
}

auto graphite::quickdraw::pict::write_direct_bits_rect(data::writer &writer) -> void
{
    writer.write_enum<enum opcode>(opcode::direct_bits_rect);

    quickdraw::pixmap pm(m_frame);
    writer.write<quickdraw::pixmap>(pm);

    // Source and destination frames - identical to the image frame.
    writer.write<rect<std::int16_t>>(m_frame);
    writer.write<rect<std::int16_t>>(m_frame);

    // Specify the transfer mode.
    writer.write_short(0);

    // Prepare to write out the actual image data.
    data::block scanline_data(m_frame.size.width * pm.component_count());
    data::writer scanline(&scanline_data);

    for (std::int16_t y = 0; y < m_frame.size.height; ++y) {
        scanline_data.set(static_cast<std::uint32_t>(0));

        if (pm.component_count() == 3) {
            for (std::int16_t x = 0; x < m_frame.size.width; ++x) {
                auto pixel = m_surface.at(x, y);

                scanline.set_position(x);
                scanline.write_byte(pixel.components.red);

                scanline.set_position(x + m_frame.size.width);
                scanline.write_byte(pixel.components.green);

                scanline.set_position(x + m_frame.size.width * 2);
                scanline.write_byte(pixel.components.blue);
            }
        }
        else if (pm.component_count() == 4) {
            for (std::int16_t x = 0; x < m_frame.size.width; ++x) {
                auto pixel = m_surface.at(x, y);

                scanline.set_position(x);
                scanline.write_byte(pixel.components.alpha);

                scanline.set_position(x + m_frame.size.width);
                scanline.write_byte(pixel.components.red);

                scanline.set_position(x + m_frame.size.width * 2);
                scanline.write_byte(pixel.components.green);

                scanline.set_position(x + m_frame.size.width * 3);
                writer.write_byte(pixel.components.blue);
            }
        }

        auto packed = std::move(compression::packbits<8>::compress(scanline_data));
        if (pm.row_bytes() > 250) {
            writer.write_short(packed.size());
        }
        else {
            writer.write_byte(packed.size());
        }
        writer.write_data(&packed);
    }
}