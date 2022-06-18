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

#include "libGraphite/quickdraw/format/clut.hpp"
#include "libGraphite/data/writer.hpp"
#include "libGraphite/data/reader.hpp"

// MARK: - Construction

graphite::quickdraw::clut::clut(const data::block &data, rsrc::resource::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::quickdraw::clut::clut(data::reader &reader)
    : m_id(0), m_name("Embedded `clut` resource")
{
    decode(reader);
}

// MARK: - Accessors

auto graphite::quickdraw::clut::size() const -> size_type
{
    return m_entries.size();
}

auto graphite::quickdraw::clut::at(index_type index) const -> union color
{
    const auto& it = m_entries.find(index);
    return (it != m_entries.end()) ? it->second : colors::black();
}

auto graphite::quickdraw::clut::set(union color color) -> index_type
{
    index_type value = 0;
    for (const auto& entry : m_entries) {
        if (entry.second.value == color.value) {
            return entry.first;
        }
        else {
            value = std::max(value, entry.first);
        }
    }
    m_entries.emplace_back(std::pair(value, color));
    m_size = m_entries.size();
    return value;
}

auto graphite::quickdraw::clut::set(index_type index, union color color) -> void
{
    for (auto& entry : m_entries) {
        if (entry.first == index) {
            entry.second = color;
            return;
        }
    }
    m_entries.emplace_back(std::pair(index, color));
}

// MARK: - Coding

auto graphite::quickdraw::clut::encode(data::writer &writer) -> void
{
    writer.write_long(m_seed);
    writer.write_enum(m_flags);
    writer.write_short(m_size - 1);

    for (auto entry : m_entries) {
        writer.write_short(entry.first);
        writer.write_short(static_cast<std::uint16_t>((entry.second.components.red / 255.0) * 65535.0));
        writer.write_short(static_cast<std::uint16_t>((entry.second.components.green / 255.0) * 65535.0));
        writer.write_short(static_cast<std::uint16_t>((entry.second.components.blue / 255.0) * 65535.0));
    }
}

auto graphite::quickdraw::clut::decode(data::reader &reader) -> void
{
    m_seed = reader.read_long();
    m_flags = reader.read_enum<enum flags>();
    m_size = reader.read_short() + 1;

    for (std::uint16_t i = 0; i < m_size; ++i) {
        auto value = reader.read_short();
        std::uint16_t index = m_flags == device ? i : value;
        m_entries.emplace_back(std::pair(index, rgb(
            static_cast<std::uint8_t>((reader.read_short() / 65535.0) * 255.0),
            static_cast<std::uint8_t>((reader.read_short() / 65535.0) * 255.0),
            static_cast<std::uint8_t>((reader.read_short() / 65535.0) * 255.0)
        )));
    }
}

// MARK: - Iterators

auto graphite::quickdraw::clut::begin() -> iterator
{
    return { this, 0 };
}

auto graphite::quickdraw::clut::end() -> iterator
{
    return { this, m_size };
}

auto graphite::quickdraw::clut::begin() const -> iterator
{
    return { const_cast<clut *>(this), 0 };
}

auto graphite::quickdraw::clut::end() const -> iterator
{
    return { const_cast<clut *>(this), m_size };
}