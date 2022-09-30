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

#pragma once

#include <type_traits>

namespace graphite::hashing
{
    /**
     * A 64-bit hash value.
     * This type definition is intended to help provide semantic context.
     */
    typedef std::uint64_t value64;

    /**
     * A 32-bit hash value.
     * This type definition is intended to help provide semantic context.
     */
    typedef std::uint32_t value32;

    /**
     * Produce a hash value using the 64-bit variant of the XXHash algorithm, using the specified data.
     * @param ptr           A pointer to the byte sequence to produce a hash from.
     * @param length        The length of the data being supplied.
     * @return              A hash value.
     */
    auto xxh64(const void *ptr, std::size_t length) -> hashing::value64;

    /**
     * Produce a hash value using the 32-bit variant of the XXHash algorithm, using the specified data.
     * @param ptr           A pointer to the byte sequence to produce a hash from.
     * @param length        The length of the data being supplied.
     * @return              A hash value.
     */
    auto xxh32(const void *ptr, std::size_t length) -> hashing::value32;
}

