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

#include "libGraphite/quicktime/image_description.hpp"
#include "libGraphite/quickdraw/format/pict.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include "libGraphite/quicktime/raw.hpp"
#include "libGraphite/quicktime/animation.hpp"
#include "libGraphite/quicktime/planar.hpp"

// MARK: - Construction

graphite::quicktime::image_description::image_description(data::reader &reader)
{
    // http://mirror.informatimago.com/next/developer.apple.com/documentation/QuickTime/INMAC/QT/iqImageCompMgr.17.htm
    auto start = reader.position();
    m_length = reader.read_signed_long();
    if (m_length < 86) {
        throw std::runtime_error("Invalid QuickTime image description.");
    }

    m_compressor = reader.read_enum<enum compression_type>();
    reader.move(8);
    m_version = reader.read_long();
    reader.move(12);
    m_width = reader.read_signed_short();
    m_height = reader.read_signed_short();
    reader.move(8);
    m_data_size = reader.read_signed_long();
    reader.move(34);
    m_depth = reader.read_signed_short();
    if (m_depth > 32) {
        m_depth -= 32; // grayscale
    }
    auto clut = reader.read_signed_short();
    if (clut == 0) {
        m_clut = reader.read<quickdraw::clut>();
    } else if (clut > 0) {
        if (auto resource = rsrc::manager::shared_manager().find<quickdraw::clut>(clut)) {
            m_clut = quickdraw::clut(resource->data());
        }
        else {
            throw std::runtime_error("Color table not found: clut " + std::to_string(clut));
        }
    }

    // Record the number remaining bytes of the image description before the data start
    m_data_offset = m_length - static_cast<int32_t>(reader.position() - start);
    decode(reader);
}

// MARK: - Accessors

auto graphite::quicktime::image_description::length() const -> std::int32_t
{
    return m_length;
}

auto graphite::quicktime::image_description::compressor() const -> enum compression_type
{
    return m_compressor;
}

auto graphite::quicktime::image_description::version() const -> std::uint32_t
{
    return m_version;
}

auto graphite::quicktime::image_description::width() const -> std::int16_t
{
    return m_width;
}

auto graphite::quicktime::image_description::height() const -> std::int16_t
{
    return m_height;
}

auto graphite::quicktime::image_description::data_size() const -> std::int32_t
{
    return m_data_size;
}

auto graphite::quicktime::image_description::depth() const -> std::int16_t
{
    return m_depth;
}

auto graphite::quicktime::image_description::data_offset() const -> std::int32_t
{
    return m_data_offset;
}

auto graphite::quicktime::image_description::clut() const -> const quickdraw::clut&
{
    return m_clut;
}

auto graphite::quicktime::image_description::surface() const -> const quickdraw::surface&
{
    return m_surface;
}

// MARK: - Decoding

auto graphite::quicktime::image_description::decode(data::reader &reader) -> void
{
    switch (m_compressor) {
        case compression_type::rle: {
            m_surface = std::move(format::animation::decode(*this, reader));
            break;
        }
        case compression_type::planar: {
            m_surface = std::move(format::planar::decode(*this, reader));
            break;
        }
        case compression_type::raw: {
            m_surface = std::move(format::raw::decode(*this, reader));
            break;
        }
        case compression_type::quickdraw: {
            auto pict = reader.read<quickdraw::pict>(m_data_size);
            m_surface = pict.surface();
            break;
        }
        default: {
            std::string compressor_name;
            compressor_name.push_back(m_compressor >> 24);
            compressor_name.push_back(m_compressor >> 16);
            compressor_name.push_back(m_compressor >> 8);
            compressor_name.push_back(m_compressor);

            throw std::runtime_error("Unsupported QuickTime compressor '" + compressor_name + "' at offset " + std::to_string(reader.position()));
        }
    }
}