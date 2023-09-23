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
#include <unordered_map>
#include <stdexcept>
#include <libData/block.hpp>
#include <libResourceCore/identifier.hpp>
#include <libResourceCore/structure/type.hpp>
#include <libResourceCore/concepts.hpp>

namespace resource_core
{
    class file
    {
    public:
        typedef std::uint64_t hash;
        enum class format { classic, extended, rez };

    public:
        file() = default;
        explicit file(const std::string& path);
        file(const file& file);
        file(file&& file) noexcept;

        ~file();

        auto operator=(const file& file) -> class file&;
        auto operator=(file&& file) noexcept -> class file&;

        static auto hash_for_path(const std::string& path) -> hash;

        auto read(const std::string& path) -> void;
        auto write() -> bool;
        auto write(const std::string& path) -> bool;
        auto write(const std::string& path, enum format format) -> bool;

        [[nodiscard]] auto name() const -> std::string;
        [[nodiscard]] auto path() const -> const std::string&;
        [[nodiscard]] auto type_count() const -> std::size_t;
        [[nodiscard]] auto types() const -> std::vector<type::hash>;
        [[nodiscard]] auto type_codes() const -> std::vector<std::string>;
        [[nodiscard]] auto format() const -> enum format;
        [[nodiscard]] auto hash_value() const -> hash;

        auto add_type(struct type *type) -> void;
        auto add_types(const std::vector<struct type *>& types) -> void;
        [[nodiscard]] auto type(const std::string& code, const std::unordered_map<std::string, std::string>& attributes) const -> const struct type *;
        [[nodiscard]] auto type(const std::string& code, const std::vector<attribute>& attributes) const -> const struct type *;
        [[nodiscard]] auto type(const std::string& code) const -> const struct type *;
        [[nodiscard]] auto type(type::hash hash) const -> const struct type *;

        auto add_resource(const std::string& type_code,
                          identifier id,
                          const std::string& name,
                          const data::block& data,
                          const std::unordered_map<std::string, std::string>& attributes = {}) -> void;

        template<isa_resource_type T>
        [[nodiscard]] auto find(resource_core::identifier id, const std::unordered_map<std::string, std::string>& attributes = {}) const -> const struct resource *
        {
            return find(T::type_code(), id, attributes);
        }

        [[nodiscard]] auto find(const std::string& type_code, identifier id, const std::unordered_map<std::string, std::string>& attributes = {}) const -> const struct instance *;
        [[nodiscard]] auto find(const std::string& type_code, identifier id, const std::vector<attribute>& attributes) const -> const struct instance *;

        template <isa_resource_type T>
        [[nodiscard]] auto load(identifier id) const -> T
        {
            if (const auto resource = find<T>(id)) {
                return std::move(T(resource->data(), resource->id(), resource->name()));
            }
            throw std::runtime_error("Resource not found: " + T::type_code() + ".#" + std::to_string(id));
        }

    private:
        std::string m_path;
        std::unordered_map<type::hash, struct type *> m_types;
        data::block *m_data { nullptr };
        enum format m_format { format::classic };
    };
}