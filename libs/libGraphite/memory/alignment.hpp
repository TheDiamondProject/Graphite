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

#include <cstdint>
#include <cstdlib>
#include <type_traits>

namespace graphite::memory::alignment
{
    static constexpr std::size_t width = sizeof(std::uintptr_t);
    static constexpr std::size_t mask = ~(width - 1);

    template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
    static inline auto expand_capacity(T capacity) -> T
    {
        return (capacity + width - 1) & mask;
    }

    template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
    static inline auto align(T ptr) -> T
    {
        return reinterpret_cast<T>((reinterpret_cast<std::uintptr_t>(ptr) + width - 1) & mask);
    }
}