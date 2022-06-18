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

#include <string>
#include <type_traits>
#include <unordered_map>
#include "libGraphite/rsrc/attribute.hpp"
#include "libGraphite/rsrc/resource.hpp"

namespace graphite::rsrc
{
    struct type
    {
    public:
        typedef std::uint64_t hash;

    public:
        explicit type(const std::string& code);

        static auto hash_for_type_code(const std::string& code) -> hash;

        [[nodiscard]] auto hash_value() const -> hash;
        [[nodiscard]] auto code() const -> const std::string&;
        [[nodiscard]] auto attributes() const -> const std::unordered_map<attribute::hash, attribute>&;
        [[nodiscard]] auto count() const -> std::size_t;
        [[nodiscard]] auto attribute_descriptor_string() const -> std::string;

        auto add_attribute(const std::string& name, const std::string& value) -> void;

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto add_attribute(const std::string& name, T value) -> void;

        [[nodiscard]] auto has_resource(resource::identifier id) const -> bool;
        [[nodiscard]] auto has_resource(const std::string& name) const -> bool;

        auto add_resource(const resource& resource) -> void;
        auto remove_resource(resource::identifier id) -> void;

        [[nodiscard]] auto resource_with_id(resource::identifier id) const -> resource *;
        [[nodiscard]] auto resource_with_name(const std::string& name) const -> resource *;

        auto begin() -> std::vector<resource>::iterator;
        auto end() -> std::vector<resource>::iterator;
        auto at(int64_t idx) -> resource *;

        auto sync_resource_type_references() -> void;

    private:
        std::string m_code;
        std::vector<resource> m_resources;
        std::unordered_map<resource::identifier_hash, resource *> m_resource_id_map {};
        std::unordered_map<resource::name_hash, resource *> m_resource_name_map {};
        std::unordered_map<attribute::hash , attribute> m_attributes {};

    };
}
