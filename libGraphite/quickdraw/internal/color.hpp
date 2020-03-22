//
// Created by Tom Hancocks on 20/02/2020.
//

#ifndef GRAPHITE_COLOR_HPP
#define GRAPHITE_COLOR_HPP

#include <cstdint>

namespace graphite { namespace qd {

    /**
     * A structure representing a 4-component color, as used by the internal quickdraw surface.
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

        static color black();
        static color white();
        static color red();
        static color green();
        static color blue();
        static color purple();
        static color orange();
        static color yellow();
        static color lightGrey();
        static color darkGrey();


    };

}}


#endif //GRAPHITE_COLOR_HPP
