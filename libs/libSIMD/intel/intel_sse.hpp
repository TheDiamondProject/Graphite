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

#if (__x86_64__)
#include <xmmintrin.h>
#include <cmath>

// MARK: - Types
typedef __m128_u f32x4;

// MARK: - Hints

namespace simd
{
    
    static inline auto store_vector(float *p, f32x4 a) -> void
    {
        _mm_storeu_ps(p, a);
    }
    
    static inline auto load_vector(float *p) -> f32x4
    {
        return _mm_loadu_ps(p);
    }
    
    static inline auto single_value_vector(float w) -> f32x4
    {
        return _mm_set1_ps(w);
    }
    
    static inline auto vector(float z, float y, float x, float w) -> f32x4
    {
        return _mm_setr_ps(z, y, x, w);
    }
    
    static inline auto vector_shuffle_lower_higher(f32x4 a, f32x4 b) -> f32x4
    {
        return _mm_shuffle_ps(a, b, _MM_SHUFFLE(0, 1, 2, 3));
    }
    
    static inline auto vector_slice_lower(f32x4 a) -> f32x4
    {
        return _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 0, 1, 0));
    }
    
    static inline auto vector_slice_upper(f32x4 a) -> f32x4
    {
        return _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 2, 3, 2));
    }
    
    static inline auto swap_lower_upper(f32x4 a) -> f32x4
    {
        return vector_shuffle_lower_higher(a, a);
    }
    
    static inline auto reverse(f32x4 a) -> f32x4
    {
        return _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 1, 2, 3));
    }
    
    static inline auto add(f32x4 a, f32x4 b) -> f32x4
    {
        return _mm_add_ps(a, b);
    }
    
    static inline auto sub(f32x4 a, f32x4 b) -> f32x4
    {
        return _mm_sub_ps(a, b);
    }
    
    static inline auto mul(f32x4 a, f32x4 b) -> f32x4
    {
        return _mm_mul_ps(a, b);
    }
    
    static inline auto div(f32x4 a, f32x4 b) -> f32x4
    {
        return _mm_div_ps(a, b);
    }
    
    static inline auto abs(f32x4 a) -> f32x4
    {
        return _mm_castsi128_ps(_mm_abs_epi32(_mm_castps_si128(a)));
    }
    
    static inline auto round(f32x4 a) -> f32x4
    {
        return _mm_round_ps(a, _MM_ROUND_NEAREST);
    }
    
    static inline auto floor(f32x4 a) -> f32x4
    {
        return _mm_floor_ps(a);
    }
    
    static inline auto ceil(f32x4 a) -> f32x4
    {
        return _mm_ceil_ps(a);
    }

    static inline auto pow(f32x4 a, float exp) -> f32x4
    {
        if (exp == 2.0) {
            return _mm_mul_ps(a, a);
        }
        else {
            f32x4 v = a;
            for (auto i = 0; i < 4; ++i) {
                v[i] = std::powf(v[i], exp);
            }
            return v;
        }
    }
    
    static inline auto sqrt(f32x4 a) -> f32x4
    {
        return _mm_sqrt_ps(a);
    }

    static inline auto rsqrt(f32x4 a) -> f32x4
    {
        return _mm_rsqrt_ps(a);
    }

    static inline auto sin(f32x4 a) -> f32x4
    {
        f32x4 v = a;
        for (auto i = 0; i < 4; ++i) {
            v[i] = std::sinf(v[i]);
        }
        return v;
    }

    static inline auto cos(f32x4 a) -> f32x4
    {
        f32x4 v = a;
        for (auto i = 0; i < 4; ++i) {
            v[i] = std::cosf(v[i]);
        }
        return v;
    }
    
    static inline auto rcp(f32x4 a) -> f32x4
    {
        return _mm_rcp_ps(a);
    }
    
    static inline auto min(f32x4 a, f32x4 b) -> f32x4
    {
        return _mm_min_ps(a, b);
    }

    static inline auto max(f32x4 a, f32x4 b) -> f32x4
    {
        return _mm_max_ps(a, b);
    }

}

#endif