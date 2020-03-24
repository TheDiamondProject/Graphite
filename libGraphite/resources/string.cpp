//
// Created by Tom Hancocks on 24/03/2020.
//

#include "libGraphite/resources/string.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include "libGraphite/data/reader.hpp"

// MARK: - Constructor

graphite::resources::string::string(std::string str, std::shared_ptr<data::data> data, int64_t id, const std::string name)
    : m_str(str), m_name(name), m_id(id)
{
    // TODO: Add implementation to extract a new copy of the data that is not a pointer.
}

std::shared_ptr<graphite::resources::string> graphite::resources::string::load_resource(int64_t id)
{
    if (auto str_res = graphite::rsrc::manager::shared_manager().find("STR ", id).lock()) {
        auto reader = graphite::data::reader(str_res->data());
        auto str = reader.read_pstr();
        // TODO: Rest of the resource is data
        return std::make_shared<resources::string>(str, nullptr, id, str_res->name());
    }
    return nullptr;
}

// MARK: - Accessor

std::string graphite::resources::string::value() const
{
    return m_str;
}

graphite::data::data graphite::resources::string::data() const
{
    return m_data;
}

void graphite::resources::string::set_string(const std::string& str)
{
    m_str = str;
}

void graphite::resources::string::set_data(data::data data)
{
    m_data = data;
}
