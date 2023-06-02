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

#include <libToolbox/font/nfnt.hpp>

// MARK: - Construction

toolbox::font::bitmapped_font::bitmapped_font(const data::block &data, resource_core::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

toolbox::font::bitmapped_font::bitmapped_font(data::reader &reader)
{
    decode(reader);
}

// MARK: - Decoding

auto toolbox::font::bitmapped_font::decode(data::reader &reader) -> void
{
    m_font_type = reader.read_signed_short();
    m_first_char_code = reader.read_signed_short();
    m_last_char_code = reader.read_signed_short();
    m_max_width = reader.read_signed_short();
    m_max_kerning = reader.read_signed_short();
    m_descent = reader.read_signed_short();
    m_font_rect_width = reader.read_signed_short();
    m_font_rect_height = reader.read_signed_short();
    m_width_table_offset = reader.read_signed_short();
    m_max_ascent = reader.read_signed_short();
    m_max_descent = reader.read_signed_short();
    m_leading = reader.read_signed_short();
    m_bit_image_row_width = reader.read_signed_short();
}
