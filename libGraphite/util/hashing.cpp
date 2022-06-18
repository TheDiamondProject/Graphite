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

#include <cstdint>
#include "libGraphite/util/hashing.hpp"

// MARK: - Constants

struct uint64_unaligned { uint64_t v; } __attribute__((packed));
struct uint32_unaligned { uint32_t v; } __attribute__((packed));

static uint64_t xxhash64_seed = 0;
static uint64_t xxhash64_p1 = 11400714785074694791ULL;
static uint64_t xxhash64_p2 = 14029467366897019727ULL;
static uint64_t xxhash64_p3 = 1609587929392839161ULL;
static uint64_t xxhash64_p4 = 9650029242287828579ULL;
static uint64_t xxhash64_p5 = 2870177450012600261ULL;

#define ROTL32(x,r) (((x) << (r)) | ((x) >> (32 - (r))))
#define ROTL64(x,r) (((x) << (r)) | ((x) >> (64 - (r))))
#define UNALGINED64(p) (((struct uint64_unaligned *)(p))->v)
#define UNALGINED32(p) (((struct uint32_unaligned *)(p))->v)

// MARK: - XXHash64

auto graphite::hashing::xxh64(const void *src, std::size_t length) -> hashing::value
{
    uint64_t h64 { 0 };
    auto ptr = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(src));
    auto end = ptr + length;

    if (length >= 32) {
        uint8_t *limit = end - 32;
        uint64_t v2 = xxhash64_seed + xxhash64_p2;
        uint64_t v1 = v2 + xxhash64_p1;
        uint64_t v3 = xxhash64_seed;
        uint64_t v4 = xxhash64_seed - xxhash64_p1;

        do {
            v1 += UNALGINED64(ptr) * xxhash64_p2;
            ptr += 8;
            v1 = ROTL64(v1, 31);
            v1 *= xxhash64_p1;

            v2 += UNALGINED64(ptr) * xxhash64_p2;
            ptr += 8;
            v2 = ROTL64(v2, 31);
            v2 *= xxhash64_p1;

            v3 += UNALGINED64(ptr) * xxhash64_p2;
            ptr += 8;
            v3 = ROTL64(v3, 31);
            v3 *= xxhash64_p1;

            v4 += UNALGINED64(ptr) * xxhash64_p2;
            ptr += 8;
            v4 = ROTL64(v4, 31);
            v4 *= xxhash64_p1;
        }
        while (ptr <= limit);

        h64 = ROTL64(v1, 1) + ROTL64(v2, 7) + ROTL64(v3, 12) + ROTL64(v4, 18);

        v1 *= xxhash64_p2;
        v1 = ROTL64(v1, 31);
        v1 *= xxhash64_p1;
        h64 ^= v1;
        h64 = h64 * xxhash64_p1 + xxhash64_p4;

        v2 *= xxhash64_p2;
        v2 = ROTL64(v2, 31);
        v2 *= xxhash64_p1;
        h64 ^= v2;
        h64 = h64 * xxhash64_p1 + xxhash64_p4;

        v3 *= xxhash64_p2;
        v3 = ROTL64(v3, 31);
        v3 *= xxhash64_p1;
        h64 ^= v3;
        h64 = h64 * xxhash64_p1 + xxhash64_p4;

        v4 *= xxhash64_p2;
        v4 = ROTL64(v4, 31);
        v4 *= xxhash64_p1;
        h64 ^= v4;
        h64 = h64 * xxhash64_p1 + xxhash64_p4;
    }
    else {
        h64 = xxhash64_seed + xxhash64_p5;
    }

    h64 += length;

    while (ptr+8 <= end) {
        uint64_t k1 = UNALGINED64(ptr);
        k1 *= xxhash64_p2;
        k1 = ROTL64(k1, 31);
        k1 *= xxhash64_p1;
        h64 ^= k1;
        h64 = ROTL64(h64, 27) * xxhash64_p1 + xxhash64_p4;
        ptr += 8;
    }

    if (ptr+4 <= end) {
        h64 ^= (uint64_t)(UNALGINED32(ptr)) * xxhash64_p1;
        ptr += 4;
        h64 = ROTL64(h64, 23) * xxhash64_p2 + xxhash64_p3;
    }

    while (ptr < end) {
        h64 ^= *ptr * xxhash64_p5;
        h64 = ROTL64(h64, 11) * xxhash64_p1;
        ++ptr;
    }

    h64 ^= h64 >> 33;
    h64 *= xxhash64_p2;
    h64 ^= h64 >> 29;
    h64 *= xxhash64_p3;
    h64 ^= h64 >> 32;

    return h64;
}