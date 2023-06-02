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
#include <libQuickdraw/type/rect.hpp>
#include <libQuickdraw/format/color_lookup_table.hpp>
#include <libQuickdraw/pixmap/pixmap.hpp>
#include <libQuickdraw/surface/surface.hpp>

namespace quickdraw
{
    struct pixel_pattern
    {
    public:
        static auto type_code() -> std::string { return "ppat"; }

    public:
        pixel_pattern() = default;
        explicit pixel_pattern(const data::block& data, resource_core::identifier id = 0, const std::string& name = "");
        explicit pixel_pattern(data::reader& reader);
        explicit pixel_pattern(surface& surface);

        auto surface() -> quickdraw::surface&;

        auto encode(data::writer& writer) -> void;
        auto data() -> data::block;

    private:
        resource_core::identifier m_id;
        std::string m_name;
        std::uint16_t m_pat_type;
        std::uint32_t m_pmap_base_address;
        std::uint32_t m_pat_base_address;
        quickdraw::pixmap m_pixmap;
        quickdraw::surface m_surface;
        quickdraw::color_lookup_table m_clut;

        auto decode(data::reader& reader) -> void;
    };
}