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
#include <libGraphite/quickdraw/type/point.hpp>
#include <libGraphite/data/reader.hpp>
#include <libGraphite/sys/types.hpp>

using namespace graphite;

// MARK: - Test Data

static constexpr std::uint8_t point_test_data[] = {
    0x00, 0x80, 0x00, 0x20
};

// MARK: - Construction Tests

TEST(point_constructUsingEqualCoordinates)
{
    quickdraw::point<DWRD> point(10);
    test::equal(point.x, 10);
    test::equal(point.y, 10);
}

TEST(point_constructUsingSeparateCoordinates)
{
    quickdraw::point<DWRD> point(5, 7);
    test::equal(point.x, 5);
    test::equal(point.y, 7);
}

TEST(point_constructUsingReader_quickdraw)
{
    data::block block(point_test_data, sizeof(point_test_data), false);
    data::reader reader(&block);

    quickdraw::point<DWRD> point(reader, quickdraw::coding_type::quickdraw);
    test::equal(point.x, 32);
    test::equal(point.y, 128);
}

TEST(point_constructUsingReader_macintosh)
{
    data::block block(point_test_data, sizeof(point_test_data), false);
    data::reader reader(&block);

    quickdraw::point<DWRD> point(reader, quickdraw::coding_type::macintosh);
    test::equal(point.x, 128);
    test::equal(point.y, 32);
}

// MARK: - Static Reader

TEST(point_readPointFromReader)
{
    data::block block(point_test_data, sizeof(point_test_data), false);
    data::reader reader(&block);

    auto point = quickdraw::point<DWRD>::read(reader, quickdraw::coding_type::quickdraw);
    test::equal(point.x, 32);
    test::equal(point.y, 128);
}

// MARK: - Encoding

TEST(point_encodeInToWriter_quickdraw)
{
    data::writer writer(data::byte_order::msb);

    quickdraw::point<DWRD> point(32, 128);
    point.encode(writer, quickdraw::coding_type::quickdraw);

    test::equal(writer.size(), sizeof(point_test_data));
    test::bytes_equal(writer.data()->get<std::uint8_t *>(), point_test_data, sizeof(point_test_data));
}

TEST(point_encodeInToWriter_macintosh)
{
    data::writer writer(data::byte_order::msb);

    quickdraw::point<DWRD> point(128, 32);
    point.encode(writer, quickdraw::coding_type::macintosh);

    test::equal(writer.size(), sizeof(point_test_data));
    test::bytes_equal(writer.data()->get<std::uint8_t *>(), point_test_data, sizeof(point_test_data));
}

// MARK: - Operators

TEST(point_assignmentOperator_copyPoint)
{
    quickdraw::point<DWRD> point(5);
    test::equal(point.x, 5);
    test::equal(point.y, 5);

    quickdraw::point<DWRD> newPoint(10);
    point = newPoint;
    test::equal(point.x, 10);
    test::equal(point.y, 10);
}

TEST(point_assignmentOperator_movePoint)
{
    quickdraw::point<DWRD> point(5);
    test::equal(point.x, 5);
    test::equal(point.y, 5);

    point = quickdraw::point<DWRD>(10);
    test::equal(point.x, 10);
    test::equal(point.y, 10);
}

TEST(point_equalsOperator_shouldBeEqual)
{
    test::is_true(quickdraw::point<DWRD>(5) == quickdraw::point<DWRD>(5));
}

TEST(point_equalsOperator_shouldNotBeEqual)
{
    test::is_false(quickdraw::point<DWRD>(5) == quickdraw::point<DWRD>(7));
}

TEST(point_notEqualsOperator_shouldNotBeEqual)
{
    test::is_true(quickdraw::point<DWRD>(5) != quickdraw::point<DWRD>(7));
}

TEST(point_notEqualsOperator_shouldBeEqual)
{
    test::is_false(quickdraw::point<DWRD>(5) != quickdraw::point<DWRD>(5));
}

TEST(point_addPoint_returnsExpectedPoint)
{
    auto result = quickdraw::point<DWRD>(5) + quickdraw::point<DWRD>(3);
    test::equal(result.x, 8);
    test::equal(result.y, 8);
}

TEST(point_subtractPoint_returnsExpectedPoint)
{
    auto result = quickdraw::point<DWRD>(5) - quickdraw::point<DWRD>(3);
    test::equal(result.x, 2);
    test::equal(result.y, 2);
}

TEST(point_multiplyPoint_returnsExpectedPoint)
{
    auto result = quickdraw::point<DWRD>(5) * 3;
    test::equal(result.x, 15);
    test::equal(result.y, 15);
}

TEST(point_dividePoint_returnsExpectedPoint)
{
    auto result = quickdraw::point<DWRD>(9) / 3;
    test::equal(result.x, 3);
    test::equal(result.y, 3);
}

// MARK: - Casting Tests

TEST(point_castToDifferentType_retainsSameValue)
{
    quickdraw::point<DWRD> point(76);
    auto new_point = point.cast<DBYT>();
    test::equal(new_point.x, 76);
    test::equal(new_point.y, 76);
}