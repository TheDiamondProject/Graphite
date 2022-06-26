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

// MARK: - Singleton / Construction

auto graphite::rsrc::manager::shared_manager() -> manager &
{
    static rsrc::manager manager;
    return manager;
}

// MARK: - File Management

auto graphite::rsrc::manager::import_file(class file file) -> class file *
{
    auto hash = file.hash_value();
    m_files.emplace(std::pair(hash, std::move(file)));
    return &m_files.at(hash);
}

auto graphite::rsrc::manager::import_file(const std::string &path) -> class file *
{
    graphite::rsrc::file file(path);
    return import_file(std::move(file));
}

auto graphite::rsrc::manager::unload_file(file::hash file) -> void
{
    m_files.erase(file);
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
        return &it->second;
    }
    return nullptr;
}

auto graphite::rsrc::manager::file(const std::string &path) -> class file *
{
    return file(file::hash_for_path(path));
}

auto graphite::rsrc::manager::files() const -> std::vector<file::hash>
{
    std::vector<file::hash> files;
    for (const auto& it : m_files) {
        files.emplace_back(it.first);
    }
    return std::move(files);
}

auto graphite::rsrc::manager::file_references() const -> std::vector<class file *>
{
    std::vector<class file *> files;
    for (const auto& it : m_files) {
        files.emplace_back(const_cast<class file *>(&it.second));
    }
    return std::move(files);
}

// MARK: - Searching

auto graphite::rsrc::manager::all_types(const std::string &type_code, const std::vector<attribute> &attributes) const -> std::vector<struct type *>
{
    std::vector<struct type *> types;
    for (const auto& it : m_files) {
        auto type = it.second.type(type_code, attributes);

        if (type) {
            types.emplace_back(const_cast<struct type *>(type));
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
            result.add(const_cast<struct resource *>(&resource));
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