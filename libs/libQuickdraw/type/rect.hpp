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
#include <libData/reader.hpp>
#include <libData/writer.hpp>
#include <libQuickdraw/type/coding_type.hpp>
#include <libQuickdraw/type/point.hpp>
#include <libQuickdraw/type/size.hpp>

namespace quickdraw
{
    /**
     * Represents a rectangle or frame within a surface.
     * @tparam T    The type being used for the origin and size values of the rect.
     *              This can be any arithmetic type.
     */
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    struct rect
    {
    public:
        /**
         * The origin of the rect.
         */
        point<T> origin { 0 };

        /**
         * The size of the rect
         */
        size<T> size { 0 };

        rect() = default;

        /**
         * Construct a rect where the X and Y coordinates of the origin, as well as the width and height dimensions
         * of the size, are all equal.
         * @param v     The value to set both of the origin and size components to.
         */
        explicit rect(T v) : origin(v), size(v) {}

        /**
         * Construct a rect.
         * @param x             The value of the origin X coordinate.
         * @param y             The value of the origin Y coordinate.
         * @param width         The value of the width dimension of the size.
         * @param height        The value of the height dimension of the size.
         */
        rect(T x, T y, T width, T height) : origin(x, y), size(width, height) {}

        /**
         * Construct a rect.
         * @param origin        The origin of the rect.
         * @param size          The size of the rect.
         */
        rect(struct point<T> origin, struct quickdraw::size<T> size) : origin(origin), size(size) {}

        rect(const rect&) = default;
        rect(rect&&) noexcept = default;

        /**
         * Construct a point, reading the origin and the size from the specified data reader.
         * Defaults to using the QuickDraw ordering for both the origin and size.
         * @param reader     The data reader to read the origin and size from.
         * @param type       The coding type (ordering) to use when reading the origin and size.
         */
        explicit rect(data::reader& reader, coding_type type = coding_type::quickdraw)
            : origin(reader, type), size(reader, type)
        {
            size.width -= origin.x;
            size.height -= origin.y;
        }

        /**
         * Construct a rect, reading the origin and size from the specified data reader.
         * @param reader    The data reader to read the origin and size from.
         * @param type      The coding type (ordering) to use when reading the origin and size.
         * @return          A size with the origin and size read from the reader.
         */
        static auto read(data::reader& reader, coding_type type) -> rect
        {
            return rect(reader, type);
        }

        /**
         * Write the origin and size of the rect into the provided data writer, using the specified coding type.
         * @param writer    The data writer to write the origin and size to.
         * @param type      The coding type (ordering) to use when writing the origin and size.
         */
        auto encode(data::writer& writer, coding_type type = coding_type::quickdraw) -> void
        {
            origin.encode(writer, type);
            (size + quickdraw::size<T>(origin.x, origin.y)).encode(writer, type);
        }

        auto operator=(const rect& r) -> rect& { origin = r.origin; size = r.size; return *this; }
        auto operator=(rect&& r) noexcept -> rect& { origin = r.origin; size = r.size; return *this; }

        auto operator==(const rect& r) const -> bool { return origin == r.origin && size == r.size; }
        auto operator!=(const rect& r) const -> bool { return origin != r.origin && size != r.size; }

        auto operator+(const rect& r) const -> rect { return rect(origin + r.origin, size + r.size); }
        auto operator-(const rect& r) const -> rect { return rect(origin - r.origin, size - r.size); }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
        auto operator*(U v) const -> rect { return rect(origin * static_cast<T>(v), size * static_cast<T>(v)); }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
        auto operator/(U v) const -> rect { return rect(origin / static_cast<T>(v), size / static_cast<T>(v)); }

        /**
         * Cast the typename of the rect to a different compatible type.
         * @tparam U    The new arithmetic type in which to cast to.
         * @return      A rect using the new arithmetic type.
         */
        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
        auto cast() const -> rect<U> { return rect<U>(origin.template cast<U>(), size.template cast<U>()); }
    };

}