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

#include <stdexcept>
#include <unordered_map>
#include "libGraphite/rsrc/file.hpp"
#include "libGraphite/rsrc/type.hpp"
#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/rsrc/attribute.hpp"
#include "libGraphite/rsrc/result.hpp"
#include "libGraphite/util/concepts.hpp"

namespace graphite::rsrc
{
    class manager
    {
    public:
        manager(const manager&) = delete;
        manager(manager&&) = delete;
        auto operator=(const manager&) -> manager& = delete;
        auto operator=(manager&&) -> manager& = delete;

        static auto shared_manager() -> manager&;

        auto import_file(file file) -> class file *;
        auto import_file(const std::string& path) -> class file *;

        auto unload_file(class file *file) -> void;
        auto unload_file(file::hash file) -> void;
        auto unload_file(const std::string& path) -> void;

        auto file(file::hash file) -> class file *;
        auto file(const std::string& path) -> class file *;

        [[nodiscard]] auto files() const -> std::vector<file::hash>;
        [[nodiscard]] auto file_references() const -> std::vector<class file *>;

        [[nodiscard]] auto find(const std::string& type_code, const std::vector<attribute>& attributes = {}) const -> resource_result;
        [[nodiscard]] auto find(const std::string& type_code, resource::identifier id, const std::vector<attribute>& attributes = {}) const -> struct resource *;
        [[nodiscard]] auto find(const std::string& type_code, const std::string& name_prefix, const std::vector<attribute>& attributes = {}) const -> resource_result;

        template<resource_type T>
        [[nodiscard]] auto find(const std::vector<attribute>& attributes = {}) const -> resource_result
        {
            return find(T::type_code(), attributes);
        }

        template<resource_type T>
        [[nodiscard]] auto find(resource::identifier id, const std::vector<attribute>& attributes = {}) const -> struct resource *
        {
            return find(T::type_code(), id, attributes);
        }

        template<resource_type T>
        [[nodiscard]] auto find(const std::string& name_prefix, const std::vector<attribute>& attributes = {}) const -> resource_result
        {
            return find(T::type_code(), name_prefix, attributes);
        }

        [[nodiscard]] auto all_types(const std::string& type_code, const std::vector<attribute>& attributes = {}) const -> std::vector<struct type *>;

        template<resource_type T>
        [[nodiscard]] auto all_types(const std::vector<attribute>& attributes = {}) const -> std::vector<struct vector *>
        {
            return all_types(T::type_code(), attributes);
        }

        template<resource_type T>
        [[nodiscard]] auto load(resource::identifier id, const std::vector<attribute>& attributes = {}) const -> T
        {
            if (const auto resource = find<T>(id, attributes)) {
                return std::move(T(resource->data(), resource->id(), resource->name()));
            }
            throw std::runtime_error("Resource not found: " + T::type_code() + ".#" + std::to_string(id));
        };

    private:
        std::unordered_map<file::hash, class file> m_files;

        manager() = default;
    };
}

