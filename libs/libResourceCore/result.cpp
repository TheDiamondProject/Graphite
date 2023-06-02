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

#include <limits>
#include <libResourceCore/result.hpp>
#include <libResourceCore/structure/instance.hpp>
#include <libHashing/xxhash/xxhash.hpp>

// MARK: - Hashing

auto resource_core::result::sort_key(struct instance *resource) -> hash
{
    return sort_key(resource->type_code(), resource->id());
}

auto resource_core::result::sort_key(const std::string& type_code, identifier id) -> hash
{
    std::string key = type_code + "." + std::to_string(id);
    return hashing::xxh64(key.c_str(), key.size());
}

// MARK: - Item Management

auto resource_core::result::add(struct instance *instance) -> void
{
    if (!instance) {
        return;
    }

    if (m_finalized) {
        // TODO: Issue a warning
        return;
    }

    auto key = sort_key(instance);
    m_resources.emplace(key, instance);
    m_sorted_keys.emplace_back(key);
}

auto resource_core::result::finalize() -> void
{
    m_finalized = true;
    sort();
}

auto resource_core::result::size() const -> std::size_t
{
    return m_resources.size();
}

// MARK: - Sorting

auto resource_core::result::sort() -> void
{
    if (!m_finalized) {
        // TODO: Issue a warning
        return;
    }

    // TODO: Make this customisable in the future and more advanced?
    std::sort(m_sorted_keys.begin(), m_sorted_keys.end(), [&] (resource_sort_key lhs, resource_sort_key rhs) {
        const auto& lhs_resource = m_resources.at(lhs);
        const auto& rhs_resource = m_resources.at(rhs);
        return lhs_resource->id() < rhs_resource->id();
    });
}

// MARK: - Look up

auto resource_core::result::begin() -> iterator
{
    return { this, 0 };
}

auto resource_core::result::end() -> iterator
{
    return { this, std::numeric_limits<uint64_t>::max() };
}

auto resource_core::result::at(std::uint64_t idx) const -> struct instance *
{
    auto it = m_resources.find(m_sorted_keys.at(idx));
    return (it != m_resources.end()) ? it->second : nullptr;
}

auto resource_core::result::id(identifier id) const -> struct instance *
{
    for (const auto resource : m_resources) {
        if (resource.second->id() == id) {
            return resource.second;
        }
    }
    return nullptr;
}

auto resource_core::result::resource(const std::string &type_code, identifier id) const -> struct instance *
{
    auto key = sort_key(type_code, id);
    auto it = m_resources.find(key);
    return (it != m_resources.end()) ? it->second : nullptr;
}

auto resource_core::result::filter(const std::function<auto(struct instance *)->bool>& fn) const -> result
{
    result result;

    for (const auto resource : m_resources) {
        if (fn(resource.second)) {
            result.add(resource.second);
        }
    }
    result.sort();

    return std::move(result);
}
