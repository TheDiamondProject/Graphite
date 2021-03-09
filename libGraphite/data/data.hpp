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

#if !defined(GRAPHITE_DATA)
#define GRAPHITE_DATA

#include <memory>
#include <vector>
#include <string>
#include <type_traits>
#include <stdexcept>

namespace graphite::data {

/**
 * The `graphite::data::data` class is used to store binary data in memory,
 * and can be written to and read from disk.
 */
    class data: public std::enable_shared_from_this<data>
    {
    public:
        /**
         * The byte order represents the endianness of a data value.
         */
        enum byte_order { msb, lsb };

    private:
        enum data::byte_order m_bo { msb };
        std::shared_ptr<std::vector<char>> m_data { nullptr };
        std::size_t m_start { 0 };
        std::size_t m_size { 0 };

    public:
        /**
         * Construct a new empty `graphite::data::data` object.
         */
        explicit data(enum data::byte_order bo = msb);

        /**
         * Construct a new `graphite::data::data` object with the specified capacity.
         */
        explicit data(std::size_t capacity, enum data::byte_order bo = msb);

        /**
         * Construct a new `graphite::data::data` object with the provided data.
         */
        data(std::shared_ptr<std::vector<char>> bytes, std::size_t size, std::size_t start = 0, enum data::byte_order bo = msb);

        /**
         * Calculate an offset within the receiver's data.
         */
        auto relative_offset(int64_t offset) const -> int64_t;

        /**
         * Returns the current size of the receiver.
         */
        auto size() const -> std::size_t;

        /**
         * Returns the start location of the receiver.
         */
        auto start() const -> std::size_t;

        /**
         * Returns the intended byte order of the receiver.
         */
        auto current_byte_order() const -> enum data::byte_order;

        /**
         * Set a new byte order for the receiver.
         *
         * Warning: This can cause corruption of values being read or written if the
         * byte order is set incorrectly.
         */
        auto set_byte_order(enum data::byte_order bo) -> void;

        /**
         * Returns a pointer to the internal data vector of the receiver.
         */
        auto get() -> std::shared_ptr<std::vector<char>>;

        /**
         * Returns the element at the specified index.
         */
        auto at(std::size_t offset) const -> char;

    };

}

#endif
