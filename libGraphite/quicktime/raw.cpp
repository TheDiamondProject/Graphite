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

#include "libGraphite/quicktime/raw.hpp"

// MARK: - Decoding

auto graphite::quicktime::format::raw::decode(const quicktime::image_description &desc, data::reader &reader) -> quickdraw::surface
{
    auto depth = desc.depth();
    if (depth > 8) {
        throw std::runtime_error("Unsupported raw bit depth: " + std::to_string(depth));
    }
    auto width = desc.width();
    auto height = desc.height();
    auto clut = desc.clut();
    auto surface = quickdraw::surface(width, height);

    if (depth == 8) {
        for (auto y = 0; y < height; ++y) {
            for (auto x = 0; x < width; ++x) {
                surface.set(x, y, clut.at(reader.read_byte()));
            }
        }
    }
    else {
        auto pixels_per_byte = 8 / depth;
        auto mask = (1 << depth) - 1;
        auto row_bytes = desc.data_size() / height;

        for (auto y = 0; y < height; ++y) {
            auto x = 0;
            auto raw = reader.read_bytes(row_bytes);
            for (auto byte : raw) {
                for (auto i = 1; i <= pixels_per_byte; ++i) {
                    auto byte_offset = 8 - (i * depth);
                    auto v = (byte >> byte_offset) & mask;
                    surface.set(x++, y, clut.at(v));
                }
            }
        }
    }

    return std::move(surface);
}