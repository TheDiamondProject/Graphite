//
// Created by Tom Hancocks on 24/03/2020.
//

#include "libGraphite/resources/string.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include "libGraphite/data/reader.hpp"

// MARK: - Constructor

graphite::resources::string::string(std::string str, std::shared_ptr<data::data> data, int64_t id, std::string name)
    : m_str(std::move(str)), m_name(std::move(name)), m_id(id)
{
    // TODO: Add implementation to extract a new copy of the data that is not a pointer.
}

auto graphite::resources::string::load_resource(int64_t id) -> std::shared_ptr<graphite::resources::string>
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

auto graphite::resources::string::value() const -> std::string
{
    return m_str;
}

auto graphite::resources::string::data() const -> graphite::data::data
{
    return m_data;
}

auto graphite::resources::string::set_string(const std::string& str) -> void
{
    m_str = str;
}

auto graphite::resources::string::set_data(const data::data& data) -> void
{
    m_data = data;
}
