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

#include "libGraphite/quicktime/planar.hpp"
#include "libGraphite/compression/packbits.hpp"

// MARK: - Decoding

auto graphite::quicktime::format::planar::decode(const quicktime::image_description &desc, data::reader &reader) -> quickdraw::surface
{
    auto depth = desc.depth();
    if (depth != 1 && depth != 8 && depth != 24 && depth != 32) {
        throw std::runtime_error("Unsupported planar bit depth: " + std::to_string(depth));
    }

    // Parse the remaining atoms to determine the channel count
    auto channel_count = 1;
    auto remaining = desc.data_offset();
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

    auto width = desc.width();
    auto height = desc.height();
    auto data_size = desc.data_size();
    auto row_bytes = (width * depth + 7) / 8; // +7 to ensure result is rounded up
    auto surface = quickdraw::surface(width, height);

    data::block raw;
    if (desc.version() == 0) {
        raw = reader.read_data(row_bytes * height);
    } else {
        // Packbits - all counts are stored first
        std::vector<uint16_t> pack_counts(height * channel_count);
        for (auto i=0; i < pack_counts.size(); ++i) {
            pack_counts[i] = reader.read_short();
        }
        for (auto count : pack_counts) {
            raw = reader.read_compressed_data<compression::packbits<16>>(count);
        }
    }

    if (depth == 1) {
        // Monochrome
        for (auto y = 0; y < height; ++y) {
            for (auto offset = 0; offset < row_bytes; ++offset) {
                auto byte = raw.get<std::uint8_t>(y * row_bytes + offset);
                for (auto i = 0; i < 8; ++i) {
                    auto v = byte & (1 << (7 - i));
                    surface.set(offset * 8 + i, y, v ? quickdraw::colors::black() : quickdraw::colors::white());
                }
            }
        }
    }
    else if (depth == 8) {
        // 8-bit indexed
        auto clut = desc.clut();
        for (auto y = 0; y < height; ++y) {
            for (auto x = 0; x < width; ++x) {
                surface.set(x, y, clut.at(raw.get<std::uint8_t>(y * width + x)));
            }
        }
    }
    else if (depth == 24 || depth == 32) {
        // Planar RGB
        auto plane_size = width * height;
        for (auto y=0; y < height; ++y) {
            for (auto x=0; x < width; ++x) {
                auto color = quickdraw::rgb(
                    raw.get<std::uint8_t>(y * width + x),
                    raw.get<std::uint8_t>(y * width + x + plane_size),
                    raw.get<std::uint8_t>(y * width + x + plane_size * 2)
                );
                surface.set(x, y, color);
            }
        }
    }

    return std::move(surface);
}