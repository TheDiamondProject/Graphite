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

#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/rsrc/type.hpp"
#include "libGraphite/util/hashing.hpp"

// MARK: - Construction

graphite::rsrc::resource::resource(resource::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
}

graphite::rsrc::resource::resource(struct type *type, resource::identifier id, const std::string &name, data::block data)
    : m_type(type), m_id(id), m_name(name), m_data(std::move(data))
{
}

graphite::rsrc::resource::resource(const resource &resource)
    : m_type(resource.m_type),
      m_id(resource.m_id),
      m_name(resource.m_name),
      m_data(data::block(resource.m_data, true))
{
}

graphite::rsrc::resource::resource(resource &&resource) noexcept
    : m_type(resource.m_type),
      m_id(resource.m_id),
      m_name(std::move(resource.m_name)),
      m_data(std::move(resource.m_data))
{
    resource.m_type = nullptr;
}

// MARK: - Destruction

graphite::rsrc::resource::~resource()
{
}

// MARK: - Operators

auto graphite::rsrc::resource::operator=(const resource &resource) -> struct resource &
{
    if (this == const_cast<struct resource *>(&resource)) {
        return *this;
    }

    m_id = resource.m_id;
    m_type = resource.m_type;
    m_name = resource.m_name;
    m_data = resource.m_data;
    m_data_offset = resource.m_data_offset;

    return *this;
}

auto graphite::rsrc::resource::operator=(resource &&resource) noexcept -> struct resource &
{
    if (this != &resource) {
        m_id = resource.m_id;
        m_type = resource.m_type;
        m_name = std::move(resource.m_name);
        m_data = std::move(resource.m_data);
        m_data_offset = resource.m_data_offset;
    }
    return *this;
}

// MARK: - Accessors

auto graphite::rsrc::resource::id() const -> resource::identifier
{
    return m_id;
}

auto graphite::rsrc::resource::type() const -> struct type *
{
    return m_type;
}

auto graphite::rsrc::resource::name() const -> const std::string&
{
    return m_name;
}

auto graphite::rsrc::resource::type_code() const -> std::string
{
    if (m_type) {
        return m_type->code();
    }
    else {
        return "????";
    }
}

auto graphite::rsrc::resource::data() const -> const data::block&
{
    return m_data;
}

auto graphite::rsrc::resource::set_id(resource::identifier id) -> void
{
    m_id = id;
}

auto graphite::rsrc::resource::set_name(const std::string &name) -> void
{
    m_name = name;
}

auto graphite::rsrc::resource::set_type(struct type *type) -> void
{
    m_type = type;
}

auto graphite::rsrc::resource::set_data(graphite::data::block &data) -> void
{
    m_data = data;
    m_data_offset = 0;
}

// MARK: - Hashing

auto graphite::rsrc::resource::hash(identifier id) -> identifier_hash
{
    return hashing::xxh64(&id, sizeof(id));
}

auto graphite::rsrc::resource::hash(const std::string &name) -> name_hash
{
    return hashing::xxh64(name.c_str(), name.size());
}

// MARK: - Data Offsets

auto graphite::rsrc::resource::set_data_offset(std::size_t offset) -> void
{
    m_data_offset = offset;
}

auto graphite::rsrc::resource::data_offset() const -> std::size_t
{
    return m_data_offset;
}
