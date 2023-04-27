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

#include "libGraphite/rsrc/type.hpp"
#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/util/hashing.hpp"

// MARK: - Construction

graphite::rsrc::type::type(const std::string &code)
    : m_code(code)
{
}

graphite::rsrc::type::type(const type &type)
    : m_code(type.m_code),
      m_attributes(type.m_attributes)
{
    for (const auto& resource : type.m_resources) {
        add_resource(new rsrc::resource(*resource));
    }
}

graphite::rsrc::type::type(type &&type) noexcept
    : m_code(std::move(type.m_code)),
      m_resources(std::move(type.m_resources)),
      m_resource_id_map(std::move(type.m_resource_id_map)),
      m_resource_name_map(std::move(type.m_resource_name_map)),
      m_attributes(std::move(type.m_attributes))
{
    type.m_resources = {};
    type.m_resource_id_map = {};
    type.m_resource_name_map = {};
}

// MARK: - Destruction

graphite::rsrc::type::~type()
{
    for (auto it : m_resources) {
        delete it;
    }
}

// MARK: - Operators

auto graphite::rsrc::type::operator=(const type &type) -> struct type&
{
    if (this == const_cast<struct type *>(&type)) {
        return *this;
    }

    m_code = type.m_code;
    m_attributes = type.m_attributes;

    for (const auto& resource : type.m_resources) {
        add_resource(new rsrc::resource(*resource));
    }

    return *this;
}

auto graphite::rsrc::type::operator=(type &&type) noexcept -> struct type&
{
    if (this != &type) {
        m_code = std::move(type.m_code);
        m_resources = std::move(type.m_resources);
        m_resource_id_map = std::move(type.m_resource_id_map);
        m_resource_name_map = std::move(type.m_resource_name_map);
        m_attributes = std::move(type.m_attributes);

        type.m_resources = {};
        type.m_resource_id_map = {};
        type.m_resource_name_map = {};
    }
    return *this;
}

// MARK: - Accessors

auto graphite::rsrc::type::attribute_string(const std::unordered_map<attribute::hash, attribute> &attributes) -> std::string
{
    std::string descriptor;
    for (const auto& attribute : attributes) {
        descriptor += "<" + attribute.second.name() + ":" + attribute.second.string_value() + ">";
    }
    return std::move(descriptor);
}

auto graphite::rsrc::type::hash_for_type_code(const std::string &code) -> hash
{
    return hashing::xxh64(code.c_str(), code.size());
}

auto graphite::rsrc::type::hash_for_type_code(const std::string &code, const std::unordered_map<attribute::hash, attribute> &attributes) -> graphite::rsrc::type::hash
{
    std::string assembled_code { code };
    if (!attributes.empty()) {
        assembled_code += ":" + attribute_string(attributes);
    }
    return hash_for_type_code(assembled_code);
}

auto graphite::rsrc::type::hash_value() const -> hash
{
    std::string code { m_code };
    if (!m_attributes.empty()) {
        code += ":" + attribute_descriptor_string();
    }
    return hash_for_type_code(code);
}

auto graphite::rsrc::type::code() const -> const std::string&
{
    return m_code;
}

auto graphite::rsrc::type::attributes() const -> const std::unordered_map<attribute::hash, attribute>&
{
    return m_attributes;
}

auto graphite::rsrc::type::count() const -> std::size_t
{
    return m_resources.size();
}

auto graphite::rsrc::type::attribute_descriptor_string() const -> std::string
{
    return std::move(attribute_string(m_attributes));
}

// MARK: - Attribute Management

auto graphite::rsrc::type::add_attribute(const std::string& name, const std::string& value) -> void
{
    attribute attr { name, value };
    m_attributes.emplace(std::pair(attr.hash_value(), std::move(attr)));
}

template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type*>
auto graphite::rsrc::type::add_attribute(const std::string &name, T value) -> void
{
    attribute attr { name, value };
    m_attributes.template emplace(std::pair(attr.hash_value(), std::move(attr)));
}

// MARK: - Resource Management

auto graphite::rsrc::type::has_resource(resource::identifier id) const -> bool
{
    auto hash = hashing::xxh64(&id, sizeof(id));
    return (m_resource_id_map.find(hash) != m_resource_id_map.end());
}

auto graphite::rsrc::type::has_resource(const std::string &name) const -> bool
{
    auto hash = hashing::xxh64(name.c_str(), name.size());
    return (m_resource_name_map.find(hash) != m_resource_name_map.end());
}

auto graphite::rsrc::type::add_resource(resource *resource) -> void
{
    m_resources.emplace_back(resource);
    resource->set_type(this);

    auto id_hash = resource::hash(resource->id());
    auto name_hash = resource::hash(resource->name());

    m_resource_id_map.emplace(std::pair(id_hash, resource));
    m_resource_name_map.emplace(std::pair(name_hash, resource));
}

auto graphite::rsrc::type::remove_resource(resource::identifier id) -> void
{
    // TODO:
}

auto graphite::rsrc::type::resource_with_id(resource::identifier id) const -> resource *
{
    for (const auto& it : m_resources) {
        if (it->id() == id) {
            return it;
        }
    }
    return nullptr;
}

auto graphite::rsrc::type::resource_with_name(const std::string &name) const -> resource *
{
    for (const auto& it : m_resources) {
        if (it->name() == name) {
            return it;
        }
    }
    return nullptr;
}

auto graphite::rsrc::type::begin() -> std::vector<resource *>::iterator
{
    return m_resources.begin();
}

auto graphite::rsrc::type::end() -> std::vector<resource *>::iterator
{
    return m_resources.end();
}

auto graphite::rsrc::type::begin() const -> std::vector<resource *>::const_iterator
{
    return m_resources.cbegin();
}

auto graphite::rsrc::type::end() const -> std::vector<resource *>::const_iterator
{
    return m_resources.end();
}

auto graphite::rsrc::type::at(int64_t idx) -> resource *
{
    if (idx < 0 || idx >= m_resources.size()) {
        return nullptr;
    }
    return m_resources.at(idx);
}

