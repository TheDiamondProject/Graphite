//
// Created by Tom Hancocks on 24/03/2020.
//

#include <libToolbox/strings/string.hpp>
#include <libData/reader.hpp>

// MARK: - Constructor

toolbox::string::string(const data::block& data, resource_core::identifier id, const std::string& name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

// MARK: - Accessor

auto toolbox::string::value() const -> const std::string&
{
    return m_str;
}

auto toolbox::string::data() const -> const data::block&
{
    return m_data;
}

// MARK: - Decoder

auto toolbox::string::decode(data::reader &reader) -> void
{
    m_str = reader.read_pstr();
    auto length = reader.size() - reader.position();
    if (length > 0) {
        m_data = reader.read_data(length);
    }
}