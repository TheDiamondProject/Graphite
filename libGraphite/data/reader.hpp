// Copyright (c) 2020 Tom Hancocks
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

#if !defined(GRAPHITE_DATA_READER)
#define GRAPHITE_DATA_READER

#include <memory>
#include "libGraphite/data/data.hpp"

namespace graphite { namespace data {

/**
 * The `graphite::data::reader` class is used to read values from a
 * `graphite::data::data` object.
 */
    class reader
    {
    public:
        /**
         * The reading mode of the receiver.
         *
         * ::peek - peek at a value at the specified offset from the current
         *          position, without affecting the current position.
         * ::advance - advance the current position after reading the value at
         *          the specified offset from the current position.
         */
        enum mode { advance, peek };

    private:
        graphite::data::data::byte_order m_native_bo { data::byte_order::lsb };
        std::shared_ptr<data> m_data { nullptr };
        std::vector<uint64_t> m_pos_stack;
        uint64_t m_pos { 0 };

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto read_integer(int64_t offset, reader::mode mode = advance, int64_t size = -1) -> T;

        /**
         * Swap the bytes of an integer value from the source byte order to the specified
         * destination byte order.
         */
        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto swap(
            T value,
            enum graphite::data::data::byte_order value_bo,
            enum graphite::data::data::byte_order result_bo,
            int64_t size = -1
        ) -> T;

    public:

        /**
         * Construct a new `graphite::data::reader` object using the data in
         * the specified file.
         */
        reader(const std::string& path);

        /**
         * Construct a new `graphite::data::reader` object using the
         * specified `graphite::data::data` object, constraining the available
         * read region of the data.
         */
        reader(std::shared_ptr<data> data, uint64_t pos = 0);

        /**
         * Returns a shared copy of the raw data being used by the data slice.
         */
        auto get() -> std::shared_ptr<data>;

        /**
         * Returns the current size of the underlying `graphite::data::data` object
         * being referenced by the reader.
         *
         * Note: This size will be the _constrained_ size, if the reader was
         * constrained.
         */
        auto size() const -> std::size_t;

        /**
         * Reports if the position of the receiver is at or past the end of the
         * the underlying data.
         */
        auto eof() const -> bool;

        /**
         * Reports the current position of the receiver relative to its own
         * constraints.
         */
        auto position() const -> uint64_t;

        /**
         * Set the position of the receiver, relative to its own constraints.
         */
        auto set_position(uint64_t pos) -> void;

        /**
         * Move the current position of the receiver by the specified amount.
         */
        auto move(int64_t delta = 1) -> void;

        /**
         * Read a single unsigned byte from data.
         */
        auto read_byte(int64_t offset = 0, reader::mode mode = advance) -> uint8_t;

        /**
         * Read a single signed byte from data.
         */
        auto read_signed_byte(int64_t offset = 0, reader::mode mode = advance) -> int8_t;

        /**
         * Read a single unsigned short from data.
         */
        auto read_short(int64_t offset = 0, reader::mode mode = advance) -> uint16_t;

        /**
         * Read a single signed short from data.
         */
        auto read_signed_short(int64_t offset = 0, reader::mode mode = advance) -> int16_t;

        /**
         * Read a single unsigned triple from data. (3 bytes)
         */
        auto read_triple(int64_t offset = 0, reader::mode mode = advance) -> uint32_t;

        /**
         * Read a single unsigned long from data.
         */
        auto read_long(int64_t offset = 0, reader::mode mode = advance) -> uint32_t;

        /**
         * Read a single signed long from data.
         */
        auto read_signed_long(int64_t offset = 0, reader::mode mode = advance) -> int32_t;

        /**
         * Read a single unsigned quad from data.
         */
        auto read_quad(int64_t offset = 0, reader::mode mode = advance) -> uint64_t;

        /**
         * Read a single signed quad from data.
         */
        auto read_signed_quad(int64_t offset = 0, reader::mode mode = advance) -> int64_t;

        /**
         * Read a C-String from data.
         */
        auto read_cstr(int64_t size = -1, int64_t offset = 0, reader::mode mode = advance) -> std::string;

        /**
         * Read a Pascal String from data.
         */
        auto read_pstr(int64_t offset = 0, reader::mode mode = advance) -> std::string;

        /**
         * Read a chunk of data from the source data.
         */
        auto read_data(int64_t size, int64_t offset = 0, reader::mode mode = advance) -> std::shared_ptr<data>;

        /**
         * Read a series of bytes from the source data.
         */
        auto read_bytes(int64_t size, int64_t offset = 0, reader::mode mode = advance) -> std::vector<char>;

        /**
         * Save the current position of the reader.
         */
        auto save_position() -> void;

        /**
         * Restore a previous position of the reader.
         */
        auto restore_position() -> void;

    };

}}

#endif
