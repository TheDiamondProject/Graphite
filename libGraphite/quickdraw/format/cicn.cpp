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

#include <stdexcept>
#include "libGraphite/quickdraw/format/cicn.hpp"
#include "libGraphite/quickdraw/support/drawing/monochrome.hpp"
#include "libGraphite/quickdraw/support/drawing/depth_2_bpp.hpp"
#include "libGraphite/quickdraw/support/drawing/depth_4bpp.hpp"
#include "libGraphite/quickdraw/support/drawing/true_color.hpp"

// MARK: - Construction

graphite::quickdraw::cicn::cicn(const data::block &data, rsrc::resource::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::quickdraw::cicn::cicn(data::reader &reader)
{
    decode(reader);
}

// MARK: - Accessors

auto graphite::quickdraw::cicn::surface() const -> const quickdraw::surface&
{
    return m_surface;
}

auto graphite::quickdraw::cicn::data() -> data::block
{
    data::block data;
    data::writer writer(&data);
    encode(writer);
    return std::move(data);
}

// MARK: - Coding

auto graphite::quickdraw::cicn::encode(data::writer &writer) -> void
{
    auto width = m_surface.size().width;
    auto height = m_surface.size().height;

    m_mask_row_bytes = (width - 1) / 8 + 1;
    m_bmap_row_bytes = 0;

    data::block color_values(width * height * sizeof(std::uint16_t));
    data::block mask_data(m_mask_row_bytes * height);
    data::writer colors(&color_values);
    data::writer mask(&mask_data);

    std::uint8_t pass = 0;
    std::uint8_t scratch = 0;

    do {
        if (pass++ > 0) {
            for (std::int16_t y = 0; y < height; ++y) {
                for (std::int16_t x = 0; x < width; ++x) {
                    auto color = m_surface.at(x, y);
                    m_surface.set(x, y, rgb(
                        color.components.red & ~(1 << pass),
                        color.components.green & ~(1 << pass),
                        color.components.blue & ~(1 << pass),
                        color.components.alpha
                    ));
                }
            }
        }

        m_clut = {};
        colors.set_position(0);
        mask.set_position(0);
        color_values.set(static_cast<std::uint32_t>(0));
        mask_data.set(static_cast<std::uint32_t>(0));

        for (std::int16_t y = 0; y < height; ++y) {
            scratch = 0;
            for (std::int16_t x = 0; x < width; ++x) {
                auto color = m_surface.at(x, y);
                colors.write_short(m_clut.set(color));

                auto bit_offset = x % 8;
                if (bit_offset == 0 && x != 0) {
                    mask.write_byte(scratch);
                    scratch = 0;
                }

                auto mask_value = (color.components.alpha & 0x80) == 0x1;
                mask_value <<= (7 - bit_offset);
                scratch |= mask_value;
            }
            mask.write_byte(scratch);
        }
    }
    while (m_clut.size() > 256);

    // Determine what component configuration we need.
    m_pixmap = pixmap(rect<std::int16_t>({ 0, 0 }, { width, height }));
    data::block pmap_data;

    if (m_clut.size() > 256) {
        throw std::runtime_error("Implementation does not currently handle more than 256 colors in a CICN");
    }
    else if (m_clut.size() > 16) {
        pmap_data = m_pixmap.build_pixel_data(color_values, 8);
    }
    else if (m_clut.size() > 4) {
        pmap_data = m_pixmap.build_pixel_data(color_values, 4);
    }
    else if (m_clut.size() > 2) {
        pmap_data = m_pixmap.build_pixel_data(color_values, 2);
    }
    else {
        pmap_data = m_pixmap.build_pixel_data(color_values, 1);
    }

    // Calculate some offsets
    m_mask_base_address = 4;
    m_bmap_base_address = m_mask_base_address + mask_data.size();

    // Write out the image data for the CICN.
    writer.write(m_pixmap);
    writer.write_long(0);
    writer.write_short(m_mask_row_bytes);
    m_pixmap.bounds().encode(writer);
    writer.write_long(0);
    writer.write_short(m_bmap_row_bytes);
    m_pixmap.bounds().encode(writer);
    writer.write_long(0);

    writer.write_data(&mask_data);
    writer.write(m_clut);
    writer.write_data(&pmap_data);
}

auto graphite::quickdraw::cicn::decode(data::reader &reader) -> void
{
    m_pixmap = reader.read<pixmap>();

    auto cfg = std::move(m_pixmap.basic_draw_configuration());
    cfg.mask.base_address = reader.read_long();
    cfg.mask.row_bytes = reader.read_short();
    cfg.mask.bounds = reader.read<rect<std::int16_t>>();
    cfg.bitmap.base_address = reader.read_long();
    cfg.bitmap.row_bytes = reader.read_short();
    cfg.bitmap.bounds = reader.read<rect<std::int16_t>>();

    reader.move(4);

    auto mask_data = reader.read_data(cfg.mask.expected_data_size());
    auto bmap_data = reader.read_data(cfg.bitmap.expected_data_size());
    cfg.color_table = &(m_clut = reader.read<clut>());
    auto pmap_data = reader.read_data(cfg.pixmap.expected_data_size());

    m_surface = quickdraw::surface(cfg.pixmap.bounds.size);

    if (m_pixmap.total_component_width() == 1) {
        drawing::monochrome::pixmap::draw(cfg, m_surface);
    }
    else if (m_pixmap.total_component_width() == 2) {
        drawing::depth_2bpp::pixmap::draw(cfg, m_surface);
    }
    else if (m_pixmap.total_component_width() == 4) {
        drawing::depth_4bpp::pixmap::draw(cfg, m_surface);
    }
    else if (m_pixmap.total_component_width() == 8) {
        drawing::true_color::pixmap::draw(cfg, m_surface);
    }
    else {
        throw std::runtime_error("Currently unsupported cicn configuration: cmp_size=" +
                                 std::to_string(m_pixmap.component_size()) +
                                 ", cmp_count=" + std::to_string(m_pixmap.component_count()));
    }
}