//
// Created by Tom Hancocks on 20/02/2020.
//

#include <libGraphite/rsrc/manager.hpp>
#include "libGraphite/quickdraw/pict.hpp"
#include "libGraphite/quickdraw/internal/packbits.hpp"
#include "libGraphite/quickdraw/clut.hpp"

// MARK: - Constants

#define kPICT_V2_MAGIC          0x001102ff
#define kPACK_BITS_THRESHOLD    4

// MARK: - Constructors

graphite::qd::pict::pict(std::shared_ptr<graphite::data::data> data, int64_t id, std::string name)
        : m_surface(nullptr), m_frame(0, 0, 100, 100), m_id(id), m_name(name)
{
    // Setup a reader for the PICT data, and then parse it.
    data::reader pict_reader(data);
    parse(pict_reader);
}

graphite::qd::pict::pict(std::shared_ptr<graphite::qd::surface> surface)
        : m_surface(surface), m_frame(qd::point::zero(), surface->size()), m_id(0), m_name("Picture")
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

auto graphite::qd::pict::read_region(graphite::data::reader& pict_reader) -> graphite::qd::rect
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

auto graphite::qd::pict::read_pack_bits_rect(graphite::data::reader & pict_reader) -> void
{
    graphite::qd::pixel_format color_model = b16_rgb555;
    int16_t cmp_size;
    int16_t row_bytes;
    graphite::qd::rect bounds(0, 0, 0, 0);
    qd::clut color_table;

    // Determine if we're dealing a PixMap or an old style BitMap
    bool is_pixmap = pict_reader.read_short(sizeof(uint32_t), data::reader::peek) & 0x8000;
    if (!is_pixmap) {
        // Most like an indexed Bitmap...
        // WARNING: This is unverified code and will need to be further investigated to ensure correctness.
        cmp_size = 1;
        color_model = monochrome;
        row_bytes = pict_reader.read_short() & 0x7FFF;
        bounds = qd::rect::read(pict_reader, qd::rect::qd);

        pict_reader.move(6);

        double h_res = static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16));
        double v_res = static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16));

        pict_reader.move(22);

        // Color Table
        color_table = qd::clut(pict_reader);
    }
    else {
        graphite::qd::pixmap pm = graphite::qd::pixmap(pict_reader.read_data(qd::pixmap::length));
        cmp_size = pm.cmp_size();
        color_model = pm.pixel_format();
        row_bytes = pm.row_bytes();
        bounds = pm.bounds();
    }

    // Read the source and destination bounds
    auto source_rect = qd::rect::read(pict_reader, qd::rect::qd);
    auto destination_rect = qd::rect::read(pict_reader, qd::rect::qd);

    auto transfer_mode = pict_reader.read_short();

    // Setup pixel buffer for RGB Values
    std::vector<uint8_t> raw;
    std::vector<uint8_t> rgb;
    std::size_t rgb_buffer_offset = 0;
    uint16_t packed_bytes_count = 0;
    uint32_t source_length = source_rect.width() * source_rect.height();
    std::vector<graphite::qd::color> surface_data;

    for (auto scanline = 0; scanline < source_rect.height(); ++scanline) {
        raw.clear();

        if (row_bytes >= 8) {
            if (row_bytes > 250) {
                packed_bytes_count = pict_reader.read_short();
            }
            else {
                packed_bytes_count = static_cast<uint16_t>(pict_reader.read_byte());
            }

            auto packed_data = read_bytes(pict_reader, packed_bytes_count);
            qd::packbits::decode(raw, packed_data, sizeof(uint8_t));
            // Decoded packbits may contain more data than the row requires - trim it to width
            raw.resize(source_rect.width());
        }
        else {
            raw = read_bytes(pict_reader, row_bytes);
        }

        rgb_buffer_offset += row_bytes;
        if (cmp_size == 1) {
            for (auto component : raw) {
                surface_data.emplace_back(color_table.get(component));
            }
        }
        else {
            rgb.insert(rgb.end(), raw.begin(), raw.end());
        }
    }

    m_surface = std::make_shared<graphite::qd::surface>(destination_rect.width(), destination_rect.height(), surface_data);
}

auto graphite::qd::pict::read_direct_bits_rect(graphite::data::reader &pict_reader) -> void
{
    graphite::qd::pixel_format color_model = b16_rgb555;
    int16_t pack_type;
    int16_t cmp_count;
    int16_t row_bytes;
    graphite::qd::rect bounds(0, 0, 0, 0);

    // Determine if we're dealing a PixMap or an old style BitMap
    bool is_pixmap = pict_reader.read_short(sizeof(uint32_t), data::reader::peek) & 0x8000;
    if (is_pixmap) {
        graphite::qd::pixmap pm = graphite::qd::pixmap(pict_reader.read_data(qd::pixmap::length));
        pack_type = pm.pack_type();
        cmp_count = pm.cmp_count();
        color_model = pm.pixel_format();
        row_bytes = pm.row_bytes();
        bounds = pm.bounds();
    }
    else {
        // Old style bitmap
        pack_type = 1;
        cmp_count = 1;
        color_model = monochrome;
        row_bytes = pict_reader.read_short() & 0x7FFF;
        bounds = qd::rect::read(pict_reader, qd::rect::qd);
    }

    // Read the source and destination bounds
    auto source_rect = qd::rect::read(pict_reader, qd::rect::qd);
    auto destination_rect = qd::rect::read(pict_reader, qd::rect::qd);

    auto transfer_mode = pict_reader.read_short();

    // Verify the type of PixMap. We can only accept certain types for the time being, until
    // support for decoding/rendering other types is added.
    std::vector<uint8_t> raw;
    std::size_t raw_size = 0;
    switch (pack_type) {
        case 1:
        case 2:
        case 3: {
            raw_size = row_bytes;
            break;
        }
        case 4: {
            raw_size = cmp_count * row_bytes / 4;
            break;
        }
        default: {
            throw std::runtime_error("Unsupported pack type " + std::to_string(pack_type) + " encountered in PICT: " + std::to_string(m_id) + ", " + m_name);
        }
    }

    // Allocate a private memory buffer before going to the surface.
    std::vector<uint16_t> px_short_buffer;
    std::vector<uint32_t> px_long_buffer;

    if (pack_type == 3) {
        px_short_buffer = std::vector<uint16_t>(source_rect.height() * (row_bytes + 1) / 2);
    }
    else {
        px_long_buffer = std::vector<uint32_t>(source_rect.height() * (row_bytes + 3) / 4);
    }

    uint32_t px_buffer_offset = 0;
    uint16_t packed_bytes_count = 0;
    uint32_t height = source_rect.height();
    uint32_t width = source_rect.width();
    uint32_t bounds_width = bounds.width();
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
                            | ((raw[bounds_width] & 0xFF) << 16)
                            | ((raw[2 * bounds_width + x] & 0xFF) << 8)
                            | (raw[3 * bounds_width + x] & 0xFF);
                }
            }
        }

        px_buffer_offset += width;
    }

    uint32_t source_length = width * height;
    std::vector<graphite::qd::color> rgb(source_length, graphite::qd::color::purple());

    if (pack_type == 3) {
        for (uint32_t p = 0, i = 0; i < source_length; ++i) {
            uint16_t v = px_short_buffer[i];
            rgb[p++] = graphite::qd::color(static_cast<uint8_t>(((v & 0x7c00) >> 10) << 3),
                                           static_cast<uint8_t>(((v & 0x03e0) >> 5) << 3),
                                           static_cast<uint8_t>((v & 0x001f) << 3));
        }
    }
    else {
        for (uint32_t p = 0, i = 0; i < source_length; ++i) {
            uint32_t v = px_long_buffer[i];
            rgb[p++] = graphite::qd::color(static_cast<uint8_t>((v & 0xFF0000) >> 16),
                                           static_cast<uint8_t>((v & 0xFF00) >> 8),
                                           static_cast<uint8_t>(v & 0xFF),
                                           static_cast<uint8_t>((v & 0xFF000000) >> 24));
        }
    }

    m_size = rgb.size();
    m_surface = std::make_shared<graphite::qd::surface>(width, height, rgb);
}

auto graphite::qd::pict::parse(graphite::data::reader& pict_reader) -> void
{
    pict_reader.move(2);

    m_frame = qd::rect::read(pict_reader, qd::rect::qd);

    // We are only dealing with v2 Pictures for the time being...
    if (pict_reader.read_long() != kPICT_V2_MAGIC) {
        throw std::runtime_error("Encountered an incompatible PICT: " + std::to_string(m_id) + ", " + m_name);
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

    // Begin parsing PICT opcodes.
    qd::rect clip_rect(0, 0, 0, 0);

    while (!pict_reader.eof()) {
        // Make sure we are correctly aligned.
        pict_reader.move(pict_reader.position() % sizeof(uint16_t));
        auto op = static_cast<opcode>(pict_reader.read_short());

        if (op == opcode::eof) {
            break;
        }

        switch (op) {
            case opcode::clip_region: {
                clip_rect = read_region(pict_reader);
                break;
            }
            case opcode::pack_bits_rect: {
                read_pack_bits_rect(pict_reader);
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
            case opcode::nop:
            case opcode::eof:
            case opcode::ext_header:
            case opcode::def_hilite: {
                break;
            }
            case opcode::compressed_quicktime:
            case opcode::uncompressed_quicktime: {
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
