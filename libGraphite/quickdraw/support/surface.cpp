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

#include "libGraphite/quickdraw/support/surface.hpp"

// MARK: - Construction

graphite::quickdraw::surface::surface(const quickdraw::size<std::int16_t> &size)
    : m_size(size),
      m_data(size.width * size.height * constants::color_width)
{

}

graphite::quickdraw::surface::surface(std::int16_t width, std::int16_t height)
    : m_size(width, height),
      m_data(width * height * constants::color_width)
{

}

graphite::quickdraw::surface::surface(const quickdraw::size<std::int16_t>& size, union color color)
    : m_size(size),
      m_data(size.width * size.height * constants::color_width)
{
    m_data.set(color.value);
}

graphite::quickdraw::surface::surface(std::int16_t width, std::int16_t height, union color color)
    : m_size(width, height),
      m_data(width * height * constants::color_width)
{
    m_data.set(color.value);
}

graphite::quickdraw::surface::surface(const surface &surface)
{

}

graphite::quickdraw::surface::surface(surface &&surface) noexcept
{

}

// MARK: - Accessors

auto graphite::quickdraw::surface::raw() const -> const data::block&
{
    return m_data;
}

auto graphite::quickdraw::surface::size() const -> struct quickdraw::size<std::int16_t>
{
    return m_size;
}

// MARK: - Pixel Access

auto graphite::quickdraw::surface::at(const point<std::int16_t> &p) const -> union color
{
    return at(p.x, p.y);
}

auto graphite::quickdraw::surface::at(std::int16_t x, std::int16_t y) const -> union color
{
    return at(y * m_size.width + x);
}

auto graphite::quickdraw::surface::at(std::uint32_t offset) const -> union color
{
    return m_data.get<union color *>()[offset];
}

auto graphite::quickdraw::surface::set(const point<std::int16_t>& p, union color color) -> void
{
    set(p.x, p.y, color);
}

auto graphite::quickdraw::surface::set(std::int16_t x, std::int16_t y, union color color) -> void
{
    set(y * m_size.width + x, color);
}

auto graphite::quickdraw::surface::set(std::uint32_t offset, union color color) -> void
{
    m_data.set(color.value, 4, offset);
}