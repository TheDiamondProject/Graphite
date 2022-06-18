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

#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>
#include "libGraphite/rsrc/resource.hpp"

namespace graphite::rsrc
{
    struct resource_result
    {
    public:
        typedef std::uint64_t hash;

        struct iterator
        {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = struct resource;
            using pointer = value_type*;
            using reference = value_type&;

            iterator(resource_result *ptr, std::uint64_t index) : m_ptr(ptr), m_index(index) {};

            auto operator*() const -> reference { return *get(); }
            auto operator->() -> pointer { return const_cast<pointer>(get()); }

            auto operator++() -> iterator& {
                m_index++;
                if (m_index >= m_ptr->m_sorted_keys.size()) {
                    m_index = std::numeric_limits<uint64_t>::max();
                }
                return *this;
            }
            auto operator++(int) -> iterator { auto tmp = *this; ++(*this); return tmp; }

            friend auto operator==(const iterator& lhs, const iterator& rhs) -> bool
            {
                return (lhs.m_ptr == rhs.m_ptr) && (lhs.m_index == rhs.m_index);
            }

            friend auto operator!= (const iterator& lhs, const iterator& rhs) -> bool
            {
                return (lhs.m_ptr != rhs.m_ptr) || (lhs.m_index != rhs.m_index);
            }

        private:
            resource_result *m_ptr { nullptr };
            std::uint64_t m_index { 0 };

            auto get() const -> pointer
            {
                if (m_index == std::numeric_limits<uint64_t>::max()) {
                    return nullptr;
                }
                auto key = m_ptr->m_sorted_keys.at(m_index);
                auto value = m_ptr->m_resources.at(key);
                return value;
            };
        };

    public:
        resource_result() = default;

        static auto sort_key(struct resource *resource) -> hash;
        static auto sort_key(const std::string& type_code, resource::identifier id) -> hash;

        auto add(struct resource *resource) -> void;

        auto finalize() -> void;
        auto sort() -> void;

        auto begin() -> iterator;
        auto end() -> iterator;

        [[nodiscard]] auto filter(const std::function<auto(struct resource *)->bool>& fn) const -> resource_result;

        [[nodiscard]] auto size() const -> std::size_t;
        [[nodiscard]] auto at(std::uint64_t idx) const -> struct resource *;
        [[nodiscard]] auto id(resource::identifier id) const -> struct resource *;
        [[nodiscard]] auto resource(const std::string& type_code, resource::identifier id) const -> struct resource *;

    private:
        typedef std::uint64_t resource_sort_key;
        bool m_finalized { false };
        std::unordered_map<resource_sort_key, struct resource *> m_resources;
        std::vector<resource_sort_key> m_sorted_keys;
    };

}
