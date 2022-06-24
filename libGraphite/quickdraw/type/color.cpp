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

#include "libGraphite/quickdraw/type/color.hpp"

// MARK: - Construction

auto graphite::quickdraw::rgb(color_component r, color_component g, color_component b, color_component a) -> union color
{
    return {
        .components = {
            .red = r, .green = g, .blue = b, .alpha = a
        }
    };
}

auto graphite::quickdraw::rgb(std::uint16_t rgb555) -> union color
{
    union color c = {
        .components = {
            .red = static_cast<uint8_t>((rgb555 & 0x7c00) >> 7),
            .green = static_cast<uint8_t>((rgb555 & 0x03e0) >> 2),
            .blue = static_cast<uint8_t>((rgb555 & 0x001f) << 3),
            .alpha = 255
        }
    };

    c.components.red |= c.components.red >> 5;
    c.components.green |= c.components.green >> 5;
    c.components.blue |= c.components.blue >> 5;

    return c;
}

// MARK: - Pre-defined Colors

auto graphite::quickdraw::colors::white() -> union color
{
    return rgb(255, 255, 255);
}

auto graphite::quickdraw::colors::black() -> union color
{
    return rgb(0, 0, 0);
}

auto graphite::quickdraw::colors::clear() -> union color
{
    return rgb(0, 0, 0, 0);
}