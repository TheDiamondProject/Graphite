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

#include <libQuickdraw/surface/surface.hpp>

// MARK: - Construction

quickdraw::surface::surface(const quickdraw::size<std::int16_t> &size)
    : m_size(size),
      m_row_bytes(size.width * constants::color_width),
      m_data(new data::block(size.width * size.height * constants::color_width))
{
    m_data->set(static_cast<std::uint32_t>(0), m_data->size());
}

quickdraw::surface::surface(std::int16_t width, std::int16_t height)
    : m_size(width, height),
      m_row_bytes(width * constants::color_width),
      m_data(new data::block(width * height * constants::color_width))
{
    m_data->set(static_cast<std::uint32_t>(0), m_data->size());
}

quickdraw::surface::surface(const quickdraw::size<std::int16_t>& size, union color color)
    : m_size(size),
      m_row_bytes(size.width * constants::color_width),
      m_data(new data::block(size.width * size.height * constants::color_width))
{
    m_data->set(color.value, m_data->size());
}

quickdraw::surface::surface(std::int16_t width, std::int16_t height, union color color)
    : m_size(width, height),
      m_row_bytes(width * constants::color_width),
      m_data(new data::block(width * height * constants::color_width))
{
    m_data->set(color.value, m_data->size());
}

quickdraw::surface::surface(const surface &surface)
    : m_size(surface.m_size),
      m_row_bytes(surface.m_row_bytes),
      m_data(new data::block(*surface.m_data))
{
}

quickdraw::surface::surface(surface &&surface) noexcept
    : m_size(surface.m_size),
      m_row_bytes(surface.m_row_bytes),
      m_data(surface.m_data)
{
    surface.m_data = nullptr;
}

// MARK: - Destruction

quickdraw::surface::~surface()
{
    if (!m_weak_data && m_data) {
        delete m_data;
    }
}

// MARK: - Operators

auto quickdraw::surface::operator=(const surface& surface) -> class surface&
{
    if (this == const_cast<class surface *>(&surface)) {
        return *this;
    }

    m_data = new data::block(*surface.m_data);
    m_weak_data = false;
    m_size = surface.m_size;
    m_row_bytes = surface.m_row_bytes;

    return *this;
}

auto quickdraw::surface::operator=(class surface&& surface) noexcept -> class surface&
{
    if (this != &surface) {
        delete m_data;

        m_data = surface.m_data;
        m_weak_data = false;
        m_size = surface.m_size;
        m_row_bytes = surface.m_row_bytes;

        surface.m_data = nullptr;
    }
    return *this;
}

// MARK: - Accessors

auto quickdraw::surface::raw() const -> const data::block&
{
    return *m_data;
}

auto quickdraw::surface::size() const -> struct quickdraw::size<std::int16_t>
{
    return m_size;
}

// MARK: - Pixel Access

auto quickdraw::surface::at(const point<std::int16_t> &p) const -> union color
{
    return at(p.x, p.y);
}

auto quickdraw::surface::at(std::int16_t x, std::int16_t y) const -> union color
{
    return at((y * m_size.width) + x);
}

auto quickdraw::surface::at(std::uint32_t offset) const -> union color
{
    return m_data->get<union color *>()[offset];
}

auto quickdraw::surface::set(const point<std::int16_t>& p, union color color) -> void
{
    set(p.x, p.y, color);
}

auto quickdraw::surface::set(std::int16_t x, std::int16_t y, union color color) -> void
{
    set((y * m_size.width) + x, color);
}

auto quickdraw::surface::set(std::uint32_t offset, union color color) -> void
{
    m_data->set(color.value, constants::color_width, offset * constants::color_width);
}

// MARK: - Operations

auto quickdraw::surface::clear() -> void
{
    m_data->set(colors::clear().value, m_data->size());
}