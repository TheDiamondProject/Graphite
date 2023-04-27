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
#include <libSIMD/integer8.hpp>

// MARK: - Construction

simd::integer8_s::integer8_s(std::int8_t i0, std::int8_t i1, std::int8_t i2, std::int8_t i3, std::int8_t i4,
                             std::int8_t i5, std::int8_t i6, std::int8_t i7, std::int8_t i8, std::int8_t i9,
                             std::int8_t i10, std::int8_t i11, std::int8_t i12, std::int8_t i13, std::int8_t i14,
                             std::int8_t i15)
{
    _mm_store_si128((__m128i *)&m_values[0], _mm_set_epi8(i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15));
}

simd::integer8_s::integer8_s(std::int16_t i0, std::int16_t i1, std::int16_t i2, std::int16_t i3, std::int16_t i4,
                             std::int16_t i5, std::int16_t i6, std::int16_t i7)
{
    _mm_store_si128((__m128i *)&m_values[0], _mm_set_epi16(i0, i1, i2, i3, i4, i5, i6, i7));
}

simd::integer8_s::integer8_s(std::int32_t i0, std::int32_t i1, std::int32_t i2, std::int32_t i3)
{
    _mm_store_si128((__m128i *)&m_values[0], _mm_set_epi32(i0, i1, i2, i3));
}

simd::integer8_s::integer8_s(std::int64_t i0, std::int64_t i1)
{
    _mm_store_si128((__m128i *)&m_values[0], _mm_set_epi64x(i0, i1));
}

simd::integer8_s::integer8_s(i8x16 v)
{
    _mm_store_si128((__m128i *)&m_values[0], v);
}

auto simd::integer8_s::constant(std::int8_t v) -> integer8
{
    return integer8(_mm_set1_epi8(v));
}

// MARK: - Accessors

auto simd::integer8_s::operator[](int i) const -> std::int8_t
{
    assert(i >= 0 && i < element_count);
    return m_values[element_count - 1 - i];
}

auto simd::integer8_s::set(int i, std::int8_t v) -> integer8&
{
    assert(i >= 0 && i < element_count);
    m_values[element_count - 1 - i] = v;
    return *this;
}

auto simd::integer8_s::vector() const -> i8x16
{
    return _mm_load_si128((__m128i *)&m_values[0]);
}

auto simd::integer8_s::set(i8x16 v) -> void
{
    _mm_store_si128((__m128i *)&m_values[0], v);
}

// MARK: - Operators

auto simd::integer8_s::operator+(const integer8& other) const -> integer8
{
    return integer8(_mm_add_epi8(vector(), other.vector()));
}

auto simd::integer8_s::operator+(std::int8_t i) const -> integer8
{
    return *this + integer8::constant(i);
}

auto simd::integer8_s::operator+=(const integer8& other) -> integer8&
{
    set(_mm_add_epi8(vector(), other.vector()));
    return *this;
}

auto simd::integer8_s::operator+=(std::int8_t i) -> integer8 &
{
    set(_mm_add_epi8(vector(), integer8::constant(i).vector()));
    return *this;
}

auto simd::integer8_s::operator-(const integer8& other) const -> integer8
{
    return integer8(_mm_sub_epi8(vector(), other.vector()));
}

auto simd::integer8_s::operator-(std::int8_t i) const -> integer8
{
    return *this - integer8::constant(i);
}

auto simd::integer8_s::operator-=(const integer8& other) -> integer8&
{
    set(_mm_sub_epi8(vector(), other.vector()));
    return *this;
}

auto simd::integer8_s::operator-=(std::int8_t i) -> integer8 &
{
    set(_mm_sub_epi8(vector(), integer8::constant(i).vector()));
    return *this;
}

auto simd::integer8_s::operator*(const integer8& other) const -> integer8
{
    std::int16_t words[element_count << 1] = {
        // Source - Op1
        m_values[0], m_values[1],  m_values[2],  m_values[3],
        m_values[4],  m_values[5],  m_values[6],  m_values[7],

        // Source Op2
        m_values[8], m_values[9], m_values[10], m_values[11],
        m_values[12], m_values[13], m_values[14], m_values[15],

        // Destination Op1
        other.m_values[0], other.m_values[1],  other.m_values[2],  other.m_values[3],
        other.m_values[4],  other.m_values[5],  other.m_values[6],  other.m_values[7],

        // Destination Op2
        other.m_values[8], other.m_values[9], other.m_values[10], other.m_values[11],
        other.m_values[12], other.m_values[13], other.m_values[14], other.m_values[15],
    };

    // Load the source and destinations
    auto s1 = _mm_loadu_si16((__m128i *)&words[0]);
    auto s2 = _mm_loadu_si16((__m128i *)&words[8]);
    auto d1 = _mm_loadu_si16((__m128i *)&words[16]);
    auto d2 = _mm_loadu_si16((__m128i *)&words[24]);

    // Perform the calculations, and clip down to single byte values.
    // Shift the second result to occupy the hi byte, and the first result to occupy the lo byte
    auto r1 = _mm_and_si128(_mm_mullo_epi16(s1, d1), _mm_set1_epi32(0x00FF00FF));
    auto r2 = _mm_sll_epi32(_mm_and_si128(_mm_mullo_epi16(s2, d2), _mm_set1_epi32(0x00FF00FF)), _mm_set1_epi32(16));

    // Merge the results
    auto r = _mm_or_si128(r1, r2);
    return integer8(r);
}

auto simd::integer8_s::operator*(std::int8_t i) const -> integer8
{
    return *this * integer8::constant(i);
}

auto simd::integer8_s::operator*=(const integer8& other) -> integer8&
{
    set((*this * other).vector());
    return *this;
}

auto simd::integer8_s::operator*=(std::int8_t i) -> integer8 &
{
    set((*this * integer8::constant(i)).vector());
    return *this;
}

auto simd::integer8::operator*(float f) const -> integer8
{
    std::int32_t words[element_count] = {
        // Source - Op1
        m_values[0], m_values[1],  m_values[2],  m_values[3],

        // Source - Op2
        m_values[4],  m_values[5],  m_values[6],  m_values[7],

        // Source - Op3
        m_values[8], m_values[9], m_values[10], m_values[11],

        // Source - Op4
        m_values[12], m_values[13], m_values[14], m_values[15],
    };

    // Load the source and destinations
    __m128i r1, r2, r3, r4;
    auto op = _mm_set1_ps(f);
    auto s1 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[0]));
    auto s2 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[4]));
    auto s3 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[8]));
    auto s4 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[12]));
    r1 = _mm_castps_si128(_mm_mul_ps(s1, op));
    r2 = _mm_castps_si128(_mm_mul_ps(s2, op));
    r3 = _mm_castps_si128(_mm_mul_ps(s3, op));
    r4 = _mm_castps_si128(_mm_mul_ps(s4, op));
    r1 = _mm_and_si128(r1, _mm_set1_epi32(0x000000FF));
    r2 = _mm_and_si128(r2, _mm_set1_epi32(0x000000FF));
    r3 = _mm_and_si128(r3, _mm_set1_epi32(0x000000FF));
    r4 = _mm_and_si128(r4, _mm_set1_epi32(0x000000FF));
    r2 = _mm_sll_epi32(r2, _mm_set1_epi32(8));
    r3 = _mm_sll_epi32(r3, _mm_set1_epi32(16));
    r4 = _mm_sll_epi32(r4, _mm_set1_epi32(24));
    r1 = _mm_or_si128(r1, r2);
    r1 = _mm_or_si128(r1, r3);
    r1 = _mm_or_si128(r1, r4);

    return integer8(r1);
}

auto simd::integer8_s::operator*=(float f) -> integer8&
{
    set((*this * f).vector());
    return *this;
}

auto simd::integer8_s::operator/(const integer8& other) const -> integer8
{
    std::int32_t words[element_count] = {
        // Source - Op1
        m_values[0], m_values[1],  m_values[2],  m_values[3],

        // Source - Op2
        m_values[4],  m_values[5],  m_values[6],  m_values[7],

        // Source - Op3
        m_values[8], m_values[9], m_values[10], m_values[11],

        // Source - Op4
        m_values[12], m_values[13], m_values[14], m_values[15],
    };

    // Load the source and destinations
    __m128i r1, r2, r3, r4;
    auto op = _mm_castsi128_ps(other.vector());
    auto s1 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[0]));
    auto s2 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[4]));
    auto s3 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[8]));
    auto s4 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[12]));
    r1 = _mm_castps_si128(_mm_div_ps(s1, op));
    r2 = _mm_castps_si128(_mm_div_ps(s2, op));
    r3 = _mm_castps_si128(_mm_div_ps(s3, op));
    r4 = _mm_castps_si128(_mm_div_ps(s4, op));
    r1 = _mm_and_si128(r1, _mm_set1_epi32(0x000000FF));
    r2 = _mm_and_si128(r2, _mm_set1_epi32(0x000000FF));
    r3 = _mm_and_si128(r3, _mm_set1_epi32(0x000000FF));
    r4 = _mm_and_si128(r4, _mm_set1_epi32(0x000000FF));
    r2 = _mm_sll_epi32(r2, _mm_set1_epi32(8));
    r3 = _mm_sll_epi32(r3, _mm_set1_epi32(16));
    r4 = _mm_sll_epi32(r4, _mm_set1_epi32(24));
    r1 = _mm_or_si128(r1, r2);
    r1 = _mm_or_si128(r1, r3);
    r1 = _mm_or_si128(r1, r4);

    return integer8(r1);
}

auto simd::integer8_s::operator/(std::int8_t i) const -> integer8
{
    return *this / integer8::constant(i);
}

auto simd::integer8_s::operator/=(const integer8& other) -> integer8&
{
    set((*this / other).vector());
    return *this;
}

auto simd::integer8_s::operator/=(std::int8_t i) -> integer8 &
{
    set((*this / integer8::constant(i)).vector());
    return *this;
}

auto simd::integer8_s::operator/(float f) const -> integer8
{
    std::int32_t words[element_count] = {
        // Source - Op1
        m_values[0], m_values[1],  m_values[2],  m_values[3],

        // Source - Op2
        m_values[4],  m_values[5],  m_values[6],  m_values[7],

        // Source - Op3
        m_values[8], m_values[9], m_values[10], m_values[11],

        // Source - Op4
        m_values[12], m_values[13], m_values[14], m_values[15],
    };

    // Load the source and destinations
    __m128i r1, r2, r3, r4;
    auto op = _mm_set1_ps(f);
    auto s1 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[0]));
    auto s2 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[4]));
    auto s3 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[8]));
    auto s4 = _mm_castsi128_ps(_mm_loadu_si128((__m128i *)&words[12]));
    r1 = _mm_castps_si128(_mm_div_ps(s1, op));
    r2 = _mm_castps_si128(_mm_div_ps(s2, op));
    r3 = _mm_castps_si128(_mm_div_ps(s3, op));
    r4 = _mm_castps_si128(_mm_div_ps(s4, op));
    r1 = _mm_and_si128(r1, _mm_set1_epi32(0x000000FF));
    r2 = _mm_and_si128(r2, _mm_set1_epi32(0x000000FF));
    r3 = _mm_and_si128(r3, _mm_set1_epi32(0x000000FF));
    r4 = _mm_and_si128(r4, _mm_set1_epi32(0x000000FF));
    r2 = _mm_sll_epi32(r2, _mm_set1_epi32(8));
    r3 = _mm_sll_epi32(r3, _mm_set1_epi32(16));
    r4 = _mm_sll_epi32(r4, _mm_set1_epi32(24));
    r1 = _mm_or_si128(r1, r2);
    r1 = _mm_or_si128(r1, r3);
    r1 = _mm_or_si128(r1, r4);

    return integer8(r1);
}

auto simd::integer8_s::operator/=(float f) -> integer8&
{
    set((*this / f).vector());
    return *this;
}

auto simd::integer8_s::operator&(const integer8& other) const -> integer8
{
    return integer8(_mm_and_si128(vector(), other.vector()));
}

auto simd::integer8_s::operator&(std::int8_t i) const -> integer8
{
    return *this & integer8::constant(i);
}

auto simd::integer8_s::operator&=(const integer8& other) -> integer8&
{
    set((*this & other).vector());
    return *this;
}

auto simd::integer8_s::operator&=(std::int8_t i) -> integer8&
{
    set((*this & i).vector());
    return *this;
}

auto simd::integer8_s::operator|(const integer8& other) const -> integer8
{
    return integer8(_mm_or_si128(vector(), other.vector()));
}

auto simd::integer8_s::operator|(std::int8_t i) const -> integer8
{
    return *this | integer8::constant(i);
}

auto simd::integer8_s::operator|=(const integer8& other) -> integer8&
{
    set((*this | other).vector());
    return *this;
}

auto simd::integer8_s::operator|=(std::int8_t i) -> integer8&
{
    set((*this & i).vector());
    return *this;
}

auto simd::integer8_s::operator^(const integer8& other) const -> integer8
{
    return integer8(_mm_or_si128(vector(), other.vector()));
}

auto simd::integer8_s::operator^(std::int8_t i) const -> integer8
{
    return *this ^ integer8::constant(i);
}

auto simd::integer8_s::operator^=(const integer8& other) -> integer8&
{
    set((*this ^ other).vector());
    return *this;
}

auto simd::integer8_s::operator^=(std::int8_t i) -> integer8&
{
    set((*this ^ i).vector());
    return *this;
}

auto simd::integer8_s::operator~() const -> integer8
{
    return integer8(_mm_xor_si128(vector(), _mm_set1_epi8(0xFF)));
}

auto simd::integer8_s::abs() const -> integer8
{
    return integer8(_mm_abs_epi8(vector()));
}

#endif