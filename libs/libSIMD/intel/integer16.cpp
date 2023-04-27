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

#if INTEL_SIMD
#include <cassert>
#include <libSIMD/cpu.hpp>
#include <libSIMD/integer16.hpp>

// MARK: - Construction

simd::integer16_s::integer16_s(std::int8_t i0, std::int8_t i1, std::int8_t i2, std::int8_t i3, std::int8_t i4,
                               std::int8_t i5, std::int8_t i6, std::int8_t i7, std::int8_t i8, std::int8_t i9,
                               std::int8_t i10, std::int8_t i11, std::int8_t i12, std::int8_t i13, std::int8_t i14,
                               std::int8_t i15)
{
    _mm_store_si128((__m128i *)&m_values[0], _mm_set_epi8(i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15));
}

simd::integer16_s::integer16_s(std::int16_t i0, std::int16_t i1, std::int16_t i2, std::int16_t i3, std::int16_t i4,
                               std::int16_t i5, std::int16_t i6, std::int16_t i7)
{
    _mm_store_si128((__m128i *)&m_values[0], _mm_set_epi16(i0, i1, i2, i3, i4, i5, i6, i7));
}

simd::integer16_s::integer16_s(std::int32_t i0, std::int32_t i1, std::int32_t i2, std::int32_t i3)
{
    _mm_store_si128((__m128i *)&m_values[0], _mm_set_epi32(i0, i1, i2, i3));
}

simd::integer16_s::integer16_s(std::int64_t i0, std::int64_t i1)
{
    _mm_store_si128((__m128i *)&m_values[0], _mm_set_epi64x(i0, i1));
}

simd::integer16_s::integer16_s(i16x8 v)
{
    _mm_store_si128((__m128i *)&m_values[0], v);
}

auto simd::integer16_s::constant(std::int16_t v) -> integer16
{
    return integer16(_mm_set1_epi16(v));
}

// MARK: - Accessors

auto simd::integer16_s::operator[](int i) const -> std::int16_t
{
    assert(i >= 0 && i < element_count);
    return m_values[element_count - 1 - i];
}

auto simd::integer16_s::set(int i, std::int16_t v) -> integer16&
{
    assert(i >= 0 && i < element_count);
    m_values[element_count - 1 - i] = v;
    return *this;
}

auto simd::integer16_s::vector() const -> i16x8
{
    return _mm_loadu_si16((__m128i *)&m_values[0]);
}

auto simd::integer16_s::set(i8x16 v) -> void
{
    _mm_store_si128((__m128i *)&m_values[0], v);
}

// MARK: - Operators

auto simd::integer16_s::operator+(const integer16& other) const -> integer16
{
    return integer16(_mm_add_epi16(vector(), other.vector()));
}

auto simd::integer16_s::operator+(std::int16_t i) const -> integer16
{
    return *this + integer16::constant(i);
}

auto simd::integer16_s::operator+=(const integer16& other) -> integer16&
{
    set(_mm_add_epi16(vector(), other.vector()));
    return *this;
}

auto simd::integer16_s::operator+=(std::int16_t i) -> integer16 &
{
    set(_mm_add_epi16(vector(), integer16::constant(i).vector()));
    return *this;
}

auto simd::integer16_s::operator-(const integer16& other) const -> integer16
{
    return integer16(_mm_sub_epi16(vector(), other.vector()));
}

auto simd::integer16_s::operator-(std::int16_t i) const -> integer16
{
    return *this - integer16::constant(i);
}

auto simd::integer16_s::operator-=(const integer16& other) -> integer16&
{
    set(_mm_sub_epi16(vector(), other.vector()));
    return *this;
}

auto simd::integer16_s::operator-=(std::int16_t i) -> integer16 &
{
    set(_mm_sub_epi16(vector(), integer16::constant(i).vector()));
    return *this;
}

auto simd::integer16_s::operator*(const integer16& other) const -> integer16
{
    // Load the source and destinations
    auto s1 = _mm_loadu_si16((__m128i *)&m_values[0]);
    auto d1 = _mm_loadu_si16((__m128i *)&other.m_values[0]);
    auto r1 = _mm_mullo_epi16(s1, d1);
    return integer16(r1);
}

auto simd::integer16_s::operator*(std::int16_t i) const -> integer16
{
    return *this * integer16::constant(i);
}

auto simd::integer16_s::operator*=(const integer16& other) -> integer16&
{
    set((*this * other).vector());
    return *this;
}

auto simd::integer16_s::operator*=(std::int16_t i) -> integer16 &
{
    set((*this * integer16::constant(i)).vector());
    return *this;
}

auto simd::integer16::operator*(float f) const -> integer16
{
    // Load the source and destinations
    __m128i r1;
    auto op = _mm_set1_ps(f);
    auto s1 = _mm_castsi128_ps(_mm_loadu_si32((__m128i *)&m_values[0]));
    auto s2 = _mm_castsi128_ps(_mm_loadu_si32((__m128i *)&m_values[4]));
    s1 = _mm_mul_ps(s1, op);
    s2 = _mm_mul_ps(s2, op);

    auto d1 = _mm_castps_si128(s1);
    auto d2 = _mm_castps_si128(s2);
    d1 = _mm_and_si128(d1, _mm_set1_epi32(0x0000FFFF));
    d2 = _mm_and_si128(d2, _mm_set1_epi32(0x0000FFFF));
    d2 = _mm_sll_epi32(d2, _mm_set1_epi32(16));
    r1 = _mm_or_si128(d1, d2);
    return integer16(r1);
}

auto simd::integer16_s::operator*=(float f) -> integer16&
{
    set((*this * f).vector());
    return *this;
}

auto simd::integer16_s::operator/(const integer16& other) const -> integer16
{
    // Load the source and destinations
    __m128i r1;
    auto s1 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&m_values[0]));
    auto s2 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&m_values[4]));
    auto op1 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&other.m_values[0]));
    auto op2 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&other.m_values[4]));
    s1 = _mm_div_ps(s1, op1);
    s2 = _mm_div_ps(s2, op2);

    auto d1 = _mm_castps_si128(s1);
    auto d2 = _mm_castps_si128(s2);
    d1 = _mm_and_si128(d1, _mm_set1_epi32(0x0000FFFF));
    d2 = _mm_and_si128(d2, _mm_set1_epi32(0x0000FFFF));
    d2 = _mm_sll_epi32(d2, _mm_set1_epi32(16));
    r1 = _mm_or_si128(d1, d2);
    return integer16(r1);
}

auto simd::integer16_s::operator/(std::int16_t i) const -> integer16
{
    return *this / integer16::constant(i);
}

auto simd::integer16_s::operator/=(const integer16& other) -> integer16&
{
    set((*this / other).vector());
    return *this;
}

auto simd::integer16_s::operator/=(std::int16_t i) -> integer16 &
{
    set((*this / integer16::constant(i)).vector());
    return *this;
}

auto simd::integer16_s::operator/(float f) const -> integer16
{
    // Load the source and destinations
    __m128i r1;
    auto op = _mm_set1_ps(f);
    auto s1 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&m_values[0]));
    auto s2 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&m_values[4]));
    s1 = _mm_div_ps(s1, op);
    s2 = _mm_div_ps(s2, op);

    auto d1 = _mm_castps_si128(s1);
    auto d2 = _mm_castps_si128(s2);
    d1 = _mm_and_si128(d1, _mm_set1_epi32(0x0000FFFF));
    d2 = _mm_and_si128(d2, _mm_set1_epi32(0x0000FFFF));
    d2 = _mm_sll_epi32(d2, _mm_set1_epi32(16));
    r1 = _mm_or_si128(d1, d2);
    return integer16(r1);
}

auto simd::integer16_s::operator/=(float f) -> integer16&
{
    set((*this / f).vector());
    return *this;
}

auto simd::integer16_s::operator&(const integer16& other) const -> integer16
{
    return integer16(_mm_and_si128(vector(), other.vector()));
}

auto simd::integer16_s::operator&(std::int16_t i) const -> integer16
{
    return *this & integer16::constant(i);
}

auto simd::integer16_s::operator&=(const integer16& other) -> integer16&
{
    set((*this & other).vector());
    return *this;
}

auto simd::integer16_s::operator&=(std::int16_t i) -> integer16&
{
    set((*this & i).vector());
    return *this;
}

auto simd::integer16_s::operator|(const integer16& other) const -> integer16
{
    return integer16(_mm_or_si128(vector(), other.vector()));
}

auto simd::integer16_s::operator|(std::int16_t i) const -> integer16
{
    return *this | integer16::constant(i);
}

auto simd::integer16_s::operator|=(const integer16& other) -> integer16&
{
    set((*this | other).vector());
    return *this;
}

auto simd::integer16_s::operator|=(std::int16_t i) -> integer16&
{
    set((*this & i).vector());
    return *this;
}

auto simd::integer16_s::operator^(const integer16& other) const -> integer16
{
    return integer16(_mm_or_si128(vector(), other.vector()));
}

auto simd::integer16_s::operator^(std::int16_t i) const -> integer16
{
    return *this ^ integer16::constant(i);
}

auto simd::integer16_s::operator^=(const integer16& other) -> integer16&
{
    set((*this ^ other).vector());
    return *this;
}

auto simd::integer16_s::operator^=(std::int16_t i) -> integer16&
{
    set((*this ^ i).vector());
    return *this;
}

auto simd::integer16_s::operator~() const -> integer16
{
    return integer16(_mm_xor_si128(vector(), _mm_set1_epi16(0xFF)));
}

auto simd::integer16_s::operator<<(const integer16& other) const -> integer16
{
    auto s1 = _mm_loadu_si128((__m128i *)&m_values[0]);
    auto d1 = _mm_loadu_si128((__m128i *)&other.m_values[0]);
    auto r1 = _mm_sll_epi16(s1, d1);
    return integer16(r1);
}

auto simd::integer16_s::operator<<(std::int16_t i) const -> integer16
{
    return *this << integer16::constant(i);
}

auto simd::integer16_s::operator<<=(const integer16& other) -> integer16&
{
    set((*this << other).vector());
    return *this;
}

auto simd::integer16_s::operator<<=(std::int16_t i) -> integer16&
{
    set((*this << i).vector());
    return *this;
}

auto simd::integer16_s::operator>>(const integer16& other) const -> integer16
{
    auto s1 = _mm_loadu_si128((__m128i *)&m_values[0]);
    auto d1 = _mm_loadu_si128((__m128i *)&other.m_values[0]);
    auto r1 = _mm_sra_epi16(s1, d1);
    return integer16(r1);
}

auto simd::integer16_s::operator>>(std::int16_t i) const -> integer16
{
    return *this << integer16::constant(i);
}

auto simd::integer16_s::operator>>=(const integer16& other) -> integer16&
{
    set((*this << other).vector());
    return *this;
}

auto simd::integer16_s::operator>>=(std::int16_t i) -> integer16&
{
    set((*this << i).vector());
    return *this;
}

auto simd::integer16_s::abs() const -> integer16
{
    return integer16(_mm_abs_epi16(vector()));
}

#endif