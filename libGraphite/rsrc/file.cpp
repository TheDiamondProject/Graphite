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

#include "libGraphite/rsrc/file.hpp"
#include "libGraphite/rsrc/type.hpp"
#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/data/reader.hpp"

#include "libGraphite/rsrc/classic/classic.hpp"
#include "libGraphite/rsrc/extended/extended.hpp"
#include "libGraphite/rsrc/rez/rez.hpp"

#include "libGraphite/util/hashing.hpp"

// MARK: - Construction

graphite::rsrc::file::file(const std::string &path)
{
    read(path);
}

// MARK: - Hashing

auto graphite::rsrc::file::hash_for_path(const std::string &path) -> hash
{
    return hashing::xxh64(path.c_str(), path.size());
}

// MARK: - Accessors

auto graphite::rsrc::file::name() const -> std::string
{
    auto pos = m_path.find_last_of('/');
    return m_path.substr(pos + 1);
}

auto graphite::rsrc::file::path() const -> const std::string&
{
    return m_path;
}

auto graphite::rsrc::file::type_count() const -> std::size_t
{
    return m_types.size();
}

auto graphite::rsrc::file::types() const -> std::vector<type::hash>
{
    std::vector<type::hash> types;
    for (const auto& type : m_types) {
        types.emplace_back(type.first);
    }
    return std::move(types);
}

auto graphite::rsrc::file::type_codes() const -> std::vector<std::string>
{
    std::vector<std::string> types;
    for (const auto& type : m_types) {
        types.emplace_back(type.second.code());
    }
    return std::move(types);
}

auto graphite::rsrc::file::format() const -> enum format
{
    return m_format;
}

auto graphite::rsrc::file::hash_value() const -> hash
{
    return hash_for_path(m_path);
}


// MARK: - Type Management

auto graphite::rsrc::file::add_type(const struct type &type) -> void
{
    m_types.emplace(std::pair(type.hash_value(), std::move(type)));
}

auto graphite::rsrc::file::add_types(const std::vector<struct type> &types) -> void
{
    for (const auto& type : types) {
        m_types.emplace(std::pair(type.hash_value(), std::move(type)));

        // We need to resync all of the type references inside the resources for the type.
        m_types.at(type.hash_value()).sync_resource_type_references();
    }
}

auto graphite::rsrc::file::type(const std::string &code) const -> const struct type *
{
    auto it = m_types.find(type::hash_for_type_code(code));
    return (it == m_types.end()) ? nullptr : &it->second;
}

auto graphite::rsrc::file::type(type::hash hash) const -> const struct type *
{
    auto it = m_types.find(hash);
    return (it == m_types.end()) ? nullptr : &it->second;
}

auto graphite::rsrc::file::find(const std::string &type_code, resource::identifier id) const -> const struct resource *
{
    if (auto type = this->type(type_code)) {
        return type->resource_with_id(id);
    }
    return nullptr;
}

// MARK: - File Access

auto graphite::rsrc::file::read(const std::string &path) -> void
{
    m_path = path;
    m_data = new graphite::data::block(m_path);
    graphite::data::reader reader { m_data };

    if (rsrc::format::extended::parse(reader, *this)) {
        m_format = format::extended;
    }
    else if (rsrc::format::rez::parse(reader, *this)) {
        m_format = format::rez;
    }
    else if (rsrc::format::classic::parse(reader, *this)) {
        m_format = format::classic;
    }
    else {
        throw std::runtime_error("Failed to read resource file. Format not recognised: " + m_path);
    }
}

auto graphite::rsrc::file::write() -> bool
{
    return write(m_path, m_format);
}

auto graphite::rsrc::file::write(const std::string &path) -> bool
{
    return write(path, m_format);
}

auto graphite::rsrc::file::write(const std::string &path, enum format format) -> bool
{
    if (m_path != path) {
        m_path = path;
    }
    m_format = format;

    switch (m_format) {
        case format::extended:
            return rsrc::format::extended::write(*this, m_path);

        case format::rez:
            return rsrc::format::rez::write(*this, m_path);

        case format::classic:
            return rsrc::format::classic::write(*this, m_path);

        default:
            return false;
    }
}