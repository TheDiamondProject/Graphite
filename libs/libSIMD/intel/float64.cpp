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

#include <libSIMD/float64.hpp>
#include <libSIMD/SIMD.hpp>

#if INTEL_SIMD
#include <cassert>
#include <cmath>
#include <libSIMD/cpu.hpp>

// MARK: - Construction

simd::float64_s::float64_s(double f0, double f1)
{
    m_values[0] = f0;
    m_values[1] = f1;
}

simd::float64_s::float64_s(f64x2 v)
{
    _mm_store_pd(&m_values[0], v);
}

auto simd::float64_s::constant(double v) -> float64
{
    return float64(_mm_set1_ps(v));
}

auto simd::float64_s::lower_upper_merge(const float64 &lower, const float64 &upper) -> float64
{
    return float64(lower[0], upper[1]);
}

auto simd::float64_s::lower(const float64& lower) -> float64
{
    return float64(lower[0], lower[0]);
}

auto simd::float64_s::upper(const float64& upper) -> float64
{
    return float64(upper[1], upper[1]);
}

// MARK: - Accessors

auto simd::float64_s::operator[](int i) const -> double
{
    assert(i >= 0 && i < element_count);
    return m_values[element_count - 1 - i];
}

auto simd::float64_s::set(int i, double f) -> float64&
{
    assert(i >= 0 && i < element_count);
    m_values[element_count - 1 - i] = f;
    return *this;
}

auto simd::float64_s::vector() const -> f64x2
{
    return _mm_loadu_pd(&m_values[0]);
}

auto simd::float64_s::set(f64x2 v) -> void
{
    _mm_storeu_pd(&m_values[0], v);
}

// MARK: - Operators

auto simd::float64_s::operator+ (const float64& other) const -> float64
{
    return float64(_mm_add_pd(vector(), other.vector()));
}

auto simd::float64_s::operator+ (double f) const -> float64
{
    return *this + float64::constant(f);
}

auto simd::float64_s::operator+=(const float64& other) -> float64&
{
    set((*this + other).vector());
    return *this;
}

auto simd::float64_s::operator+=(double f) -> float64&
{
    set((*this + float64::constant(f)).vector());
    return *this;
}

auto simd::float64_s::operator- (const float64& other) const -> float64
{
    return float64(_mm_sub_pd(vector(), other.vector()));
}

auto simd::float64_s::operator- (double f) const -> float64
{
    return *this - float64::constant(f);
}

auto simd::float64_s::operator-=(const float64& other) -> float64&
{
    set((*this - other).vector());
    return *this;
}

auto simd::float64_s::operator-=(double f) -> float64&
{
    set((*this - float64::constant(f)).vector());
    return *this;
}

auto simd::float64_s::operator* (const float64& other) const -> float64
{
    return float64(_mm_mul_pd(vector(), other.vector()));
}

auto simd::float64_s::operator* (double f) const -> float64
{
    return *this * float64::constant(f);
}

auto simd::float64_s::operator*=(const float64& other) -> float64&
{
    set((*this * other).vector());
    return *this;
}

auto simd::float64_s::operator*=(double f) -> float64&
{
    set((*this * float64::constant(f)).vector());
    return *this;
}

auto simd::float64_s::operator/ (const float64& other) const -> float64
{
    return float64(_mm_div_pd(vector(), other.vector()));
}

auto simd::float64_s::operator/ (double f) const -> float64
{
    return *this / float64::constant(f);
}

auto simd::float64_s::operator/=(const float64& other) -> float64&
{
    set((*this / other).vector());
    return *this;
}

auto simd::float64_s::operator/=(double f) -> float64&
{
    set((*this / float64::constant(f)).vector());
    return *this;
}

// MARK: - Operations

auto simd::float64_s::abs() const -> float64
{
    // TODO: Implement this
    return *this;
}

auto simd::float64_s::round() const -> float64
{
    return float64(_mm_round_pd(vector(), _MM_ROUND_NEAREST));
}

auto simd::float64_s::floor() const -> float64
{
    return float64(_mm_floor_pd(vector()));
}

auto simd::float64_s::ceil() const -> float64
{
    return float64(_mm_ceil_pd(vector()));
}

auto simd::float64_s::pow(double exp) const -> float64
{
    if (exp == 2.0) {
        auto v = vector();
        return float64(_mm_mul_pd(v, v));
    }
    else {
        // TODO: Find an implementation that uses SSE/AVX in the future
        double v[element_count];
        for (auto n = 0; n < element_count; ++n) {
            v[n] = std::pow(m_values[n], m_values[n]);
        }
        return float64(v[0], v[1]);
    }
}

auto simd::float64_s::sqrt() const -> float64
{
    return float64(_mm_sqrt_pd(vector()));
}

auto simd::float64_s::sin() const -> float64
{
    // TODO: Find an implementation that uses SSE/AVX in the future
    double v[element_count];
    for (auto n = 0; n < element_count; ++n) {
        v[n] = std::sin(m_values[n]);
    }
    return float64(v[0], v[1]);
}

auto simd::float64_s::cos() const -> float64
{
    // TODO: Find an implementation that uses SSE/AVX in the future
    double v[element_count];
    for (auto n = 0; n < element_count; ++n) {
        v[n] = std::cos(m_values[n]);
    }
    return float64(v[0], v[1]);
}

auto simd::float64_s::rcp() const -> float64
{
    // TODO: Implement this
    return *this;
}

auto simd::float64_s::min(const float64 &other) const -> float64
{
    return float64(_mm_min_pd(vector(), other.vector()));
}

auto simd::float64_s::min(double f) const -> float64
{
    return float64(_mm_min_pd(vector(), float64::constant(f).vector()));
}

auto simd::float64_s::max(const float64 &other) const -> float64
{
    return float64(_mm_max_pd(vector(), other.vector()));
}

auto simd::float64_s::max(double f) const -> float64
{
    return float64(_mm_max_pd(vector(), float64::constant(f).vector()));
}

auto simd::float64_s::upper() const -> float64
{
    return float64(m_values[1], m_values[1]);
}

auto simd::float64_s::lower() const -> float64
{
    return float64(m_values[0], m_values[0]);
}

auto simd::float64_s::swapped() const -> float64
{
    return float64(m_values[1], m_values[0]);
}

auto simd::float64_s::reversed() const -> float64
{
    return float64(m_values[1], m_values[0]);
}

auto simd::float64_s::set_lower(const float64& lower) -> void
{
    m_values[0] = lower[0];
}

auto simd::float64_s::set_upper(const float64& upper) -> void
{
    m_values[1] = upper[1];
}

#endif