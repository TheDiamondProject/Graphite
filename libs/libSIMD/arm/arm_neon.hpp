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

#if (__ARM_NEON || __ARM_NEON__)
#include <arm_neon.h>
#include <cmath>

// Hints / Macros
#define SIMD_FUNCTION               __attribute__((__always_inline__, __nodebug__, __min_vector_width__(128)))

// Types
typedef float f32x4 __attribute__((neon_vector_type((4))));

namespace simd
{

    SIMD_FUNCTION
    static inline auto store_vector(float *p, f32x4 a) -> void
    {
        vst1q_f32(p, a);
    }

    SIMD_FUNCTION
    static inline auto load_vector(float *p) -> f32x4
    {
        return vld1q_f32(p);
    }

    SIMD_FUNCTION
    static inline auto single_value_vector(float w) -> f32x4
    {
        return vdupq_n_f32(w);
    }

    SIMD_FUNCTION
    static inline auto vector(float z, float y, float x, float w) -> f32x4
    {
        float __attribute__((__aligned__(16))) data[4] = { z, y, x, w };
        return vld1q_f32(data);
    }

    SIMD_FUNCTION
    static inline auto vector_shuffle_lower_higher(f32x4 a, f32x4 b) -> f32x4
    {
        return vcombine_f32(vget_high_f32(b), vget_low_f32(a));
    }

    SIMD_FUNCTION
    static inline auto vector_slice_lower(f32x4 a) -> f32x4
    {
        return vcombine_f32(vrev64_f32(vget_low_f32(a)), vrev64_f32(vget_low_f32(a)));
    }

    SIMD_FUNCTION
    static inline auto vector_slice_upper(f32x4 a) -> f32x4
    {
        return vcombine_f32(vrev64_f32(vget_high_f32(a)), vrev64_f32(vget_high_f32(a)));
    }

    SIMD_FUNCTION
    static inline auto swap_lower_upper(f32x4 a) -> f32x4
    {
        return vector_shuffle_lower_higher(a, a);
    }

    SIMD_FUNCTION
    static inline auto reverse(f32x4 a) -> f32x4
    {
        return vcombine_f32(vget_high_f32(a), vget_low_f32(a));
    }

    SIMD_FUNCTION
    static inline auto add(f32x4 a, f32x4 b) -> f32x4
    {
        return vaddq_f32(a, b);
    }

    SIMD_FUNCTION
    static inline auto sub(f32x4 a, f32x4 b) -> f32x4
    {
        return vsubq_f32(a, b);
    }

    SIMD_FUNCTION
    static inline auto mul(f32x4 a, f32x4 b) -> f32x4
    {
        return vmulq_f32(a, b);
    }

    SIMD_FUNCTION
    static inline auto div(f32x4 a, f32x4 b) -> f32x4
    {
        return vdivq_f32(a, b);
    }

    SIMD_FUNCTION
    static inline auto abs(f32x4 a) -> f32x4
    {
        return vabsq_f32(a);
    }

    SIMD_FUNCTION
    static inline auto round(f32x4 a) -> f32x4
    {
        auto r = vaddq_f32(a, vdupq_n_f32(0.5f));
        return vreinterpretq_f32_s32(vreinterpretq_s32_f32(r));
    }

    SIMD_FUNCTION
    static inline auto floor(f32x4 a) -> f32x4
    {
        return vreinterpretq_f32_s32(vreinterpretq_s32_f32(a));
    }

    SIMD_FUNCTION
    static inline auto ceil(f32x4 a) -> f32x4
    {
        auto r = vaddq_f32(a, vdupq_n_f32(0.999999f));
        return vreinterpretq_f32_s32(vreinterpretq_s32_f32(r));
    }

    SIMD_FUNCTION
    static inline auto pow(f32x4 a, float exp) -> f32x4
    {
        if (exp == 2.0) {
            return vmulq_f32(a, a);
        }
        else {
            vector_float4 v = a;
            for (auto i = 0; i < 4; ++i) {
                v[i] = std::powf(v[i], exp);
            }
            return v;
        }
    }

    SIMD_FUNCTION
    static inline auto sqrt(f32x4 a) -> f32x4
    {
        return vsqrtq_f32(a);
    }

    SIMD_FUNCTION
    static inline auto rsqrt(f32x4 a) -> f32x4
    {
        return vrsqrteq_f32(a);
    }

    SIMD_FUNCTION
    static inline auto sin(f32x4 a) -> f32x4
    {
        vector_float4 v = a;
        for (auto i = 0; i < 4; ++i) {
            v[i] = std::sinf(v[i]);
        }
        return v;
    }

    SIMD_FUNCTION
    static inline auto cos(f32x4 a) -> f32x4
    {
        vector_float4 v = a;
        for (auto i = 0; i < 4; ++i) {
            v[i] = std::cosf(v[i]);
        }
        return v;
    }

    SIMD_FUNCTION
    static inline auto rcp(f32x4 a) -> f32x4
    {
        auto r = vrecpeq_f32(a);
        r = vmulq_f32(r, vrecpsq_f32(r, a));
        return r;
    }

    SIMD_FUNCTION
    static inline auto min(f32x4 a, f32x4 b) -> f32x4
    {
        return vminq_f32(a, b);
    }

    SIMD_FUNCTION
    static inline auto max(f32x4 a, f32x4 b) -> f32x4
    {
        return vmaxq_f32(a, b);
    }

}

#endif