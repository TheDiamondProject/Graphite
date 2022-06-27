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

#include <stdexcept>
#include <fstream>
#include <iostream>
#include "libGraphite/data/data.hpp"

// MARK: - SIMD Support

#if __x86_64__
    typedef          __int128    int128_t;
    typedef unsigned __int128   uint128_t;

    typedef uint128_t           simd_wide_type;
    typedef simd_wide_type      simd_type;
    typedef uint64_t            simd_wide_field_type;

    static constexpr std::size_t simd_alignment_width = 16;
    static constexpr std::size_t simd_fields = 4;
    static constexpr std::size_t simd_wide_fields = 2;

#elif __arm64__
    typedef uint64_t            simd_wide_type;
    typedef simd_wide_type      simd_type;
    typedef simd_type           simd_wide_field_type;

    static constexpr std::size_t simd_alignment_width = 8;
    static constexpr std::size_t simd_fields = 2;
    static constexpr std::size_t simd_wide_fields = 1;

#else
    typedef uint32_t            simd_wide_type;
    typedef simd_wide_type      simd_type;

    static constexpr std::size_t simd_alignment_width = 4;
    static constexpr std::size_t simd_fields = 1;
    static constexpr std::size_t simd_wide_fields = 0;

#endif

typedef uint32_t            simd_field_type;
typedef uint16_t            simd_half_field_type;
static constexpr std::size_t simd_field_size = sizeof(simd_field_type);

union alignas(simd_alignment_width) simd_value
{
    simd_type wide;
#if __x86_64__ || __arm64__
    simd_wide_field_type wide_fields[simd_wide_fields];
#endif
    simd_field_type fields[simd_fields];
    uint8_t bytes[sizeof(simd_type)];
};

static constexpr std::size_t simd_alignment_mask = ~(simd_alignment_width - 1);

template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
static inline auto simd_expand_capacity(T capacity) -> T
{
    return (capacity + simd_alignment_width - 1) & simd_alignment_mask;
}

template<typename T, typename std::enable_if<std::is_pointer<T>::value>::type* = nullptr>
static inline auto simd_align(T ptr) -> T
{
    return reinterpret_cast<T>((reinterpret_cast<std::uintptr_t>(ptr) + simd_alignment_width - 1) & simd_alignment_mask);
}

// MARK: - Construction

graphite::data::block::block(std::size_t capacity, enum byte_order order)
    : m_raw_size(simd_expand_capacity(capacity)),
      m_data_size(capacity),
      m_raw(malloc(m_raw_size)),
      m_data(simd_align(m_raw)),
      m_allocation_owner(nullptr),
      m_byte_order(order)
{
}

graphite::data::block::block(const std::string &path, enum byte_order order)
    : m_byte_order(order),
      m_allocation_owner(nullptr)
{
    std::ifstream file { path, std::ios::binary };
    if (!file.is_open() || file.fail()) {
        throw std::runtime_error("Failed to open data file: " + path);
    }

    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);
    m_data_size = file.tellg();
    file.seekg(0, std::ios::beg);

    m_raw_size = simd_expand_capacity(m_data_size);
    m_raw = malloc(m_raw_size);
    m_data = simd_align(m_raw);
    file.read(reinterpret_cast<char *>(m_data), m_data_size);

    file.close();
}

graphite::data::block::block(const std::vector<char>& bytes, enum byte_order order)
    : m_byte_order(order),
      m_allocation_owner(nullptr)
{
    m_data_size = bytes.size();
    m_raw_size = simd_expand_capacity(m_data_size);
    m_raw = malloc(m_raw_size);
    m_data = simd_align(m_raw);

    // TODO: This is slow, and should be speeded up in the future.
    auto ptr = static_cast<char *>(m_data);
    for (const auto& byte : bytes) {
        *ptr++ = byte;
    }
}

graphite::data::block::block(const block &source, bool copy)
    : m_raw_size(source.m_raw_size),
      m_data_size(source.m_data_size),
      m_start_position(source.m_start_position),
      m_count(source.m_count),
      m_byte_order(source.m_byte_order),
      m_allocation_owner(copy ? nullptr : &source)
{
    clone_from(source);
}

graphite::data::block::block(const block &source, block::position pos, std::size_t amount, bool copy)
    : m_allocation_owner(copy ? nullptr : &source),
      m_byte_order(source.m_byte_order)
{
    if (m_allocation_owner) {
        m_raw = source.m_raw;
        m_data = source.m_data;
        m_raw_size = source.m_raw_size;
        m_data_size = source.m_data_size;
        m_start_position = pos;
        m_count = amount;

        const_cast<block*>(m_allocation_owner)->m_users++;
    }
    else {
        m_raw_size = simd_expand_capacity(amount);
        m_data_size = amount;
        m_raw = malloc(m_raw_size);
        m_data = simd_align(m_raw);
        m_start_position = 0;
        m_count = 0;
    }
}

graphite::data::block::block(const block &data)
    : m_raw_size(data.m_raw_size),
      m_data_size(data.m_data_size),
      m_allocation_owner(nullptr),
      m_start_position(data.m_start_position),
      m_users(data.m_users),
      m_count(data.m_count),
      m_byte_order(data.m_byte_order)
{
    m_raw = malloc(m_raw_size);
    m_data = simd_align(m_raw);
    copy_from(data);
}

graphite::data::block::block(block &&data) noexcept
    : m_raw_size(data.m_raw_size),
      m_data_size(data.m_data_size),
      m_raw(data.m_raw),
      m_data(data.m_data),
      m_allocation_owner(data.m_allocation_owner),
      m_start_position(data.m_start_position),
      m_users(data.m_users),
      m_count(data.m_count),
      m_byte_order(data.m_byte_order)
{
    data.m_raw = nullptr;
    data.m_data = nullptr;
    data.m_raw_size = 0;
    data.m_data_size = 0;
    data.m_users = 0;
    data.m_allocation_owner = nullptr;
}

auto graphite::data::block::operator=(const block &data) -> struct block &
{
    if (this == const_cast<block *>(&data)) {
        return *this;
    }

    m_raw_size = data.m_raw_size;
    m_data_size = data.m_data_size;
    m_allocation_owner = nullptr;
    m_start_position = data.m_start_position;
    m_users = data.m_users;
    m_count = data.m_count;
    m_byte_order = data.m_byte_order;

    m_raw = malloc(m_raw_size);
    m_data = simd_align(m_raw);
    copy_from(data);

    return *this;
}

auto graphite::data::block::operator=(block &&data) noexcept -> struct block &
{
    if (this != &data) {
        // Clean up the current data...
        if (m_allocation_owner) {
            const_cast<struct block*>(m_allocation_owner)->m_users--;
        }
        else if (!m_allocation_owner) {
            assert(m_users == 0);
            free(m_raw);
        }

        // Move over the data...
        m_raw_size = data.m_raw_size;
        m_data_size = data.m_data_size;
        m_raw = data.m_raw;
        m_data = data.m_data;
        m_allocation_owner = data.m_allocation_owner;
        m_start_position = data.m_start_position;
        m_users = data.m_users;
        m_count = data.m_count;
        m_byte_order = data.m_byte_order;

        data.m_allocation_owner = nullptr;
        data.m_raw = nullptr;
        data.m_data = nullptr;

    }
    return *this;
}

// MARK: - Destruction

graphite::data::block::~block()
{
    if (m_allocation_owner) {
        const_cast<block*>(m_allocation_owner)->m_users--;
    }
    else if (!m_allocation_owner) {
        assert(m_users == 0);
        free(m_raw);
    }
}

// MARK: - Copy/Clone

auto graphite::data::block::clone_from(const graphite::data::block &source) -> void
{
    if (m_allocation_owner) {
        m_raw = source.m_raw;
        m_data = source.m_data;
        const_cast<block*>(m_allocation_owner)->m_users++;
    }
    else {
        m_raw = malloc(m_raw_size);
        m_data = simd_align(m_raw);
        copy_from(source);
    }
}

auto graphite::data::block::copy_from(const block &source) const -> void
{
    auto source_ptr = source.get<uint32_t *>();
    auto dest_ptr = get<uint32_t *>();

    std::size_t len = std::min(source.size(), size());
    std::size_t n = 0;
    auto simd_fields_len = simd_fields * simd_field_size;
    while (n < len) {
        if ((reinterpret_cast<uintptr_t>(source_ptr) & simd_alignment_width) || (len - n) < simd_fields_len) {
            *dest_ptr = *source_ptr;
            ++dest_ptr;
            ++source_ptr;
            n += simd_field_size;
        }
        else {
            *reinterpret_cast<simd_wide_type *>(dest_ptr) = *reinterpret_cast<simd_wide_type *>(source_ptr);
            dest_ptr += simd_fields;
            source_ptr += simd_fields;
            n += simd_alignment_width;
        }
    }
}

// MARK: - Operations

static inline auto inline_set(graphite::data::block *dst, union simd_value v, std::size_t bytes, std::size_t start) -> void
{
    auto len = std::min(dst->size() - start, bytes);
    auto ptr = dst->get<uint32_t *>(start);
    std::size_t n = 0;

    auto simd_fields_len = simd_fields * simd_field_size;
    while (n < len) {
        if ((reinterpret_cast<uintptr_t>(ptr) & simd_alignment_width) || (len - n) < simd_fields_len) {
            *ptr = v.fields[n & (simd_fields - 1)];
            ++ptr;
            n += simd_field_size;
        }
        else {
            *reinterpret_cast<simd_wide_type *>(ptr) = v.wide;
            ptr += simd_fields;
            n += simd_alignment_width;
        }
    }
}

auto graphite::data::block::clear() -> void
{
    set((uint32_t)0);
}

auto graphite::data::block::set(uint8_t value, std::size_t bytes, block::position start) -> void
{
    union simd_value v;
    for (auto n = 0; n < simd_alignment_width; ++n) {
        v.bytes[n] = value;
    }
    inline_set(this, v, bytes, start);
}

auto graphite::data::block::set(uint16_t value, std::size_t bytes, block::position start) -> void
{
    union simd_value v;
    for (auto n = 0; n < simd_fields; ++n) {
        v.fields[n] = (value << 16) | value;
    }
    inline_set(this, v, bytes, start);
}

auto graphite::data::block::set(uint32_t value, std::size_t bytes, block::position start) -> void
{
    union simd_value v { 0 };
    for (auto n = 0; n < simd_fields; ++n) {
        v.fields[n] = value;
    }
    inline_set(this, v, bytes, start);
}

// MARK: - Slicing

auto graphite::data::block::slice(block::position pos, std::size_t size, bool copy) const -> block
{
    return std::move(block(*this, m_start_position + pos, size, copy));
}
