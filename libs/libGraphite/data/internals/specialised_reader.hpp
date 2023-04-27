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

#include "libGraphite/data/endianess.hpp"

namespace graphite::data::internals
{

    class specialised_reader
    {
    public:

        virtual auto swap(std::uint8_t value, std::size_t size = 1) -> std::uint8_t
        {
            return value;
        };

        virtual auto swap(std::uint16_t value, std::size_t size = 2) -> std::uint16_t
        {
            return value;
        };

        virtual auto swap(std::uint32_t value, std::size_t size = 4) -> std::uint32_t
        {
            return value;
        };

        virtual auto swap(std::uint64_t value, std::size_t size = 1) -> std::uint64_t
        {
            return value;
        };

        virtual auto swap(std::int8_t value, std::size_t size = 1) -> std::int8_t
        {
            return value;
        };

        virtual auto swap(std::int16_t value, std::size_t size = 2) -> std::int16_t
        {
            return value;
        };

        virtual auto swap(std::int32_t value, std::size_t size = 4) -> std::int32_t
        {
            return value;
        };

        virtual auto swap(std::int64_t value, std::size_t size = 1) -> std::int64_t
        {
            return value;
        };

    };

}