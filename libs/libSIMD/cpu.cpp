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

#include <libSIMD/cpu.hpp>

// MARK: - Static Information

static struct
{
    std::string vendor  { "Unknown" };
    std::string name    { "Unknown" };
    struct {
        bool mmx        { false };
        bool sse        { false };
        bool sse2       { false };
        bool sse3       { false };
        bool ssse3      { false };
        bool sse4_1     { false };
        bool sse4_2     { false };
        bool sse4_a     { false };
        bool avx        { false };
        bool avx2       { false };
        bool neon       { false };
    } features;
} s_cpu_info;

// MARK: - Initialisation and Setup


#if INTEL_SIMD
#include <cpuid.h>

__attribute__((constructor))
static auto simd_initialise_cpu_info() -> void
{
    s_cpu_info.vendor = "???"; // TODO: Find the vendor
    s_cpu_info.name = "???"; // TODO: Find the name of the CPU.

    std::uint32_t info[4];
    __cpuid(0x00000000, info[0], info[1], info[2], info[3]);
    auto nIds = info[0];

    __cpuid(0x80000000, info[0], info[1], info[2], info[3]);
    auto nExIds = info[0];

    // Features
    if (nIds & 0x00000001) {
        __cpuid(0x00000001, info[0], info[1], info[2], info[3]);
        s_cpu_info.features.mmx    = (info[3] & ((std::uint32_t)1 << 23)) != 0;
        s_cpu_info.features.sse    = (info[3] & ((std::uint32_t)1 << 25)) != 0;
        s_cpu_info.features.sse2   = (info[3] & ((std::uint32_t)1 << 26)) != 0;
        s_cpu_info.features.sse3   = (info[2] & ((std::uint32_t)1 <<  0)) != 0;
        s_cpu_info.features.ssse3  = (info[2] & ((std::uint32_t)1 <<  9)) != 0;
        s_cpu_info.features.sse4_1 = (info[2] & ((std::uint32_t)1 << 19)) != 0;
        s_cpu_info.features.sse4_2 = (info[2] & ((std::uint32_t)1 << 20)) != 0;
        s_cpu_info.features.avx    = (info[2] & ((std::uint32_t)1 << 28)) != 0;
    }

    if (nIds & 0x00000007) {
        __cpuid(0x00000007, info[0], info[1], info[2], info[3]);
        s_cpu_info.features.avx2   = (info[2] & ((std::uint32_t)1 << 28)) != 0;
    }

    if (nExIds & 0x80000001) {
        __cpuid(0x80000001, info[0], info[1], info[2], info[3]);
        s_cpu_info.features.sse4_a = (info[2] & ((std::uint32_t)1 << 6)) != 0;
    }
}

#elif APPLE_SILICON
__attribute__((constructor))
static auto simd_initialise_cpu_info() -> void
{
    s_cpu_info.vendor = "Apple Silcon";
    s_cpu_info.name = "???"; // TODO: Find the name of the CPU.
    s_cpu_info.features.neon = true;
}

#elif ARM
__attribute__((constructor))
static auto simd_initialise_cpu_info() -> void
{
    s_cpu_info.vendor = "ARM";
    s_cpu_info.name = "???"; // TODO: Find the name of the CPU.
    s_cpu_info.features.neon = true;
}

#endif