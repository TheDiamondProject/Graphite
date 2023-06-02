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

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <libData/reader.hpp>
#include <libData/endianess.hpp>
#include <libEncoding/macroman/macroman.hpp>

// MARK: - Construction

data::reader::reader(const class block *data, block::position pos, bool owns)
    : m_data(data),
      m_owns_data(owns),
      m_position(pos)
{
    update_swap_wrapper();
}

auto data::reader::file(const std::string &path, block::position pos) -> reader
{
    auto data = new class block(path, byte_order::msb);
    return reader(data, pos, true);
}

// MARK: - Destruction

data::reader::~reader()
{
    if (m_owns_data) {
        delete m_data;
    }
}

// MARK: - Position Management

auto data::reader::set_position(block::position pos) -> void
{
    m_position = static_cast<block::position>(std::max(static_cast<std::size_t>(0), std::min(static_cast<std::size_t>(pos), size() + 1)));
}

auto data::reader::move(block::position delta) -> void
{
    set_position(position() + delta);
}

auto data::reader::save_position() -> void
{
    m_position_stack.emplace_back(m_position);
}

auto data::reader::restore_position() -> void
{
    if (m_position_stack.empty()) {
        throw std::runtime_error("Attempted to restore position of data reader, when no saved positions exist.");
    }
    set_position(m_position_stack.back());
    m_position_stack.pop_back();
}

// MARK: - Read Operations

auto data::reader::read_byte(block::position offset, mode mode) -> std::uint8_t
{
    return read_integer<std::uint8_t>(offset, mode);
}

auto data::reader::read_signed_byte(block::position offset, mode mode) -> std::int8_t
{
    return read_integer<std::int8_t>(offset, mode);
}

auto data::reader::read_short(block::position offset, mode mode) -> std::uint16_t
{
    return read_integer<std::uint16_t>(offset, mode);
}

auto data::reader::read_signed_short(block::position offset, mode mode) -> std::int16_t
{
    return read_integer<std::int16_t>(offset, mode);
}

auto data::reader::read_fixed_point(block::position offset, mode mode) -> double
{
    return static_cast<double>(read_signed_long(offset, mode)) / static_cast<double>(1 << 16);
}

auto data::reader::read_triple(block::position offset, mode mode) -> std::uint32_t
{
    return read_integer<std::uint32_t>(offset, mode, 3);
}

auto data::reader::read_long(block::position offset, mode mode) -> std::uint32_t
{
    return read_integer<std::uint32_t>(offset, mode);
}

auto data::reader::read_signed_long(block::position offset, mode mode) -> std::int32_t
{
    return read_integer<std::int32_t>(offset, mode);
}

auto data::reader::read_quad(block::position offset, mode mode) -> std::uint64_t
{
    return read_integer<std::uint64_t>(offset, mode);
}

auto data::reader::read_signed_quad(block::position offset, mode mode) -> std::int64_t
{
    return read_integer<std::int64_t>(offset, mode);
}

auto data::reader::read_pstr(block::position offset, mode mode) -> std::string
{
    switch (mode) {
        case mode::advance: {
            auto length = read_byte(offset, mode);
            return length == 0 ? "" : read_cstr(length, 0, mode);
        }
        case mode::peek: {
            auto length = read_byte(offset, mode);
            return length == 0 ? "" : read_cstr(length, offset + 1, mode);
        }
        default: {
            return "";
        }
    }
}

auto data::reader::read_cstr(std::size_t length, block::position offset, mode mode) -> std::string
{
    std::vector<std::uint8_t> bytes;

    if (length == 0) {
        block::position i = 0;
        while (read_byte(offset + i, mode::peek)) {
            bytes.push_back(read_byte(offset + i++, mode::peek));
        }

        if (mode == mode::advance) {
            move(offset + bytes.size() + 1);
        }
    }
    else {
        auto data = std::move(read_bytes(length, offset, mode));
        for (const auto& byte : data) {
            if (byte == '\0') {
                break;
            }
            bytes.emplace_back(static_cast<std::uint8_t>(byte));
        }
    }

    return encoding::mac_roman::to_utf8(bytes);
}

auto data::reader::read_data(std::size_t length, block::position offset, mode mode) -> class block
{
    auto sliced = std::move(m_data->slice(m_position + offset, length));
    if (mode == mode::advance) {
        move(offset + length);
    }
    return std::move(sliced);
}

auto data::reader::read_bytes(std::size_t length, block::position offset, mode mode) -> std::vector<char>
{
    auto start = m_data->get<char *>(m_position + offset);
    move(offset + length);

    std::vector<char> bytes(std::max(length, static_cast<std::size_t>(28)), '\0');
    for (auto idx = 0; idx < length; ++idx) {
        bytes[idx] = start[idx];
    }
    return std::move(bytes);
}