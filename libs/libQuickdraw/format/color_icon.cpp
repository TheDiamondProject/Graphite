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
#include <libQuickdraw/format/color_icon.hpp>
#include <libQuickdraw/colorspace/monochrome.hpp>
#include <libQuickdraw/colorspace/depth_2_bpp.hpp>
#include <libQuickdraw/colorspace/depth_4bpp.hpp>
#include <libQuickdraw/colorspace/true_color.hpp>

// MARK: - Construction

quickdraw::color_icon::color_icon(const data::block &data, resource_core::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

quickdraw::color_icon::color_icon(data::reader &reader)
{
    decode(reader);
}

quickdraw::color_icon::color_icon(quickdraw::surface& surface)
    : m_surface(std::move(surface))
{}

// MARK: - Accessors

auto quickdraw::color_icon::surface() -> quickdraw::surface&
{
    return m_surface;
}

auto quickdraw::color_icon::data() -> data::block
{
    data::writer writer;
    encode(writer);
    return std::move(*const_cast<data::block *>(writer.data()));
}

// MARK: - Coding

auto quickdraw::color_icon::encode(data::writer &writer) -> void
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

    pmap_data = m_pixmap.build_pixel_data(color_values, m_clut.size());

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

auto quickdraw::color_icon::decode(data::reader &reader) -> void
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
    cfg.color_table = &(m_clut = reader.read<color_lookup_table>());
    auto pmap_data = reader.read_data(cfg.pixmap.expected_data_size());

    cfg.mask.data = &mask_data;
    cfg.bitmap.data = &bmap_data;
    cfg.pixmap.data = &pmap_data;

    m_surface = quickdraw::surface(cfg.pixmap.bounds.size);

    if (m_pixmap.total_component_width() == 1) {
        colorspace::monochrome::draw(cfg, m_surface);
    }
    else if (m_pixmap.total_component_width() == 2) {
        colorspace::depth_2bpp::draw(cfg, m_surface);
    }
    else if (m_pixmap.total_component_width() == 4) {
        colorspace::depth_4bpp::draw(cfg, m_surface);
    }
    else if (m_pixmap.total_component_width() == 8) {
        colorspace::true_color::draw(cfg, m_surface);
    }
    else {
        throw std::runtime_error("Currently unsupported cicn configuration: cmp_size=" +
                                 std::to_string(m_pixmap.component_size()) +
                                 ", cmp_count=" + std::to_string(m_pixmap.component_count()));
    }
}