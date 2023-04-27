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

#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/quickdraw/type/color.hpp"

namespace graphite::quickdraw
{
    struct clut
    {
    public:
        typedef std::uint16_t size_type;
        typedef std::uint16_t index_type;

        static auto type_code() -> std::string { return "clut"; }

        enum flags : std::uint16_t
        {
            pixmap = 0x0000,
            device = 0x8000
        };

        struct iterator
        {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = union color;
            using pointer = value_type*;
            using reference = value_type&;

            iterator(struct clut *clut, index_type idx) : m_clut(clut), m_idx(idx) {}

            auto operator*() const -> reference { return m_clut->m_entries[m_idx].second; }
            auto operator->() -> pointer { return &m_clut->m_entries[m_idx].second; }
            auto operator++() -> iterator& { ++m_idx; return *this; }
            auto operator++(int) -> iterator { iterator tmp = *this; ++(*this); return tmp; }
            friend auto operator==(const iterator& a, const iterator& b) -> bool { return (a.m_clut == b.m_clut) && (a.m_idx == b.m_idx); }
            friend auto operator!=(const iterator& a, const iterator& b) -> bool { return (a.m_clut != b.m_clut) && (a.m_idx == b.m_idx); }

        private:
            struct clut *m_clut;
            index_type m_idx;
        };

    public:
        clut() = default;
        explicit clut(const data::block& data, rsrc::resource::identifier id = 0, const std::string& name = "");
        explicit clut(data::reader& reader);

        [[nodiscard]] auto size() const -> size_type;

        [[nodiscard]] auto at(index_type index) const -> union color;
        auto set(union color color) -> index_type;
        auto set(index_type index, union color color) -> void;

        auto begin() -> iterator;
        auto end() -> iterator;
        [[nodiscard]] auto begin() const -> iterator;
        [[nodiscard]] auto end() const -> iterator;

        auto encode(data::writer& writer) -> void;

    private:
        rsrc::resource::identifier m_id;
        std::string m_name;
        std::uint32_t m_seed;
        enum flags m_flags { pixmap };
        size_type m_size;
        std::vector<std::pair<index_type, union color>> m_entries;

        auto decode(data::reader& reader) -> void;
    };
}