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

#include <libQuickdraw/surface/surface.hpp>
#include <libQuickdraw/format/color_lookup_table.hpp>

namespace quicktime
{
    struct image_description
    {
    public:
        enum compression_type : std::uint32_t
        {
            unknown = 0,
            rle = 'rle ',
            planar = '8BPS',
            raw = 'raw ',
            quickdraw = 'qdrw',
        };

    public:
        image_description() = default;
        explicit image_description(data::reader& reader);

        [[nodiscard]] auto length() const -> std::int32_t;
        [[nodiscard]] auto compressor() const -> enum compression_type;
        [[nodiscard]] auto version() const -> std::uint32_t;
        [[nodiscard]] auto width() const -> std::int16_t;
        [[nodiscard]] auto height() const -> std::int16_t;
        [[nodiscard]] auto data_size() const -> std::int32_t;
        [[nodiscard]] auto depth() const -> std::int16_t;
        [[nodiscard]] auto data_offset() const -> std::int32_t;
        [[nodiscard]] auto clut() const -> const quickdraw::color_lookup_table&;
        [[nodiscard]] auto surface() const -> const quickdraw::surface&;

    private:
        std::int32_t m_length { 0 };
        enum compression_type m_compressor { unknown };
        std::uint32_t m_version { 0 };
        std::int16_t m_width { 0 };
        std::int16_t m_height { 0 };
        std::int32_t m_data_size { 0 };
        std::int16_t m_depth { 0 };
        std::int32_t m_data_offset { 0 };
        quickdraw::color_lookup_table m_clut;
        quickdraw::surface m_surface;

        auto decode(data::reader& reader) -> void;
    };
}


