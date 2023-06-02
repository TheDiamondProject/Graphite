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
#include <libQuickdraw/surface/surface.hpp>
#include <libQuickdraw/type/rect.hpp>
#include <libResource/structure/instance.hpp>

namespace quickdraw
{
    struct picture
    {
    public:
        static auto type_code() -> std::string { return "PICT"; }

    public:
        picture() = default;
        explicit picture(const data::block& data, resource::identifier id = 0, const std::string& name = "");
        explicit picture(data::reader& reader);
        explicit picture(quickdraw::surface& surface);

        auto encode(data::writer& writer) -> void;
        auto data() -> data::block;

        auto surface() -> quickdraw::surface&;

    private:
        enum class opcode : std::uint16_t {
            nop = 0x0000,
            clip_region = 0x0001,
            pen_size = 0x0007,
            pen_mode = 0x0008,
            pen_pattern = 0x0009,
            fill_pattern = 0x000a,
            origin = 0x000c,
            rgb_fg_color = 0x001a,
            rgb_bg_color = 0x001b,
            hilite_mode = 0x001c,
            hilite_color = 0x001d,
            def_hilite = 0x001e,
            op_color = 0x001f,
            line = 0x0020,
            line_from = 0x0021,
            short_line = 0x0022,
            short_line_from = 0x0023,
            frame_rect = 0x0030,
            paint_rect = 0x0031,
            erase_rect = 0x0032,
            invert_rect = 0x0033,
            fill_rect = 0x0034,
            frame_same_rect = 0x0038,
            paint_same_rect = 0x0039,
            erase_same_rect = 0x003a,
            invert_same_rect = 0x003b,
            fill_same_rect = 0x003c,
            bits_rect = 0x0090,
            bits_region = 0x0091,
            pack_bits_rect = 0x0098,
            pack_bits_region = 0x0099,
            direct_bits_rect = 0x009a,
            direct_bits_region = 0x009b,
            frame_region = 0x0080,
            paint_region = 0x0081,
            erase_region = 0x0082,
            invert_region = 0x0083,
            fill_region = 0x0084,
            short_comment = 0x00a0,
            long_comment = 0x00a1,
            eof = 0x00ff,
            ext_header = 0x0c00,
            compressed_quicktime = 0x8200,
            uncompressed_quicktime = 0x8201,
        };

    private:
        resource::identifier m_id { 0 };
        std::string m_name;
        quickdraw::surface m_surface;
        rect<std::int16_t> m_frame;
        point<double> m_dpi;
        std::size_t m_size;
        std::uint32_t m_format { 0 };

        auto decode(data::reader& reader) -> void;
        auto read_region(data::reader& reader) const -> rect<std::int16_t>;
        auto read_long_comment(data::reader& reader) const -> void;
        auto read_short_comment(data::reader& reader) const -> void;
        auto read_direct_bits_rect(data::reader& reader, bool region) -> void;
        auto read_indirect_bits_rect(data::reader& reader, bool packed, bool region) -> void;
        auto read_compressed_quicktime(data::reader& reader) -> void;
        auto read_uncompressed_quicktime(data::reader& reader) -> void;
        auto read_image_description(data::reader& reader) -> void;

        auto write_header(data::writer& writer) -> void;
        auto write_def_hilite(data::writer& writer) -> void;
        auto write_clip_region(data::writer& writer) -> void;
        auto write_direct_bits_rect(data::writer& writer) -> void;

    };
}