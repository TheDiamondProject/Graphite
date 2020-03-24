//
// Created by Tom Hancocks on 24/03/2020.
//

#include <complex>
#include "libGraphite/quickdraw/rle.hpp"
#include "libGraphite/rsrc/manager.hpp"

static const auto rle_grid_width = 6;

// MARK: - Constructor

graphite::qd::rle::rle(std::shared_ptr<data::data> data)
{
    auto reader = data::reader(data);
    parse(reader);
}

std::shared_ptr<graphite::qd::rle> graphite::qd::rle::load_resource(int64_t id)
{
    if (auto rle_res = graphite::rsrc::manager::shared_manager().find("rlëD", id).lock()) {
        return std::make_shared<graphite::qd::rle>(rle_res->data());
    }
    return nullptr;
}

// MARK: - Accessors

std::weak_ptr<graphite::qd::surface> graphite::qd::rle::surface() const
{
    return m_surface;
}

std::vector<graphite::qd::rect> graphite::qd::rle::frames() const
{
    return m_frames;
}

int graphite::qd::rle::frame_count() const
{
    return static_cast<int>(m_frame_count);
}

graphite::qd::rect graphite::qd::rle::frame_at(int frame) const
{
    return m_frames[frame];
}

// MARK: - Parsing

void graphite::qd::rle::parse(data::reader &reader)
{
    // Read the header of the RLE information. This will tell us what we need to do in order to
    // actually decode the frames.
    m_frame_size = qd::size::read(reader, qd::size::pict);
    m_bpp = reader.read_short();
    m_palette_id = reader.read_short();
    m_frame_count = reader.read_short();
    reader.move(6);

    // Ensure that the RLE has a BPP of 16. This is the only format that we support currently.
    if (m_bpp != 16) {
        throw std::runtime_error("Incorrect color depth for rlëD resource");
    }

    // Determine what the grid will be. We need to round up to the next whole number and have blank tiles
    // if the frame count is not divisible by the grid width constant.
    m_grid_size = qd::size(static_cast<int16_t>(rle_grid_width),
                           static_cast<int16_t>(std::ceil(m_frame_count / static_cast<double>(rle_grid_width))));

    // Create the surface in which all frames will be drawn to, and other working variables required to parse and decode
    // the RLE data correctly.
    m_surface = std::make_shared<qd::surface>(m_grid_size.width() * m_frame_size.width(),
                                              m_grid_size.height() * m_frame_size.height());

    rle::opcode opcode = eof;
    uint64_t position = 0;
    uint32_t row_start = 0;
    int32_t current_line = -1;
    uint64_t current_offset = 0;
    int32_t count = 0;
    uint16_t pixel = 0;
    int32_t current_frame = 0;
    uint32_t pixel_run = 0;

    while (!reader.eof()) {
        if ((row_start != 0) && ((position - row_start) & 0x03)) {
            position += 4 - ((position - row_start) & 0x03);
            reader.move(4 - (count & 0x03));
        }

        count = reader.read_signed_long();
        opcode = static_cast<rle::opcode>(count >> 24);
        count &= 0x00FFFFFF;

        switch (opcode) {
            case rle::opcode::eof: {
                // Check that we're not erroneously encountering an EOF.
                if (current_line != static_cast<int32_t>(m_frame_size.height() - 1)) {
                    throw std::runtime_error("Incorrect number of scanlines in rlëD resource.");
                }

                // Have we finished decoding the last frame in the data?
                if (++current_frame >= m_frame_count) {
                    goto COMPLETED_LAST_FRAME;
                }

                // Prepare for the next frame.
                current_line = -1;
                break;
            }

            case rle::opcode::line_start: {
                current_offset = surface_offset(current_frame, ++current_line);
                row_start = static_cast<int32_t>(reader.position());
                break;
            }

            case rle::opcode::pixel_data: {
                for (auto i = 0; i < count; i += 2) {
                    pixel = reader.read_short();
                    write_pixel(pixel, 0xff, current_offset);
                    current_offset += 4;
                }

                if (count & 0x03) {
                    reader.move(4 - (count & 0x03));
                }

                break;
            }

            case rle::opcode::pixel_run: {
                pixel_run = reader.read_long();
                for (auto i = 0; i < count; ++i) {
                    write_pixel_variant1(pixel_run, 0xff, current_offset);
                    current_offset += 4;

                    if (i + 2 < count) {
                        write_pixel_variant2(pixel_run, 0xff, current_offset);
                        current_offset += 4;
                    }
                }
                break;
            }

            case rle::opcode::transparent_run: {
                current_offset += (count >> ((m_bpp >> 3) - 1)) << 2;
                break;
            }
        }
    }

    COMPLETED_LAST_FRAME:
    // Finished decoding rlëD data.
    return;
}

uint64_t graphite::qd::rle::surface_offset(int32_t frame, int32_t line) const
{
    qd::point fo(frame % rle_grid_width, frame / rle_grid_width);
    qd::point p(fo.x() * m_frame_size.width(), (fo.y() * m_frame_size.height()) + line);
    return static_cast<uint64_t>(p.y() * m_surface->size().width() + p.x());
}

void graphite::qd::rle::write_pixel(uint16_t pixel, uint8_t mask, uint64_t offset)
{
    auto r = static_cast<uint8_t>((pixel & 0x7C00) >> 7);
    auto g = static_cast<uint8_t>((pixel & 0x03E0) >> 2);
    auto b = static_cast<uint8_t>((pixel & 0x001F) << 3);
    m_surface->set(static_cast<int>(offset), qd::color(r, g, b));
}

void graphite::qd::rle::write_pixel_variant1(uint32_t pixel, uint8_t mask, uint64_t offset)
{
    auto r = static_cast<uint8_t>((pixel & 0x7C000000) >> 23);
    auto g = static_cast<uint8_t>((pixel & 0x03E00000) >> 18);
    auto b = static_cast<uint8_t>((pixel & 0x001F0000) << 13);
    m_surface->set(static_cast<int>(offset), qd::color(r, g, b));
}

void graphite::qd::rle::write_pixel_variant2(uint32_t pixel, uint8_t mask, uint64_t offset)
{
    auto r = static_cast<uint8_t>((pixel & 0x00007C00) >> 7);
    auto g = static_cast<uint8_t>((pixel & 0x000003E0) >> 2);
    auto b = static_cast<uint8_t>((pixel & 0x0000001F) << 3);
    m_surface->set(static_cast<int>(offset), qd::color(r, g, b));
}