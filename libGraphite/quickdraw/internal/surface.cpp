//
// Created by Tom Hancocks on 20/02/2020.
//

#include "libGraphite/quickdraw/internal/surface.hpp"

// MARK: - Constructor

graphite::qd::surface::surface(int width, int height)
    : m_width(width), m_height(height), m_data(width * height, graphite::qd::color::clear())
{
}

graphite::qd::surface::surface(int width, int height, std::vector<graphite::qd::color> rgb)
: m_width(width), m_height(height), m_data(std::move(rgb))
{
}

// MARK: - Surface Access

auto graphite::qd::surface::raw() const -> std::vector<uint32_t>
{
    auto out = std::vector<uint32_t>();
    for (const auto& i : m_data) {
        uint32_t color = (i.alpha_component() << 24)
                | i.red_component()
                | (i.green_component() << 8UL)
                | (i.blue_component() << 16UL);
        out.push_back(color);
    }
    return out;
}

auto graphite::qd::surface::size() const -> graphite::qd::size
{
    return graphite::qd::size(m_width, m_height);
}

auto graphite::qd::surface::at(int x, int y) const -> graphite::qd::color
{
    return m_data[(y * m_width) + x];
}

auto graphite::qd::surface::set(int x, int y, graphite::qd::color color) -> void
{
    if (x >= m_width || y >= m_height) {
        throw std::runtime_error("Attempted to set pixel beyond bounds of surface.");
    }
    m_data[(y * m_width) + x] = color;
}

auto graphite::qd::surface::set(int offset, graphite::qd::color color) -> void
{
    if (offset >= m_data.size()) {
        throw std::runtime_error("Attempted to set pixel beyond bounds of surface.");
    }
    m_data[offset] = color;
}

// MARK: - Drawing Operations

auto graphite::qd::surface::draw_line(int x0, int y0, int x1, int y1, graphite::qd::color color) -> void
{
    int delta_x = abs(x1 - x0);
    int delta_y = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = delta_x - delta_y;

    for (;;) {
        if (x0 >= 0 && y0 >= 0 && x0 < m_width && y0 < m_height) {
            set(x0, y0, color);
        }
        if (x0 == x1 & y0 == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -delta_y) {
            err -= delta_y;
            x0 += sx;
        }
        if (e2 < delta_x) {
            err += delta_x;
            y0 += sy;
        }
    }
}
