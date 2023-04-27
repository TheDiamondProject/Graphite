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

#include <vector>
#include <type_traits>
#include "libGraphite/data/data.hpp"
#include "libGraphite/util/concepts.hpp"
#include "libGraphite/data/encoding.hpp"
#include "libGraphite/data/internals/specialised_reader.hpp"
#include "libGraphite/data/internals/swap_reader.hpp"

namespace graphite::data
{
    class reader;

    template<class T>
    concept readable_resource_type = resource_type<T> && decodable<T>;

    class reader
    {
    public:
        enum class mode { advance, peek };

    public:
        explicit reader(const class block *data, block::position pos = 0, bool owns = false);
        ~reader();

        static auto file(const std::string& path, block::position pos = 0) -> reader;

        [[nodiscard]] inline auto data() const -> const class block * { return m_data; }
        [[nodiscard]] inline auto owns_data() const -> bool { return m_owns_data; }

        [[nodiscard]] inline auto position() const -> block::position { return m_position; }
        [[nodiscard]] inline auto size() const -> std::size_t { return m_data->size(); }
        [[nodiscard]] inline auto eof() const -> bool { return position() >= size(); }
        [[nodiscard]] inline auto byte_order() const -> byte_order { return m_data->byte_order(); }

        auto change_byte_order(enum byte_order order) -> void
        {
            const_cast<class block *>(m_data)->change_byte_order(order);
            update_swap_wrapper();
        }

        auto update_swap_wrapper() -> void
        {
            if (m_data && byte_order() == native_byte_order()) {
                m_swap_wrapper = std::make_shared<internals::specialised_reader>();
            }
            else {
                m_swap_wrapper = std::make_shared<internals::swap_reader>();
            }
        }

        auto set_position(block::position pos) -> void;
        auto move(block::position delta = 1) -> void;

        auto save_position() -> void;
        auto restore_position() -> void;

        template<typename T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value>>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> T { return 0; }

        template<std::int8_t>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> std::int8_t { return read_signed_byte(offset, mode); }

        template<std::int16_t>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> std::int16_t { return read_signed_short(offset, mode); }

        template<std::int32_t>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> std::int32_t { return read_signed_long(offset, mode); }

        template<std::int64_t>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> std::int64_t { return read_signed_quad(offset, mode); }

        template<typename T, typename std::enable_if<std::is_integral<T>::value>>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> T { return 0; }

        template<std::uint8_t>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> std::uint8_t { return read_byte(offset, mode); }

        template<std::uint16_t>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> std::uint16_t { return read_short(offset, mode); }

        template<std::uint32_t>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> std::uint32_t { return read_long(offset, mode); }

        template<std::uint64_t>
        auto read_integer(block::position offset = 0, mode mode = mode::advance) -> std::uint64_t { return read_quad(offset, mode); }

        auto read_byte(block::position offset = 0, mode mode = mode::advance) -> uint8_t;
        auto read_signed_byte(block::position offset = 0, mode mode = mode::advance) -> int8_t;

        auto read_short(block::position offset = 0, mode mode = mode::advance) -> uint16_t;
        auto read_signed_short(block::position offset = 0, mode mode = mode::advance) -> int16_t;

        auto read_fixed_point(block::position offset = 0, mode mode = mode::advance) -> double;

        auto read_triple(block::position offset = 0, mode mode = mode::advance) -> uint32_t;

        auto read_long(block::position offset = 0, mode mode = mode::advance) -> uint32_t;
        auto read_signed_long(block::position offset = 0, mode mode = mode::advance) -> int32_t;

        auto read_quad(block::position offset = 0, mode mode = mode::advance) -> uint64_t;
        auto read_signed_quad(block::position offset = 0, mode mode = mode::advance) -> int64_t;

        auto read_cstr(std::size_t length = 0, block::position offset = 0, mode mode = mode::advance) -> std::string;
        auto read_pstr(block::position offset = 0, mode mode = mode::advance) -> std::string;

        auto read_data(std::size_t length, block::position offset = 0, mode mode = mode::advance) -> class block;
        auto read_bytes(std::size_t length, block::position offset = 0, mode mode = mode::advance) -> std::vector<char>;

        template<decodable T>
        auto read(block::position offset = 0, mode mode = mode::advance) -> T
        {
            if (mode == mode::peek) {
                save_position();
            }

            move(offset);
            graphite::data::decodable auto object = T(*this);

            if (mode == mode::peek) {
                restore_position();
            }

            return std::move(object);
        }

        template<decompressible_block T>
        auto read_compressed_data(std::size_t length, block::position offset = 0, mode mode = mode::advance) -> class block
        {
            return T::decompress(std::move(read_data(length, offset, mode)));
        }

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto read_integer(block::position offset = 0, mode mode = mode::advance, std::size_t size = sizeof(T)) -> T
        {
            T v = *reinterpret_cast<T *>(m_data->template get<std::uint8_t *>(m_position + offset));
            v = m_swap_wrapper->swap(v, size);

            if (mode == mode::advance) {
                move(offset + size);
            }

            return v;
        }

        template<typename E, typename std::enable_if<std::is_enum<E>::value>::type* = nullptr>
        auto read_enum(block::position offset = 0, mode mode = mode::advance, std::size_t size = sizeof(E)) -> E
        {
            if (sizeof(E) == sizeof(std::uint8_t)) {
                return static_cast<E>(read_byte(offset, mode));
            }
            else if (sizeof(E) == sizeof(std::uint16_t)) {
                return static_cast<E>(read_short(offset, mode));
            }
            else if (sizeof(E) == sizeof(std::uint32_t)) {
                return static_cast<E>(read_long(offset, mode));
            }
            else if (sizeof(E) == sizeof(std::uint64_t)) {
                return static_cast<E>(read_quad(offset, mode));
            }
        }

    private:
        bool m_owns_data { false };
        const class block *m_data { nullptr };
        std::shared_ptr<internals::specialised_reader> m_swap_wrapper;
        std::vector<block::position> m_position_stack;
        block::position m_position;
    };

}
