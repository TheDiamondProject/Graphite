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

#include <string>
#include <limits>
#include <vector>
#include <array>
#include "libGraphite/data/reader.hpp"
#include "libGraphite/rsrc/resource.hpp"

namespace graphite::font
{
    struct descriptor
    {
    public:
        static auto type_code() -> std::string { return "FOND"; }

    public:
        descriptor() = default;
        explicit descriptor(const data::block& data, rsrc::resource::identifier = 0, const std::string& name = "");
        explicit descriptor(data::reader& reader);

    private:
        struct assoc
        {
            std::uint16_t size { 0 };
            std::uint16_t style { 0 };
            std::uint16_t id { 0 };
        };

        struct style_widths
        {
            std::uint16_t style { 0 };
            std::vector<std::uint16_t> width_tabs;
        };

        struct kern
        {
            std::uint8_t ch1 { 0 };
            std::uint8_t ch2 { 0 };
            std::uint16_t offset { 0 };
        };

        struct style_kern
        {
            std::uint16_t style { 0 };
            std::uint16_t kern_pairs { 0 };
            std::vector<struct kern> kerns;
        };

        rsrc::resource::identifier m_id { INT64_MIN };
        std::string m_name;
        bool m_fixed { false };
        std::uint16_t m_family_id { 0 };
        std::uint16_t m_first { 0 };
        std::uint16_t m_last { 0 };

        std::uint16_t m_ascent { 0 };
        std::int16_t m_descent { 0 };
        std::uint16_t m_leading { 0 };
        std::uint16_t m_widmax { 0 };

        std::int32_t m_widoff { 0 };
        std::int32_t m_kernoff { 0 };
        std::int32_t m_styleoff { 0 };

        std::uint16_t m_assoc_count { 0 };
        std::vector<struct assoc> m_assoc;

        std::uint16_t m_style_width_count { 0 };
        std::vector<struct style_widths> m_style_widths;

        std::uint16_t m_style_kern_count { 0 };
        std::vector<struct style_kern> m_style_kerns;

        std::uint16_t m_style_class { 0 };
        std::uint16_t m_glyph_encoding { 0 };

        std::array<std::string, 48> m_ps_names;
        std::string m_family;

        auto decode(data::reader& reader) -> void;
    };
}