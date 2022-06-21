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
#include "libGraphite/data/encoding.hpp"
#include "libGraphite/quickdraw/type/coding_type.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/data/writer.hpp"

namespace graphite::quickdraw
{
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    struct point
    {
    public:
        T x { 0 };
        T y { 0 };

        point() = default;
        explicit point(T v) : x(v), y(v) {}
        point(T x, T y) : x(x), y(y) {}
        point(const point&) = default;
        point(point&&) noexcept = default;

        explicit point(data::reader& reader)
        {
            y = read_component(reader);
            x = read_component(reader);
        }

        point(data::reader& reader, coding_type type)
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

        static auto read(data::reader& reader, coding_type type) -> point { return { reader, type }; }

        auto encode(data::writer& writer) -> void
        {
            encode(writer, coding_type::quickdraw);
        }

        auto encode(data::writer& writer, coding_type type) -> void
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

        auto operator+(const point& p) const -> point { return { x + p.x, y + p.y }; }
        auto operator-(const point& p) const -> point { return { x - p.x, y - p.y }; }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>>
        auto operator*(U v) const -> point { return { x * static_cast<T>(v), y * static_cast<T>(v) }; }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>>
        auto operator/(U v) const -> point { return { x / static_cast<T>(v), y / static_cast<T>(v) }; }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>>
        auto cast() const -> point<U> { return { static_cast<U>(x), static_cast<U>(y) }; }

    private:
        auto read_component(data::reader& reader) -> T { return reader.read_integer<T>(); }
        auto write_component(T value, data::writer& writer) -> void { writer.write_integer<T>(value); }
    };

    template<>
    auto point<double>::read_component(data::reader &reader) -> double { return reader.read_fixed_point(); }

    template<>
    auto point<float>::read_component(data::reader &reader) -> float { return static_cast<float>(reader.read_fixed_point()); }

    template<>
    auto point<double>::write_component(double value, data::writer &writer) -> void { writer.write_fixed_point(value); }

    template<>
    auto point<float>::write_component(float value, data::writer &writer) -> void { writer.write_fixed_point(static_cast<double>(value)); }

}