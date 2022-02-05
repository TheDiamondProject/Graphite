//
// Created by Tom Hancocks on 25/03/2020.
//

#include <limits>
#include <stdexcept>
#include "libGraphite/quickdraw/clut.hpp"
#include "libGraphite/rsrc/manager.hpp"

// MARK: - Constructors


graphite::qd::clut::clut(std::shared_ptr<graphite::data::data> data, int64_t id, std::string name)
    : m_id(id), m_name(std::move(name))
{
    data::reader reader(std::move(data));
    parse(reader);
}

graphite::qd::clut::clut(graphite::data::reader& reader)
    : m_id(std::numeric_limits<int64_t>::max()), m_name("Embedded `clut` resource")
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
    return m_entries[index];
}

auto graphite::qd::clut::get(int value) const -> graphite::qd::color
{
    return m_entries[value];
}

auto graphite::qd::clut::set(const qd::color& color) -> uint16_t
{
    for (auto i = 0; i < m_size; ++i) {
        if (m_entries[i] == color) {
            return i;
        }
    }
    m_entries.emplace_back(color);
    return m_size++;
}

// MARK: - Parser

auto graphite::qd::clut::parse(graphite::data::reader& reader) -> void
{
    m_seed = reader.read_long();
    m_flags = static_cast<flags>(reader.read_short());
    m_size = reader.read_short() + 1;

    for (auto i = 0; i < m_size; ++i) {
        auto value = reader.read_short();
        auto r = static_cast<uint8_t>((reader.read_short() / 65535.0) * 255);
        auto g = static_cast<uint8_t>((reader.read_short() / 65535.0) * 255);
        auto b = static_cast<uint8_t>((reader.read_short() / 65535.0) * 255);
        auto color = qd::color(r, g, b);
        // Values are usually sequential but this is not guaranteed. E.g. black may be placed at 255 with a gap in between.
        // In this case we just resize the entries and fill the gap with the same color.
        if (m_flags == device || value == m_entries.size()) {
            m_entries.emplace_back(color);
        } else if (value < m_entries.size()) {
            m_entries[value] = color;
        } else {
            m_entries.resize(value+1, color);
        }
    }
}

// MARK: - Writer

auto graphite::qd::clut::write(graphite::data::writer& writer) -> void
{
    writer.write_long(m_seed);
    writer.write_short(static_cast<uint16_t>(m_flags));
    writer.write_short(m_size - 1);

    for (auto value = 0; value < m_size; ++value) {
        auto color = m_entries[value];
        writer.write_short(value);
        writer.write_short(static_cast<uint16_t>((color.red_component() / 255.0) * 65535.0));
        writer.write_short(static_cast<uint16_t>((color.green_component() / 255.0) * 65535.0));
        writer.write_short(static_cast<uint16_t>((color.blue_component() / 255.0) * 65535.0));
    }
}
