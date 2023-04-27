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

#include <cstdint>
#include <memory>
#include <vector>
#include <type_traits>
#include <concepts>
#include "libGraphite/data/endianess.hpp"
//#include "libGraphite/data/simd.hpp"

namespace graphite::data
{
    struct block;

    template<class T>
    concept compressible_block = requires(const T& block, const data::block& uncompressed) {
        { T::compress(uncompressed) } -> std::same_as<data::block>;
    };

    template<class T>
    concept decompressible_block = requires(const T& block, const data::block& compressed) {
        { T::decompress(compressed) } -> std::same_as<data::block>;
    };

    struct block
    {
    public:
        typedef std::int64_t position;

    public:
        block() = default;
        explicit block(std::size_t capacity, enum byte_order order = byte_order::msb);
        block(std::size_t capacity, std::size_t allocation_size, enum byte_order order = byte_order::msb);
        explicit block(const std::string& path, enum byte_order order = byte_order::msb);
        explicit block(const std::vector<char>& bytes, enum byte_order order = byte_order::msb);
        block(const void *data, std::size_t count, bool take_ownership = true, enum byte_order order = byte_order::msb);
        block(const block& source);
        block(const block& source, bool copy);
        block(const block& source, block::position pos, std::size_t count, bool copy = true);

        block(block&& data) noexcept;

        auto operator=(const block& data) -> struct block&;
        auto operator=(block&& data) noexcept -> struct block&;

        ~block();

        template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
        [[nodiscard]] inline auto get(block::position offset = 0) const -> T { return reinterpret_cast<T>(get_offset_data(offset)); }

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        [[nodiscard]] inline auto get(block::position offset = 0) const -> T { return swap(*get<T *>(offset), m_byte_order); }

        [[nodiscard]] inline auto raw_size() const -> std::size_t { return m_raw_size; }
        [[nodiscard]] inline auto size() const -> std::size_t { return m_count > 0 ? m_count : m_data_size; }
        [[nodiscard]] inline auto start() const -> block::position { return m_start_position; }
        [[nodiscard]] inline auto byte_order() const -> byte_order { return m_byte_order; }
        [[nodiscard]] inline auto has_ownership() const -> bool { return m_has_ownership; }

        auto originates_from_extended_format() -> void { m_extended = true; }
        [[nodiscard]] inline auto is_extended_format() const -> bool { return m_extended; }

        auto change_byte_order(enum byte_order order) -> void { m_byte_order = order; }
        auto increase_size_to(std::size_t new_size) -> void;

        auto clear() -> void;

        auto set(std::uint8_t value, std::size_t bytes = 0, block::position start = 0) -> void;
        auto set(std::uint16_t value, std::size_t bytes = 0, block::position start = 0) -> void;
        auto set(uint32_t value, std::size_t bytes = 0, block::position start = 0) -> void;

        auto copy_from(const block& source) -> void;

        [[nodiscard]] auto slice(block::position pos, std::size_t size, bool copy = false) const -> block;

    private:
        enum byte_order m_byte_order { byte_order::msb };
        bool m_extended { false };
        std::size_t m_raw_size { 0 };
        std::size_t m_data_size { 0 };
        block::position m_start_position { 0 };
        std::size_t m_count { 0 };
        void *m_raw { nullptr };
        void *m_data { nullptr };

        bool m_has_ownership { false };
        const block *m_allocation_owner { nullptr };
        std::uint32_t m_users { 0 };

        [[nodiscard]] inline auto get_offset_data(block::position offset) const -> void *
        {
            return reinterpret_cast<void *>(reinterpret_cast<block::position>(m_data) + m_start_position + offset);
        }

        auto clone_from(const graphite::data::block& source) -> void;
    };
}

