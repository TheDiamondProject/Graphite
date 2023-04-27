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

// MARK: - Helper Macros

#if defined(GRAPHITE_USE_CAMEL_CASE)
#   define GRAPHITE_SYMBOL(_snake, _camel, _posix) _camel
#else
#   define GRAPHITE_SYMBOL(_snake, _camel, _posix) _snake
#endif

// MARK: - Classic Macintosh Integer types

namespace graphite
{
    typedef       signed char           GRAPHITE_SYMBOL(sint8, SInt8, int8_t);
    typedef     unsigned char           GRAPHITE_SYMBOL(uint8, UInt8, uint8_t);
    typedef       signed short          GRAPHITE_SYMBOL(sint16, SInt16, int16_t);
    typedef     unsigned short          GRAPHITE_SYMBOL(uint16, UInt16, uint16_t);
    typedef       signed long           GRAPHITE_SYMBOL(sint32, SInt32, int32_t);
    typedef     unsigned long           GRAPHITE_SYMBOL(uint32, UInt32, uint32_t);
    typedef       signed long long      GRAPHITE_SYMBOL(sint64, SInt64, int64_t);
    typedef     unsigned long long      GRAPHITE_SYMBOL(uint64, UInt64, uint64_t);
}

// MARK: - ResEdit Integer Types

namespace graphite
{
    typedef       signed char           DBYT;
    typedef     unsigned char           HBYT;
    typedef       signed short          DWRD;
    typedef     unsigned short          HWRD;
    typedef       signed long           DLNG;
    typedef     unsigned long           HLNG;
    typedef       signed long long      DQWD;
    typedef     unsigned long long      HQWD;
}