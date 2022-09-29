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
#include <cassert>
#include "libGraphite/data/writer.hpp"
#include "libGraphite/encoding/macroman/macroman.hpp"

// MARK: - Construction

graphite::data::writer::writer(enum byte_order order)
    : m_owns_data(true),
      m_data(new class block(0, order))
{

}

graphite::data::writer::writer(const class block *data)
    : m_owns_data(false),
      m_data(data)
{

}

// MARK: - Destruction

graphite::data::writer::~writer()
{
    if (m_owns_data) {
        delete m_data;
    }
}

// MARK: - Storage Management

auto graphite::data::writer::expand_storage(std::size_t amount) -> void
{
    // NOTE: We only allow resizing of data objects that we own.
    assert(m_owns_data);

    const_cast<class block *>(m_data)->increase_size_to(size() + amount);

}

auto graphite::data::writer::ensure_required_space(block::position position, std::size_t amount) -> void
{
    auto remaining = static_cast<block::position>(this->size()) - position;
    auto delta = 0;
    if (amount > remaining) {
        delta = amount - remaining;
        expand_storage(delta);
    }
}

// MARK: - Position Management

auto graphite::data::writer::set_position(block::position pos) -> void
{
    if (pos < 0 || pos > size()) {
        throw std::runtime_error("Attempted to set position of data reader out of bounds.");
    }
    m_position = pos;
}

auto graphite::data::writer::move(block::position delta) -> void
{
    set_position(m_position + delta);
}

// MARK: - Write Operations

auto graphite::data::writer::write_byte(uint8_t value, std::size_t count) -> void
{
    write_integer(value, count);
}

auto graphite::data::writer::write_signed_byte(int8_t value, std::size_t count) -> void
{
    write_integer(static_cast<uint8_t>(value), count);
}

auto graphite::data::writer::write_short(uint16_t value, std::size_t count) -> void
{
    write_integer(value, count);
}

auto graphite::data::writer::write_signed_short(int16_t value, std::size_t count) -> void
{
    write_integer(static_cast<uint16_t>(value), count);
};

auto graphite::data::writer::write_fixed_point(double value, std::size_t count) -> void
{
    auto integral_value = static_cast<int32_t>(value * (1 << 16));
    write_integer(integral_value, count);
}

auto graphite::data::writer::write_triple(uint32_t value, std::size_t count) -> void
{
    write_integer(value, count, 3);
};

auto graphite::data::writer::write_long(uint32_t value, std::size_t count) -> void
{
    write_integer(value, count);
};

auto graphite::data::writer::write_signed_long(int32_t value, std::size_t count) -> void
{
    write_integer(static_cast<uint32_t>(value), count);
};

auto graphite::data::writer::write_quad(uint64_t value, std::size_t count) -> void
{
    write_integer(value, count);
};

auto graphite::data::writer::write_signed_quad(int64_t value, std::size_t count) -> void
{
    write_integer(static_cast<uint64_t>(value), count);
}

auto graphite::data::writer::write_pstr(const std::string &str) -> std::size_t
{
    auto bytes = encoding::mac_roman::from_utf8(str);

    if (bytes.size() > 0xFF) {
        bytes.resize(0xFF);
    }

    write_byte(static_cast<uint8_t>(bytes.size()));
    write_bytes(bytes);
    return bytes.size();
}

auto graphite::data::writer::write_cstr(const std::string &str, std::size_t size) -> std::size_t
{
    std::vector<uint8_t> bytes { encoding::mac_roman::from_utf8(str) };

    if (size == 0) {
        // NULL terminated C-String
        bytes.push_back( '\0' );
    }
    else {
        // Fixed length C-String
        bytes.resize(size, '\0');
    }

    write_bytes(bytes);
    return bytes.size();
}

auto graphite::data::writer::write_bytes(const std::vector<uint8_t> &bytes) -> void
{
    ensure_required_space(position(), bytes.size());
    auto ptr = m_data->template get<uint8_t *>(position());

    for (auto v : bytes) {
        *ptr++ = v;
        move();
    }
}

auto graphite::data::writer::write_bytes(const std::vector<char> &bytes) -> void
{
    write_bytes(std::vector<uint8_t>(bytes.begin(), bytes.end()));
}

auto graphite::data::writer::write_data(const class block *data) -> void
{
    assert(data != m_data);

    ensure_required_space(position(), data->size());
    auto ptr = m_data->template get<uint8_t *>(position());

    for (auto i = 0; i < data->size(); ++i) {
        ptr[i] = *data->get<uint8_t *>(i);
        move();
    }
}

// MARK: - Padding

auto graphite::data::writer::pad_to_size(std::size_t size) -> void
{
    if (this->size() >= size) {
        return;
    }

    auto required = size - this->size();
    set_position(this->size());
    write_byte(0, required);
}

// MARK: - Saving / File Access

auto graphite::data::writer::save(const std::string &path, std::size_t size) const -> void
{
    std::ofstream file { path, std::ios::out | std::ios::binary };
    file.write(m_data->get<char *>(), size == 0 ? m_data->size() : size);
    file.close();
}
