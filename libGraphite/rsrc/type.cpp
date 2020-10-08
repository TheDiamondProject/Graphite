// Copyright (c) 2020 Tom Hancocks
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

#include <utility>

// MARK: - Constructor

graphite::rsrc::type::type(std::string  code, std::map<std::string, std::string>  attributes)
	: m_code(std::move(code)), m_attributes(std::move(attributes))
{
	
}

// MARK: - Metadata Accessors

auto graphite::rsrc::type::code() const -> std::string
{
	return m_code;
}

auto graphite::rsrc::type::attributes() const -> std::map<std::string, std::string>
{
    return m_attributes;
}

auto graphite::rsrc::type::attributes_string() const -> std::string
{
    std::string text;

    for (const auto& m_attribute : m_attributes) {
        text.append(":" + m_attribute.first + "=" + (m_attribute.second));
    }

    return text;
}

// MARK: - Resource Management

auto graphite::rsrc::type::count() const -> std::size_t
{
	return m_resources.size();
}

auto graphite::rsrc::type::add_resource(const std::shared_ptr<graphite::rsrc::resource>& resource) -> void
{
	m_resources.push_back(resource);
}

auto graphite::rsrc::type::resources() const -> std::vector<std::shared_ptr<graphite::rsrc::resource>>
{
	return m_resources;
}

auto graphite::rsrc::type::get(int16_t id) const -> std::weak_ptr<graphite::rsrc::resource>
{
    for (const auto& resource : m_resources) {
        if (resource->id() == id) {
            return resource;
        }
    }
    return std::weak_ptr<graphite::rsrc::resource>();
}

auto graphite::rsrc::type::get(const std::string &name_prefix) const -> std::vector<std::shared_ptr<resource>>
{
    std::vector<std::shared_ptr<resource>> v;
    for (const auto& resource : m_resources) {
        const auto& name = resource->name();
        if (name.length() == name_prefix.length() && name == name_prefix) {
            v.emplace_back(resource);
        }
        else if (name.length() >= name_prefix.length() && name.substr(0, name_prefix.length()) == name_prefix) {
            v.emplace_back(resource);
        }
    }
    return v;
}
