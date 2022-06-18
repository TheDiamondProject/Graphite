//
// Created by Tom Hancocks on 20/02/2020.
//

#ifndef GRAPHITE_COLOR_HPP
#define GRAPHITE_COLOR_HPP

#include <cstdint>

namespace graphite::qd {

    /**
     * A structure representing a 4-component color, as used by the internal QuickDraw surface.
     */
    struct color
    {
    private:
        uint8_t m_red;
        uint8_t m_green;
        uint8_t m_blue;
        uint8_t m_alpha;

    public:

        /**
         * Construct a new color using the specified color component values.
         * @param red       The red component of the color
         * @param green     The green component of the color
         * @param blue      The blue component of the color
         * @param alpha     The alpha component of the color
         */
        color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);

        [[nodiscard]] auto red_component() const -> uint8_t;
        [[nodiscard]] auto green_component() const -> uint8_t;
        [[nodiscard]] auto blue_component() const -> uint8_t;
        [[nodiscard]] auto alpha_component() const -> uint8_t;
        
        static auto black() -> color;
        static auto white() -> color;
        static auto red() -> color;
        static auto green() -> color;
        static auto blue() -> color;
        static auto purple() -> color;
        static auto orange() -> color;
        static auto yellow() -> color;
        static auto lightGrey() -> color;
        static auto darkGrey() -> color;
        static auto clear() -> color;

        auto operator== (const color& rhs) const -> bool;
    };

}


#endif //GRAPHITE_COLOR_HPP
