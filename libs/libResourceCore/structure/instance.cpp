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

#include <libResourceCore/structure/instance.hpp>
#include <libResourceCore/structure/type.hpp>
#include <libHashing/xxhash/xxhash.hpp>

// MARK: - Construction

resource_core::instance::instance(resource_core::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{}

resource_core::instance::instance(struct type *type, resource_core::identifier id, const std::string &name, data::block data)
    : m_type(type), m_id(id), m_name(name), m_data(std::move(data))
{
}

resource_core::instance::instance(const instance &instance)
    : m_type(instance.m_type),
      m_id(instance.m_id),
      m_name(instance.m_name),
      m_data(data::block(instance.m_data, true))
{
}

resource_core::instance::instance(instance &&instance) noexcept
    : m_type(instance.m_type),
      m_id(instance.m_id),
      m_name(std::move(instance.m_name)),
      m_data(std::move(instance.m_data))
{
    instance.m_type = nullptr;
}

// MARK: - Operators

auto resource_core::instance::operator=(const instance &instance) -> struct instance&
{
    if (this == const_cast<struct instance *>(&instance)) {
        return *this;
    }

    m_id = instance.m_id;
    m_type = instance.m_type;
    m_name = instance.m_name;
    m_data = instance.m_data;
    m_data_offset = instance.m_data_offset;

    return *this;
}

auto resource_core::instance::operator=(instance &&instance) noexcept -> struct instance&
{
    if (this != &instance) {
        m_id = instance.m_id;
        m_type = instance.m_type;
        m_name = std::move(instance.m_name);
        m_data = std::move(instance.m_data);
        m_data_offset = instance.m_data_offset;
    }
    return *this;
}

// MARK: - Accessors

auto resource_core::instance::id() const -> identifier
{
    return m_id;
}

auto resource_core::instance::type() const -> struct type *
{
    return m_type;
}

auto resource_core::instance::name() const -> const std::string&
{
    return m_name;
}

auto resource_core::instance::type_code() const -> std::string
{
    if (m_type) {
        return m_type->code();
    }
    else {
        return type::unknown_type_code;
    }
}

auto resource_core::instance::data() const -> const data::block&
{
    return m_data;
}

auto resource_core::instance::set_id(identifier id) -> void
{
    m_id = id;
}

auto resource_core::instance::set_name(const std::string &name) -> void
{
    m_name = name;
}

auto resource_core::instance::set_type(struct type *type) -> void
{
    m_type = type;
}

auto resource_core::instance::set_data(data::block& data) -> void
{
    m_data = data;
    m_data_offset = 0;
}

// MARK: - Hashing

auto resource_core::instance::hash(identifier id) -> identifier_hash
{
    return hashing::xxh64(&id, sizeof(id));
}

auto resource_core::instance::hash(const std::string &name) -> name_hash
{
    return hashing::xxh64(name.c_str(), name.size());
}

// MARK: - Data Offsets

auto resource_core::instance::set_data_offset(std::size_t offset) -> void
{
    m_data_offset = offset;
}

auto resource_core::instance::data_offset() const -> std::size_t
{
    return m_data_offset;
}
