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

    auto points = (size - 10) >> 2;
    pict_reader.move(points * 2 * sizeof(uint16_t));

    return rect;
}

auto graphite::qd::pict::read_long_comment(graphite::data::reader& pict_reader) -> void
{
    pict_reader.move(2);
    auto length = pict_reader.read_short();
    pict_reader.move(length);
}

auto graphite::qd::pict::read_indirect_bits_rect(graphite::data::reader& pict_reader, bool packed, bool skip_region) -> void
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
        pm.set_pack_type(1);
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
    
    if (skip_region) {
        auto skip = pict_reader.read_short();
        if (skip > 2) {
            pict_reader.move(skip - 2);
        }
    }

    // Setup pixel buffer for raw values
    std::vector<uint8_t> raw;
    auto row_bytes = pm.row_bytes();
    auto width = pm.bounds().width();
    auto height = pm.bounds().height();
    
    if (packed) {
        uint16_t packed_bytes_count = 0;
        for (auto scanline = 0; scanline < height; ++scanline) {
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
    
    destination_rect.set_x(destination_rect.x() - m_frame.origin().x());
    destination_rect.set_y(destination_rect.y() - m_frame.origin().y());
    pm.build_surface(m_surface, raw, color_table, destination_rect);
    m_size += width * height;
}

auto graphite::qd::pict::read_direct_bits_rect(graphite::data::reader &pict_reader) -> void
{
    graphite::qd::pixmap pm = graphite::qd::pixmap(pict_reader.read_data(qd::pixmap::length));
    auto pack_type = pm.pack_type();
    auto cmp_count = pm.cmp_count();
    auto color_model = static_cast<graphite::qd::pixel_format>(pm.pixel_format());
    auto row_bytes = pm.row_bytes();
    auto bounds = pm.bounds();

    // Read the source and destination bounds
    auto source_rect = qd::rect::read(pict_reader, qd::rect::qd);
    auto destination_rect = qd::rect::read(pict_reader, qd::rect::qd);

    auto transfer_mode = pict_reader.read_short();

    // Verify the type of PixMap. We can only accept certain types for the time being, until
    // support for decoding/rendering other types is added.
    std::vector<uint8_t> raw;
    switch (pack_type) {
        case 1:
        case 2:
        case 3: {
            raw.reserve(row_bytes);
            break;
        }
        case 4: {
            raw.reserve(cmp_count * row_bytes / 4);
            break;
        }
        default: {
            throw std::runtime_error("Unsupported pack type " + std::to_string(pack_type) + " encountered in PICT: " + std::to_string(m_id) + ", " + m_name);
        }
    }

    // Allocate a private memory buffer before going to the surface.
    std::vector<uint16_t> px_short_buffer;
    std::vector<uint32_t> px_long_buffer;
    uint32_t px_buffer_offset = 0;
    uint16_t packed_bytes_count = 0;
    uint32_t height = source_rect.height();
    uint32_t width = source_rect.width();
    uint32_t bounds_width = bounds.width();
    uint32_t source_length = width * height;

    if (pack_type == 3) {
        px_short_buffer = std::vector<uint16_t>(source_length);
    }
    else {
        px_long_buffer = std::vector<uint32_t>(source_length);
    }

    auto packing_enabled = ((pack_type == 3 ? 2 : 1) + (row_bytes > 250 ? 2 : 1)) <= bounds_width;

    for (uint32_t scanline = 0; scanline < height; ++scanline) {
        raw.clear();

        if (pack_type > 2 && packing_enabled) {
            if (row_bytes > 250) {
                packed_bytes_count = pict_reader.read_short();
            }
            else {
                packed_bytes_count = pict_reader.read_byte();
            }

            // Create a temporary buffer to read the packed data into, on the stack.
            auto packed_data = read_bytes(pict_reader, packed_bytes_count);
            qd::packbits::decode(raw, packed_data, pack_type == 3 ? sizeof(uint16_t) : sizeof(uint8_t));
        }
        else {
            raw = read_bytes(pict_reader, row_bytes);
        }

        if (pack_type == 3) {
            for (uint32_t x = 0; x < width; ++x) {
                px_short_buffer[px_buffer_offset + x] = (0xFF & raw[2 * x + 1]) | ((0xFF & raw[2 * x]) << 8);
            }
        }
        else {
            if (cmp_count == 3) {
                // RGB Formatted Data
                for (uint32_t x = 0; x < width; x++) {
                    px_long_buffer[px_buffer_offset + x] =
                            0xFF000000
                            | ((raw[x] & 0xFF) << 16)
                            | ((raw[bounds_width + x] & 0xFF) << 8)
                            | (raw[2 * bounds_width + x] & 0xFF);
                }
            }
            else if (cmp_count == 4) {
                // ARGB Formatted Data
                for (uint32_t x = 0; x < width; x++) {
                    px_long_buffer[px_buffer_offset + x] =
                            ((raw[x] & 0xFF) << 24)
                            | ((raw[bounds_width + x] & 0xFF) << 16)
                            | ((raw[2 * bounds_width + x] & 0xFF) << 8)
                            | (raw[3 * bounds_width + x] & 0xFF);
                }
            }
        }

        px_buffer_offset += width;
    }

    if (pack_type == 3) {
        for (auto v : px_short_buffer) {
            graphite::qd::color color(static_cast<uint8_t>(((v & 0x7c00) >> 10) * 0xFF / 0x1F),
                                      static_cast<uint8_t>(((v & 0x03e0) >> 5) * 0xFF / 0x1F),
                                      static_cast<uint8_t>((v & 0x001f) * 0xFF / 0x1F));
            m_surface->set(m_size++, color);
        }
    }
    else {
        for (auto v : px_long_buffer) {
            graphite::qd::color color(static_cast<uint8_t>((v & 0xFF0000) >> 16),
                                      static_cast<uint8_t>((v & 0xFF00) >> 8),
                                      static_cast<uint8_t>(v & 0xFF),
                                      static_cast<uint8_t>((v & 0xFF000000) >> 24));
            m_surface->set(m_size++, color);
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
    
    if (compressor == 'rle ' && clut == 40) {
        // Grayscale rle is often just garbage, skip over this and hope we find the real image later.
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
            throw std::runtime_error("Unsupported QuickTime compressor '" + comp + "' in PICT: " + std::to_string(m_id) + ", " + m_name);
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
        }

        if (m_x_ratio <= 0 || m_y_ratio <= 0) {
            throw std::runtime_error("Invalid PICT resource. Content aspect ratio is not valid: " + std::to_string(m_id) + ", " + m_name);
        }
        
        pict_reader.move(4);
    }

    // Begin parsing PICT opcodes.
    qd::rect clip_rect(0, 0, 0, 0);

    m_size = 0;
    m_surface = std::make_shared<graphite::qd::surface>(m_frame.width(), m_frame.height());

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
            case opcode::bits_rect: {
                read_indirect_bits_rect(pict_reader, false, false);
                break;
            }
            case opcode::bits_region: {
                read_indirect_bits_rect(pict_reader, false, true);
                break;
            }
            case opcode::pack_bits_rect: {
                read_indirect_bits_rect(pict_reader, true, false);
                break;
            }
            case opcode::pack_bits_region: {
                read_indirect_bits_rect(pict_reader, true, true);
                break;
            }
            case opcode::direct_bits_rect: {
                read_direct_bits_rect(pict_reader);
                break;
            }
            case opcode::long_comment: {
                read_long_comment(pict_reader);
                break;
            }
            case opcode::short_comment: {
                pict_reader.move(2);
                break;
            }
            case opcode::op_color: {
                pict_reader.move(6);
                break;
            }
            case opcode::nop:
            case opcode::eof:
            case opcode::ext_header:
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
                throw std::runtime_error("Encountered an incompatible PICT: " + std::to_string(m_id) + ", " + m_name);
            }
        }
    }
}

// MARK: - Encoder / Writing

auto graphite::qd::pict::encode(graphite::data::writer& pict_encoder) -> void
{
    encode_header(pict_encoder);
    encode_def_hilite(pict_encoder);
    encode_clip_region(pict_encoder);
    encode_direct_bits_rect(pict_encoder);

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

auto graphite::qd::pict::encode_direct_bits_rect(graphite::data::writer& pict_encoder) -> void
{
    pict_encoder.write_short(static_cast<uint16_t>(opcode::direct_bits_rect));

    qd::pixmap pm(m_frame);
    pm.write(pict_encoder);

    // Source and destination frames - identical to the image frame.
    m_frame.write(pict_encoder, rect::qd);
    m_frame.write(pict_encoder, rect::qd);

    // Specify the transfer mode.
    pict_encoder.write_short(0); // Source Copy

    // Prepare to write out the actual image data.
    std::vector<uint8_t> scanline_bytes(m_frame.width() * pm.cmp_count());
    for (auto scanline = 0; scanline < m_frame.height(); ++scanline) {

        if (pm.cmp_count() == 3) {
            for (auto x = 0; x < m_frame.width(); ++x) {
                auto pixel = m_surface->at(x, scanline);
                scanline_bytes[x] = pixel.red_component();
                scanline_bytes[x + m_frame.width()] = pixel.green_component();
                scanline_bytes[x + m_frame.width() * 2] = pixel.blue_component();
            }
        }
        else if (pm.cmp_count() == 4) {
            for (auto x = 0; x < m_frame.width(); ++x) {
                auto pixel = m_surface->at(x, scanline);
                scanline_bytes[x] = pixel.alpha_component();
                scanline_bytes[x + m_frame.width()] = pixel.red_component();
                scanline_bytes[x + m_frame.width() * 2] = pixel.green_component();
                scanline_bytes[x + m_frame.width() * 3] = pixel.blue_component();
            }
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

auto graphite::qd::pict::data() -> std::shared_ptr<graphite::data::data>
{
    auto data = std::make_shared<graphite::data::data>();
    graphite::data::writer writer(data);
    encode(writer);
    return data;
}
