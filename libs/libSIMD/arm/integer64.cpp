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

#include <libSIMD/SIMD.hpp>

#if ARM_NEON
#include <cassert>
#include <libSIMD/cpu.hpp>
#include <libSIMD/integer64.hpp>

// MARK: - Construction

simd::integer64_s::integer64_s(std::int8_t i0, std::int8_t i1, std::int8_t i2, std::int8_t i3, std::int8_t i4,
                               std::int8_t i5, std::int8_t i6, std::int8_t i7, std::int8_t i8, std::int8_t i9,
                               std::int8_t i10, std::int8_t i11, std::int8_t i12, std::int8_t i13, std::int8_t i14,
                               std::int8_t i15)
{
    std::int8_t __attribute__((__aligned__(16))) data[16] = {
        i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15
    };
    vst1q_s64(&m_values[0], vld1q_s8(data));
}

simd::integer64_s::integer64_s(std::int16_t i0, std::int16_t i1, std::int16_t i2, std::int16_t i3, std::int16_t i4,
                               std::int16_t i5, std::int16_t i6, std::int16_t i7)
{
    std::int16_t __attribute__((__aligned__(16))) data[8] = {
        i0, i1, i2, i3, i4, i5, i6, i7
    };
    vst1q_s64(&m_values[0], vld1q_s16(data));
}

simd::integer64_s::integer64_s(std::int32_t i0, std::int32_t i1, std::int32_t i2, std::int32_t i3)
{
    std::int32_t __attribute__((__aligned__(16))) data[4] = {
        i0, i1, i2, i3
    };
    vst1q_s64(&m_values[0], vld1q_s32(data));
}

simd::integer64_s::integer64_s(std::int64_t i0, std::int64_t i1)
{
    std::int64_t __attribute__((__aligned__(16))) data[2] = {
        i0, i1
    };
    vst1q_s64(&m_values[0], vld1q_s64(data));
}

simd::integer64_s::integer64_s(i64x2 v)
{
    vst1q_s64(&m_values[0], v);
}

auto simd::integer64_s::constant(std::int64_t v) -> integer64
{
    return integer64(vdupq_n_s64(v));
}

// MARK: - Accessors

auto simd::integer64_s::operator[](int i) const -> std::int64_t
{
    assert(i >= 0 && i < element_count);
    return m_values[i];
}

auto simd::integer64_s::set(int i, std::int64_t v) -> integer64&
{
    assert(i >= 0 && i < element_count);
    m_values[i] = v;
    return *this;
}

auto simd::integer64_s::vector() const -> i64x2
{
    return vld1q_s64(&m_values[0]);
}

auto simd::integer64_s::set(i64x2 v) -> void
{
    vst1q_s64(&m_values[0], v);
}

// MARK: - Operators

auto simd::integer64_s::operator+(const integer64& other) const -> integer64
{
    return integer64(vaddq_s64(vector(), other.vector()));
}

auto simd::integer64_s::operator+(std::int64_t i) const -> integer64
{
    return *this + integer64::constant(i);
}

auto simd::integer64_s::operator+=(const integer64& other) -> integer64&
{
    set(vaddq_s64(vector(), other.vector()));
    return *this;
}

auto simd::integer64_s::operator+=(std::int64_t i) -> integer64 &
{
    set(vaddq_s64(vector(), integer64::constant(i).vector()));
    return *this;
}

auto simd::integer64_s::operator-(const integer64& other) const -> integer64
{
    return integer64(vsubq_s64(vector(), other.vector()));
}

auto simd::integer64_s::operator-(std::int64_t i) const -> integer64
{
    return *this - integer64::constant(i);
}

auto simd::integer64_s::operator-=(const integer64& other) -> integer64&
{
    set(vsubq_s64(vector(), other.vector()));
    return *this;
}

auto simd::integer64_s::operator-=(std::int64_t i) -> integer64 &
{
    set(vsubq_s64(vector(), integer64::constant(i).vector()));
    return *this;
}

auto simd::integer64_s::operator*(const integer64& other) const -> integer64
{
    // TODO: Implement this...
    return *this;
}

auto simd::integer64_s::operator*(std::int64_t i) const -> integer64
{
    return *this * integer64::constant(i);
}

auto simd::integer64_s::operator*=(const integer64& other) -> integer64&
{
    set((*this * other).vector());
    return *this;
}

auto simd::integer64_s::operator*=(std::int64_t i) -> integer64 &
{
    set((*this * integer64::constant(i)).vector());
    return *this;
}

auto simd::integer64::operator*(float f) const -> integer64
{
    // TODO: Implement this.
    return *this;
}

auto simd::integer64_s::operator*=(float f) -> integer64&
{
    set((*this * f).vector());
    return *this;
}

auto simd::integer64_s::operator/(const integer64& other) const -> integer64
{
    // TODO: Implement this.
    return *this;
}

auto simd::integer64_s::operator/(std::int64_t i) const -> integer64
{
    return *this / integer64::constant(i);
}

auto simd::integer64_s::operator/=(const integer64& other) -> integer64&
{
    set((*this / other).vector());
    return *this;
}

auto simd::integer64_s::operator/=(std::int64_t i) -> integer64 &
{
    set((*this / integer64::constant(i)).vector());
    return *this;
}

auto simd::integer64_s::operator/(float f) const -> integer64
{
    // TODO: Implement this.
    return *this;
}

auto simd::integer64_s::operator/=(float f) -> integer64&
{
    set((*this / f).vector());
    return *this;
}

auto simd::integer64_s::operator&(const integer64& other) const -> integer64
{
    return integer64(vandq_u8(vector(), other.vector()));
}

auto simd::integer64_s::operator&(std::int64_t i) const -> integer64
{
    return *this & integer64::constant(i);
}

auto simd::integer64_s::operator&=(const integer64& other) -> integer64&
{
    set((*this & other).vector());
    return *this;
}

auto simd::integer64_s::operator&=(std::int64_t i) -> integer64&
{
    set((*this & i).vector());
    return *this;
}

auto simd::integer64_s::operator|(const integer64& other) const -> integer64
{
    return integer64(vorrq_u16(vector(), other.vector()));
}

auto simd::integer64_s::operator|(std::int64_t i) const -> integer64
{
    return *this | integer64::constant(i);
}

auto simd::integer64_s::operator|=(const integer64& other) -> integer64&
{
    set((*this | other).vector());
    return *this;
}

auto simd::integer64_s::operator|=(std::int64_t i) -> integer64&
{
    set((*this & i).vector());
    return *this;
}

auto simd::integer64_s::operator^(const integer64& other) const -> integer64
{
    return integer64(veorq_u8(vector(), other.vector()));
}

auto simd::integer64_s::operator^(std::int64_t i) const -> integer64
{
    return *this ^ integer64::constant(i);
}

auto simd::integer64_s::operator^=(const integer64& other) -> integer64&
{
    set((*this ^ other).vector());
    return *this;
}

auto simd::integer64_s::operator^=(std::int64_t i) -> integer64&
{
    set((*this ^ i).vector());
    return *this;
}

auto simd::integer64_s::operator~() const -> integer64
{
    return integer64(veorq_u16(vector(), vdupq_n_u16(0xFFFF)));
}

auto simd::integer64_s::operator<<(const integer64& other) const -> integer64
{
    return integer64(vshlq_u8(vector(), other.vector()));
}

auto simd::integer64_s::operator<<(std::int64_t i) const -> integer64
{
    return *this << integer64::constant(i);
}

auto simd::integer64_s::operator<<=(const integer64& other) -> integer64&
{
    set((*this << other).vector());
    return *this;
}

auto simd::integer64_s::operator<<=(std::int64_t i) -> integer64&
{
    set((*this << i).vector());
    return *this;
}

auto simd::integer64_s::operator>>(const integer64& other) const -> integer64
{
    // TODO: Implement this
    return *this;
}

auto simd::integer64_s::operator>>(std::int64_t i) const -> integer64
{
    return *this << integer64::constant(i);
}

auto simd::integer64_s::operator>>=(const integer64& other) -> integer64&
{
    set((*this << other).vector());
    return *this;
}

auto simd::integer64_s::operator>>=(std::int64_t i) -> integer64&
{
    set((*this << i).vector());
    return *this;
}

auto simd::integer64_s::abs() const -> integer64
{
    return integer64(vabsq_s64(vector()));
}

#endif