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
    struct outline_font
    {
    public:
        static auto type_code() -> std::string { return "sfnt"; }

    public:
        outline_font() = default;
        explicit outline_font(const data::block& data, rsrc::resource::identifier = 0, const std::string& name = "");
        explicit outline_font(data::reader& reader);

        [[nodiscard]] auto ttf_data() const -> const data::block&;

    private:
        rsrc::resource::identifier m_id { INT64_MIN };
        std::string m_name;
        data::block m_ttf;

        auto decode(data::reader& reader) -> void;
    };
}