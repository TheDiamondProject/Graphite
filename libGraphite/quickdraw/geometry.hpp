//
// Created by Tom Hancocks on 19/03/2020.
//

#if !defined(GRAPHITE_GEOMETRY_HPP)
#define GRAPHITE_GEOMETRY_HPP

namespace graphite { namespace qd {

    struct point
    {
    public:
        short v;
        short h;

        point(short v, short h) : v(v), h(h) {};
    };

    struct rect
    {
    public:
        short top;
        short left;
        short bottom;
        short right;

        rect(short top, short left, short bottom, short right) : top(top), left(left), bottom(bottom), right(right) {};

        short width() { return right - left; };
        short height() { return bottom - top; };
    };

    struct fixed_point
    {
    public:
        double x;
        double y;

        fixed_point(double x, double y) : x(x), y(y) {};
    };

    struct fixed_rect
    {
    public:
        double left;
        double top;
        double right;
        double bottom;

        fixed_rect(double left, double top, double right, double bottom) : left(left), top(top), right(right), bottom(bottom) {};

        double width() { return right - left; };
        double height() { return bottom - top; };
    };

}};

#endif //GRAPHITE_GEOMETRY_HPP
