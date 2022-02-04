//
// Created by Tom Hancocks on 24/03/2020.
//

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "libGraphite/quickdraw/rle.hpp"
#include "libGraphite/rsrc/manager.hpp"

static const auto rle_grid_width = 6;

// MARK: - Constructor

graphite::qd::rle::rle(std::shared_ptr<data::data> data, int64_t id, std::string name)
    : m_id(id), m_name(std::move(name))
{
    auto reader = data::reader(std::move(data));
    parse(reader);
}

graphite::qd::rle::rle(qd::size frame_size, uint16_t frame_count)
    : m_id(0), m_name("RLE"), m_frame_size(frame_size), m_frame_count(frame_count), m_bpp(16), m_palette_id(0)
{
    // Determine what the grid will be. We need to round up to the next whole number and have blank tiles
    // if the frame count is not divisible by the grid width constant.
    auto grid_width = std::min(rle_grid_width, static_cast<int>(m_frame_count));
    m_grid_size = qd::size(static_cast<int16_t>(grid_width),
                           static_cast<int16_t>(std::ceil(m_frame_count / static_cast<double>(grid_width))));

    // Create the surface
    m_surface = std::make_shared<qd::surface>(m_grid_size.width() * m_frame_size.width(),
                                              m_grid_size.height() * m_frame_size.height());
}

auto graphite::qd::rle::load_resource(int64_t id) -> std::shared_ptr<graphite::qd::rle>
{
    if (auto rle_res = graphite::rsrc::manager::shared_manager().find("rlëD", id).lock()) {
        return std::make_shared<graphite::qd::rle>(rle_res->data());
    }
    return nullptr;
}

// MARK: - Accessors

auto graphite::qd::rle::surface() const -> std::weak_ptr<graphite::qd::surface>
{
    return m_surface;
}

auto graphite::qd::rle::frames() const -> std::vector<graphite::qd::rect>
{
    return m_frames;
}

auto graphite::qd::rle::frame_count() const -> int
{
    return static_cast<int>(m_frame_count);
}

auto graphite::qd::rle::frame_rect(int frame) const -> graphite::qd::rect
{
    return qd::rect((frame % rle_grid_width) * m_frame_size.width(), (frame / rle_grid_width) * m_frame_size.height(),
                    m_frame_size.width(), m_frame_size.height());
}

auto graphite::qd::rle::frame_surface(int frame) const -> std::shared_ptr<qd::surface>
{
    auto surface = std::make_shared<qd::surface>(m_frame_size.width(), m_frame_size.height());
    auto src_rect = frame_rect(frame);

    // Extract the frame area of the origin surface
    for (auto x = 0; x < src_rect.width(); x++) {
        for (auto y = 0; y < src_rect.height(); y++) {
            surface->set(x, y, m_surface->at(x + src_rect.x(), y + src_rect.y()));
        }
    }

    return surface;
}

auto graphite::qd::rle::write_frame(int frame, const std::shared_ptr<qd::surface>& surface) -> void
{
    auto dst_rect = frame_rect(frame);
    auto src_size = surface->size();

    if (src_size.width() != m_frame_size.width() || src_size.height() != m_frame_size.height()) {
        throw std::runtime_error("Incorrect frame dimensions " + std::to_string(src_size.width()) + "x" + std::to_string(src_size.height()) +
                                 ", expected " + std::to_string(m_frame_size.width()) + "x" + std::to_string(m_frame_size.height()));
    }

    // Copy from the source surface into the destination frame
    for (auto x = 0; x < dst_rect.width(); x++) {
        for (auto y = 0; y < dst_rect.height(); y++) {
            m_surface->set(x + dst_rect.x(), y + dst_rect.y(), surface->at(x, y));
        }
    }
}

// MARK: - Parsing

auto graphite::qd::rle::parse(data::reader &reader) -> void
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
        throw std::runtime_error("Incorrect color depth for rlëD resource: " + std::to_string(m_id) + ", " + m_name);
    }

    // Determine what the grid will be. We need to round up to the next whole number and have blank tiles
    // if the frame count is not divisible by the grid width constant.
    auto grid_width = std::min(rle_grid_width, static_cast<int>(m_frame_count));
    m_grid_size = qd::size(static_cast<int16_t>(grid_width),
                           static_cast<int16_t>(std::ceil(m_frame_count / static_cast<double>(grid_width))));

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

        count = reader.read_long();
        opcode = static_cast<rle::opcode>(count >> 24);
        count &= 0x00FFFFFF;

        switch (opcode) {
            case rle::opcode::eof: {
                // Check that we're not erroneously encountering an EOF.
                if (current_line > static_cast<int32_t>(m_frame_size.height() - 1)) {
                    throw std::runtime_error("Incorrect number of scanlines in rlëD resource: " + std::to_string(m_id) + ", " + m_name);
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
                    ++current_offset;
                }

                if (count & 0x03) {
                    reader.move(4 - (count & 0x03));
                }

                break;
            }

            case rle::opcode::pixel_run: {
                pixel_run = reader.read_long();
                for (auto i = 0; i < count; i += 4) {
                    write_pixel_variant1(pixel_run, 0xff, current_offset);
                    ++current_offset;

                    if (i + 2 < count) {
                        write_pixel_variant2(pixel_run, 0xff, current_offset);
                        ++current_offset;
                    }
                }
                break;
            }

            case rle::opcode::transparent_run: {
                current_offset += count >> 1;
                break;
            }
        }
    }

    COMPLETED_LAST_FRAME:
    // Finished decoding rlëD data.
    return;
}

auto graphite::qd::rle::surface_offset(int32_t frame, int32_t line) const -> uint64_t
{
    qd::point fo(frame % rle_grid_width, frame / rle_grid_width);
    qd::point p(fo.x() * m_frame_size.width(), (fo.y() * m_frame_size.height()) + line);
    return static_cast<uint64_t>(p.y() * m_surface->size().width() + p.x());
}

auto graphite::qd::rle::write_pixel(uint16_t pixel, uint8_t mask, uint64_t offset) -> void
{
    m_surface->set(static_cast<int>(offset), qd::color(pixel));
}

auto graphite::qd::rle::write_pixel_variant1(uint32_t pixel, uint8_t mask, uint64_t offset) -> void
{
    auto rgb555 = static_cast<uint16_t>(pixel >> 16);
    m_surface->set(static_cast<int>(offset), qd::color(rgb555));
}

auto graphite::qd::rle::write_pixel_variant2(uint32_t pixel, uint8_t mask, uint64_t offset) -> void
{
    auto rgb555 = static_cast<uint16_t>(pixel & 0x0000FFFF);
    m_surface->set(static_cast<int>(offset), qd::color(rgb555));
}

// MARK: - Encoder / Writing

auto graphite::qd::rle::encode(graphite::data::writer& writer) -> void
{
    // Write out the header
    m_frame_size.write(writer, qd::size::pict);
    writer.write_short(m_bpp);
    writer.write_short(m_palette_id);
    writer.write_short(m_frame_count);

    // Reserved fields
    writer.write_short(0);
    writer.write_short(0);
    writer.write_short(0);

    const auto advance = 2; // we only support 16 bits per pixel

    // Write out the RLE frames
    for (auto f = 0; f < m_frame_count; f++) {
        auto frame = frame_rect(f);
        auto line_count = 0;

        for (auto y = 0; y < frame.height(); y++) {
            line_count++;
            auto line_start_pos = writer.position();

            opcode run_state = line_start;
            auto run_start_pos = line_start_pos + 4;
            auto run_count = 0;

            for (auto x = 0; x < frame.width(); x++) {
                qd::color pixel = m_surface->at(frame.x() + x, frame.y() + y);

                if (pixel.alpha_component() == 0) {
                    if (run_state == line_start) {
                        // Start of a transparent run
                        run_state = transparent_run;
                        run_count = advance;
                    }
                    else if (run_state == transparent_run) {
                        // Continue transparent run
                        run_count += advance;
                    }
                    else {
                        // End of pixel run, start of transparent run
                        auto run_end_pos = writer.position();
                        writer.set_position(run_start_pos);
                        writer.write_long((pixel_data << 24) | (run_count & 0x00FFFFFF));
                        writer.set_position(run_end_pos);

                        // Pad to nearest 4-byte boundary
                        if (run_count & 3) {
                            writer.move(4 - (run_count & 3));
                        }

                        // Start transparent run
                        run_state = transparent_run;
                        run_count = advance;
                    }
                }
                else {
                    if (line_count != 0) {
                        // First pixel data for this line, write the line start
                        // Doing this only on demand allows us to omit trailing blank lines in the frame
                        for (auto i = 0; i < line_count; ++i) {
                            writer.write_long(line_start << 24);
                        }
                        line_count = 0;
                    }
                    if (run_state == line_start) {
                        // Start of a pixel run
                        run_start_pos = writer.position();
                        writer.write_long(0); // opcode placeholder
                        run_state = pixel_data;
                        run_count = advance;
                    }
                    else if (run_state == transparent_run) {
                        // End of transparent run, start of pixel run
                        writer.write_long((transparent_run << 24) | (run_count & 0x00FFFFFF));

                        // Start pixel run
                        run_start_pos = writer.position();
                        writer.write_long(0); // opcode placeholder
                        run_state = pixel_data;
                        run_count = advance;
                    }
                    else {
                        // Continue pixel run
                        run_count += advance;
                    }

                    // Write the pixel
                    writer.write_short(pixel.rgb555());
                }
            }

            // Terminate the current opcode
            if (run_state == pixel_data) {
                auto run_end_pos = writer.position();
                writer.set_position(run_start_pos);
                writer.write_long((pixel_data << 24) | (run_count & 0x00FFFFFF));
                writer.set_position(run_end_pos);

                // Pad to nearest 4-byte boundary
                if (run_count & 3) {
                    writer.move(4 - (run_count & 3));
                }
            }

            // Write out the opcode and size at the start of the line
            if (run_state != line_start) {
                auto line_end_pos = writer.position();
                writer.set_position(line_start_pos);
                writer.write_long((line_start << 24) | ((line_end_pos - line_start_pos - 4) & 0x00FFFFFF));
                writer.set_position(line_end_pos);
            }
        }

        // Mark end-of-frame
        writer.write_long(eof << 24);
    }
}

auto graphite::qd::rle::data() -> std::shared_ptr<graphite::data::data>
{
    auto data = std::make_shared<graphite::data::data>();
    graphite::data::writer writer(data);
    encode(writer);
    return data;
}
