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
#include <libGraphite/quickdraw/type/rect.hpp>
#include <libGraphite/data/reader.hpp>
#include <libGraphite/sys/types.hpp>

using namespace graphite;

// MARK: - Test Data

static constexpr std::uint8_t rect_test_data[] = {
    0x00, 0x20, 0x00, 0x80, 0x00, 0x30, 0x00, 0xC0
};

// MARK: - Construction Tests

TEST(rect_constructUsingEqualValues)
{
    quickdraw::rect<DWRD> rect(5);
    test::equal(rect.origin.x, 5);
    test::equal(rect.origin.y, 5);
    test::equal(rect.size.width, 5);
    test::equal(rect.size.height, 5);
}

TEST(rect_constructUsingSeparateValues)
{
    quickdraw::rect<DWRD> rect(1, 2, 3, 4);
    test::equal(rect.origin.x, 1);
    test::equal(rect.origin.y, 2);
    test::equal(rect.size.width, 3);
    test::equal(rect.size.height, 4);
}

TEST(rect_constructUsingPointAndSize)
{
    quickdraw::rect<DWRD> rect(quickdraw::point<DWRD>(1, 2), quickdraw::size<DWRD>(3, 4));
    test::equal(rect.origin.x, 1);
    test::equal(rect.origin.y, 2);
    test::equal(rect.size.width, 3);
    test::equal(rect.size.height, 4);
}

TEST(rect_constructUsingReader_quickdraw)
{
    data::block block(rect_test_data, sizeof(rect_test_data), false);
    data::reader reader(&block);

    quickdraw::rect<DWRD> rect(reader, quickdraw::coding_type::quickdraw);
    test::equal(rect.origin.x, 128);
    test::equal(rect.origin.y, 32);
    test::equal(rect.size.width, 64);
    test::equal(rect.size.height, 16);
}

TEST(rect_constructUsingReader_macintosh)
{
    data::block block(rect_test_data, sizeof(rect_test_data), false);
    data::reader reader(&block);

    quickdraw::rect<DWRD> rect(reader, quickdraw::coding_type::macintosh);
    test::equal(rect.origin.x, 32);
    test::equal(rect.origin.y, 128);
    test::equal(rect.size.width, 16);
    test::equal(rect.size.height, 64);
}

// MARK: - Static Reader

TEST(rect_readRectFromReader)
{
    data::block block(rect_test_data, sizeof(rect_test_data), false);
    data::reader reader(&block);

    auto rect = quickdraw::rect<DWRD>::read(reader, quickdraw::coding_type::quickdraw);
    test::equal(rect.origin.x, 128);
    test::equal(rect.origin.y, 32);
    test::equal(rect.size.width, 64);
    test::equal(rect.size.height, 16);
}

// MARK: - Encoding

TEST(rect_encodeInToWriter_quickdraw)
{
    data::writer writer(data::byte_order::msb);

    quickdraw::rect<DWRD> rect(128, 32, 64, 16);
    rect.encode(writer, quickdraw::coding_type::quickdraw);

    test::equal(writer.size(), sizeof(rect_test_data));
    test::bytes_equal(writer.data()->get<std::uint8_t *>(), rect_test_data, sizeof(rect_test_data));
}

TEST(rect_encodeInToWriter_macintosh)
{
    data::writer writer(data::byte_order::msb);

    quickdraw::rect<DWRD> rect(32, 128, 16, 64);
    rect.encode(writer, quickdraw::coding_type::macintosh);

    test::equal(writer.size(), sizeof(rect_test_data));
    test::bytes_equal(writer.data()->get<std::uint8_t *>(), rect_test_data, sizeof(rect_test_data));
}

// MARK: - Operators

TEST(rect_assignmentOperator_copyrect)
{
    quickdraw::rect<DWRD> rect(5);
    test::equal(rect.origin.x, 5);
    test::equal(rect.origin.y, 5);
    test::equal(rect.size.width, 5);
    test::equal(rect.size.height, 5);

    quickdraw::rect<DWRD> new_rect(10);
    rect = new_rect;
    test::equal(rect.origin.x, 10);
    test::equal(rect.origin.y, 10);
    test::equal(rect.size.width, 10);
    test::equal(rect.size.height, 10);
}

TEST(rect_assignmentOperator_moverect)
{
    quickdraw::rect<DWRD> rect(5);
    test::equal(rect.origin.x, 5);
    test::equal(rect.origin.y, 5);
    test::equal(rect.size.width, 5);
    test::equal(rect.size.height, 5);

    rect = quickdraw::rect<DWRD>(10);
    test::equal(rect.origin.x, 10);
    test::equal(rect.origin.y, 10);
    test::equal(rect.size.width, 10);
    test::equal(rect.size.height, 10);
}

TEST(rect_equalsOperator_shouldBeEqual)
{
    test::is_true(quickdraw::rect<DWRD>(5) == quickdraw::rect<DWRD>(5));
}

TEST(rect_equalsOperator_shouldNotBeEqual)
{
    test::is_false(quickdraw::rect<DWRD>(5) == quickdraw::rect<DWRD>(7));
}

TEST(rect_notEqualsOperator_shouldNotBeEqual)
{
    test::is_true(quickdraw::rect<DWRD>(5) != quickdraw::rect<DWRD>(7));
}

TEST(rect_notEqualsOperator_shouldBeEqual)
{
    test::is_false(quickdraw::rect<DWRD>(5) != quickdraw::rect<DWRD>(5));
}

TEST(rect_addRect_returnsExpectedrect)
{
    auto result = quickdraw::rect<DWRD>(5) + quickdraw::rect<DWRD>(3);
    test::equal(result.origin.x, 8);
    test::equal(result.origin.y, 8);
    test::equal(result.size.width, 8);
    test::equal(result.size.height, 8);
}

TEST(rect_subtractRect_returnsExpectedRect)
{
    auto result = quickdraw::rect<DWRD>(5) - quickdraw::rect<DWRD>(3);
    test::equal(result.origin.x, 2);
    test::equal(result.origin.y, 2);
    test::equal(result.size.width, 2);
    test::equal(result.size.height, 2);
}

TEST(rect_multiplyRect_returnsExpectedRect)
{
    auto result = quickdraw::rect<DWRD>(5) * 3;
    test::equal(result.origin.x, 15);
    test::equal(result.origin.y, 15);
    test::equal(result.size.width, 15);
    test::equal(result.size.height, 15);
}

TEST(rect_divideRect_returnsExpectedRect)
{
    auto result = quickdraw::rect<DWRD>(9) / 3;
    test::equal(result.origin.x, 3);
    test::equal(result.origin.y, 3);
    test::equal(result.size.width, 3);
    test::equal(result.size.height, 3);
}

// MARK: - Casting Tests

TEST(rect_castToDifferentType_retainsSameValue)
{
    quickdraw::rect<DWRD> rect(76);
    auto new_rect = rect.cast<DBYT>();
    test::equal(new_rect.origin.x, 76);
    test::equal(new_rect.origin.y, 76);
    test::equal(new_rect.size.width, 76);
    test::equal(new_rect.size.height, 76);
}