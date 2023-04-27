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
#include <libSIMD/arm/arm_neon.hpp>
#include <libSIMD/intel/intel_sse.hpp>

namespace simd
{
    struct float32_s;
    typedef struct float32_s float32;

    struct float32_s
    {
        // MARK: - Construction

        explicit inline float32_s(float f0 = 0.f, float f1 = 0.f, float f2 = 0.f, float f3 = 0.f)
        {
            m_values = vector(f0, f1, f2, f3);
        }

        explicit inline float32_s(f32x4 v)
            : m_values(v)
        {}

        inline float32_s(const float32&) = default;
        inline float32_s(float32&&) = default;
        auto operator=(const float32&) -> float32& = default;

        static inline auto constant(float v) -> float32
        {
            return float32(single_value_vector(v));
        }

        static inline auto lower_upper_merge(const float32& lower, const float32& upper) -> float32
        {
            return float32(vector_shuffle_lower_higher(lower.m_values, upper.m_values));
        }

        static inline auto lower(const float32& lower) -> float32 { return float32(vector_slice_lower(lower.m_values)); }
        static inline auto upper(const float32& upper) -> float32 { return float32(vector_slice_upper(upper.m_values)); }

        // MARK: - Accessors
        inline auto operator[] (int i) const -> float { return m_values[i]; };
        inline auto set(int i, float v) -> float32& { m_values[i] = v; return *this; }

        // MARK: - Operators
        inline auto operator+ (const float32& other) const -> float32 { return float32(simd::add(m_values, other.m_values)); }
        inline auto operator+ (float f) const -> float32 { return float32(simd::add(m_values, simd::single_value_vector(f))); }
        inline auto operator+=(const float32& other) -> float32& { m_values = simd::add(m_values, other.m_values); return *this; }
        inline auto operator+=(float f) -> float32& { m_values = simd::add(m_values, simd::single_value_vector(f)); return *this; }

        inline auto operator- (const float32& other) const -> float32 { return float32(simd::sub(m_values, other.m_values)); }
        inline auto operator- (float f) const -> float32 { return float32(simd::sub(m_values, simd::single_value_vector(f))); }
        inline auto operator-=(const float32& other) -> float32& { m_values = simd::sub(m_values, other.m_values); return *this; }
        inline auto operator-=(float f) -> float32& { m_values = simd::sub(m_values, simd::single_value_vector(f)); return *this; }

        inline auto operator* (const float32& other) const -> float32 { return float32(simd::mul(m_values, other.m_values)); }
        inline auto operator* (float f) const -> float32 { return float32(simd::mul(m_values, simd::single_value_vector(f))); }
        inline auto operator*=(const float32& other) -> float32& { m_values = simd::mul(m_values, other.m_values); return *this; }
        inline auto operator*=(float f) -> float32& { m_values = simd::mul(m_values, simd::single_value_vector(f)); return *this; }

        inline auto operator/ (const float32& other) const -> float32 { return float32(simd::div(m_values, other.m_values)); }
        inline auto operator/ (float f) const -> float32 { return float32(simd::div(m_values, simd::single_value_vector(f))); }
        inline auto operator/=(const float32& other) -> float32& { m_values =simd::div(m_values, other.m_values); return *this; }
        inline auto operator/=(float f) -> float32& { m_values = simd::div(m_values, simd::single_value_vector(f)); return *this; }

        [[nodiscard]] inline auto abs() const -> float32 { return float32(simd::abs(m_values)); }
        [[nodiscard]] inline auto round() const -> float32 { return float32(simd::round(m_values)); }
        [[nodiscard]] inline auto floor() const -> float32 { return float32(simd::floor(m_values)); }
        [[nodiscard]] inline auto ceil() const -> float32 { return float32(simd::ceil(m_values)); }
        [[nodiscard]] inline auto pow(float exp) const -> float32 { return float32(simd::pow(m_values, exp)); }
        [[nodiscard]] inline auto sqrt() const -> float32 { return float32(simd::sqrt(m_values)); }
        [[nodiscard]] inline auto rsqrt() const -> float32 { return float32(simd::rsqrt(m_values)); }
        [[nodiscard]] inline auto sin() const -> float32 { return float32(simd::sin(m_values)); }
        [[nodiscard]] inline auto cos() const -> float32 { return float32(simd::cos(m_values)); }
        [[nodiscard]] inline auto rcp() const -> float32 { return float32(simd::rcp(m_values)); }
        [[nodiscard]] inline auto min(const float32& other) const -> float32 { return float32(simd::min(m_values, other.m_values)); }
        [[nodiscard]] inline auto min(float f) const -> float32 { return float32(simd::min(m_values, simd::single_value_vector(f))); }
        [[nodiscard]] inline auto max(const float32& other) const -> float32 { return float32(simd::max(m_values, other.m_values)); }
        [[nodiscard]] inline auto max(float f) const -> float32 { return float32(simd::max(m_values, simd::single_value_vector(f))); }

        [[nodiscard]] inline auto upper() const -> float32 { return float32(simd::vector_slice_upper(m_values)); }
        [[nodiscard]] inline auto lower() const -> float32 { return float32(simd::vector_slice_lower(m_values)); }
        [[nodiscard]] inline auto swapped() const -> float32 { return float32(simd::swap_lower_upper(m_values)); }
        [[nodiscard]] inline auto reversed() const -> float32 { return float32(simd::reverse(m_values)); }

        inline auto set_lower(const float32& lower) -> void
        {
            m_values[0] = lower[0]; m_values[1] = lower[1];
        }

        inline auto set_upper(const float32& upper) -> void
        {
            m_values[2] = upper[2]; m_values[3] = upper[3];
        }

        friend struct integer16_s;
        friend struct integer32_s;
        friend struct integer64_s;
        friend struct float32_s;
        friend struct double64_s;

    private:
        f32x4 m_values;
    };
}