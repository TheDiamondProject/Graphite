//
// Created by Tom Hancocks on 5/02/2022.
//

#include "libGraphite/quicktime/planar.hpp"
#include "libGraphite/quickdraw/internal/packbits.hpp"

static inline auto read_bytes(graphite::data::reader& reader, std::size_t size) -> std::vector<uint8_t>
{
    auto bytes = reader.read_bytes(size);
    return std::vector<uint8_t>(bytes.begin(), bytes.end());
}

auto graphite::qt::planar::decode(const qt::imagedesc& imagedesc, data::reader& reader) -> qd::surface
{
    auto depth = imagedesc.depth();
    if (depth != 1 && depth != 8 && depth != 24 && depth != 32) {
        throw std::runtime_error("Unsupported planar bit depth: " + std::to_string(depth));
    }

    // Parse the remaining atoms to determine the channel count
    auto channel_count = 1;
    auto remaining = imagedesc.data_offset();
    while (remaining >= 10) {
        auto size = reader.read_long();
        auto type = reader.read_long();
        auto value = reader.read_short();
        remaining -= 10;
        if (type == 'chct') {
            channel_count = value;
        }
    }
    if (remaining > 0) {
        reader.move(remaining);
    }

    auto width = imagedesc.width();
    auto height = imagedesc.height();
    auto data_size = imagedesc.data_size();
    auto row_bytes = (width * depth + 7) / 8; // +7 to ensure result is rounded up
    auto surface = qd::surface(width, height);

    std::vector<uint8_t> raw;
    if (imagedesc.version() == 0) {
        raw = read_bytes(reader, row_bytes * height);
    } else {
        // Packbits - all counts are stored first
        std::vector<uint16_t> pack_counts(height * channel_count);
        for (auto i=0; i < pack_counts.size(); ++i) {
            pack_counts[i] = reader.read_short();
        }
        raw.reserve(row_bytes * height);
        for (auto count : pack_counts) {
            std::vector<uint8_t> pack = read_bytes(reader, count);
            qd::packbits::decode(raw, pack, 1);
        }
    }

    if (depth == 1) {
        // Monochrome
        for (auto y = 0; y < height; ++y) {
            for (auto offset = 0; offset < row_bytes; ++offset) {
                auto byte = raw[y * row_bytes + offset];
                for (auto i = 0; i < 8; ++i) {
                    auto v = byte & (1 << (7 - i));
                    surface.set(offset * 8 + i, y, v ? qd::color::black() : qd::color::white());
                }
            }
        }
    }
    else if (depth == 8) {
        // 8-bit indexed
        auto clut = imagedesc.clut();
        for (auto y=0; y < height; ++y) {
            for (auto x=0; x < width; ++x) {
                surface.set(x, y, clut->get(raw[y * width + x]));
            }
        }
    }
    else if (depth == 24 || depth == 32) {
        // Planar RGB
        auto plane_size = width * height;
        for (auto y=0; y < height; ++y) {
            for (auto x=0; x < width; ++x) {
                auto color = qd::color(raw[y * width + x],
                                       raw[y * width + x + plane_size],
                                       raw[y * width + x + plane_size * 2]);
                surface.set(x, y, color);
            }
        }
    }

    return surface;
}
