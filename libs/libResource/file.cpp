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

#include <libHashing/xxhash/xxhash.hpp>
#include <libData/reader.hpp>
#include <libResource/file.hpp>
#include <libResource/structure/type.hpp>
#include <libResource/structure/instance.hpp>
#include <libResource/format/classic/classic.hpp>
#include <libResource/format/extended/extended.hpp>
#include <libResource/format/rez/rez.hpp>

// MARK: - Construction

resource::file::file(const std::string &path)
{
    read(path);
}

resource::file::file(const file &file)
    : m_format(file.m_format),
      m_path(file.m_path),
      m_data(new data::block(*file.m_data))
{
    // Copy the types.
    for (const auto& it : file.m_types) {
        m_types.emplace(std::pair(it.first, new struct type(*it.second)));
    }
}

resource::file::file(file &&file) noexcept
    : m_format(file.m_format),
      m_path(std::move(file.m_path)),
      m_data(file.m_data),
      m_types(std::move(file.m_types))
{
    file.m_data = nullptr;
    file.m_types = {};
}

// MARK: - Destruction

resource::file::~file()
{
    for (auto& it : m_types) {
        delete it.second;
    }
    delete m_data;
}

// MARK: - Operators

auto resource::file::operator=(const file &file) -> class file &
{
    if (this == const_cast<class file *>(&file)) {
        return *this;
    }

    m_path = file.m_path;
    m_format = file.m_format;
    m_data = new data::block(*file.m_data);

    // Copy the types.
    for (const auto& it : file.m_types) {
        m_types.emplace(it.first, new struct type(*it.second));
    }

    return *this;
}

auto resource::file::operator=(file &&file) noexcept -> class file &
{
    if (this != &file) {
        m_path = std::move(file.m_path);
        m_format = file.m_format;
        m_data = file.m_data;
        m_types = std::move(file.m_types);

        file.m_data = nullptr;
        file.m_types = {};
    }
    return *this;
}

// MARK: - Hashing

auto resource::file::hash_for_path(const std::string &path) -> hash
{
    return hashing::xxh64(path.c_str(), path.size());
}

// MARK: - Accessors

auto resource::file::name() const -> std::string
{
    auto pos = m_path.find_last_of('/');
    return m_path.substr(pos + 1);
}

auto resource::file::path() const -> const std::string&
{
    return m_path;
}

auto resource::file::type_count() const -> std::size_t
{
    return m_types.size();
}

auto resource::file::types() const -> std::vector<type::hash>
{
    std::vector<type::hash> types;
    for (const auto& type : m_types) {
        types.emplace_back(type.first);
    }
    return std::move(types);
}

auto resource::file::type_codes() const -> std::vector<std::string>
{
    std::vector<std::string> types;
    for (const auto& type : m_types) {
        types.emplace_back(type.second->code());
    }
    return std::move(types);
}

auto resource::file::format() const -> enum format
{
    return m_format;
}

auto resource::file::hash_value() const -> hash
{
    return hash_for_path(m_path);
}


// MARK: - Type Management

auto resource::file::add_resource(const std::string &type_code,
                                  identifier id,
                                  const std::string &name,
                                  const data::block &data,
                                  const std::unordered_map<std::string, std::string> &attributes) -> void
{
    auto resource = new struct instance(nullptr, id, name, data);

    std::unordered_map<attribute::hash, attribute> type_attributes;
    for (const auto& it : attributes) {
        attribute attr(it.first, it.second);
        type_attributes.emplace(std::pair(attr.hash_value(), std::move(attr)));
    }

    auto type_hash = type::hash_for_type_code(type_code, type_attributes);
    auto it = m_types.find(type_hash);
    if (it == m_types.end()) {
        // The type doesn't exist, so we need to create it.
        auto type = new struct type(type_code);

        for (const auto& attr : type_attributes) {
            type->add_attribute(attr.second.name(), attr.second.string_value());
        }

        m_types.emplace(std::pair(type_hash, type));
        type->add_resource(resource);
    }
    else {
        // Found the type, add the resource to it.
        it->second->add_resource(resource);
    }
}

auto resource::file::add_type(struct type *type) -> void
{
    m_types.emplace(std::pair(type->hash_value(), type));
}

auto resource::file::add_types(const std::vector<struct type *> &types) -> void
{
    for (const auto type : types) {
        m_types.emplace(std::pair(type->hash_value(), type));
    }
}

auto resource::file::type(const std::string &code, const std::unordered_map<std::string, std::string>& attributes) const -> const struct type *
{
    // Convert the unordered map to what is required.
    std::unordered_map<attribute::hash, attribute> attributes_map;
    for (const auto& it : attributes) {
        attribute attr(it.first, it.second);
        attributes_map.emplace(attr.hash_value(), std::move(attr));
    }

    auto it = m_types.find(type::hash_for_type_code(code, attributes_map));
    return (it == m_types.end()) ? nullptr : it->second;
}

auto resource::file::type(const std::string& code, const std::vector<attribute>& attributes) const -> const struct type *
{
    bool universal_namespace = false;
    std::unordered_map<attribute::hash, attribute> attributes_map;
    for (const auto& it : attributes) {
        if (it.name() == "namespace" && it.string_value() == "*") {
            universal_namespace = true;
        }
        else {
            attributes_map.emplace(std::pair(it.hash_value(), it));
        }
    }

    if (universal_namespace) {
        // TODO: Merge types together so that we can browse all namespaces.
        for (const auto& it : m_types) {
            if (it.second->code() == code) {
                return it.second;
            }
        }
    }
    else {
        auto it = m_types.find(type::hash_for_type_code(code, attributes_map));
        return (it == m_types.end()) ? nullptr : it->second;
    }

    return nullptr;
}

auto resource::file::type(const std::string& code) const -> const struct type *
{
    auto it = m_types.find(type::hash_for_type_code(code));
    return (it == m_types.end()) ? nullptr : it->second;
}


auto resource::file::type(type::hash hash) const -> const struct type *
{
    auto it = m_types.find(hash);
    return (it == m_types.end()) ? nullptr : it->second;
}

auto resource::file::find(const std::string &type_code, identifier id, const std::unordered_map<std::string, std::string>& attributes) const -> const struct instance *
{
    if (auto type = const_cast<::resource::type *>(this->type(type_code, attributes))) {
        return type->resource_with_id(id);
    }
    return nullptr;
}

auto resource::file::find(const std::string &type_code, identifier id, const std::vector<attribute>& attributes) const -> const struct instance *
{
    if (auto type = const_cast<::resource::type *>(this->type(type_code, attributes))) {
        return type->resource_with_id(id);
    }
    return nullptr;
}

// MARK: - File Access

auto resource::file::read(const std::string &path) -> void
{
    m_path = path;
    m_data = new data::block(m_path);
    data::reader reader(m_data);

    if (::resource::format::extended::parse(reader, *this)) {
        m_format = format::extended;
    }
    else if (::resource::format::rez::parse(reader, *this)) {
        m_format = format::rez;
    }
    else if (::resource::format::classic::parse(reader, *this)) {
        m_format = format::classic;
    }
    else {
        throw std::runtime_error("Failed to read resource file. Format not recognised: " + m_path);
    }
}

auto resource::file::write() -> bool
{
    return write(m_path, m_format);
}

auto resource::file::write(const std::string &path) -> bool
{
    return write(path, m_format);
}

auto resource::file::write(const std::string &path, enum format format) -> bool
{
    if (m_path != path) {
        m_path = path;
    }
    m_format = format;

    switch (m_format) {
        case format::extended:
            return ::resource::format::extended::write(*this, m_path);

        case format::rez:
            return ::resource::format::rez::write(*this, m_path);

        case format::classic:
            return ::resource::format::classic::write(*this, m_path);

        default:
            return false;
    }
}