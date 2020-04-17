//
// Created by Tom Hancocks on 20/02/2020.
//

#include "libGraphite/quickdraw/internal/color.hpp"

// MARK: - Constructor

graphite::qd::color::color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    : m_red(red), m_blue(blue), m_green(green), m_alpha(alpha)
{

}

// MARK: - Accessors

auto graphite::qd::color::red_component() const -> uint8_t
{
    return m_red;
}

auto graphite::qd::color::green_component() const -> uint8_t
{
    return m_green;
}

auto graphite::qd::color::blue_component() const -> uint8_t
{
    return m_blue;
}

auto graphite::qd::color::alpha_component() const -> uint8_t
{
    return m_alpha;
}


// MARK: - Quick Colors

auto graphite::qd::color::black() -> graphite::qd::color
{
    return graphite::qd::color(0, 0, 0);
}

auto graphite::qd::color::white() -> graphite::qd::color
{
    return graphite::qd::color(255, 255, 255);
}

auto graphite::qd::color::red() -> graphite::qd::color
{
    return graphite::qd::color(255, 0, 0);
}

auto graphite::qd::color::green() -> graphite::qd::color
{
    return graphite::qd::color(0, 255, 0);
}

auto graphite::qd::color::blue() -> graphite::qd::color
{
    return graphite::qd::color(0, 0, 255);
}

auto graphite::qd::color::purple() -> graphite::qd::color
{
    return graphite::qd::color(150, 0, 255);
}

auto graphite::qd::color::orange() -> graphite::qd::color
{
    return graphite::qd::color(255, 150, 0);
}

auto graphite::qd::color::yellow() -> graphite::qd::color
{
    return graphite::qd::color(255, 255, 0);
}

auto graphite::qd::color::lightGrey() -> graphite::qd::color
{
    return graphite::qd::color(200, 200, 200);
}

auto graphite::qd::color::darkGrey() -> graphite::qd::color
{
    return graphite::qd::color(100, 100, 100);
}

auto graphite::qd::color::clear() -> graphite::qd::color
{
    return graphite::qd::color(0, 0, 0, 0);
}

// MARK: - Operators

auto graphite::qd::color::operator==(const graphite::qd::color &rhs) -> bool
{
    return (m_red == rhs.m_red) && (m_green == rhs.m_green) && (m_blue == rhs.m_blue) && (m_alpha == rhs.m_alpha);
}
