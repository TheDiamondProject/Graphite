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

#pragma once

#include <cstdint>
#include <type_traits>
#include <libData/encoding.hpp>
#include <libQuickdraw/type/coding_type.hpp>
#include <libData/reader.hpp>
#include <libData/writer.hpp>

namespace quickdraw
{
    /**
     * Represents a 2D point on a surface.
     * @tparam T    The type being used for the X & Y values of the point. This can be any arithmetic type.
     */
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    struct point
    {
    public:
        /**
         * The x-coordinate of the point.
         */
        T x { 0 };

        /**
         * The y-coordinate of the point.
         */
        T y { 0 };

        point() = default;

        /**
         * Construct a point where the X and Y coordinate are equal.
         * @param v     The value to set both of the X and Y coordinates to.
         */
        explicit point(T v) : x(v), y(v) {}

        /**
         * Construct a point.
         * @param x     The value of the X coordinate.
         * @param y     The value of the Y coordinate.
         */
        point(T x, T y) : x(x), y(y) {}

        point(const point&) = default;
        point(point&&) noexcept = default;

        /**
         * Construct a point, reading the X and Y coordinates from the specified data reader.
         * Defaults to using the QuickDraw ordering for coordinates (Y and then X)
         * @param reader    The data reader to read the X and Y coordinates from.
         * @param type      The coding type (coordinate ordering) to use when reading the coordinates.
         */
        explicit point(data::reader& reader, coding_type type = coding_type::quickdraw)
        {
            switch (type) {
                case coding_type::macintosh: {
                    x = read_component(reader);
                    y = read_component(reader);
                    break;
                }
                case coding_type::quickdraw: {
                    y = read_component(reader);
                    x = read_component(reader);
                    break;
                }
            }
        }

        /**
         * Construct a point, reading the X and Y coordinates from the specified data reader.
         * @param reader    The data reader to read the X and Y coordinates from.
         * @param type      The coding type (coordinate ordering) to use when reading the coordinates.
         * @return          A point with the X and Y coordinate read from the reader.
         */
        static auto read(data::reader& reader, coding_type type) -> point
        {
            return point(reader, type);
        }

        /**
         * Write the X and Y coordinates of the point into the provided data writer, using the specified coding type.
         * @param writer    The data writer to write the X and Y coordinates to.
         * @param type      The coding type (coordinate ordering) to use when writing the coordinates.
         */
        auto encode(data::writer& writer, coding_type type = coding_type::quickdraw) -> void
        {
            switch (type) {
                case coding_type::macintosh: {
                    write_component(x, writer);
                    write_component(y, writer);
                    break;
                }
                case coding_type::quickdraw: {
                    write_component(y, writer);
                    write_component(x, writer);
                    break;
                }
            }
        }

        auto operator=(const point& p) -> point& { x = p.x; y = p.y; return *this; }
        auto operator=(point&& p) noexcept -> point& { x = p.x; y = p.y; return *this; }

        auto operator==(const point& p) const -> bool { return x == p.x && y == p.y; }
        auto operator!=(const point& p) const -> bool { return x != p.x && y != p.y; }

        auto operator+(const point& p) const -> point { return point(x + p.x, y + p.y); }
        auto operator-(const point& p) const -> point { return point(x - p.x, y - p.y); }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
        auto operator*(U v) const -> point { return point(x * static_cast<T>(v), y * static_cast<T>(v)); }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
        auto operator/(U v) const -> point { return point(x / static_cast<T>(v), y / static_cast<T>(v)); }

        /**
         * Cast the typename of the point to a different compatible type.
         * @tparam U    The new arithmetic type in which to cast to.
         * @return      A point using the new arithmetic type.
         */
        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
        auto cast() const -> point<U> { return { static_cast<U>(x), static_cast<U>(y) }; }

    private:
        static auto read_component(data::reader& reader) -> T { return reader.read_integer<T>(); }
        static auto write_component(T value, data::writer& writer) -> void { writer.write_integer<T>(value); }
    };


    template<>
    inline auto point<double>::read_component(data::reader &reader) -> double { return reader.read_fixed_point(); }

    template<>
    inline auto point<float>::read_component(data::reader &reader) -> float { return static_cast<float>(reader.read_fixed_point()); }

    template<>
    inline auto point<double>::write_component(double value, data::writer &writer) -> void { writer.write_fixed_point(value); }

    template<>
    inline auto point<float>::write_component(float value, data::writer &writer) -> void { writer.write_fixed_point(static_cast<double>(value)); }

}
