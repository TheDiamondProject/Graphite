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

#include <cstdint>
#include <concepts>
#include <type_traits>
#include "libGraphite/data/data.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/quickdraw/format/clut.hpp"
#include "libGraphite/quickdraw/type/rect.hpp"
#include "libGraphite/quickdraw/support/surface.hpp"
#include "libGraphite/quickdraw/type/pixel_format.hpp"

namespace graphite::quickdraw
{
    struct pixmap
    {
    public:

        struct draw_configuration
        {
            struct aspect {
                data::block *data { nullptr };
                std::uint32_t base_address { 0 };
                std::uint16_t row_bytes { 0 };
                rect<std::int16_t> bounds;

                [[nodiscard]] auto expected_data_size() const -> std::size_t;
            };

            struct aspect pixmap;
            struct aspect bitmap;
            struct aspect mask;

            struct clut *color_table { nullptr };
        };

        enum pack_type : std::uint16_t
        {
            none = 0,
            argb = 1,
            rgb = 2,
            packbits_word = 3,
            packbits_component = 4
        };

    public:
        static constexpr std::size_t length = 50;

        pixmap() = default;
        explicit pixmap(const rect<std::int16_t>& frame);
        explicit pixmap(const data::block& data, rsrc::resource::identifier id = 0, const std::string& name = "");
        explicit pixmap(data::reader& reader);
        pixmap(const pixmap&) noexcept = default;

        auto operator=(pixmap&&) -> pixmap& = default;
        auto operator=(const pixmap&) -> pixmap& = default;

        [[nodiscard]] auto basic_draw_configuration() const -> struct draw_configuration;

        [[nodiscard]] auto bounds() const -> rect<std::int16_t>;
        [[nodiscard]] auto row_bytes() const -> std::int16_t;
        [[nodiscard]] auto pack_type() const -> enum pack_type;
        [[nodiscard]] auto pack_size() const -> std::int16_t;
        [[nodiscard]] auto pixel_type() const -> std::int16_t;
        [[nodiscard]] auto pixel_size() const -> std::int16_t;
        [[nodiscard]] auto component_count() const -> std::int16_t;
        [[nodiscard]] auto component_size() const -> std::int16_t;
        [[nodiscard]] auto pixel_format() const -> enum pixel_format;
        [[nodiscard]] auto pm_table() const -> std::uint32_t;

        [[nodiscard]] auto total_component_width() const -> std::size_t;

        auto set_bounds(const rect<std::int16_t>& rect) -> void;
        auto set_row_bytes(std::int16_t row_bytes) -> void;
        auto set_pack_type(enum pack_type pack_type) -> void;
        auto set_pack_size(std::int16_t pack_size) -> void;
        auto set_pixel_type(std::int16_t pixel_type) -> void;
        auto set_pixel_size(std::int16_t pixel_size) -> void;
        auto set_component_count(std::int16_t component_count) -> void;
        auto set_component_size(std::int16_t component_size) -> void;
        auto set_pixel_format(enum pixel_format format) -> void;
        auto set_pm_table(std::uint32_t table) -> void;

        auto draw(quickdraw::surface& surface) -> void;

        auto build_surface(surface& surface, const data::block& pixel_data, const clut& clut, const rect<std::int16_t>& destination) -> void;
        auto build_pixel_data(const data::block& color_data, std::uint16_t pixel_size) -> data::block;

        auto encode(data::writer& writer) -> void;

    private:
        // Resource Data
        std::uint32_t m_base_address { 0xFF };
        std::int16_t m_row_bytes { 0 };
        rect<std::int16_t> m_bounds;
        std::int16_t m_pm_version { 0 };
        enum pack_type m_pack_type { none };
        std::int32_t m_pack_size { 0 };
        quickdraw::size<double> m_dpi { 0.001098632812 };
        std::int16_t m_pixel_type { 16 };
        std::int16_t m_pixel_size { 32 };
        std::int16_t m_component_count { 3 };
        std::int16_t m_component_size { 8 };
        enum pixel_format m_pixel_format { unknown };
        std::uint32_t m_pm_table { 0 };
        std::uint32_t m_pm_extension { 0 };

        auto decode(data::reader& reader) -> void;
    };
}
