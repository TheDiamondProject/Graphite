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
#include "libGraphite/data/reader.hpp"
#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/quickdraw/type/rect.hpp"

namespace graphite::toolbox
{
    struct dialog_item_list
    {
    public:
        static auto type_code() -> std::string { return "DITL"; }

        enum class item_type : std::uint8_t
        {
            user_item = 0,
            help_item = 1,
            button = 4,
            checkbox = 5,
            radio = 6,
            control = 7,
            static_text = 8,
            edit_text = 16,
            icon = 32,
            picture = 64,
            disable = 128
        };

        struct item
        {
            quickdraw::rect<std::int16_t> frame;
            enum item_type type { item_type::user_item };
            std::string info;
        };

    public:
        dialog_item_list() = default;
        explicit dialog_item_list(const data::block& data, rsrc::resource::identifier id = 0, const std::string& name = "");
        explicit dialog_item_list(data::reader& reader);

        auto encode(data::writer& writer) -> void;
        auto data() -> data::block;

        [[nodiscard]] auto item_count() const -> std::uint16_t { return m_items.size(); }
        [[nodiscard]] auto at(std::uint16_t idx) const -> const item& { return m_items[idx]; }

        auto begin() -> std::vector<struct item>::iterator { return m_items.begin(); }
        auto end() -> std::vector<struct item>::iterator { return m_items.end(); }

    private:
        rsrc::resource::identifier m_id { INT64_MIN };
        std::string m_name;
        std::vector<struct item> m_items;

        auto decode(data::reader& reader) -> void;
    };
}
