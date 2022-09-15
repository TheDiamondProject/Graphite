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

#include <algorithm>
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

// MARK: - YCrCb Color

auto graphite::quickdraw::ycbcr(const union color& rgb) -> union ycbcr
{
    std::uint8_t r = rgb.components.red;
    std::uint8_t g = rgb.components.green;
    std::uint8_t b = rgb.components.blue;

    auto y  = static_cast<std::int16_t>(      0.299000 * r + 0.587000 * g + 0.114000 * b);
    auto cb = static_cast<std::int16_t>(128 - 0.168736 * r - 0.331264 * g + 0.500000 * b);
    auto cr = static_cast<std::int16_t>(128 + 0.500000 * r - 0.418688 * g - 0.081312 * b);

    std::uint8_t y_clamped  = std::clamp<std::uint8_t>(y , 0, 255);
    std::uint8_t cb_clamped = std::clamp<std::uint8_t>(cb, 0, 255);
    std::uint8_t cr_clamped = std::clamp<std::uint8_t>(cr, 0, 255);

    return (union ycbcr) {
        .components.y  = y_clamped,
        .components.cb = cb_clamped,
        .components.cr = cr_clamped,
        .components.alpha = rgb.components.alpha
    };
}

auto graphite::quickdraw::rgb(const union ycbcr& color) -> union color
{
    auto r = std::clamp<std::int16_t>(color.components.y + (1.402000 * (color.components.cr - 128)), 0, 255);
    auto g = std::clamp<std::int16_t>(color.components.y - (0.344136 * (color.components.cb - 128)) - (0.714136 * (color.components.cr - 128)), 0, 255);
    auto b = std::clamp<std::int16_t>(color.components.y + (1.772000 * (color.components.cb - 128)), 0, 255);

    return rgb(r, g, b, color.components.alpha);
}
