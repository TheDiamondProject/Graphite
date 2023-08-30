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
#include <libQuickdraw/color/color.hpp>

// MARK: - Construction

auto quickdraw::rgb(color_component r, color_component g, color_component b, color_component a) -> union color
{
    return {
        .components = {
            .red = r, .green = g, .blue = b, .alpha = a
        }
    };
}

auto quickdraw::rgb(std::uint16_t rgb555) -> union color
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

auto quickdraw::colors::white() -> union color
{
    return rgb(255, 255, 255);
}

auto quickdraw::colors::black() -> union color
{
    return rgb(0, 0, 0);
}

auto quickdraw::colors::clear() -> union color
{
    return rgb(0, 0, 0, 0);
}

// MARK: - YCrCb Color

auto quickdraw::ycbcr(const union color& rgb) -> union ycbcr
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
        .components = {
            .y = y_clamped,
            .cb = cb_clamped,
            .cr = cr_clamped,
            .alpha = rgb.components.alpha
        },
    };
}

auto quickdraw::rgb(const union ycbcr& color) -> union color
{
    auto color_value = color.value & 0xFFFFFF;
    if (color.components.alpha < 0.05) {
        return rgb(0, 0, 0, 0);
    }
    else if (color_value == 0x008080) {
        return rgb(0, 0, 0, color.components.alpha);
    }
    else if (color_value == 0xFF8080) {
        return rgb(255, 255, 255, color.components.alpha);
    }

    auto r = std::clamp<std::int16_t>(color.components.y + (1.402000 * (color.components.cr - 128)), 0, 255);
    auto g = std::clamp<std::int16_t>(color.components.y - (0.344136 * (color.components.cb - 128)) - (0.714136 * (color.components.cr - 128)), 0, 255);
    auto b = std::clamp<std::int16_t>(color.components.y + (1.772000 * (color.components.cb - 128)), 0, 255);

    return rgb(r, g, b, color.components.alpha);
}
