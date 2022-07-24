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

namespace graphite::data
{
    class writer
    {
    public:
        explicit writer(enum byte_order order = native_byte_order());
        explicit writer(const class block *data);

        ~writer();

        [[nodiscard]] inline auto data() const -> const class block * { return reinterpret_cast<const class block *>(m_data); };
        [[nodiscard]] inline auto owns_data() const -> bool { return m_owns_data; }

        [[nodiscard]] inline auto position() const -> block::position { return m_position; }
        [[nodiscard]] inline auto size() const -> std::size_t { return m_data->size(); }

        auto change_byte_order(enum byte_order order) -> void { const_cast<class block *>(m_data)->change_byte_order(order); }

        auto expand_storage(std::size_t amount) -> void;
        auto ensure_required_space(block::position position, std::size_t amount) -> void;

        auto set_position(block::position pos) -> void;
        auto move(block::position delta = 1) -> void;

        template<typename T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value>>
        auto write_integer(T value, std::size_t count = 1) -> void {}

        template<std::int8_t>
        auto write_integer(std::int8_t value, std::size_t count = 1) -> void { write_signed_byte(value, count); }

        template<std::int16_t>
        auto write_integer(std::int16_t value, std::size_t count = 1) -> void { write_signed_short(value, count); }

        template<std::int32_t>
        auto write_integer(std::int32_t value, std::size_t count = 1) -> void { write_signed_long(value, count); }

        template<std::int64_t>
        auto write_integer(std::int64_t value, std::size_t count = 1) -> void { write_signed_quad(value, count); }

        template<typename T, typename std::enable_if<std::is_integral<T>::value>>
        auto write_integer(T value, std::size_t count = 1) -> void {}

        template<std::uint8_t>
        auto write_integer(std::uint8_t value, std::size_t count = 1) -> void { write_byte(value, count); }

        template<std::uint16_t>
        auto write_integer(std::uint16_t value, std::size_t count = 1) -> void { write_short(value, count); }

        template<std::uint32_t>
        auto write_integer(std::uint32_t value, std::size_t count = 1) -> void { write_long(value, count); }

        template<std::uint64_t>
        auto write_integer(std::uint64_t value, std::size_t count = 1) -> void { write_quad(value, count); }

        auto write_byte(std::uint8_t value, std::size_t count = 1) -> void;
        auto write_signed_byte(std::int8_t value, std::size_t count = 1) -> void;

        auto write_short(std::uint16_t value, std::size_t count = 1) -> void;
        auto write_signed_short(std::int16_t value, std::size_t count = 1) -> void;

        auto write_fixed_point(double value, std::size_t count = 1) -> void;

        auto write_triple(std::uint32_t value, std::size_t count = 1) -> void;

        auto write_long(std::uint32_t value, std::size_t count = 1) -> void;
        auto write_signed_long(std::int32_t value, std::size_t count = 1) -> void;

        auto write_quad(std::uint64_t value, std::size_t count = 1) -> void;
        auto write_signed_quad(std::int64_t value, std::size_t count = 1) -> void;

        auto write_pstr(const std::string& str) -> std::size_t;
        auto write_cstr(const std::string& str, std::size_t size = 0) -> std::size_t;

        auto write_bytes(const std::vector<uint8_t>& bytes) -> void;
        auto write_bytes(const std::vector<char>& bytes) -> void;

        auto write_data(const class block *data) -> void;

        template<compressible_block T>
        auto write_compressed_data(const class block *data) -> class block
        {
            return T::compress(data);
        }

        template<encodable T>
        auto write(T& value) -> void
        {
            value.encode(*this);
        }

        auto pad_to_size(std::size_t size) -> void;

        auto save(const std::string& path, std::size_t size = 0) const -> void;

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto write_integer(T value, std::size_t count = 1, std::size_t size = sizeof(T)) -> void
        {
            auto swapped = swap(value, native_byte_order(), m_data->byte_order());

            ensure_required_space(position(), size * count);
            auto ptr = m_data->template get<uint8_t *>(position());

            // Correct the alignment.
            if (m_data->byte_order() == byte_order::msb) {
                for (auto i = size; i < sizeof(swapped); ++i) {
                    swapped >>= 8;
                }
            }
            else {
                for (auto i = size; i < sizeof(swapped); ++i) {
                    swapped <<= 8;
                }
            }


            for (auto n = 0; n < count; ++n) {
                for (auto i = 0; i < size; ++i) {
                    auto b = i << 3ULL;
                    *ptr++ = (swapped >> b) & 0xFF;
                }
            }

            move(size * count);
        }

        template<typename E, typename std::enable_if<std::is_enum<E>::value>::type* = nullptr>
        auto write_enum(E value, std::size_t count = 1, std::size_t size = sizeof(E)) -> void
        {
            if (sizeof(E) == sizeof(std::uint8_t)) {
                write_byte(static_cast<std::uint8_t>(value), count);
            }
            else if (sizeof(E) == sizeof(std::uint16_t)) {
                write_short(static_cast<std::uint16_t>(value), count);
            }
            else if (sizeof(E) == sizeof(std::uint32_t)) {
                write_long(static_cast<std::uint32_t>(value), count);
            }
            else if (sizeof(E) == sizeof(std::uint64_t)) {
                write_quad(static_cast<std::uint64_t>(value), count);
            }
        }

    private:
        bool m_owns_data { false };
        const class block *m_data { nullptr };
        block::position m_position { 0 };
    };
}

