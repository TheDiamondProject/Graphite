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

using namespace graphite;

// MARK: - Construction Tests

TEST(dataBlock_constructWithPowerOfTwoCapacity_sizeIsCorrect)
{
    data::block block(64);
    test::equal(block.raw_size(), 64, "Raw size of block was expected to be 64");
    test::equal(block.size(), 64, "Size of block was expected to be 64.");
};

TEST(dataBlock_constructWithCapacity_rawSizeIsAdjustedCorrectly)
{
    data::block block(43);
    test::equal(block.raw_size(), 48, "Raw size of block was expected to be 48. Got " + std::to_string(block.raw_size()));
    test::equal(block.size(), 43, "Size of block was expected to be 43. Got " + std::to_string(block.size()));
};

TEST(dataBlock_construct_defaultByteOrderIsCorrect)
{
    data::block block(8);
    test::equal(block.byte_order(), data::byte_order::msb, "Data Blocks should have a default byte order of MSB.");
}

TEST(dataBlock_construct_usingLSBByteOrder_assignsCorrectly)
{
    data::block block(8, data::byte_order::lsb);
    test::equal(block.byte_order(), data::byte_order::lsb, "Data Blocks should have a default byte order of LSB.");
}

TEST(dataBlock_construct_hasOwnershipOfInitialData)
{
    data::block block(8);
    test::is_true(block.has_ownership(), "When constructed, Data Blocks should have ownership over the memory allocation.");
}

// MARK: - Data Access Test

static constexpr uint8_t test_data[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
    0xDE, 0xAD, 0xD0, 0x0D, 0xCA, 0xFE, 0xCA, 0xFE
};

TEST(dataBlock_getByteValue_atOffset_isExpectedValueReturned)
{
    data::block block(test_data, sizeof(test_data), false);
    test::equal(block.size(), sizeof(test_data), "Unexpected data block size when allocating from a byte array");
    test::equal(block.get<std::uint8_t>(2), 0xBE, "Expected to retrieve the value 0xBE from the data block.");
}

TEST(dataBlock_getShortValue_atOffset_isExpectedValueReturned)
{
    data::block block(test_data, sizeof(test_data), false);
    test::equal(block.size(), sizeof(test_data), "Unexpected data block size when allocating from a byte array");
    test::equal(block.get<std::uint16_t>(2), 0xBEEF, "Expected to retrieve the value 0xBEEF from the data block.");
}

TEST(dataBlock_getLongValue_atOffset_isExpectedValueReturned)
{
    data::block block(test_data, sizeof(test_data), false);
    test::equal(block.size(), sizeof(test_data), "Unexpected data block size when allocating from a byte array");
    test::equal(block.get<std::uint32_t>(4), 0xCAFEBABE, "Expected to retrieve the value 0xCAFEBABE from the data block.");
}

TEST(dataBlock_getQuadValue_atOffset_isExpectedValueReturned)
{
    data::block block(test_data, sizeof(test_data), false);
    test::equal(block.size(), sizeof(test_data), "Unexpected data block size when allocating from a byte array");
    test::equal(block.get<std::uint64_t>(8), 0xDEADD00DCAFECAFE, "Expected to retrieve the value 0xDEADD00DCAFECAFE from the data block.");
}

