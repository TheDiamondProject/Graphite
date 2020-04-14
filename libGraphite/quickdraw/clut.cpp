//
// Created by Tom Hancocks on 25/03/2020.
//

#include "libGraphite/quickdraw/clut.hpp"
#include "libGraphite/rsrc/manager.hpp"

// MARK: - Constructors

graphite::qd::clut::clut()
{

}

graphite::qd::clut::clut(std::shared_ptr<graphite::data::data> data, int64_t id, std::string name)
    : m_id(id), m_name(name)
{
    data::reader reader(data);
    parse(reader);
}

graphite::qd::clut::clut(graphite::data::reader& reader)
    : m_id(INT64_MAX), m_name("Embedded `clut` resource")
{
    parse(reader);
}

auto graphite::qd::clut::load_resource(int64_t id) -> std::shared_ptr<graphite::qd::clut>
{
    if (auto res = graphite::rsrc::manager::shared_manager().find("clut", id).lock()) {
        return std::make_shared<graphite::qd::clut>(res->data(), id, res->name());
    }
    return nullptr;
}

// MARK: - Accessors

auto graphite::qd::clut::size() const -> int
{
    return m_size;
}

auto graphite::qd::clut::at(int index) const -> graphite::qd::color
{
    return std::get<1>(m_entries[index]);
}

auto graphite::qd::clut::get(int value) const -> graphite::qd::color
{
    for (auto entry : m_entries) {
        if (std::get<0>(entry) == value) {
            return std::get<1>(entry);
        }
    }
    throw std::runtime_error("Access invalid entry/value '" + std::to_string(value) + "' of color table: " + std::to_string(m_id));
}

// MARK: - Parser

auto graphite::qd::clut::parse(graphite::data::reader& reader) -> void
{
    m_seed = reader.read_long();
    m_flags = static_cast<flags>(reader.read_short());
    m_size = reader.read_short() + 1;

    for (auto i = 0; i < m_size; ++i) {
        uint16_t value = reader.read_short();
        uint8_t r = static_cast<uint8_t>((reader.read_short() / 65535.0) * 255);
        uint8_t g = static_cast<uint8_t>((reader.read_short() / 65535.0) * 255);
        uint8_t b = static_cast<uint8_t>((reader.read_short() / 65535.0) * 255);
        m_entries.emplace_back(std::make_tuple(value, qd::color(r, g, b)));
    }
}
