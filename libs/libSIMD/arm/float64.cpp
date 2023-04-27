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

#if ARM_NEON
#include <cassert>
#include <cmath>
#include <libSIMD/cpu.hpp>

// MARK: - Construction

simd::float64_s::float64_s(double f0, double f1)
{
    double __attribute__((__aligned__(16))) data[4] = { f0, f1 };
    vst1q_f64(&m_values[0], vld1q_f64(data));
}

simd::float64_s::float64_s(f64x2 v)
{
    vst1q_f64(&m_values[0], v);
}

auto simd::float64_s::constant(double v) -> float64
{
    return float64(vdupq_n_f64(v));
}

// MARK: - Accessors

auto simd::float64_s::operator[](int i) const -> double
{
    assert(i >= 0 && i < element_count);
    return m_values[i];
}

auto simd::float64_s::set(int i, double f) -> float64&
{
    assert(i >= 0 && i < element_count);
    m_values[i] = f;
    return *this;
}

auto simd::float64_s::vector() const -> f64x2
{
    return vld1q_f64(&m_values[0]);
}

auto simd::float64_s::set(f64x2 v) -> void
{
    vst1q_f64(&m_values[0], v);
}

// MARK: - Operators

auto simd::float64_s::operator+ (const float64& other) const -> float64
{
    return float64(vaddq_f64(vector(), other.vector()));
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
    return float64(vsubq_f32(vector(), other.vector()));
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
    return float64(vmulq_f64(vector(), other.vector()));
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
    return float64(vdivq_f64(vector(), other.vector()));
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
    return float64(vabsq_f64(vector()));
}

auto simd::float64_s::round() const -> float64
{
    return float64(vrndnq_f64(vector()));
}

auto simd::float64_s::floor() const -> float64
{
    return float64(vrndmq_f64(vector()));
}

auto simd::float64_s::ceil() const -> float64
{
    return float64(vrndpq_f64(vector()));
}

auto simd::float64_s::pow(double exp) const -> float64
{
    if (exp == 2.0) {
        auto v = vector();
        return float64(vmulq_f64(v, v));
    }
    else {
        // TODO: Find an implementation that uses SSE/AVX in the future
        double v[element_count];
        for (auto n = 0; n < element_count; ++n) {
            v[n] = std::powf(m_values[n], m_values[n]);
        }
        return float64(v[0], v[1]);
    }
}

auto simd::float64_s::sqrt() const -> float64
{
    return float64(vsqrtq_f64(vector()));
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
    auto recip = vrecpeq_f64(vector());
    recip = vmulq_f64(recip, vrecpsq_f64(recip, vector()));
    return float64(recip);
}

auto simd::float64_s::min(const float64 &other) const -> float64
{
    return float64(vminq_f64(vector(), other.vector()));
}

auto simd::float64_s::min(double f) const -> float64
{
    return float64(vminq_f64(vector(), float64::constant(f).vector()));
}

auto simd::float64_s::max(const float64 &other) const -> float64
{
    return float64(vmaxq_f64(vector(), other.vector()));
}

auto simd::float64_s::max(double f) const -> float64
{
    return float64(vmaxq_f64(vector(), float64::constant(f).vector()));
}

#endif