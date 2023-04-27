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

#include "libGraphite/toolbox/string_list.hpp"
#include "libGraphite/data/reader.hpp"

// MARK: - Construction

graphite::toolbox::string_list::string_list(const data::block &data, rsrc::resource::identifier id, const std::string& name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

// MARK: - Decoder

auto graphite::toolbox::string_list::decode(data::reader &reader) -> void
{
    auto count = static_cast<std::size_t>(reader.read_signed_short());
    m_strings = { count, "" };

    for (std::int32_t i = 0; i < count; ++i) {
        m_strings[i] = std::move(reader.read_pstr());
    }
}

// MARK: - Access

auto graphite::toolbox::string_list::string_count() const -> std::size_t
{
    return m_strings.size();
}

auto graphite::toolbox::string_list::at(std::uint32_t idx) const -> std::string
{
    return m_strings.at(idx);
}

// MARK: - Iterator

auto graphite::toolbox::string_list::begin() noexcept -> iterator
{
    return m_strings.begin();
}

auto graphite::toolbox::string_list::end() noexcept -> iterator
{
    return m_strings.end();
}