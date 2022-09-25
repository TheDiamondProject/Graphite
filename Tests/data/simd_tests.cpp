// Copyright (c) 2022 Tom Hancocks
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

#include <libTesting/testing.hpp>
#include <libGraphite/data/simd.hpp>

// MARK: - SIMD Capacity Expansion

#if __x86_64__

TEST(simd_x86_64_expandCapacity_expansionRoundsUpCorrectly) {
    test::equal(graphite::data::simd::expand_capacity(0), 0, "Expanded capacity from 0 bytes should remain the same.");
    test::equal(graphite::data::simd::expand_capacity(8), 16, "Expanded capacity from 8 bytes should expand to 16 bytes.");
    test::equal(graphite::data::simd::expand_capacity(16), 16, "Expanded capacity from 16 bytes should remain the same.");
};

#elif __arm64__

TEST(simd_arm64_expandCapacity_expansionRoundsUpCorrectly) {
    test::equal(graphite::data::simd::expand_capacity(0), 0, "Expanded capacity from 0 bytes should remain the same.");
    test::equal(graphite::data::simd::expand_capacity(2), 8, "Expanded capacity from 2 bytes should expand to 8 bytes.");
    test::equal(graphite::data::simd::expand_capacity(8), 8, "Expanded capacity from 2 bytes should remain the same.");
};

#endif

