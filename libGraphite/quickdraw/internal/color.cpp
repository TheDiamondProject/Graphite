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

uint8_t graphite::qd::color::red_component() const
{
    return m_red;
}

uint8_t graphite::qd::color::green_component() const
{
    return m_green;
}

uint8_t graphite::qd::color::blue_component() const
{
    return m_blue;
}

uint8_t graphite::qd::color::alpha_component() const
{
    return m_alpha;
}


// MARK: - Quick Colors

graphite::qd::color graphite::qd::color::black()
{
    return graphite::qd::color(0, 0, 0);
}

graphite::qd::color graphite::qd::color::white()
{
    return graphite::qd::color(255, 255, 255);
}

graphite::qd::color graphite::qd::color::red()
{
    return graphite::qd::color(255, 0, 0);
}

graphite::qd::color graphite::qd::color::green()
{
    return graphite::qd::color(0, 255, 0);
}

graphite::qd::color graphite::qd::color::blue()
{
    return graphite::qd::color(0, 0, 255);
}

graphite::qd::color graphite::qd::color::purple()
{
    return graphite::qd::color(150, 0, 255);
}

graphite::qd::color graphite::qd::color::orange()
{
    return graphite::qd::color(255, 150, 0);
}

graphite::qd::color graphite::qd::color::yellow()
{
    return graphite::qd::color(255, 255, 0);
}

graphite::qd::color graphite::qd::color::lightGrey()
{
    return graphite::qd::color(200, 200, 200);
}

graphite::qd::color graphite::qd::color::darkGrey()
{
    return graphite::qd::color(100, 100, 100);
}
