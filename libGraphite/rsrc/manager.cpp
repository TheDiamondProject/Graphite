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

#include "libGraphite/rsrc/manager.hpp"
#include "libGraphite/font/manager.hpp"
#include "libGraphite/font/sfnt.hpp"

// MARK: - Singleton / Construction

auto graphite::rsrc::manager::shared_manager() -> manager &
{
    static rsrc::manager manager;
    return manager;
}

// MARK: - File Management

auto graphite::rsrc::manager::import_file(class file *file) -> class file *
{
    m_files.emplace(std::pair(file->hash_value(), file));
    m_file_load_order.insert(m_file_load_order.begin(), file->hash_value());

    if (
        file->type(font::outline_font::type_code()) ||
        file->type(font::descriptor::type_code()) ||
        file->type(font::bitmapped_font::type_code())
    ) {
        font::manager::shared_manager().update_font_table();
    }

    return file;
}

auto graphite::rsrc::manager::import_file(const std::string &path) -> class file *
{
    auto file = new graphite::rsrc::file(path);
    return import_file(file);
}

auto graphite::rsrc::manager::unload_file(file::hash file) -> void
{
    auto it = m_files.find(file);
    if (it != m_files.end()) {
        delete it->second;
        m_files.erase(file);
    }
}

auto graphite::rsrc::manager::unload_file(class file *file) -> void
{
    if (file) {
        unload_file(file->hash_value());
    }
}

auto graphite::rsrc::manager::unload_file(const std::string &path) -> void
{
    unload_file(file::hash_for_path(path));
}

auto graphite::rsrc::manager::file(file::hash file) -> class file *
{
    auto it = m_files.find(file);
    if (it != m_files.end()) {
        return it->second;
    }
    return nullptr;
}

auto graphite::rsrc::manager::file(file::hash file) const -> class file *
{
    auto it = m_files.find(file);
    if (it != m_files.end()) {
        return it->second;
    }
    return nullptr;
}

auto graphite::rsrc::manager::file(const std::string &path) -> class file *
{
    return file(file::hash_for_path(path));
}

auto graphite::rsrc::manager::files() const -> const std::vector<file::hash>&
{
    return m_file_load_order;
}

auto graphite::rsrc::manager::file_references() const -> std::vector<class file *>
{
    std::vector<class file *> files;
    for (auto hash : this->files()) {
        files.emplace_back(this->file(hash));
    }
    return std::move(files);
}

// MARK: - Searching

auto graphite::rsrc::manager::all_types(const std::string &type_code, const std::vector<attribute> &attributes) const -> std::vector<const struct type *>
{
    std::vector<const struct type *> types;
    for (const auto& it : m_files) {
        auto type = it.second->type(type_code, attributes);
        if (type) {
            types.emplace_back(type);
        }
    }
    return types;
}

auto graphite::rsrc::manager::find(const std::string &type_code, const std::vector<attribute> &attributes) const -> resource_result
{
    std::unordered_map<resource::identifier_hash, struct resource *> resources;
    resource_result result;

    // Gather all of the resources for the type into a single location to draw upon.
    const auto& types = all_types(type_code, attributes);
    for (const auto type : types) {
        for (const auto& resource : *type) {
            result.add(resource);
        }
    }

    return std::move(result);
}

auto graphite::rsrc::manager::find(const std::string &type_code, resource::identifier id, const std::vector<attribute> &attributes) const -> struct resource *
{
    auto resources = std::move(find(type_code, attributes));
    return resources.resource(type_code, id);
}

auto graphite::rsrc::manager::find(const std::string &type_code, const std::string &name_prefix, const std::vector<attribute> &attributes) const -> resource_result
{
    auto resources = std::move(find(type_code, attributes));
    return std::move(resources.filter([&] (struct resource *subject) -> bool {
        const auto& name = subject->name();
        if (name.length() == name_prefix.length() && name == name_prefix) {
            return true;
        }
        else if (name.length() >= name_prefix.length() && name.substr(0, name_prefix.length()) == name_prefix) {
            return true;
        }
        else {
            return false;
        }
    }));
}