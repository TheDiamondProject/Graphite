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
#include "libGraphite/quickdraw/format/ppat.hpp"
#include "libGraphite/quickdraw/support/surface.hpp"

// MARK: - Construction

graphite::quickdraw::ppat::ppat(const data::block &data, rsrc::resource::identifier id, const std::string& name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::quickdraw::ppat::ppat(data::reader &reader)
{
    decode(reader);
}

graphite::quickdraw::ppat::ppat(graphite::quickdraw::surface& surface)
    : m_surface(surface)
{
}

// MARK: - Accessors

auto graphite::quickdraw::ppat::surface() const -> const struct surface &
{
    return m_surface;
}

auto graphite::quickdraw::ppat::data() -> data::block
{
    data::writer writer;
    encode(writer);
    return std::move(*const_cast<data::block *>(writer.data()));
}

// MARK: - Coding

auto graphite::quickdraw::ppat::encode(data::writer &writer) -> void
{
    auto width = m_surface.size().width;
    auto height = m_surface.size().height;

    // TODO: This is a brute force method of bringing down the color depth/number of colors required,
    // for a ppat image. It doesn't optimise for image quality at all, and should be replaced at some point.
    data::block color_data(width * height * sizeof(std::uint16_t));
    data::writer colors(&color_data);

    std::uint8_t pass = 0;
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

        // Rebuild the color table for the surface. To do this we want to create an empty table and populate it.
        m_clut = {};
        colors.set_position(0);
        color_data.set(static_cast<std::uint32_t>(0));
        for (std::int16_t y = 0; y < height; ++y) {
            for (std::int16_t x = 0; x < width; ++x) {
                auto color = m_surface.at(x, y);
                colors.write_short(m_clut.set(color));
            }
        }
    }
    while (m_clut.size() > 256);

    // Determine what component configuration we need
    m_pixmap = pixmap(rect<std::int16_t>({ 0, 0 }, { width, height }));
    data::block pmap_data;

    if (m_clut.size() > 256) {
        throw std::runtime_error("Implementation does not currently handle more than 256 colors in a PPAT");
    }
    else if (m_clut.size() > 16) {
        pmap_data = m_pixmap.build_pixel_data(color_data, 8);
    }
    else if (m_clut.size() > 4) {
        pmap_data = m_pixmap.build_pixel_data(color_data, 4);
    }
    else if (m_clut.size() > 2) {
        pmap_data = m_pixmap.build_pixel_data(color_data, 2);
    }
    else {
        pmap_data = m_pixmap.build_pixel_data(color_data, 1);
    }

    // Calculate some offsets
    m_pat_type = 1;
    m_pmap_base_address = 28;
    m_pat_base_address = m_pmap_base_address + 50;
    m_pixmap.set_pm_table(m_pat_base_address + pmap_data.size());

    // Write out the image data for the ppat.
    writer.write_short(m_pat_type);
    writer.write_long(m_pmap_base_address);
    writer.write_long(m_pat_base_address);
    writer.write_long(0);
    writer.write_short(0);
    writer.write_long(0);
    writer.write_quad(0);
    writer.write(m_pixmap);
    writer.write_data(&pmap_data);
    writer.write(m_clut);
}

auto graphite::quickdraw::ppat::decode(data::reader &reader) -> void
{
    m_pat_type = reader.read_short();
    if (m_pat_type != 1) {
        throw std::runtime_error("Currently unsupported ppat configuration: pat_type=" + std::to_string(m_pat_type));
    }

    m_pmap_base_address = reader.read_long();
    m_pat_base_address = reader.read_long();

    reader.set_position(m_pmap_base_address);
    m_pixmap = reader.read<pixmap>();

    reader.set_position(m_pat_base_address);
    auto pmap_data_size = m_pixmap.row_bytes() * m_pixmap.bounds().size.height;
    auto pmap_data = reader.read_data(pmap_data_size);

    reader.set_position(m_pixmap.pm_table());
    m_clut = reader.read<clut>();

    auto surface_size = m_pixmap.bounds().size;
    m_surface = quickdraw::surface(surface_size);
    m_pixmap.build_surface(m_surface, pmap_data, m_clut, m_pixmap.bounds());
}