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

#pragma once

#include <cstdint>
#include <cstdlib>

namespace quickdraw
{
    typedef std::uint8_t color_component;

    union color
    {
        std::uint32_t value;
        struct {
            color_component red;
            color_component green;
            color_component blue;
            color_component alpha;
        } components;
    };

    union ycbcr
    {
        std::uint32_t value;
        struct {
            color_component y;
            color_component cb;
            color_component cr;
            color_component alpha;
        } components;
    };

    static auto operator==(const union color& lhs, const union color& rhs) -> bool { return lhs.value == rhs.value; }
    static auto operator!=(const union color& lhs, const union color& rhs) -> bool { return lhs.value != rhs.value; }

    [[nodiscard]] auto rgb(color_component r, color_component g, color_component b, color_component a = 255) -> union color;
    [[nodiscard]] auto rgb(std::uint16_t rgb555) -> union color;

    namespace constants
    {
        constexpr std::size_t color_width = sizeof(union color);
        constexpr std::size_t rgb555_color_width = 2;
    }

    namespace colors
    {
        [[nodiscard]] auto black() -> union color;
        [[nodiscard]] auto white() -> union color;
        [[nodiscard]] auto clear() -> union color;
    }

    [[nodiscard]] auto ycbcr(const union color& rgb) -> union ycbcr;
    [[nodiscard]] auto rgb(const union ycbcr& color) -> union color;
}