//
// Created by Tom Hancocks on 20/02/2020.
//

#include "libGraphite/quickdraw/internal/color.hpp"

// MARK: - Constructor

graphite::qd::color::color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    : m_red(red), m_blue(blue), m_green(green), m_alpha(alpha)
{

}

graphite::qd::color::color(uint16_t rgb555)
    : m_red(static_cast<uint8_t>((rgb555 & 0x7c00) >> 7)),
      m_green(static_cast<uint8_t>((rgb555 & 0x03e0) >> 2)),
      m_blue(static_cast<uint8_t>((rgb555 & 0x001f) << 3)),
      m_alpha(255)
{
    // Copy the upper 3 bits to the empty lower 3 bits
    m_red |= m_red >> 5;
    m_green |= m_green >> 5;
    m_blue |= m_blue >> 5;
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

auto graphite::qd::color::rgb555() const -> uint16_t
{
    return (m_red >> 3) << 10 | (m_green >> 3) << 5 | (m_blue >> 3);
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

auto graphite::qd::color::operator==(const graphite::qd::color &rhs) const -> bool
{
    return (m_red == rhs.m_red) && (m_green == rhs.m_green) && (m_blue == rhs.m_blue) && (m_alpha == rhs.m_alpha);
}
