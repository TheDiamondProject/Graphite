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
#include <libResourceCore/structure/attribute.hpp>
#include <libResourceCore/structure/instance.hpp>

namespace resource_core
{
    struct type
    {
    public:
        typedef std::uint64_t hash;

        static constexpr const char *unknown_type_code = "????";

    public:
        explicit type(const std::string& code);
        type(const type& type);
        type(type&& type) noexcept;

        ~type();

        auto operator=(const type& type) -> struct type&;
        auto operator=(type&& type) noexcept -> struct type&;

        static auto attribute_string(const std::unordered_map<attribute::hash, attribute>& attributes) -> std::string;
        static auto hash_for_type_code(const std::string& code) -> hash;
        static auto hash_for_type_code(const std::string& code, const std::unordered_map<attribute::hash, attribute>& attributes) -> hash;

        [[nodiscard]] auto hash_value() const -> hash;
        [[nodiscard]] auto code() const -> const std::string&;
        [[nodiscard]] auto attributes() const -> const std::unordered_map<attribute::hash, attribute>&;
        [[nodiscard]] auto count() const -> std::size_t;
        [[nodiscard]] auto attribute_descriptor_string() const -> std::string;

        auto add_attribute(const std::string& name, const std::string& value) -> void;

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        auto add_attribute(const std::string& name, T value) -> void;

        [[nodiscard]] auto has_resource(identifier id) const -> bool;
        [[nodiscard]] auto has_resource(const std::string& name) const -> bool;

        auto add_resource(instance *resource) -> void;
        auto remove_resource(identifier id) -> void;

        [[nodiscard]] auto resource_with_id(identifier id) const -> instance *;
        [[nodiscard]] auto resource_with_name(const std::string& name) const -> instance *;

        auto begin() -> std::vector<instance *>::iterator;
        auto end() -> std::vector<instance *>::iterator;
        [[nodiscard]] auto begin() const -> std::vector<instance *>::const_iterator;
        [[nodiscard]] auto end() const-> std::vector<instance *>::const_iterator;
        auto at(std::int64_t idx) -> instance *;

    private:
        std::string m_code;
        std::vector<instance *> m_resources;
        std::unordered_map<instance::identifier_hash, instance *> m_resource_id_map {};
        std::unordered_map<instance::name_hash, instance *> m_resource_name_map {};
        std::unordered_map<attribute::hash , attribute> m_attributes {};

    };
}
