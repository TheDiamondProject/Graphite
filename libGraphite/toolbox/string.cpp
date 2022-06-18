//
// Created by Tom Hancocks on 24/03/2020.
//

#include "libGraphite/toolbox/string.hpp"
#include "libGraphite/data/reader.hpp"

// MARK: - Constructor

graphite::toolbox::string::string(const data::block& data, rsrc::resource::identifier id, const std::string& name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

// MARK: - Accessor

auto graphite::toolbox::string::value() const -> const std::string&
{
    return m_str;
}

auto graphite::toolbox::string::data() const -> const data::block&
{
    return m_data;
}

// MARK: - Decoder

auto graphite::toolbox::string::decode(data::reader &reader) -> void
{
    m_str = reader.read_pstr();

    auto length = reader.size() - reader.position();
    if (length > 0) {
        m_data = reader.read_data(length);
    }
}