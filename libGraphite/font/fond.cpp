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

#include "libGraphite/font/fond.hpp"
#include <cstring>

// MARK: - Construction

graphite::font::descriptor::descriptor(const data::block &data, rsrc::resource::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::font::descriptor::descriptor(data::reader &reader)
{
    decode(reader);
}

// MARK: - Decoding

auto graphite::font::descriptor::decode(data::reader &reader) -> void
{
    m_fixed = reader.read_short() & 0x8000 ? true : false;
    m_family_id = reader.read_short();
    m_first = reader.read_short();
    m_last = reader.read_short();

    m_ascent = reader.read_short();
    m_descent = static_cast<std::int16_t>(reader.read_short());
    m_leading = reader.read_short();
    m_widmax = reader.read_short();

    m_widoff = reader.read_signed_long();
    m_kernoff = reader.read_signed_long();
    m_styleoff = reader.read_signed_long();

    reader.move(sizeof(std::uint16_t) * 9);
    reader.move(sizeof(std::uint32_t));
    reader.move(sizeof(std::uint16_t));

    m_assoc_count = reader.read_short() + 1;
    m_assoc = std::vector<struct assoc>(m_assoc_count);
    for (auto i = 0; i < m_assoc_count; ++i) {
        m_assoc[i].size = reader.read_short();
        m_assoc[i].style = reader.read_short();
        m_assoc[i].id = reader.read_short();
    }

    if (m_widoff != 0) {
        reader.set_position(m_widoff);
        m_style_width_count = reader.read_short() + 1;
        m_style_widths = std::vector<struct style_widths>(m_style_width_count);
        for (auto i = 0; i < m_style_width_count; ++i) {
            m_style_widths[i].style = reader.read_short();
            m_style_widths[i].width_tabs = std::vector<std::uint16_t>((m_last - m_first + 3) * sizeof(std::uint16_t));
            for (auto j = m_first; j <= m_last + 2; ++j) {
                m_style_widths[i].width_tabs[j] = reader.read_short();
            }
        }
    }

    if (m_kernoff != 0) {
        reader.set_position(m_kernoff);
        m_style_kern_count = reader.read_short() + 1;
        m_style_kerns = std::vector<struct style_kern>(m_style_kern_count);
        for (auto i = 0; i < m_style_kern_count; ++i) {
            m_style_kerns[i].style = reader.read_short();
            m_style_kerns[i].kern_pairs = reader.read_short();
            m_style_kerns[i].kerns = std::vector<struct kern>(m_style_kerns[i].kern_pairs);
            for (auto j = 0; j < m_style_kerns[i].kern_pairs; ++j) {
                m_style_kerns[i].kerns[j].ch1 = reader.read_byte();
                m_style_kerns[i].kerns[j].ch2 = reader.read_byte();
                m_style_kerns[i].kerns[j].offset = reader.read_short();
            }
        }
    }

    if (m_styleoff != 0) {
        reader.set_position(m_styleoff);
        m_style_class = reader.read_short();
        m_glyph_encoding = reader.read_short();
        reader.move(sizeof(std::uint32_t));

        std::uint8_t offsets[48] = { 0 };
        for (unsigned char& offset : offsets) {
            offset = reader.read_byte();
        }

        std::uint16_t string_count = reader.read_short();
        char **strings = (char **)malloc(string_count * sizeof(char *));
        for (auto j = 0; j < string_count; ++j) {
            std::uint8_t len = reader.read_byte();
            strings[j] = (char *)malloc(len + 2);
            strings[j][0] = len;
            strings[j][len + 1] = '\0';
            for (auto k = 0; k < len; ++k) {
                strings[j][k + 1] = reader.read_byte();
            }
        }

        for (auto j = 0; j < 48; ++j) {
            auto k = j - 1;
            for (; k >= 0; --k) {
                if (offsets[j] == offsets[k]) {
                    break;
                }
            }

            if (k != -1 || offsets[j] == 0) {
                continue;
            }

            std::int32_t format = offsets[j] - 1;
            std::uint8_t len = strings[0][0];
            if (format != 0 && format != -1) {
                for (k =0; k < strings[format][0]; ++k) {
                    len += strings[strings[format][k + 1] - 1][0];
                }
            }

            char *ptr_str = (char *)malloc(len + 1);
            char *pt = ptr_str;
            strlcpy(pt, strings[0] + 1, len + 1);
            pt += strings[0][0];
            if (format != 0 && format != -1) {
                for (k = 0; k < strings[format][0]; ++k) {
                    strlcpy(pt, strings[strings[format][k + 1] - 1] + 1, len + 1);
                    pt += strings[strings[format][k + 1] - 1][0];
                }
            }
            *pt = '\0';

            m_ps_names[j] = std::string(ptr_str);
            free(ptr_str);
        }

        m_family = std::string(strdup(strings[0]));
        for (auto j = 0; j < string_count; ++j) {
            free(strings[j]);
        }
        free(strings);
    }
}