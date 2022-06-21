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

#include "libGraphite/rsrc/result.hpp"
#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/util/hashing.hpp"

// MARK: - Hashing

auto graphite::rsrc::resource_result::sort_key(struct resource *resource) -> hash
{
    return sort_key(resource->type_code(), resource->id());
}

auto graphite::rsrc::resource_result::sort_key(const std::string& type_code, resource::identifier id) -> hash
{
    std::string key { type_code + "." + std::to_string(id) };
    return hashing::xxh64(key.c_str(), key.size());
}

// MARK: - Item Management

auto graphite::rsrc::resource_result::add(struct resource *resource) -> void
{
    if (!resource) {
        return;
    }

    if (m_finalized) {
        // TODO: Issue a warning
        return;
    }

    auto key = resource_result::sort_key(resource);
    m_resources.emplace(std::pair(key, resource));
    m_sorted_keys.emplace_back(key);
}

auto graphite::rsrc::resource_result::finalize() -> void
{
    m_finalized = true;
    sort();
}

auto graphite::rsrc::resource_result::size() const -> std::size_t
{
    return m_resources.size();
}

// MARK: - Sorting

auto graphite::rsrc::resource_result::sort() -> void
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

auto graphite::rsrc::resource_result::begin() -> iterator
{
    return { this, 0 };
}

auto graphite::rsrc::resource_result::end() -> iterator
{
    return { this, std::numeric_limits<uint64_t>::max() };
}

auto graphite::rsrc::resource_result::at(std::uint64_t idx) const -> struct resource *
{
    return m_resources.at(m_sorted_keys.at(idx));
}

auto graphite::rsrc::resource_result::id(resource::identifier id) const -> struct resource *
{
    for (const auto resource : m_resources) {
        if (resource.second->id() == id) {
            return resource.second;
        }
    }
    return nullptr;
}

auto graphite::rsrc::resource_result::resource(const std::string &type_code, resource::identifier id) const -> struct resource *
{
    return m_resources.at(resource_result::sort_key(type_code, id));
}

auto graphite::rsrc::resource_result::filter(const std::function<auto(struct resource *)->bool>& fn) const -> resource_result
{
    resource_result result;

    for (const auto resource : m_resources) {
        if (fn(resource.second)) {
            result.add(resource.second);
        }
    }
    result.sort();

    return std::move(result);
}
