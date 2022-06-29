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

#include "libGraphite/quickdraw/type/point.hpp"
#include "libGraphite/quickdraw/type/size.hpp"
#include "libGraphite/quickdraw/type/color.hpp"
#include "libGraphite/data/data.hpp"

namespace graphite::quickdraw
{
    struct surface
    {
    public:
        surface() = default;
        explicit surface(const size<std::int16_t>& size);
        surface(std::int16_t width, std::int16_t height);
        surface(const size<std::int16_t>& size, union color color);
        surface(std::int16_t width, std::int16_t height, union color color);
        surface(const surface& surface);
        surface(surface&& surface) noexcept;

        ~surface();

        auto operator=(const surface&) -> surface&;
        auto operator=(surface&&) -> surface&;

        [[nodiscard]] auto raw() const -> const data::block&;
        [[nodiscard]] auto size() const -> struct size<std::int16_t>;

        [[nodiscard]] auto at(const point<std::int16_t>& p) const -> union color;
        [[nodiscard]] auto at(std::uint32_t offset) const -> union color;
        [[nodiscard]] auto at(std::int16_t x, std::int16_t y) const -> union color;

        auto set(const point<std::int16_t>& p, union color color) -> void;
        auto set(std::int16_t x, std::int16_t y, union color color) -> void;
        auto set(std::uint32_t offset, union color color) -> void;

        auto clear() -> void;

    private:
        std::uint32_t m_row_bytes;
        quickdraw::size<std::int16_t> m_size;
        data::block *m_data { nullptr };
    };
}
