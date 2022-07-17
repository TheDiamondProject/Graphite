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

#include "libGraphite/font/manager.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include "libGraphite/font/sfnt.hpp"

// MARK: - Singleton

auto graphite::font_manager::shared_manager() -> graphite::font_manager &
{
    static font_manager instance;
    return instance;
}

// MARK: - Font Management

auto graphite::font_manager::update_font_table() -> void
{
    auto sfnt_resources = rsrc::manager::shared_manager().find<sfnt>();
    for (const auto& res : sfnt_resources) {
        auto font_name = res.name();
        auto it = m_fonts.find(font_name);
        if (it != m_fonts.end()) {
            // Already aware of font... ignore it.
            continue;
        }

        struct font_reference ref;
        ref.name = font_name;

        struct sfnt sfnt(res.data());
        ref.ttf = sfnt.ttf_data();

        m_fonts.emplace(std::pair(font_name, std::move(ref)));
    }
}

auto graphite::font_manager::font_named(const std::string &name) const -> const graphite::data::block *
{
    auto it = m_fonts.find(name);
    if (it == m_fonts.end()) {
        return nullptr;
    }
    return &it->second.ttf;
}
