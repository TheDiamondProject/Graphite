//
// Created by Tom Hancocks on 5/02/2022.
//

#include "libGraphite/quicktime/raw.hpp"
#include "libGraphite/quickdraw/pixmap.hpp"

static inline auto read_bytes(graphite::data::reader& reader, std::size_t size) -> std::vector<uint8_t>
{
    auto bytes = reader.read_bytes(size);
    return std::vector<uint8_t>(bytes.begin(), bytes.end());
}

auto graphite::qt::raw::decode(const qt::imagedesc& imagedesc, data::reader& reader) -> qd::surface
{
    auto depth = imagedesc.depth();
    if (depth > 8) {
        throw std::runtime_error("Unsupported raw bit depth: " + std::to_string(depth));
    }
    auto width = imagedesc.width();
    auto height = imagedesc.height();
    auto clut = imagedesc.clut();
    auto surface = qd::surface(width, height);
    
    if (depth == 8) {
        for (auto y = 0; y < height; ++y) {
            auto raw = read_bytes(reader, width);
            for (auto x = 0; x < width; ++x) {
                surface.set(x, y, clut->get(raw[x]));
            }
        }
    }
    else {
        auto pixels_per_byte = 8 / depth;
        auto mask = (1 << depth) - 1;
        auto row_bytes = imagedesc.data_size() / height;

        for (auto y = 0; y < height; ++y) {
            auto x = 0;
            auto raw = read_bytes(reader, row_bytes);
            for (auto byte : raw) {
                for (auto i = 1; i <= pixels_per_byte; ++i) {
                    auto byte_offset = 8 - (i * depth);
                    auto v = (byte >> byte_offset) & mask;
                    surface.set(x++, y, clut->get(v));
                }
            }
        }
    }
    
    return surface;
}
