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
#include <libGraphite/data/data.hpp>

// MARK: - Construction Tests

TEST(dataBlock_constructWithPowerOfTwoCapacity_sizeIsCorrect) {
    graphite::data::block block(64);
    test::equal(block.raw_size(), 64, "Raw size of block was expected to be 64");
    test::equal(block.size(), 64, "Size of block was expected to be 64.");
};

TEST(dataBlock_constructWithCapacity_rawSizeIsAdjustedCorrectly) {
    graphite::data::block block(43);
    test::equal(block.raw_size(), 48, "Raw size of block was expected to be 48. Got " + std::to_string(block.raw_size()));
    test::equal(block.size(), 43, "Size of block was expected to be 43. Got " + std::to_string(block.size()));
};

TEST(dataBlock_construct_defaultByteOrderIsCorrect) {
    graphite::data::block block(8);
    test::equal(block.byte_order(), graphite::data::byte_order::msb, "Data Blocks should have a default byte order of MSB.");
}

TEST(dataBlock_construct_usingLSBByteOrder_assignsCorrectly) {
    graphite::data::block block(8, graphite::data::byte_order::lsb);
    test::equal(block.byte_order(), graphite::data::byte_order::lsb, "Data Blocks should have a default byte order of LSB.");
}