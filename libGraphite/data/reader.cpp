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

#include "libGraphite/data/reader.hpp"
#include "libGraphite/data/data.hpp"
#include "libGraphite/encoding/macroman/macroman.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>

// MARK: - Constructor

graphite::data::reader::reader(const std::string& path)
{
    // Attempt to open the file, and throw and exception if we failed to do so.
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open() || file.fail()) {
        throw std::runtime_error("Failed to open resource file: " + path);
    }
    
    // Make sure we don't skip newlines.
    file.unsetf(std::ios::skipws);
    
    // Get the size of the file.
    file.seekg(0UL, std::ios::end);
    auto file_size = file.tellg();
    file.seekg(0UL, std::ios::beg);
    
    // Read the contents of the file into the vector.
    auto data = std::make_shared<std::vector<char>>(file_size);
    file.read(&(*data)[0], file_size);
    m_data = std::make_shared<graphite::data::data>(data, file_size, graphite::data::data::byte_order::msb);
    
    // Close the file and clean up.
    file.close();
}

graphite::data::reader::reader(std::shared_ptr<graphite::data::data> data, uint64_t pos)
    : m_data(std::move(data)), m_pos(pos)
{

}

// MARK: - Byte Order

template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type*>
auto graphite::data::reader::swap(
    T value,
    enum graphite::data::data::byte_order value_bo,
    enum graphite::data::data::byte_order result_bo,
    uint64_t size
) -> T {
    // Return the value immediately if the value byte order matches the result byte order.
    if (value_bo == result_bo) {
        return value;
    }
    
    T v = 0;
    size = size == -1 ? sizeof(T) : size;
    
    for (decltype(size) i = 0; i < size; ++i) {
        auto b = (size - i - 1) << 3ULL;
        v |= ((value >> b) & 0xFF) << (i << 3ULL);
    }
    
    return v;
}


// MARK: - Internal Data

auto graphite::data::reader::get() -> std::shared_ptr<graphite::data::data>
{
    return m_data;
}

// MARK: - Size

auto graphite::data::reader::size() const -> std::size_t
{
    return m_data->size();
}

// MARK: - Position

auto graphite::data::reader::eof() const -> bool
{
    return (m_pos >= m_data->size());
}

auto graphite::data::reader::position() const -> uint64_t
{
    return m_pos;
}

auto graphite::data::reader::set_position(uint64_t pos) -> void
{
    m_pos = pos;
}

auto graphite::data::reader::move(int64_t delta) -> void
{
    // TODO: Bounds checking
    m_pos += delta;
}

auto graphite::data::reader::save_position() -> void
{
    m_pos_stack.push_back(m_pos);
}

auto graphite::data::reader::restore_position() -> void
{
    if (m_pos_stack.empty()) {
        throw std::logic_error("Attempted to restore reader position that did not exist.");
    }
    m_pos = m_pos_stack.back();
    m_pos_stack.pop_back();
}

// MARK: - Template Read

template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type*>
auto graphite::data::reader::read_integer(int64_t offset, graphite::data::reader::mode mode, uint64_t size) -> T
{
    T v = 0;
    size = size == -1 ? sizeof(T) : size;
    if (size <= 0 || size > sizeof(T)) {
        throw std::logic_error("Invalid integer read size specified.");
    }
    
    if (m_data->get() == nullptr) {
        throw std::runtime_error("Invalid data being read from.");
    }
    
    for (decltype(size) i = 0; i < size; ++i) {
        auto b = static_cast<uint8_t>(m_data->at(m_pos + offset + i));
        v |= static_cast<T>(b) << (i << 3ULL);
    }
    
    if (size > 1) {
        v = swap(v, m_data->current_byte_order(), m_native_bo, size);
    }
    
    if (mode == graphite::data::reader::mode::advance) {
        move(offset + size);
    }
    
    return v;
}

// MARK: - Read Integer Functions

auto graphite::data::reader::read_byte(int64_t offset, graphite::data::reader::mode mode) -> uint8_t
{
    return read_integer<uint8_t>(offset, mode);
}

auto graphite::data::reader::read_signed_byte(int64_t offset, graphite::data::reader::mode mode) -> int8_t
{
    return static_cast<int8_t>(read_byte(offset, mode));
}

auto graphite::data::reader::read_short(int64_t offset, graphite::data::reader::mode mode) -> uint16_t
{
    return read_integer<uint16_t>(offset, mode);
}

auto graphite::data::reader::read_signed_short(int64_t offset, graphite::data::reader::mode mode) -> int16_t
{
    return static_cast<int16_t>(read_short(offset, mode));
}

auto graphite::data::reader::read_triple(int64_t offset, graphite::data::reader::mode mode) -> uint32_t
{
    return read_integer<uint32_t>(offset, mode, 3);
}

auto graphite::data::reader::read_long(int64_t offset, graphite::data::reader::mode mode) -> uint32_t
{
    return read_integer<uint32_t>(offset, mode);
}

auto graphite::data::reader::read_signed_long(int64_t offset, graphite::data::reader::mode mode) -> int32_t
{
    return static_cast<int32_t>(read_long(offset, mode));
}

auto graphite::data::reader::read_quad(int64_t offset, graphite::data::reader::mode mode) -> uint64_t
{
    return read_integer<uint64_t>(offset, mode);
}

auto graphite::data::reader::read_signed_quad(int64_t offset, graphite::data::reader::mode mode) -> int64_t
{
    return static_cast<int64_t>(read_quad(offset, mode));
}

// MARK: - Read String Functions

auto graphite::data::reader::read_cstr(int64_t size, int64_t offset, graphite::data::reader::mode mode) -> std::string
{
    if (size == -1) {
        // Read until a NUL byte is encountered. This is the slowest form of
        // read, and will required stepping through bytes one by one.
        auto vec = std::vector<uint8_t>(0);
        auto i = 0;
        while (m_data->at(m_pos + offset + i)) {
            vec.push_back(static_cast<uint8_t>(m_data->at(m_pos + offset + i++)));
        }
        
        if (mode == reader::mode::advance) {
            m_pos += offset + vec.size() + 1;
        }
        
        return graphite::encoding::mac_roman::to_utf8(vec);
    }
    else {
        // Read a fixed chunk of memory and convert it to a string.
        auto data = read_bytes(size, offset, mode);
        std::vector<uint8_t> bytes(data.begin(), data.end());
        while (!bytes.empty() && bytes.back() == 0) {
            bytes.pop_back();
        }
        return graphite::encoding::mac_roman::to_utf8(bytes);
    }
}

auto graphite::data::reader::read_pstr(int64_t offset, graphite::data::reader::mode mode) -> std::string
{
    switch (mode) {
        case advance: {
            auto length = read_byte(offset, advance);
            return read_cstr(length, 0, advance);
        }
            
        case peek: {
            auto length = read_byte(offset, peek);
            return read_cstr(length, offset + 1, peek);
        }
    }
}

// MARK: - Read Data Functions

auto graphite::data::reader::read_data(int64_t size, int64_t offset, graphite::data::reader::mode mode) -> std::shared_ptr<graphite::data::data>
{
    auto data = std::make_shared<graphite::data::data>(m_data->get(), size, m_data->start() + m_pos + offset);
    move(offset + size);
    return data;
}

auto graphite::data::reader::read_bytes(int64_t size, int64_t offset, graphite::data::reader::mode mode) -> std::vector<char>
{
    char *start = &(*m_data->get())[m_data->relative_offset(m_pos + offset)];
    char *end = &(*m_data->get())[m_data->relative_offset(m_pos + offset + size)];
    
    if (mode == graphite::data::reader::mode::advance) {
        m_pos += offset + size;
    }
    
    return std::vector<char>(start, end);
}
