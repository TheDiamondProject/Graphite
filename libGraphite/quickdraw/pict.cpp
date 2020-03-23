//
// Created by Tom Hancocks on 20/02/2020.
//

#include <libGraphite/rsrc/manager.hpp>
#include "libGraphite/quickdraw/pict.hpp"
#include "libGraphite/quickdraw/internal/packbits.hpp"

// MARK: - Constants

#define kPICT_V2_MAGIC          0x001102ff
#define kPACK_BITS_THRESHOLD    4

// MARK: - Helper Functions

static inline graphite::qd::rect read_rect(graphite::data::reader& pict_reader)
{
    return graphite::qd::rect(pict_reader.read_signed_short(),
                              pict_reader.read_signed_short(),
                              pict_reader.read_signed_short(),
                              pict_reader.read_signed_short());
}

static inline graphite::qd::fixed_rect read_fixed_rect(graphite::data::reader& pict_reader)
{
    return graphite::qd::fixed_rect(static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16)),
                                    static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16)),
                                    static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16)),
                                    static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16)));
}

static inline std::vector<uint8_t> read_bytes(graphite::data::reader& pict_reader, std::size_t size)
{
    auto out = std::vector<uint8_t>(size);
    auto in = pict_reader.read_bytes(size);
    for (auto i = 0; i < in.size(); ++i) {
        out[i] = in[i];
    }
    return out;
}

// MARK: - Parsing

graphite::qd::rect graphite::qd::pict::read_region(graphite::data::reader& pict_reader)
{
    auto size = pict_reader.read_short();
    auto rect = read_rect(pict_reader);

    rect.left /= m_x_ratio;
    rect.right /= m_x_ratio;
    rect.top /= m_y_ratio;
    rect.bottom /= m_y_ratio;

    auto points = (size - 10) >> 2;
    pict_reader.move(points * 2 * sizeof(uint16_t));

    return rect;
}

void graphite::qd::pict::read_long_comment(graphite::data::reader& pict_reader)
{
    pict_reader.move(2);
    auto length = pict_reader.read_short();
    pict_reader.move(length);
}

void graphite::qd::pict::read_direct_bits_rect(graphite::data::reader &pict_reader)
{
    m_pixmap = graphite::qd::pixmap(pict_reader.read_data(qd::pixmap::length));

    auto source_rect = read_rect(pict_reader);
    auto destination_rect = read_rect(pict_reader);
    pict_reader.move(2);

    // Verify the type of PixMap. We can only accept certain types for the time being, until
    // support for decoding/rendering other types is added.
    if (m_pixmap.pack_type() != 3 && m_pixmap.pack_type() != 4){
        throw std::runtime_error("Unsupported PixMap pack type " + std::to_string(m_pixmap.pack_type()) + " encountered in PICT.");
    }

    // Allocate a private memory buffer before going to the surface.
    // WARNING: Use of raw memory here!
    std::vector<uint8_t> raw;
    std::vector<uint16_t> px_short_buffer;
    std::vector<uint32_t> px_long_buffer;

    if (m_pixmap.pack_type() == 3) {
        px_short_buffer = std::vector<uint16_t>((source_rect.height() * (m_pixmap.row_bytes() + 1)) >> 1);
    }
    else if (m_pixmap.pack_type() == 4) {
        px_long_buffer = std::vector<uint32_t>((source_rect.height() * (m_pixmap.row_bytes() + 3)) >> 1);
    }

    uint32_t px_buffer_offset = 0;
    uint16_t packed_bytes_count = 0;
    uint32_t height = source_rect.height();
    uint32_t width = source_rect.width();
    uint32_t bounds_width = m_pixmap.bounds().width();

    for (uint32_t scanline = 0; scanline < height; ++scanline) {
        raw.clear();

        if (m_pixmap.row_bytes() <= kPACK_BITS_THRESHOLD) {
            // No pack bits compression
            raw = read_bytes(pict_reader, m_pixmap.row_bytes());
        }
        else {
            if (m_pixmap.row_bytes() > 250) {
                // Pack bits compression is in place, with the length encoded as a short.
                packed_bytes_count = pict_reader.read_short();
            }
            else {
                // Pack bits compression is in place, with the length encoded as a byte.
                packed_bytes_count = static_cast<uint16_t>(pict_reader.read_byte());
            }

            // Create a temporary buffer to read the packed data into, on the stack.
            auto packed_data = read_bytes(pict_reader, packed_bytes_count);

            if (m_pixmap.pack_type() == 3) {
                qd::packbits::decode(raw, packed_data, sizeof(uint16_t));
            }
            else if (m_pixmap.pack_type() == 4) {
                qd::packbits::decode(raw, packed_data, sizeof(uint8_t));
            }
        }

        if (m_pixmap.pack_type() == 3) {
            for (uint32_t x = 0; x < width; ++x) {
                px_short_buffer[px_buffer_offset + x] = (0xFF & raw[2 * x + 1]) | ((0xFF & raw[2 * x]) << 8);
            }
        }
        else {
            if (m_pixmap.cmp_count() == 3) {
                // RGB Formatted Data
                for (uint32_t x = 0; x < width; x++) {
                    px_long_buffer[px_buffer_offset + x] =
                            0xFF000000
                            | ((raw[x] & 0xFF) << 16)
                            | ((raw[bounds_width + x] & 0xFF) << 8)
                            | (raw[2 * bounds_width + x] & 0xFF);
                }
            }
            else {
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

    if (m_pixmap.pack_type() == 3) {
        for (uint32_t p = 0, i = 0; i < source_length; ++i) {
            uint16_t v = px_short_buffer[i];
            rgb[p++] = graphite::qd::color(static_cast<uint8_t>((v & 0x001f) << 3),
                                           static_cast<uint8_t>(((v & 0x03e0) >> 5) << 3),
                                           static_cast<uint8_t>(((v & 0x7c00) >> 10) << 3));
        }
    }
    else if (m_pixmap.pack_type() == 4) {
        for (uint32_t p = 0, i = 0; i < source_length; ++i) {
            uint32_t v = px_long_buffer[i];
            rgb[p++] = graphite::qd::color(static_cast<uint8_t>(v & 0xFF),
                                           static_cast<uint8_t>((v & 0xFF00) >> 8),
                                           static_cast<uint8_t>((v & 0xFF0000) >> 16),
                                           static_cast<uint8_t>((v & 0xFF000000) >> 24));
        }
    }

    m_size = rgb.size();
    m_surface = std::make_shared<graphite::qd::surface>(width, height, rgb);
}

void graphite::qd::pict::parse(graphite::data::reader& pict_reader)
{
    pict_reader.move(2);

    m_frame = read_rect(pict_reader);

    // We are only dealing with v2 Pictures for the time being...
    if (pict_reader.read_long() != kPICT_V2_MAGIC) {
        throw std::runtime_error("Encountered an incompatible PICT");
    }

    // The very first thing we should find is an extended header opcode. Read this
    // outside of the main opcode loop as it should only appear once.
    if (static_cast<opcode>(pict_reader.read_short()) != opcode::ext_header) {
        throw std::runtime_error("Expected to find PICT Extended Header.");
    }

    if ((pict_reader.read_long() >> 16) != 0xFFFE) {
        // Standard header variant
        graphite::qd::fixed_rect rect(static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16)),
                                      static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16)),
                                      static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16)),
                                      static_cast<double>(pict_reader.read_signed_long() / static_cast<double>(1 << 16)));
        m_x_ratio = m_frame.width() / rect.width();
        m_y_ratio = m_frame.height() / rect.height();
    }
    else {
        // Extended header variant
        pict_reader.move(sizeof(uint32_t) * 2);
        graphite::qd::rect rect(pict_reader.read_signed_short(),
                                pict_reader.read_signed_short(),
                                pict_reader.read_signed_short(),
                                pict_reader.read_signed_short());
        m_x_ratio = static_cast<double>(m_frame.width()) / rect.width();
        m_y_ratio = static_cast<double>(m_frame.height()) / rect.height();
    }

    if (m_x_ratio <= 0 || m_y_ratio <= 0) {
        throw std::runtime_error("Invalid PICT resource. Content aspect ratio is not valid.");
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
        }
    }
}


// MARK: - Constructors

graphite::qd::pict::pict(std::shared_ptr<graphite::data::data> data)
    : m_surface(nullptr), m_frame(0, 0, 100, 100)
{
    // Setup a reader for the PICT data, and then parse it.
    data::reader pict_reader(data);
    parse(pict_reader);
}

std::shared_ptr<graphite::qd::pict> graphite::qd::pict::open(int64_t id)
{
    if (auto pict_res = graphite::rsrc::manager::shared_manager().find("PICT", 128).lock()) {
        return std::make_shared<graphite::qd::pict>(pict_res->data());
    }
    return nullptr;
}

// MARK: - Accessors

std::weak_ptr<graphite::qd::surface> graphite::qd::pict::image_surface() const
{
    return m_surface;
}