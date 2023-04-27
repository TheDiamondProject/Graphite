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
#include <libSIMD/SIMD.hpp>

namespace simd
{
    struct float64_s;
    typedef struct float64_s float64;

    struct float64_s
    {
        static constexpr std::int32_t element_count = 2;

        // MARK: - Construction

        /**
         * Creates a 128-bit SIMD Vector from 3 single precision floats.
         * @param f0    Element 1
         * @param f1    Element 2
         */
        explicit float64_s(double f0 = 0.f, double f1 = 0.f);

        explicit float64_s(f64x2 v);

        float64_s(const float64_s&) = default;
        float64_s(float64_s&&) = default;
        auto operator=(const float64&) -> float64& = default;

        /**
         * Creates a 128-bit SIMD Vector using one single precision float for each of the elements.
         * @param v     Value to duplicate into all elements.
         */
        static auto constant(double v) -> float64;

        static auto lower_upper_merge(const float64& lower, const float64& upper) -> float64;
        static auto lower(const float64& lower) -> float64;
        static auto upper(const float64& upper) -> float64;

        // MARK: - Accessors
        auto operator[] (int i) const -> double;
        auto set(int i, double v) -> float64&;

        // MARK: - Operators
        auto operator+ (const float64& other) const -> float64;
        auto operator+ (double f) const -> float64;
        auto operator+=(const float64& other) -> float64&;
        auto operator+=(double f) -> float64&;

        auto operator- (const float64& other) const -> float64;
        auto operator- (double f) const -> float64;
        auto operator-=(const float64& other) -> float64&;
        auto operator-=(double f) -> float64&;

        auto operator* (const float64& other) const -> float64;
        auto operator* (double f) const -> float64;
        auto operator*=(const float64& other) -> float64&;
        auto operator*=(double f) -> float64&;

        auto operator/ (const float64& other) const -> float64;
        auto operator/ (double f) const -> float64;
        auto operator/=(const float64& other) -> float64&;
        auto operator/=(double f) -> float64&;

        [[nodiscard]] auto abs() const -> float64;
        [[nodiscard]] auto round() const -> float64;
        [[nodiscard]] auto floor() const -> float64;
        [[nodiscard]] auto ceil() const -> float64;
        [[nodiscard]] auto pow(double exp) const -> float64;
        [[nodiscard]] auto sqrt() const -> float64;
        [[nodiscard]] auto sin() const -> float64;
        [[nodiscard]] auto cos() const -> float64;
        [[nodiscard]] auto rcp() const -> float64;
        [[nodiscard]] auto min(const float64& other) const -> float64;
        [[nodiscard]] auto min(double f) const -> float64;
        [[nodiscard]] auto max(const float64& other) const -> float64;
        [[nodiscard]] auto max(double f) const -> float64;

        [[nodiscard]] auto upper() const -> float64;
        [[nodiscard]] auto lower() const -> float64;
        [[nodiscard]] auto swapped() const -> float64;
        [[nodiscard]] auto reversed() const -> float64;

        auto set_lower(const float64& lower) -> void;
        auto set_upper(const float64& upper) -> void;

        friend struct integer16_s;
        friend struct integer32_s;
        friend struct integer64_s;
        friend struct float64_s;
        friend struct float32_s;

    private:
        double __attribute__((__aligned__(16))) m_values[element_count] { 0 };

        [[nodiscard]] auto vector() const -> f64x2;
        auto set(f64x2 v) -> void;
    };
}