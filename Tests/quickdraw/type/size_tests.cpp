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
#include <libGraphite/quickdraw/type/size.hpp>
#include <libGraphite/data/reader.hpp>
#include <libGraphite/sys/types.hpp>

using namespace graphite;

// MARK: - Test Data

static constexpr std::uint8_t size_test_data[] = {
    0x00, 0x80, 0x00, 0x20
};

// MARK: - Construction Tests

TEST(size_constructUsingEqualDimensions)
{
    quickdraw::size<DWRD> size(10);
    test::equal(size.width, 10);
    test::equal(size.height, 10);
}

TEST(size_constructUsingSeparateDimensions)
{
    quickdraw::size<DWRD> size(5, 7);
    test::equal(size.width, 5);
    test::equal(size.height, 7);
}

TEST(size_constructUsingReader_quickdraw)
{
    data::block block(size_test_data, sizeof(size_test_data), false);
    data::reader reader(&block);

    quickdraw::size<DWRD> size(reader, quickdraw::coding_type::quickdraw);
    test::equal(size.width, 32);
    test::equal(size.height, 128);
}

TEST(size_constructUsingReader_macintosh)
{
    data::block block(size_test_data, sizeof(size_test_data), false);
    data::reader reader(&block);

    quickdraw::size<DWRD> size(reader, quickdraw::coding_type::macintosh);
    test::equal(size.width, 128);
    test::equal(size.height, 32);
}

// MARK: - Static Reader

TEST(size_readsizeFromReader)
{
    data::block block(size_test_data, sizeof(size_test_data), false);
    data::reader reader(&block);

    auto size = quickdraw::size<DWRD>::read(reader, quickdraw::coding_type::quickdraw);
    test::equal(size.width, 32);
    test::equal(size.height, 128);
}

// MARK: - Encoding

TEST(size_encodeInToWriter_quickdraw)
{
    data::writer writer(data::byte_order::msb);

    quickdraw::size<DWRD> size(32, 128);
    size.encode(writer, quickdraw::coding_type::quickdraw);

    test::equal(writer.size(), sizeof(size_test_data));
    test::bytes_equal(writer.data()->get<std::uint8_t *>(), size_test_data, sizeof(size_test_data));
}

TEST(size_encodeInToWriter_macintosh)
{
    data::writer writer(data::byte_order::msb);

    quickdraw::size<DWRD> size(128, 32);
    size.encode(writer, quickdraw::coding_type::macintosh);

    test::equal(writer.size(), sizeof(size_test_data));
    test::bytes_equal(writer.data()->get<std::uint8_t *>(), size_test_data, sizeof(size_test_data));
}

// MARK: - Operators

TEST(size_assignmentOperator_copysize)
{
    quickdraw::size<DWRD> size(5);
    test::equal(size.width, 5);
    test::equal(size.height, 5);

    quickdraw::size<DWRD> newsize(10);
    size = newsize;
    test::equal(size.width, 10);
    test::equal(size.height, 10);
}

TEST(size_assignmentOperator_movesize)
{
    quickdraw::size<DWRD> size(5);
    test::equal(size.width, 5);
    test::equal(size.height, 5);

    size = quickdraw::size<DWRD>(10);
    test::equal(size.width, 10);
    test::equal(size.height, 10);
}

TEST(size_equalsOperator_shouldBeEqual)
{
    test::is_true(quickdraw::size<DWRD>(5) == quickdraw::size<DWRD>(5));
}

TEST(size_equalsOperator_shouldNotBeEqual)
{
    test::is_false(quickdraw::size<DWRD>(5) == quickdraw::size<DWRD>(7));
}

TEST(size_notEqualsOperator_shouldNotBeEqual)
{
    test::is_true(quickdraw::size<DWRD>(5) != quickdraw::size<DWRD>(7));
}

TEST(size_notEqualsOperator_shouldBeEqual)
{
    test::is_false(quickdraw::size<DWRD>(5) != quickdraw::size<DWRD>(5));
}

TEST(size_addsize_returnsExpectedsize)
{
    auto result = quickdraw::size<DWRD>(5) + quickdraw::size<DWRD>(3);
    test::equal(result.width, 8);
    test::equal(result.height, 8);
}

TEST(size_subtractsize_returnsExpectedsize)
{
    auto result = quickdraw::size<DWRD>(5) - quickdraw::size<DWRD>(3);
    test::equal(result.width, 2);
    test::equal(result.height, 2);
}

TEST(size_multiplysize_returnsExpectedsize)
{
    auto result = quickdraw::size<DWRD>(5) * 3;
    test::equal(result.width, 15);
    test::equal(result.height, 15);
}

TEST(size_dividesize_returnsExpectedsize)
{
    auto result = quickdraw::size<DWRD>(9) / 3;
    test::equal(result.width, 3);
    test::equal(result.height, 3);
}

// MARK: - Casting Tests

TEST(size_castToDifferentType_retainsSameValue)
{
    quickdraw::size<DWRD> size(76);
    auto new_size = size.cast<DBYT>();
    test::equal(new_size.width, 76);
    test::equal(new_size.height, 76);
}