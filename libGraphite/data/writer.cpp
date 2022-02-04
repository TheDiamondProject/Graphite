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

#include "libGraphite/data/writer.hpp"
#include "libGraphite/data/data.hpp"
#include "libGraphite/encoding/macroman/macroman.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>

// MARK: - Constructors

graphite::data::writer::writer()
    : m_data(std::make_shared<graphite::data::data>())
{
    
}

graphite::data::writer::writer(std::shared_ptr<graphite::data::data> data)
    : m_data(std::move(data))
{
    
}

// MARK: - Byte Order

template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type*>
auto graphite::data::writer::swap(
    T value,
    enum graphite::data::byte_order value_bo,
    enum graphite::data::byte_order result_bo
) -> T {
    // Return the value immediately if the value byte order matches the result byte order.
    if (value_bo == result_bo) {
        return value;
    }
    
    T v = 0;
    unsigned int size = sizeof(T);
    
    for (unsigned int i = 0; i < size; ++i) {
        auto b = (size - i - 1) << 3U;
        v |= ((value >> b) & 0xFF) << (i << 3U);
    }
    
    return v;
}

// MARK: - Data

auto graphite::data::writer::data() -> std::shared_ptr<graphite::data::data>
{
    return m_data;
}

// MARK: - Size

auto graphite::data::writer::size() const -> std::size_t
{
    return m_data->size();
}

// MARK: - Position

auto graphite::data::writer::position() const -> uint64_t
{
    return m_pos;
}

auto graphite::data::writer::set_position(uint64_t pos) -> void
{
    m_pos = pos;
}

auto graphite::data::writer::move(int64_t delta) -> void
{
    m_pos += delta;
    auto data = m_data->get();
    if (m_pos > data->size()) {
        data->resize(m_pos, 0);
    }
}

// MARK: - Template Write

template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type*>
auto graphite::data::writer::write_integer(T value) -> void
{
    unsigned int size = sizeof(T);
    auto swapped = swap(value, m_native_bo, m_data->current_byte_order());
    auto data = m_data->get();
    
    for (unsigned int i = 0; i < size; ++i) {
        auto b = i << 3U;
        
        // Two modes for writing. If we're at the very end of data then we need to insert.
        // If we're not at the end of the data then we can either overwrite the current byte
        // or insert a new byte.
        if (m_pos >= data->size()) {
            data->push_back((swapped >> b) & 0xFF);
            m_pos++;
        }
        else {
            (*data)[m_pos++] = (swapped >> b) & 0xFF;
        }
    }

    m_data->resync_size();
}


// MARK: - Write Integer Functions

auto graphite::data::writer::write_byte(uint8_t value, std::size_t count) -> void
{
    for (auto i = 0; i < count; ++i) {
        write_integer<uint8_t>(value);
    }
}

auto graphite::data::writer::write_signed_byte(int8_t value) -> void
{
    write_byte(static_cast<uint8_t>(value));
}

auto graphite::data::writer::write_short(uint16_t value) -> void
{
    write_integer<uint16_t>(value);
}

auto graphite::data::writer::write_signed_short(int16_t value) -> void
{
    write_short(static_cast<uint16_t>(value));
}

auto graphite::data::writer::write_long(uint32_t value) -> void
{
    write_integer<uint32_t>(value);
}

auto graphite::data::writer::write_signed_long(int32_t value) -> void
{
    write_long(static_cast<uint32_t>(value));
}

auto graphite::data::writer::write_quad(uint64_t value) -> void
{
    write_integer<uint64_t>(value);
}

auto graphite::data::writer::write_signed_quad(int64_t value) -> void
{
    write_quad(static_cast<uint64_t>(value));
}

// MARK: - Write Strings

auto graphite::data::writer::write_cstr(const std::string& str, std::size_t size) -> void
{
    std::vector<uint8_t> bytes;
    
    if (size == 0) {
        // NUL Terminated C-String.
        bytes = graphite::encoding::mac_roman::from_utf8(str);
        bytes.push_back(0);
    }
    else {
        // Fixed length C-String
        bytes = graphite::encoding::mac_roman::from_utf8(str);
        bytes.resize(size, 0x00);
    }
    
    write_bytes(bytes);
}

auto graphite::data::writer::write_pstr(const std::string& str) -> void
{
    auto bytes = graphite::encoding::mac_roman::from_utf8(str);
    
    if (bytes.size() > 0xFF) {
        bytes.resize(0xFF);
    }
    auto size = static_cast<uint8_t>(bytes.size());
    write_byte(static_cast<uint8_t>(size));
    write_bytes(bytes);
}

// MARK: - Write Bytes

 auto graphite::data::writer::write_bytes(const std::vector<uint8_t>& bytes) -> void
{
    std::vector<char> converted_bytes(bytes.begin(), bytes.end());
    write_bytes(converted_bytes);
}

 auto graphite::data::writer::write_bytes(const std::vector<char>& bytes) -> void
{
    auto vec = m_data->get();
    vec->insert(vec->end(), bytes.begin(), bytes.end());
    m_pos += bytes.size();
    m_data->resync_size();
}

auto graphite::data::writer::write_data(const std::shared_ptr<graphite::data::data>& data) -> void
{
    auto bytes = data->get();
    auto vec = m_data->get();
    vec->insert(vec->end(), bytes->begin() + data->start(), bytes->begin() + data->start() + data->size());
    m_pos += data->size();
    m_data->resync_size();
}

auto graphite::data::writer::pad_to_size(std::size_t size) -> void
{
    while (m_data->size() < size) {
        write_byte(0);
    }
}

// MARK: - Saving

auto graphite::data::writer::save(const std::string& path) const -> void
{
    std::ofstream f(path, std::ios::out | std::ios::binary);
    auto data = m_data->get()->data();
    f.write(data, m_data->size());
    f.close();
}
