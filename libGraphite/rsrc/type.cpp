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

// MARK: - Accessors

auto graphite::rsrc::type::hash_for_type_code(const std::string &code) -> hash
{
    return hashing::xxh64(code.c_str(), code.size());
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
    std::string descriptor;
    for (const auto& attribute : m_attributes) {
        descriptor += "<" + attribute.second.name() + ":" + attribute.second.string_value() + ">";
    }
    return std::move(descriptor);
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

auto graphite::rsrc::type::add_resource(resource resource) -> void
{
    m_resources.emplace_back(std::move(resource));

    auto ref = &m_resources.back();
    auto id_hash = resource::hash(ref->id());
    auto name_hash = resource::hash(ref->name());

    m_resource_id_map.emplace(std::pair(id_hash, ref));
    m_resource_name_map.emplace(std::pair(name_hash, ref));
}

auto graphite::rsrc::type::remove_resource(resource::identifier id) -> void
{
    // TODO:
}

auto graphite::rsrc::type::resource_with_id(resource::identifier id) const -> resource *
{
    for (const auto& it : m_resources) {
        if (it.id() == id) {
            auto ptr = const_cast<resource *>(&it);
            return ptr;
        }
    }
    return nullptr;
}

auto graphite::rsrc::type::resource_with_name(const std::string &name) const -> resource *
{
    for (const auto& it : m_resources) {
        if (it.name() == name) {
            auto ptr = const_cast<resource *>(&it);
            return ptr;
        }
    }
    return nullptr;
}

auto graphite::rsrc::type::begin() -> std::vector<resource>::iterator
{
    return m_resources.begin();
}

auto graphite::rsrc::type::end() -> std::vector<resource>::iterator
{
    return m_resources.end();
}

auto graphite::rsrc::type::at(int64_t idx) -> resource *
{
    if (idx < 0 || idx >= m_resources.size()) {
        return nullptr;
    }
    return &m_resources.at(idx);
}

auto graphite::rsrc::type::sync_resource_type_references() -> void
{
    for (auto& resource : *this) {
        resource.set_type(this);
    }
}