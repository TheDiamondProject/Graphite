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

#include <memory>
#include "libGraphite/data/data.hpp"

#if !defined(GRAPHITE_DATA_WRITER)
#define GRAPHITE_DATA_WRITER

namespace graphite::data {

/**
 * The `graphite::data::writer` class is used to write values into a
 * `graphite::data::data` object.
 */
    class writer
    {
    private:
        graphite::data::byte_order m_native_bo { lsb };
        std::shared_ptr<graphite::data::data> m_data { nullptr };
        uint64_t m_pos { 0 };

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto write_integer(T value) -> void;

        /**
         * Swap the bytes of an integer value from the source byte order to the specified
         * destination byte order.
         */
        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto swap(
                T value,
                enum graphite::data::byte_order value_bo,
                enum graphite::data::byte_order result_bo
        ) -> T;

    public:
        /**
         * Construct a new `graphite::data::writer` object and the underlying
         * `graphite::data::data` object.
         */
        writer();

        /**
         * Construct a new `graphite::data::writer` object using the specified
         * `graphite::data::data` object.
         */
        explicit writer(std::shared_ptr<graphite::data::data> data);

        /**
         * Returns the internal data object.
         */
        auto data() -> std::shared_ptr<graphite::data::data>;

        /**
         * Returns the current size of the underlying `graphite::data::data:: object.
         */
        [[nodiscard]] auto size() const -> std::size_t;

        /**
         * Returns the current insertion position of the receiver within the underlying
         * `graphite::data::data` object.
         */
        [[nodiscard]] auto position() const -> uint64_t;

        /**
         * Set the insertion position of the receiver within the underlying
         * `graphite::data::data` object.
         */
        auto set_position(uint64_t pos) -> void;

        /**
         * Move the current insertion position of the receiver by the specified amount.
         */
        auto move(int64_t delta = 1) -> void;

        /**
         * Write a single unsigned byte into the data.
         */
        auto write_byte(uint8_t value, std::size_t count = 1) -> void;

        /**
         * Write a singled signed byte into the data.
         */
        auto write_signed_byte(int8_t value) -> void;

        /**
         * Write a single unsigned short into the data.
         */
        auto write_short(uint16_t value) -> void;

        /**
         * Write a singled signed short into the data.
         */
        auto write_signed_short(int16_t value) -> void;

        /**
         * Write a single unsigned long into the data.
         */
        auto write_long(uint32_t value) -> void;

        /**
         * Write a singled signed long into the data.
         */
        auto write_signed_long(int32_t value) -> void;

        /**
         * Write a single unsigned quad into the data.
         */
        auto write_quad(uint64_t value) -> void;

        /**
         * Write a singled signed quad into the data.
         */
        auto write_signed_quad(int64_t value) -> void;

        /**
         * Write a C-String into the data.
         */
        auto write_cstr(const std::string& str, std::size_t size = 0) -> void;

        /**
         * Write a Pascal String into the data.
         */
        auto write_pstr(const std::string& str) -> void;

        /**
         * Write a series of bytes into the data.
         */
        auto write_bytes(const std::vector<uint8_t>& bytes) -> void;
        auto write_bytes(const std::vector<char>& bytes) -> void;

        /**
         * Write the specified `graphite::data::data` object into the output
         * data stream.
         */
        auto write_data(const std::shared_ptr<graphite::data::data>& data) -> void;

        /**
         * Pad the contents of the underlying data object to the specified number of bytes.
         */
        auto pad_to_size(std::size_t size) -> void;

        /**
         * Write the contents of the data to the specified file.
         */
        auto save(const std::string& path) const -> void;

    };

}

#endif
