//
// Created by Tom Hancocks on 26/03/2021.
//

#include "libGraphite/quicktime/imagedesc.hpp"
#include "libGraphite/quicktime/animation.hpp"

// MARK: - Constructors

graphite::qt::imagedesc::imagedesc(data::reader& reader)
{
    // http://mirror.informatimago.com/next/developer.apple.com/documentation/QuickTime/INMAC/QT/iqImageCompMgr.17.htm
    auto start = reader.position();
    m_length = reader.read_signed_long();
    if (m_length < 86) {
        throw std::runtime_error("Invalid QuickTime image description.");
    }
    m_compressor = reader.read_long();
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
        m_clut = std::make_shared<qd::clut>(qd::clut(reader));
    } else if (clut > 0) {
        m_clut = qd::clut::load_resource(clut);
        if (m_clut == nullptr) {
            throw std::runtime_error("Color table not found: clut " + std::to_string(clut));
        }
    }
    auto bytes_read = reader.position() - start;
    if (bytes_read < m_length) {
        reader.move(m_length - bytes_read);
    }

    read_image_data(reader);
}

// MARK: - Accessors

auto graphite::qt::imagedesc::length() const -> int32_t
{
    return m_length;
}

auto graphite::qt::imagedesc::compressor() const -> uint32_t
{
    return m_compressor;
}

auto graphite::qt::imagedesc::version() const -> uint32_t
{
    return m_version;
}

auto graphite::qt::imagedesc::width() const -> int16_t
{
    return m_width;
}

auto graphite::qt::imagedesc::height() const -> int16_t
{
    return m_height;
}

auto graphite::qt::imagedesc::data_size() const -> int32_t
{
    return m_data_size;
}

auto graphite::qt::imagedesc::depth() const -> int16_t
{
    return m_depth;
}

auto graphite::qt::imagedesc::clut() const -> std::shared_ptr<qd::clut>
{
    return m_clut;
}

auto graphite::qt::imagedesc::surface() const -> std::shared_ptr<qd::surface>
{
    return m_surface;
}

// MARK: - Decoding

auto graphite::qt::imagedesc::read_image_data(data::reader &reader) -> void {
    switch (m_compressor) {
        case 'rle ': {
            m_surface = std::make_shared<graphite::qd::surface>(qt::animation::decode(*this, reader));
            break;
        }
        default: {
            std::string comp;
            comp.push_back(m_compressor >> 24);
            comp.push_back(m_compressor >> 16);
            comp.push_back(m_compressor >> 8);
            comp.push_back(m_compressor);
            throw std::runtime_error("Unsupported QuickTime compressor '" + comp + "' (offset " + std::to_string(reader.position()) + ")");
        }
    }
}
