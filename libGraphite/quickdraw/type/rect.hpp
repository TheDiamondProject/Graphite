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
#include "libGraphite/data/reader.hpp"
#include "libGraphite/data/writer.hpp"
#include "libGraphite/quickdraw/type/coding_type.hpp"
#include "libGraphite/quickdraw/type/point.hpp"
#include "libGraphite/quickdraw/type/size.hpp"

namespace graphite::quickdraw
{
    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    struct rect
    {
    public:
        point<T> origin { 0 };
        size<T> size { 0 };

        rect() = default;
        explicit rect(T v) : origin(v), size(v) {}
        rect(struct point<T> origin, struct quickdraw::size<T> size) : origin(origin), size(size) {}
        rect(const rect&) = default;
        rect(rect&&) noexcept = default;

        explicit rect(data::reader& reader) : origin(reader), size(reader) {}
        rect(data::reader& reader, coding_type type) : origin(reader, type), size(reader, type) {}

        static auto read(data::reader& reader, coding_type type) -> rect { return { reader, type }; }

        auto encode(data::writer& writer) -> void
        {
            encode(writer, coding_type::quickdraw);
        }

        auto encode(data::writer& writer, coding_type type) -> void
        {
            origin.encode(writer, type);
            size.encode(writer, type);
        }

        auto operator=(const rect& r) -> rect& { origin = r.origin; size = r.size; return *this; }
        auto operator=(rect&& r) noexcept -> rect& { origin = r.origin; size = r.size; return *this; }

        auto operator==(const rect& r) const -> bool { return origin == r.origin && size == r.size; }
        auto operator!=(const rect& r) const -> bool { return origin != r.origin && size != r.size; }

        auto operator+(const rect& r) const -> rect { return { origin + r.origin, size + r.size }; }
        auto operator-(const rect& r) const -> rect { return { origin - r.origin, size - r.size }; }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>>
        auto operator*(U v) const -> rect { return { origin * static_cast<T>(v), size * static_cast<T>(v) }; }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>>
        auto operator/(U v) const -> rect { return { origin / static_cast<T>(v), size / static_cast<T>(v) }; }

        template<typename U, typename std::enable_if<std::is_convertible<T, U>::value>>
        auto cast() const -> rect<U> { return { origin.cast<U>(), size.cast<U>() }; }
    };

}