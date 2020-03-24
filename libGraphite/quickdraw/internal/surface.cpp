//
// Created by Tom Hancocks on 20/02/2020.
//

#include "libGraphite/quickdraw/internal/surface.hpp"

// MARK: - Constructor

graphite::qd::surface::surface(int width, int height)
    : m_width(width), m_height(height), m_data(width * height, graphite::qd::color::black())
{
}

graphite::qd::surface::surface(int width, int height, std::vector<graphite::qd::color> rgb)
: m_width(width), m_height(height), m_data(rgb)
{
}

// MARK: - Surface Access

std::vector<uint32_t> graphite::qd::surface::raw() const
{
    auto out = std::vector<uint32_t>();
    for (auto i = m_data.begin(); i != m_data.end(); ++i) {
        uint32_t color = ((*i).alpha_component() << 24)
                | ((*i).red_component() << 16)
                | ((*i).green_component() << 8)
                | (*i).blue_component();
        out.push_back(color);
    }
    return out;
}

graphite::qd::size graphite::qd::surface::size() const
{
    return graphite::qd::size(m_width, m_height);
}

graphite::qd::color graphite::qd::surface::at(int x, int y) const
{
    return m_data[(y * m_width) + x];
}

void graphite::qd::surface::set(int x, int y, graphite::qd::color color)
{
    m_data[(y * m_width) + x] = color;
}

// MARK: - Drawing Operations

void graphite::qd::surface::draw_line(int x0, int y0, int x1, int y1, graphite::qd::color color)
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
