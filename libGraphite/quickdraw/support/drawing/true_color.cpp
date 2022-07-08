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

#include <limits>
#include "libGraphite/quickdraw/support/drawing/true_color.hpp"
#include "libGraphite/data/data.hpp"

static_assert(CHAR_BIT == 8, "We require CHAR_BIT to be equal to 8 here.");

// MARK: - Drawing

auto graphite::quickdraw::drawing::true_color::pixmap::draw(const quickdraw::pixmap::draw_configuration& cfg, quickdraw::surface &surface) -> void
{
    if (cfg.mask.data) {
        draw_masked(cfg, surface);
        return;
    }

    for (std::int16_t y = 0; y < cfg.pixmap.bounds.size.height; ++y) {
        auto y_offset = y * cfg.pixmap.row_bytes;

        for (std::int16_t x = 0; x < cfg.pixmap.bounds.size.width; ++x) {
            auto byte = cfg.pixmap.data->get<std::uint8_t>(y_offset + x);
            surface.set(x, y, cfg.color_table->at(byte));
        }
    }
}

auto graphite::quickdraw::drawing::true_color::pixmap::draw_masked(const quickdraw::pixmap::draw_configuration& cfg, quickdraw::surface &surface) -> void
{
    for (std::int16_t y = 0; y < cfg.pixmap.bounds.size.height; ++y) {
        auto y_offset = y * cfg.pixmap.row_bytes;
        auto mask_y_offset = y * cfg.mask.row_bytes;

        for (std::int16_t x = 0; x < cfg.pixmap.bounds.size.width; ++x) {
            auto mask_offset = (7 - (x % CHAR_BIT));

            auto byte = cfg.pixmap.data->get<std::uint8_t>(y_offset + x);
            auto mask = cfg.mask.data->get<std::uint8_t>(mask_y_offset + (x / CHAR_BIT));

            if ((mask >> mask_offset) & 0x1) {
                surface.set(x, y, cfg.color_table->at(byte));
            }
        }
    }
}
