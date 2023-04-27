// Copyright (c) 2023 Tom Hancocks
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

namespace simd
{
    struct integer16_s;
    typedef struct integer16_s integer16;

    struct integer16_s
    {
        static constexpr std::int32_t element_count = 8;

        // MARK: - Construction

        /**
         * Creates a 128-bit SIMD Vector from 16 8-bit integers.
         * @param i0    Element 1
         * @param i1    Element 2
         * @param i2    Element 3
         * @param i3    Element 4
         * @param i4    Element 5
         * @param i5    Element 6
         * @param i6    Element 7
         * @param i7    Element 8
         * @param i8    Element 9
         * @param i9    Element 10
         * @param i10   Element 11
         * @param i11   Element 12
         * @param i12   Element 13
         * @param i13   Element 14
         * @param i14   Element 15
         * @param i16   Element 16
         */
        explicit integer16_s(
            std::int8_t i0 = 0, std::int8_t i1 = 0, std::int8_t i2 = 0, std::int8_t i3 = 0,
            std::int8_t i4 = 0, std::int8_t i5 = 0, std::int8_t i6 = 0, std::int8_t i7 = 0,
            std::int8_t i8 = 0, std::int8_t i9 = 0, std::int8_t i10 = 0, std::int8_t i11 = 0,
            std::int8_t i12 = 0, std::int8_t i13 = 0, std::int8_t i14 = 0, std::int8_t i15 = 0
        );

        /**
         * Creates a 128-bit SIMD Vector from 8 16-bit integers.
         * @param i0    Element 1
         * @param i1    Element 2
         * @param i2    Element 3
         * @param i3    Element 4
         * @param i4    Element 5
         * @param i5    Element 6
         * @param i6    Element 7
         * @param i7    Element 8
         */
        explicit integer16_s(
            std::int16_t i0 = 0, std::int16_t i1 = 0, std::int16_t i2 = 0, std::int16_t i3 = 0,
            std::int16_t i4 = 0, std::int16_t i5 = 0, std::int16_t i6 = 0, std::int16_t i7 = 0
        );

        /**
         * Creates a 128-bit SIMD Vector from 4 32-bit integers.
         * @param i0    Element 1
         * @param i1    Element 2
         * @param i2    Element 3
         * @param i3    Element 4
         */
        explicit integer16_s(std::int32_t i0 = 0, std::int32_t i1 = 0, std::int32_t i2 = 0, std::int32_t i3 = 0);

        /**
         * Creates a 128-bit SIMD Vector from 2 64-bit integers.
         * @param i0    Element 1
         * @param i1    Element 2
         */
        explicit integer16_s(std::int64_t i0 = 0, std::int64_t i1 = 0);

        explicit integer16_s(i16x8 v);

        integer16_s(const integer16&) = default;
        integer16_s(integer16&&) = default;
        auto operator=(const integer16&) -> integer16& = default;

        /**
         * Creates a 128-bit SIMD Vector using a single 8-bit integer for each of the elements.
         * @param v     Value to duplicate into all elements.
         */
        static auto constant(std::int16_t v) -> integer16_s;

        // MARK: - Accessors
        auto operator[] (int i) const -> std::int16_t;
        auto set(int i, std::int16_t v) -> integer16_s&;

        // MARK: - Operators
        auto operator+ (const integer16& other) const -> integer16_s;
        auto operator+ (std::int16_t i) const -> integer16;
        auto operator+=(const integer16& other) -> integer16&;
        auto operator+=(std::int16_t i) -> integer16&;

        auto operator- (const integer16& other) const -> integer16;
        auto operator- (std::int16_t i) const -> integer16;
        auto operator-=(const integer16& other) -> integer16&;
        auto operator-=(std::int16_t i) -> integer16&;

        auto operator* (const integer16& other) const -> integer16;
        auto operator* (std::int16_t i) const -> integer16;
        auto operator* (float f) const -> integer16;
        auto operator*=(const integer16& other) -> integer16&;
        auto operator*=(std::int16_t i) -> integer16&;
        auto operator*=(float f) -> integer16&;

        auto operator/ (const integer16& other) const -> integer16;
        auto operator/ (std::int16_t i) const -> integer16;
        auto operator/ (float f) const -> integer16;
        auto operator/=(const integer16& other) -> integer16&;
        auto operator/=(std::int16_t i) -> integer16&;
        auto operator/=(float f) -> integer16&;

        auto operator& (const integer16& other) const -> integer16;
        auto operator& (std::int16_t i) const -> integer16;
        auto operator&=(const integer16& other) -> integer16&;
        auto operator&=(std::int16_t i) -> integer16&;

        auto operator| (const integer16& other) const -> integer16;
        auto operator| (std::int16_t i) const -> integer16;
        auto operator|=(const integer16& other) -> integer16&;
        auto operator|=(std::int16_t i) -> integer16&;

        auto operator^ (const integer16& other) const -> integer16;
        auto operator^ (std::int16_t i) const -> integer16;
        auto operator^=(const integer16& other) -> integer16&;
        auto operator^=(std::int16_t i) -> integer16&;

        auto operator<<(const integer16& other) const -> integer16;
        auto operator<<(std::int16_t i) const -> integer16;
        auto operator<<=(const integer16& other) -> integer16&;
        auto operator<<=(std::int16_t i) -> integer16&;

        auto operator>>(const integer16& other) const -> integer16;
        auto operator>>(std::int16_t i) const -> integer16;
        auto operator>>=(const integer16& other) -> integer16&;
        auto operator>>=(std::int16_t i) -> integer16&;

        auto operator~() const -> integer16;

        [[nodiscard]] auto abs() const -> integer16;

        friend struct integer8_s;
        friend struct integer32_s;
        friend struct integer64_s;
        friend struct float_s;
        friend struct double_s;

    private:
        std::int16_t __attribute__((__aligned__(16))) m_values[element_count];

        [[nodiscard]] auto vector() const -> i16x8;
        auto set(i16x8 v) -> void;
    };
}