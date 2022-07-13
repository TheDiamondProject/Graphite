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
    struct dialog
    {
    public:
        static auto type_code() -> std::string { return "DLOG"; }

    public:
        dialog() = default;
        explicit dialog(const data::block& data, rsrc::resource::identifier id = 0, const std::string& name = "");
        explicit dialog(data::reader& reader);

        [[nodiscard]] auto bounds() const -> quickdraw::rect<std::int16_t>;
        [[nodiscard]] auto proc_id() const -> std::int16_t;
        [[nodiscard]] auto visible() const -> bool;
        [[nodiscard]] auto go_away() const -> bool;
        [[nodiscard]] auto ref_con() const -> std::int32_t;
        [[nodiscard]] auto interface_list() const -> rsrc::resource::identifier;
        [[nodiscard]] auto auto_position() const -> std::uint16_t;
        [[nodiscard]] auto title() const -> std::string;

        auto set_bounds(const quickdraw::rect<std::int16_t>& bounds) -> void;
        auto set_proc_id(std::int16_t id) -> void;
        auto set_visible(bool visible) -> void;
        auto set_go_away(bool go_away) -> void;
        auto set_ref_con(std::int32_t ref_con) -> void;
        auto set_interface_list(rsrc::resource::identifier id) -> void;
        auto set_auto_position(std::uint16_t position) -> void;
        auto set_title(const std::string& title) -> void;

        auto encode(data::writer& writer) -> void;
        auto data() -> data::block;

    private:
        rsrc::resource::identifier m_id { INT64_MIN };
        std::string m_name;
        std::string m_title;
        quickdraw::rect<std::int16_t> m_bounds;
        std::int16_t m_proc_id { 0 };
        bool m_visible { true };
        bool m_go_away { true };
        std::int32_t m_ref_con { 0 };
        rsrc::resource::identifier m_ditl_id { 0 };
        std::uint16_t m_auto_position { 0 };

        auto decode(data::reader& reader) -> void;
    };
}
