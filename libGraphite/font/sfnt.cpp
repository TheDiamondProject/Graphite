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

#include "libGraphite/font/sfnt.hpp"
#include "libGraphite/data/writer.hpp"

// MARK: - Construction

graphite::sfnt::sfnt(const data::block &data, rsrc::resource::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::sfnt::sfnt(data::reader &reader)
{
    decode(reader);
}

// MARK: - Decoding

auto graphite::sfnt::decode(data::reader &reader) -> void
{
    auto ilen = reader.data()->size();
    auto rlen = ilen;

    if (rlen > 16 * 1024) {
        ilen = 16 * 1024;
    }

    auto max = std::min(static_cast<std::size_t>(0x800), ilen);
    data::writer ttf(data::byte_order::lsb);

    for (auto len = 0; len < rlen;) {
        auto temp = ilen;
        if (rlen - len < ilen) {
            temp = rlen - len;
        }

        auto buffer = reader.read_data(temp);
        if (reader.eof()) {
            break;
        }

        ttf.write_data(&buffer);
        len += temp;
    }

    m_ttf = std::move(*const_cast<data::block *>(ttf.data()));
}

// MARK: - Accessors

auto graphite::sfnt::ttf_data() const -> const data::block&
{
    return m_ttf;
}