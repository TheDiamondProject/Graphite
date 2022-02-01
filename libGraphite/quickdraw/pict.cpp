//
// Created by Tom Hancocks on 20/02/2020.
//

#include <stdexcept>
#include <libGraphite/rsrc/manager.hpp>
#include "libGraphite/quickdraw/pict.hpp"
#include "libGraphite/quickdraw/internal/packbits.hpp"
#include "libGraphite/quickdraw/clut.hpp"

// MARK: - Constants

#define kPICT_V1_MAGIC          0x1101
#define kPICT_V2_MAGIC          0x001102ff

// MARK: - Constructors

graphite::qd::pict::pict(std::shared_ptr<graphite::data::data> data, int64_t id, std::string name)
    : m_surface(nullptr), m_frame(0, 0, 100, 100), m_id(id), m_name(std::move(name))
{
    // Setup a reader for the PICT data, and then parse it.
    data::reader pict_reader(std::move(data));
    parse(pict_reader);
}

graphite::qd::pict::pict(std::shared_ptr<graphite::qd::surface> surface)
    : m_surface(std::move(surface)), m_frame(qd::point::zero(), m_surface->size()), m_id(0), m_name("Picture")
{

}

auto graphite::qd::pict::load_resource(int64_t id) -> std::shared_ptr<graphite::qd::pict>
{
    if (auto pict_res = graphite::rsrc::manager::shared_manager().find("PICT", id).lock()) {
        return std::make_shared<graphite::qd::pict>(pict_res->data(), id, pict_res->name());
    }
    return nullptr;
}

auto graphite::qd::pict::from_surface(std::shared_ptr<graphite::qd::surface> surface) -> std::shared_ptr<graphite::qd::pict>
{
    return std::make_shared<graphite::qd::pict>(surface);
}

// MARK: - Accessors

auto graphite::qd::pict::image_surface() const -> std::weak_ptr<graphite::qd::surface>
{
    return m_surface;
}

// MARK: - Helper Functions

static inline auto read_bytes(graphite::data::reader& pict_reader, std::size_t size) -> std::vector<uint8_t>
{
    auto out = std::vector<uint8_t>(size);
    auto in = pict_reader.read_bytes(size);
    for (auto i = 0; i < in.size(); ++i) {
        out[i] = in[i];
    }
    return out;
}

// MARK: - Parsing / Reading

auto graphite::qd::pict::read_region(graphite::data::reader& pict_reader) const -> graphite::qd::rect
{
    auto size = pict_reader.read_short();
    auto rect = graphite::qd::rect::read(pict_reader, qd::rect::qd);

    rect.set_x(rect.x() / m_x_ratio);
    rect.set_y(rect.y() / m_y_ratio);
    rect.set_width(rect.width() / m_x_ratio);
    rect.set_height(rect.height() / m_y_ratio);

    pict_reader.move(size - 10);

    return rect;
}

auto graphite::qd::pict::read_long_comment(graphite::data::reader& pict_reader) -> void
{
    pict_reader.move(2);
    auto length = pict_reader.read_short();
    pict_reader.move(length);
}

auto graphite::qd::pict::read_indirect_bits_rect(graphite::data::reader& pict_reader, bool packed, bool region) -> void
{
    qd::pixmap pm;
    qd::clut color_table;
    // Determine if we're dealing a PixMap or an old style BitMap
    bool is_pixmap = pict_reader.read_short(0, data::reader::peek) & 0x8000;
    if (is_pixmap) {
        // The pixmap base address is omitted here, step back when reading
        pm = qd::pixmap(pict_reader.read_data(qd::pixmap::length, -sizeof(uint32_t)));
        // Color Table
        color_table = qd::clut(pict_reader);
    }
    else {
        // Old style bitmap
        pm.set_cmp_count(1);
        pm.set_cmp_size(1);
        pm.set_row_bytes(pict_reader.read_short());
        pm.set_bounds(qd::rect::read(pict_reader, qd::rect::qd));
        // Monochrome color table
        color_table.set(qd::color(255, 255, 255));
        color_table.set(qd::color(0, 0, 0));
    }

    // Read the source and destination bounds
    auto source_rect = qd::rect::read(pict_reader, qd::rect::qd);
    auto destination_rect = qd::rect::read(pict_reader, qd::rect::qd);

    auto transfer_mode = pict_reader.read_short();
    
    if (region) {
        read_region(pict_reader);
    }

    // Setup pixel buffer for raw values
    std::vector<uint8_t> raw;
    auto row_bytes = pm.row_bytes();
    auto width = pm.bounds().width();
    auto height = pm.bounds().height();
    
    if (packed) {
        uint16_t packed_bytes_count = 0;
        for (auto y = 0; y < height; ++y) {
            if (row_bytes > 250) {
                packed_bytes_count = pict_reader.read_short();
            }
            else {
                packed_bytes_count = pict_reader.read_byte();
            }

            auto packed_data = read_bytes(pict_reader, packed_bytes_count);
            qd::packbits::decode(raw, packed_data, sizeof(uint8_t));
        }
    }
    else {
        raw = read_bytes(pict_reader, row_bytes * height);
    }
    
    destination_rect.set_x(destination_rect.x() - m_frame.x());
    destination_rect.set_y(destination_rect.y() - m_frame.y());
    pm.build_surface(m_surface, raw, color_table, destination_rect);
}

auto graphite::qd::pict::read_direct_bits_rect(graphite::data::reader &pict_reader, bool region) -> void
{
    graphite::qd::pixmap pm = graphite::qd::pixmap(pict_reader.read_data(qd::pixmap::length));

    // Read the source and destination bounds
    auto source_rect = qd::rect::read(pict_reader, qd::rect::qd);
    auto destination_rect = qd::rect::read(pict_reader, qd::rect::qd);

    auto transfer_mode = pict_reader.read_short();
    
    if (region) {
        read_region(pict_reader);
    }
    
    // Setup pixel buffer for raw values
    std::vector<uint8_t> raw;
    auto row_bytes = pm.row_bytes();
    auto width = pm.bounds().width();
    auto height = pm.bounds().height();
    auto pack_type = pm.pack_type();
    auto cmp_count = pm.cmp_count();

    // Calculate the bounds of the pixels we need to copy to the surface, clipping to fit if necessary
    auto copy_x = destination_rect.x() - m_frame.x();
    auto copy_y = destination_rect.y() - m_frame.y();
    auto copy_w = std::min(destination_rect.width(), static_cast<int16_t>(m_frame.width() - copy_x));
    auto copy_h = std::min(destination_rect.height(), static_cast<int16_t>(m_frame.height() - copy_y));

    // When row bytes < 8, data is never packed. Raw format will instead match the pixel size, either 16-bit words or 32-bit argb.
    auto packed = row_bytes >= 8 && pack_type >= packbits_word;
    if (row_bytes < 8 && pack_type != packbits_word) {
        pack_type = argb;
    } else if (pack_type == none || pack_type == rgb) {
        // Row bytes is always width * 4, but alpha is omitted here so make sure we only read width * 3.
        row_bytes = width * 3;
    }

    for (auto y = 0; y < height; ++y) {
        if (packed) {
            raw.clear();
            uint16_t packed_bytes_count;
            if (row_bytes > 250) {
                packed_bytes_count = pict_reader.read_short();
            }
            else {
                packed_bytes_count = pict_reader.read_byte();
            }

            // Create a temporary buffer to read the packed data into, on the stack.
            auto packed_data = read_bytes(pict_reader, packed_bytes_count);
            qd::packbits::decode(raw, packed_data, pack_type == packbits_word ? sizeof(uint16_t) : sizeof(uint8_t));
        }
        else {
            raw = read_bytes(pict_reader, row_bytes);
        }

        if (y >= copy_h) {
            continue;
        }

        for (auto x = 0; x < copy_w; ++x) {
            switch (pack_type) {
                case none:
                case rgb: {
                    graphite::qd::color color(raw[3 * x],
                                              raw[3 * x + 1],
                                              raw[3 * x + 2]);
                    m_surface->set(x + copy_x, y + copy_y, color);
                    break;
                }
                case argb: {
                    graphite::qd::color color(raw[4 * x + 1],
                                              raw[4 * x + 2],
                                              raw[4 * x + 3]);
                    m_surface->set(x + copy_x, y + copy_y, color);
                    break;
                }
                case packbits_word: {
                    graphite::qd::color color((raw[2 * x] << 8) | (raw[2 * x + 1]));
                    m_surface->set(x + copy_x, y + copy_y, color);
                    break;
                }
                case packbits_component: {
                    if (cmp_count == 3) {
                        graphite::qd::color color(raw[x],
                                                  raw[width + x],
                                                  raw[2 * width + x]);
                        m_surface->set(x + copy_x, y + copy_y, color);
                    } else if (cmp_count == 4) {
                        graphite::qd::color color(raw[width + x],
                                                  raw[2 * width + x],
                                                  raw[3 * width + x]);
                        m_surface->set(x + copy_x, y + copy_y, color);
                    }
                    break;
                }
            }
        }
    }
}

auto graphite::qd::pict::read_compressed_quicktime(graphite::data::reader &pict_reader) -> void
{
    auto length = pict_reader.read_long();
    pict_reader.move(38);
    auto matte_size = pict_reader.read_long();
    auto matte_rect = qd::rect::read(pict_reader, qd::rect::qd);
    pict_reader.move(2);
    auto source_rect = qd::rect::read(pict_reader, qd::rect::qd);
    pict_reader.move(4);
    auto mask_size = pict_reader.read_long();
    
    if (matte_size > 0) {
        read_image_description(pict_reader);
    }
    
    if (mask_size > 0) {
        pict_reader.move(mask_size);
    }
    
    read_image_description(pict_reader);
}

auto graphite::qd::pict::read_uncompressed_quicktime(graphite::data::reader &pict_reader) -> void
{
    auto length = pict_reader.read_long();
    pict_reader.move(38);
    auto matte_size = pict_reader.read_long();
    auto matte_rect = qd::rect::read(pict_reader, qd::rect::qd);
    
    if (matte_size > 0) {
        read_image_description(pict_reader);
    }
}

auto graphite::qd::pict::read_image_description(graphite::data::reader &pict_reader) -> void
{
    auto length = pict_reader.read_long();
    if (length != 86) {
        throw std::runtime_error("Invalid QuickTime image description in PICT: " + std::to_string(m_id) + ", " + m_name);
    }
    auto compressor = pict_reader.read_long();
    pict_reader.move(24);
    auto width = pict_reader.read_short();
    auto height = pict_reader.read_short();
    pict_reader.move(8);
    auto data_size = pict_reader.read_long();
    pict_reader.move(34);
    auto depth = pict_reader.read_short();
    if (depth > 32) {
        depth -= 32; // grayscale
    }
    auto clut = pict_reader.read_signed_short();
    
    if (compressor == 'rle ') {
        // rle is often garbage or redundant, skip over it and hope we find other image data later.
        pict_reader.move(data_size);
        return;
    }
    
    switch (compressor) {
        default:
            std::string comp;
            comp.push_back(compressor >> 24);
            comp.push_back(compressor >> 16);
            comp.push_back(compressor >> 8);
            comp.push_back(compressor);
            throw std::runtime_error("Unsupported QuickTime compressor '" + comp + "' at offset " + std::to_string(pict_reader.position())
                                     + " in PICT: " + std::to_string(m_id) + ", " + m_name);
    }
}

auto graphite::qd::pict::parse(graphite::data::reader& pict_reader) -> void
{
    pict_reader.move(2);

    m_frame = qd::rect::read(pict_reader, qd::rect::qd);
    bool v1 = false;

    // Check which version of PICT we're dealing with
    if (pict_reader.read_short(0, data::reader::peek) == kPICT_V1_MAGIC) {
        pict_reader.move(2);
        v1 = true;
    } else {
        if (pict_reader.read_long() != kPICT_V2_MAGIC) {
            throw std::runtime_error("Invalid PICT resource. Incorrect header: " + std::to_string(m_id) + ", " + m_name);
        }
        
        // The very first thing we should find is an extended header opcode. Read this
        // outside of the main opcode loop as it should only appear once.
        if (static_cast<opcode>(pict_reader.read_short()) != opcode::ext_header) {
            throw std::runtime_error("Expected to find PICT Extended Header.");
        }

        if ((pict_reader.read_long() >> 16) != 0xFFFE) {
            // Standard header variant
            auto rect = qd::fixed_rect::read(pict_reader);
            m_x_ratio = m_frame.width() / rect.width();
            m_y_ratio = m_frame.height() / rect.height();
        }
        else {
            // Extended header variant
            pict_reader.move(sizeof(uint32_t) * 2);
            auto rect = qd::rect::read(pict_reader, qd::rect::qd);
            m_x_ratio = static_cast<double>(m_frame.width()) / rect.width();
            m_y_ratio = static_cast<double>(m_frame.height()) / rect.height();
            // This isn't strictly correct but it allows us to decode some images which
            // would otherwise fail due to mismatched frame sizes. QuickDraw would normally
            // scale such images down to fit the frame but here we just expand the frame.
            m_frame.set_size(rect.size());
        }

        if (m_x_ratio <= 0 || m_y_ratio <= 0) {
            throw std::runtime_error("Invalid PICT resource. Content aspect ratio is not valid: " + std::to_string(m_id) + ", " + m_name);
        }
        
        pict_reader.move(4);
    }

    // Begin parsing PICT opcodes.
    qd::rect clip_rect(0, 0, 0, 0);

    m_surface = std::make_shared<graphite::qd::surface>(m_frame.width(), m_frame.height());
    bool has_bits = false;

    opcode op;
    while (!pict_reader.eof()) {
        if (v1) {
            op = static_cast<opcode>(pict_reader.read_byte());
        } else {
            // Make sure we are correctly aligned.
            pict_reader.move(pict_reader.position() % sizeof(uint16_t));
            op = static_cast<opcode>(pict_reader.read_short());
        }

        if (op == opcode::eof) {
            break;
        }

        switch (op) {
            case opcode::clip_region: {
                clip_rect = read_region(pict_reader);
                break;
            }
            case opcode::origin: {
                auto origin = graphite::qd::point::read(pict_reader, qd::point::pict);
                m_frame.set_origin(origin);
                break;
            }
            case opcode::bits_rect: {
                read_indirect_bits_rect(pict_reader, false, false);
                has_bits = true;
                break;
            }
            case opcode::bits_region: {
                read_indirect_bits_rect(pict_reader, false, true);
                has_bits = true;
                break;
            }
            case opcode::pack_bits_rect: {
                read_indirect_bits_rect(pict_reader, true, false);
                has_bits = true;
                break;
            }
            case opcode::pack_bits_region: {
                read_indirect_bits_rect(pict_reader, true, true);
                has_bits = true;
                break;
            }
            case opcode::direct_bits_rect: {
                read_direct_bits_rect(pict_reader, false);
                has_bits = true;
                break;
            }
            case opcode::direct_bits_region: {
                read_direct_bits_rect(pict_reader, true);
                has_bits = true;
                break;
            }
            case opcode::long_comment: {
                read_long_comment(pict_reader);
                break;
            }
            case opcode::pen_mode:
            case opcode::short_line_from:
            case opcode::short_comment: {
                pict_reader.move(2);
                break;
            }
            case opcode::pen_size:
            case opcode::line_from: {
                pict_reader.move(4);
                break;
            }
            case opcode::short_line:
            case opcode::rgb_fg_color:
            case opcode::rgb_bg_color:
            case opcode::hilite_color:
            case opcode::op_color: {
                pict_reader.move(6);
                break;
            }
            case opcode::pen_pattern:
            case opcode::line:
            case opcode::frame_rect:
            case opcode::paint_rect:
            case opcode::erase_rect:
            case opcode::invert_rect:
            case opcode::fill_rect: {
                pict_reader.move(8);
                break;
            }
            case opcode::frame_region:
            case opcode::paint_region:
            case opcode::erase_region:
            case opcode::invert_region:
            case opcode::fill_region: {
                read_region(pict_reader);
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
                read_compressed_quicktime(pict_reader);
                // Compressed quicktime data is often followed by drawing routines telling you that you need
                // quicktime to decode the image. We should skip these and just return after a successful decode.
                return;
            }
            case opcode::uncompressed_quicktime: {
                read_uncompressed_quicktime(pict_reader);
                break;
            }
            default: {
                throw std::runtime_error("Encountered an unsupported opcode " + std::to_string(op) + " in PICT: " + std::to_string(m_id) + ", " + m_name);
            }
        }
    }
    
    // This is a safety check for QuickTime rle which is skipped over. If no other image data was found, throw an error.
    if (!has_bits) {
        throw std::runtime_error("Encountered an incompatible PICT: " + std::to_string(m_id) + ", " + m_name);
    }
}

// MARK: - Encoder / Writing

auto graphite::qd::pict::encode(graphite::data::writer& pict_encoder, bool rgb555) -> void
{
    // Ensure origin is zero before starting
    m_frame.set_origin(qd::point(0, 0));
    encode_header(pict_encoder);
    encode_def_hilite(pict_encoder);
    encode_clip_region(pict_encoder);
    encode_direct_bits_rect(pict_encoder, rgb555);

    // Make sure we're word aligned and put out the end of picture opcode.
    auto align_adjust = pict_encoder.position() % sizeof(uint16_t);
    for (auto n = 0; n < align_adjust; ++n) {
        pict_encoder.write_byte(0);
    }
    pict_encoder.write_short(static_cast<uint16_t>(opcode::eof));
}

auto graphite::qd::pict::encode_header(graphite::data::writer& pict_encoder) -> void
{
    // Write the size as zero. This seems to be fine.
    pict_encoder.write_short(0);

    // Set the image frame.
    m_frame.write(pict_encoder, rect::qd);

    // We're only dealing with PICT version 2 currently.
    pict_encoder.write_long(kPICT_V2_MAGIC);
    pict_encoder.write_short(static_cast<uint16_t>(opcode::ext_header));
    pict_encoder.write_long(0xFFFE0000);

    // Image resolution (72dpi)
    pict_encoder.write_short(72);
    pict_encoder.write_short(0);
    pict_encoder.write_short(72);
    pict_encoder.write_short(0);

    // Optimal source frame. (identical to the image frame)
    m_frame.write(pict_encoder, rect::qd);

    // Reserved
    pict_encoder.write_long(0);

    // HEADER ENDS HERE
}

auto graphite::qd::pict::encode_def_hilite(graphite::data::writer& pict_encoder) -> void
{
    pict_encoder.write_short(static_cast<uint16_t>(opcode::def_hilite));
}

auto graphite::qd::pict::encode_clip_region(graphite::data::writer& pict_encoder) -> void
{
    pict_encoder.write_short(static_cast<uint16_t>(opcode::clip_region));
    pict_encoder.write_short(10);
    m_frame.write(pict_encoder, rect::qd);
}

auto graphite::qd::pict::encode_direct_bits_rect(graphite::data::writer& pict_encoder, bool rgb555) -> void
{
    pict_encoder.write_short(static_cast<uint16_t>(opcode::direct_bits_rect));

    qd::pixmap pm(m_frame, rgb555);
    pm.write(pict_encoder);

    // Source and destination frames - identical to the image frame.
    m_frame.write(pict_encoder, rect::qd);
    m_frame.write(pict_encoder, rect::qd);

    // Specify the transfer mode.
    pict_encoder.write_short(0); // Source Copy

    // Prepare to write out the actual image data.
    // Don't use packing if row bytes < 8
    if (pm.row_bytes() < 8) {
        for (auto scanline = 0; scanline < m_frame.height(); ++scanline) {
            for (auto x = 0; x < m_frame.width(); ++x) {
                auto pixel = m_surface->at(x, scanline);
                if (rgb555) {
                    pict_encoder.write_short(pixel.rgb555());
                } else {
                    pict_encoder.write_byte(0);
                    pict_encoder.write_byte(pixel.red_component());
                    pict_encoder.write_byte(pixel.green_component());
                    pict_encoder.write_byte(pixel.blue_component());
                }
            }
        }
    }
    else if (rgb555) {
        std::vector<uint16_t> scanline_bytes(m_frame.width());
        for (auto scanline = 0; scanline < m_frame.height(); ++scanline) {
            for (auto x = 0; x < m_frame.width(); ++x) {
                auto pixel = m_surface->at(x, scanline);
                scanline_bytes[x] = pixel.rgb555();
            }

            auto packed = packbits::encode(scanline_bytes);
            if (pm.row_bytes() > 250) {
                pict_encoder.write_short(packed.size());
            }
            else {
                pict_encoder.write_byte(packed.size());
            }
            pict_encoder.write_bytes(packed);
        }
    }
    else {
        std::vector<uint8_t> scanline_bytes(m_frame.width() * pm.cmp_count());
        for (auto scanline = 0; scanline < m_frame.height(); ++scanline) {
            for (auto x = 0; x < m_frame.width(); ++x) {
                auto pixel = m_surface->at(x, scanline);
                scanline_bytes[x] = pixel.red_component();
                scanline_bytes[x + m_frame.width()] = pixel.green_component();
                scanline_bytes[x + m_frame.width() * 2] = pixel.blue_component();
            }

            auto packed = packbits::encode(scanline_bytes);
            if (pm.row_bytes() > 250) {
                pict_encoder.write_short(packed.size());
            }
            else {
                pict_encoder.write_byte(packed.size());
            }
            pict_encoder.write_bytes(packed);
        }
    }
}

auto graphite::qd::pict::data(bool rgb555) -> std::shared_ptr<graphite::data::data>
{
    auto data = std::make_shared<graphite::data::data>();
    graphite::data::writer writer(data);
    encode(writer, rgb555);
    return data;
}
