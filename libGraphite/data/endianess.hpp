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

namespace graphite::data
{
    // MARK: - Byte Order Enumeration

    enum class byte_order : uint8_t { msb, lsb };


    // MARK: - Compile Time Constants

#if __LITTLE_ENDIAN__
    static graphite::data::byte_order s_native_byte_order = graphite::data::byte_order::lsb;
#else
    static graphite::data::byte_order s_native_byte_order = graphite::data::byte_order::msb;
#endif

    static auto native_byte_order() -> enum byte_order
    {
        return s_native_byte_order;
    }


    // MARK: - Endian Swap Operations

    template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    static auto swap(T value, byte_order from_byte_order, byte_order to_byte_order = native_byte_order(), std::size_t size = sizeof(T)) -> T
    {
        if (from_byte_order == to_byte_order) {
            return value;
        }

        T v = 0;

        for (auto i = 0; i < size; ++i) {
            auto b = (size - i - 1) << 3ULL;
            v |= ((value >> b) & 0xFF) << (i << 3ULL);
        }

        return v;
    }
}