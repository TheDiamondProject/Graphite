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

#include <unordered_map>
#include <string>
#include "libGraphite/data/data.hpp"

namespace graphite
{
    class font_manager
    {
    public:
        struct font_reference
        {
            std::string name;
            data::block ttf;
        };

    public:
        font_manager(const font_manager&) = delete;
        font_manager(font_manager&&) = delete;
        auto operator=(const font_manager&) -> font_manager& = delete;
        auto operator=(font_manager&&) -> font_manager& = delete;

        static auto shared_manager() -> font_manager&;

        auto update_font_table() -> void;
        [[nodiscard]] auto font_named(const std::string& name) const -> const data::block *;

    private:
        std::unordered_map<std::string, struct font_reference> m_fonts;

        font_manager() = default;
    };
}