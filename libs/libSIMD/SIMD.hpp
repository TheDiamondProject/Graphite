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

#if (__arm64 || __arm64__) && (__ARM_NEON || __ARM_NEON__)
#   define ARM                              true
#   define ARM_NEON                         true
#   if __APPLE__
#       define APPLE_SILICON                true
#   endif
#   define INTEL_SIMD                       false
#elif (__x86_64__)
#   define ARM                              false
#   define ARM_NEON                         false
#   define APPLE_SILICON                    false
#   define INTEL_SIMD                       true
#endif

#if ARM_NEON
#   include <arm_neon.h>
    typedef ::int8x16_t                     i8x16;
    typedef ::int16x8_t                     i16x8;
    typedef ::int32x4_t                     i32x4;
    typedef ::int64x2_t                     i64x2;
    typedef ::float32x4_t                   f32x4;
    typedef ::float64x2_t                   f64x2;
#endif

#if INTEL_SIMD
#   include <mmintrin.h>
#   include <xmmintrin.h>
#   include <emmintrin.h>
#   include <smmintrin.h>
#   include <pmmintrin.h>
#   include <immintrin.h>
    typedef __m128i                         i8x16;
    typedef __m128i                         i16x8;
    typedef __m128i                         i32x4;
    typedef __m128i                         i64x2;
    typedef __m128                          f32x4;
    typedef __m128                          f64x2;
#endif

/* Not all targets implement `memcpy`, `memset`, etc with SIMD operations
 * and as such we need to provide an implementation for these functions.
 */
#if !defined(USE_TARGET_MEMORY_FUNCTIONS)
#   if __APPLE__
#       define USE_TARGET_MEMORY_FUNCTIONS  true
#   else
#       define USE_TARGET_MEMORY_FUNCTIONS  false
#   endif
#endif

#if !defined(APPROX_VALUE)
#   define APPROX_VALUE
#endif