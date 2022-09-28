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
#include <cassert>
#include <cstring>
#include "libGraphite/data/data.hpp"
#include "libGraphite/data/simd.hpp"

// MARK: - Construction

graphite::data::block::block(std::size_t capacity, enum byte_order order)
    : m_raw_size(simd::expand_capacity(capacity)),
      m_data_size(capacity),
      m_raw(malloc(m_raw_size)),
      m_data(simd::align(m_raw)),
      m_allocation_owner(nullptr),
      m_byte_order(order),
      m_has_ownership(true)
{
}

graphite::data::block::block(std::size_t capacity, std::size_t allocation_size, enum byte_order order)
    : m_raw_size(simd::expand_capacity(allocation_size)),
      m_data_size(capacity),
      m_raw(malloc(m_raw_size)),
      m_data(simd::align(m_raw)),
      m_allocation_owner(nullptr),
      m_byte_order(order),
      m_has_ownership(true)
{
}

graphite::data::block::block(const std::string &path, enum byte_order order)
    : m_byte_order(order),
      m_allocation_owner(nullptr),
      m_has_ownership(true)
{
    std::ifstream file { path, std::ios::binary };
    if (!file.is_open() || file.fail()) {
        throw std::runtime_error("Failed to open data file: " + path);
    }

    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);
    m_data_size = file.tellg();
    file.seekg(0, std::ios::beg);

    m_raw_size = simd::expand_capacity(m_data_size);
    m_raw = malloc(m_raw_size);
    m_data = simd::align(m_raw);
    file.read(reinterpret_cast<char *>(m_data), m_data_size);

    file.close();
}

graphite::data::block::block(const std::vector<char>& bytes, enum byte_order order)
    : m_byte_order(order),
      m_allocation_owner(nullptr),
      m_has_ownership(true)
{
    m_data_size = bytes.size();
    m_raw_size = simd::expand_capacity(m_data_size);
    m_raw = malloc(m_raw_size);
    m_data = simd::align(m_raw);

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
      m_allocation_owner(copy ? nullptr : &source),
      m_has_ownership(copy),
      m_extended(source.m_extended)
{
    clone_from(source);
}

graphite::data::block::block(const block &source, block::position pos, std::size_t amount, bool copy)
    : m_allocation_owner(copy ? nullptr : &source),
      m_byte_order(source.m_byte_order),
      m_extended(source.m_extended)
{
    if (m_allocation_owner) {
        m_raw = source.m_raw;
        m_data = source.m_data;
        m_raw_size = source.m_raw_size;
        m_data_size = source.m_data_size;
        m_start_position = pos;
        m_count = amount;
        m_has_ownership = false;

        const_cast<block*>(m_allocation_owner)->m_users++;
    }
    else {
        m_raw_size = simd::expand_capacity(amount);
        m_data_size = amount;
        m_raw = malloc(m_raw_size);
        m_data = simd::align(m_raw);
        m_start_position = 0;
        m_count = 0;
        m_has_ownership = true;
    }
}

graphite::data::block::block(const void *data, std::size_t count, bool take_ownership, enum byte_order order)
    : m_allocation_owner(nullptr),
      m_has_ownership(take_ownership),
      m_raw(const_cast<void *>(data)),
      m_data(const_cast<void *>(data)),
      m_raw_size(count),
      m_data_size(count),
      m_start_position(0),
      m_count(count),
      m_byte_order(order)
{
}

graphite::data::block::block(const block &data)
    : m_raw_size(data.m_raw_size),
      m_data_size(data.m_data_size),
      m_allocation_owner(nullptr),
      m_start_position(data.m_start_position),
      m_users(0),
      m_count(data.m_count),
      m_byte_order(data.m_byte_order),
      m_has_ownership(data.m_has_ownership),
      m_extended(data.m_extended)
{
    if (m_has_ownership) {
        m_raw = malloc(m_raw_size);
        m_data = simd::align(m_raw);
        copy_from(data);
    }
    else {
        m_raw = data.m_raw;
        m_data = data.m_data;
    }
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
      m_byte_order(data.m_byte_order),
      m_has_ownership(data.m_has_ownership),
      m_extended(data.m_extended)
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

    // Clean up the current data...
    if (m_allocation_owner) {
        const_cast<struct block*>(m_allocation_owner)->m_users--;
    }
    else if ((!m_allocation_owner && m_has_ownership) || m_has_ownership) {
        assert(m_users == 0);
        free(m_raw);
    }

    m_raw_size = data.m_raw_size;
    m_data_size = data.m_data_size;
    m_allocation_owner = nullptr;
    m_start_position = data.m_start_position;
    m_users = 0;
    m_count = data.m_count;
    m_byte_order = data.m_byte_order;
    m_has_ownership = true;
    m_extended = data.m_extended;

    m_raw = malloc(m_raw_size);
    m_data = simd::align(m_raw);
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
        else if ((!m_allocation_owner && m_has_ownership) || m_has_ownership) {
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
        m_has_ownership = data.m_has_ownership;
        m_extended = data.m_extended;

        data.m_allocation_owner = nullptr;
        data.m_raw = nullptr;
        data.m_data = nullptr;
        data.m_has_ownership = false;

    }
    return *this;
}

// MARK: - Destruction

graphite::data::block::~block()
{
    if (m_allocation_owner) {
        const_cast<block*>(m_allocation_owner)->m_users--;
    }
    else if ((!m_allocation_owner && m_has_ownership) || m_has_ownership) {
        assert(m_users == 0);
        free(m_raw);
    }
}

// MARK: - Copy/Clone

auto graphite::data::block::clone_from(const graphite::data::block &source) -> void
{
    m_extended = source.m_extended;
    if (m_allocation_owner) {
        m_raw = source.m_raw;
        m_data = source.m_data;
        m_has_ownership = false;
        const_cast<block*>(m_allocation_owner)->m_users++;
    }
    else {
        m_raw = malloc(m_raw_size);
        m_data = simd::align(m_raw);
        m_has_ownership = true;
        copy_from(source);
    }
}

__attribute__((optnone)) auto graphite::data::block::copy_from(const block &source) -> void
{
    auto source_ptr = source.get<uint32_t *>();
    auto dest_ptr = get<uint32_t *>();

    m_extended = source.m_extended;

    std::size_t len = std::min(source.size(), size());
    memcpy(dest_ptr, source_ptr, len);
}

// MARK: - Operations

auto graphite::data::block::increase_size_to(std::size_t new_size) -> void
{
    if (new_size > m_raw_size) {
        throw std::runtime_error("Attempted to increase size of data::block beyond allowed range.");
    }
    m_data_size = new_size;
}

auto graphite::data::block::clear() -> void
{
    set((uint8_t)0, size());
}

auto graphite::data::block::set(uint8_t value, std::size_t bytes, block::position start) -> void
{
    memset(get<std::uint8_t *>(start), value, bytes);
}

__attribute__((optnone)) auto graphite::data::block::set(uint16_t value, std::size_t bytes, block::position start) -> void
{
    union simd::value v;
    v.fields[0] = (value << 16) | value;
#if __x86_64__
    v.fields[1] = v.fields[0];
    v.fields[2] = v.fields[0];
    v.fields[3] = v.fields[0];
#elif __arm64__
    v.fields[1] = v.fields[0];
#endif
    simd::set(get<std::uint32_t *>(start), size() - start, v, bytes);
}

// MARK: - Slicing

auto graphite::data::block::slice(block::position pos, std::size_t size, bool copy) const -> block
{
    return std::move(block(*this, m_start_position + pos, size, copy));
}
