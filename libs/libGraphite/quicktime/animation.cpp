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

#include "libGraphite/quicktime/animation.hpp"

auto graphite::quicktime::format::animation::decode(const quicktime::image_description &desc, data::reader &reader) -> quickdraw::surface
{
    auto depth = desc.depth();
    if (depth < 8) {
        // Depths 1, 2, 4 currently unsupported
        throw std::runtime_error("Unsupported rle bit depth: " + std::to_string(depth));
    }
    auto clut = desc.clut();
    auto surface = quickdraw::surface(desc.width(), desc.height());
    auto chunk_size = reader.read_long();
    auto header = reader.read_short();
    auto y = 0;
    if (header & 0x0008) {
        y = reader.read_short();
        reader.move(6);
    }

    std::int8_t skip;
    std::int8_t code;
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
                        auto raw = reader.read_data(4 * code);
                        for (auto i = 0; i < 4 * code; ++i) {
                            auto color = clut.at(raw.get<std::uint8_t>(i));
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 16: {
                        auto raw = reader.read_data(2 * code);
                        for (auto i = 0; i < code; ++i) {
                            auto color = quickdraw::rgb((raw.get<std::uint8_t>(i * 2) << 8) | (raw.get<std::uint8_t>(i * 2 + 1)));
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 24: {
                        auto raw = reader.read_data(3 * code);
                        for (auto i = 0; i < code; ++i) {
                            auto color = quickdraw::rgb(raw.get<std::uint8_t>(i * 3),
                                                        raw.get<std::uint8_t>(i * 3 + 1),
                                                        raw.get<std::uint8_t>(i * 3 + 2));
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 32: {
                        auto raw = reader.read_data(4 * code);
                        for (auto i = 0; i < code; ++i) {
                            auto color = quickdraw::rgb(raw.get<std::uint8_t>(i * 4 + 1),
                                                        raw.get<std::uint8_t>(i * 4 + 2),
                                                        raw.get<std::uint8_t>(i * 4 + 3),
                                                        raw.get<std::uint8_t>(i * 4));
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
                        auto raw = reader.read_data(4);
                        for (auto i = 0; i < 4 * -code; ++i) {
                            auto color = clut.at(raw.get<std::uint8_t>(i % 4));
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 16: {
                        auto color = quickdraw::rgb(reader.read_short());
                        for (auto i = 0; i < -code; ++i) {
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 24: {
                        auto color = quickdraw::rgb(reader.read_byte(), reader.read_byte(), reader.read_byte());
                        for (auto i = 0; i < -code; ++i) {
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                    case 32: {
                        auto alpha = reader.read_byte();
                        auto color = quickdraw::rgb(reader.read_byte(), reader.read_byte(), reader.read_byte(), alpha);
                        for (auto i = 0; i < -code; ++i) {
                            surface.set(x++, y, color);
                        }
                        break;
                    }
                }
            }
        }
    }

    return std::move(surface);
}
