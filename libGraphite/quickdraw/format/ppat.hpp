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
#include "libGraphite/quickdraw/type/rect.hpp"
#include "libGraphite/quickdraw/format/clut.hpp"
#include "libGraphite/quickdraw/support/pixmap.hpp"
#include "libGraphite/quickdraw/support/surface.hpp"

namespace graphite::quickdraw
{
    struct ppat
    {
    public:
        static auto type_code() -> std::string { return "ppat"; }

    public:
        ppat() = default;
        explicit ppat(const data::block& data, rsrc::resource::identifier id = 0, const std::string& name = "");
        explicit ppat(data::reader& reader);

        [[nodiscard]] auto surface() const -> const quickdraw::surface&;

        auto encode(data::writer& writer) -> void;
        auto data() -> data::block;

    private:
        rsrc::resource::identifier m_id;
        std::string m_name;
        std::uint16_t m_pat_type;
        std::uint32_t m_pmap_base_address;
        std::uint32_t m_pat_base_address;
        quickdraw::pixmap m_pixmap;
        quickdraw::surface m_surface;
        quickdraw::clut m_clut;

        auto decode(data::reader& reader) -> void;
    };
}