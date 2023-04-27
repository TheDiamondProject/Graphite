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

#pragma once

#include "libGraphite/data/data.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/rsrc/resource.hpp"

namespace graphite::font
{
    struct bitmapped_font
    {
    public:
        static auto type_code() -> std::string { return "nfnt"; }

    public:
        bitmapped_font() = default;
        explicit bitmapped_font(const data::block& data, rsrc::resource::identifier id = 0, const std::string& name = "");
        explicit bitmapped_font(data::reader& reader);

    private:
        rsrc::resource::identifier m_id { INT64_MIN };
        std::string m_name;

        std::int16_t m_font_type { 0 };
        std::int16_t m_first_char_code { 0 };
        std::int16_t m_last_char_code { 0 };
        std::int16_t m_max_width { 0 };
        std::int16_t m_max_kerning { 0 };
        std::int16_t m_descent { 0 };
        std::int16_t m_font_rect_width { 0 };
        std::int16_t m_font_rect_height { 0 };
        std::int16_t m_width_table_offset { 0 };
        std::int16_t m_max_ascent { 0 };
        std::int16_t m_max_descent { 0 };
        std::int16_t m_leading { 0 };
        std::int16_t m_bit_image_row_width { 0 };

        auto decode(data::reader& reader) -> void;
    };
}