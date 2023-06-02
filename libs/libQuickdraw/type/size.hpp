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

namespace quickdraw
{
    /**
     * Represents a size of a 2D area on a surface.
     * @tparam T    The type being used for the Width and Height values of a size.
     *              This can be any arithmetic type.
     */
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    struct size
    {
    public:
        /**
         * The width of the size.
         */
        T width { 0 };

        /**
         * The height of the size.
         */
        T height { 0 };

        size() = default;

        /**
         * Construct a size where the width and height dimensions are equal.
         * @param v     The value to set both of the width and height dimensions to.
         */
        explicit size(T v) : width(v), height(v) {}

        /**
         * Construct a size.
         * @param width     The value of the width dimension.
         * @param height    The value of the height dimension.
         */
        size(T width, T height) : width(width), height(height) {}

        size(const size&) = default;
        size(size&&) noexcept = default;

        /**
         * Construct a size, reading the width and height dimensions from the specified data reader.
         * Defaults to using the QuickDraw ordering for dimensions (height and then width)
         * @param reader    The data reader to read the width and height dimensions from.
         * @param type      The coding type (dimension ordering) to use when reading the dimensions.
         */
        explicit size(data::reader& reader, coding_type type = coding_type::quickdraw)
        {
            switch (type) {
                case coding_type::macintosh: {
                    width = read_component(reader);
                    height = read_component(reader);
                    break;
                }
                case coding_type::quickdraw: {
                    height = read_component(reader);
                    width = read_component(reader);
                    break;
                }
            }
        }

        /**
         * Construct a size, reading the width and height dimensions from the specified data reader.
         * @param reader    The data reader to read the width and height dimensions from.
         * @param type      The coding type (dimension ordering) to use when reading the dimensions.
         * @return          A size with the width and height dimensions read from the reader.
         */
        static auto read(data::reader& reader, coding_type type) -> size
        {
            return size(reader, type);
        }

        /**
         * Write the width and height coordinates of the size into the provided data writer, using the specified coding
         * type.
         * @param writer    The data writer to write the width and height dimensions to.
         * @param type      The coding type (dimension ordering) to use when writing the dimensions.
         */
        auto encode(data::writer& writer, coding_type type = coding_type::quickdraw) -> void
        {
            switch (type) {
                case coding_type::macintosh: {
                    write_component(width, writer);
                    write_component(height, writer);
                    break;
                }
                case coding_type::quickdraw: {
                    write_component(height, writer);
                    write_component(width, writer);
                    break;
                }
            }
        }

        auto operator=(const size& s) -> size& { width = s.width; height = s.height; return *this; }
        auto operator=(size&& s) noexcept -> size& { width = s.width; height = s.height; return *this; }

        auto operator==(const size& s) const -> bool { return width == s.width && height == s.height; }
        auto operator!=(const size& s) const -> bool { return width != s.width && height != s.height; }

        auto operator+(const size& s) const -> size { return size(width + s.width, height + s.height); }
        auto operator-(const size& s) const -> size { return size(width - s.width, height - s.height); }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
        auto operator*(U v) const -> size { return size(width * static_cast<T>(v), height * static_cast<T>(v)); }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
        auto operator/(U v) const -> size { return size(width / static_cast<T>(v), height / static_cast<T>(v)); }

        /**
         * Cast the typename of the size to a different compatible type.
         * @tparam U    The new arithmetic type in which to cast to.
         * @return      A size using the new arithmetic type.
         */
        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>::type* = nullptr>
        auto cast() const -> size<U> { return size<U>(static_cast<U>(width), static_cast<U>(height)); }

    private:
        static auto read_component(data::reader& reader) -> T { return reader.read_integer<T>(); }
        static auto write_component(T value, data::writer& writer) -> void { writer.write_integer<T>(value); }
    };

    template<>
    inline auto size<double>::read_component(data::reader &reader) -> double { return reader.read_fixed_point(); }

    template<>
    inline auto size<float>::read_component(data::reader &reader) -> float { return static_cast<float>(reader.read_fixed_point()); }

    template<>
    inline auto size<double>::write_component(double value, data::writer &writer) -> void { writer.write_fixed_point(value); }

    template<>
    inline auto size<float>::write_component(float value, data::writer &writer) -> void { writer.write_fixed_point(static_cast<double>(value)); }

}