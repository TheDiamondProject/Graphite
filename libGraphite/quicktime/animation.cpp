//
// Created by Tom Hancocks on 26/03/2021.
//

#include "libGraphite/quicktime/animation.hpp"

static inline auto read_bytes(graphite::data::reader& reader, std::size_t size) -> std::vector<uint8_t>
{
    auto bytes = reader.read_bytes(size);
    return std::vector<uint8_t>(bytes.begin(), bytes.end());
}

auto graphite::qt::animation::decode(const qt::imagedesc& imagedesc, data::reader& reader) -> qd::surface
{
    auto depth = imagedesc.depth();
    if (depth < 8) {
        // Depths 1, 2, 4 currently unsupported
        throw std::runtime_error("Unsupported rle bit depth: " + std::to_string(depth));
    }
    auto clut = imagedesc.clut();
    auto surface = qd::surface(imagedesc.width(), imagedesc.height());
    auto chunk_size = reader.read_long();
    auto header = reader.read_short();
    auto y = 0;
    if (header & 0x0008) {
        y = reader.read_short();
        reader.move(6);
    }
    
    int8_t skip;
    int8_t code;
    auto x = 0;
    while ((skip = reader.read_byte())) {
        x += skip-1;
        while (true) {
            code = reader.read_signed_byte();
            if (code == 0) {
                // No op
                break;
            }
            else if (code == -1) {
                // Next line
                x = 0;
                y++;
                break;
            }
            else if (code > 0) {
                // Literal
                switch (depth) {
                    case 8: {
                        auto raw = read_bytes(reader, 4 * code);
                        for (auto i = 0; i < 4 * code; ++i) {
                            auto color = clut->get(raw[i]);
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 16: {
                        auto raw = read_bytes(reader, 2 * code);
                        for (auto i = 0; i < code; ++i) {
                            auto color = qd::color((raw[i * 2] << 8) | (raw[i * 2 + 1]));
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 24: {
                        auto raw = read_bytes(reader, 3 * code);
                        for (auto i = 0; i < code; ++i) {
                            auto color = qd::color(raw[i * 3],
                                                   raw[i * 3 + 1],
                                                   raw[i * 3 + 2]);
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 32: {
                        auto raw = read_bytes(reader, 4 * code);
                        for (auto i = 0; i < code; ++i) {
                            auto color = qd::color(raw[i * 4 + 1],
                                                   raw[i * 4 + 2],
                                                   raw[i * 4 + 3],
                                                   raw[i * 4]);
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                }
            }
            else {
                // Run
                switch (depth) {
                    case 8: {
                        auto raw = read_bytes(reader, 4);
                        for (auto i = 0; i < 4 * -code; ++i) {
                            auto color = clut->get(raw[i % 4]);
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 16: {
                        auto color = qd::color(reader.read_short());
                        for (auto i = 0; i < -code; ++i) {
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 24: {
                        auto raw = read_bytes(reader, 3);
                        auto color = qd::color(raw[0], raw[1], raw[2]);
                        for (auto i = 0; i < -code; ++i) {
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 32: {
                        auto raw = read_bytes(reader, 4);
                        auto color = qd::color(raw[1], raw[2], raw[3], raw[0]);
                        for (auto i = 0; i < -code; ++i) {
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                }
            }
        }
    }
    return surface;
}
