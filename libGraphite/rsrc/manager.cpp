// Copyright (c) 2020 Tom Hancocks
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

#include <iterator>
#include "libGraphite/rsrc/manager.hpp"
#include "libGraphite/rsrc/file.hpp"

// MARK: - Singleton

graphite::rsrc::manager::manager()
{
    
}

auto graphite::rsrc::manager::shared_manager() -> graphite::rsrc::manager&
{
    static rsrc::manager manager;
    return manager;
}

// MARK: - File Management

auto graphite::rsrc::manager::import_file(std::shared_ptr<graphite::rsrc::file> file) -> void
{
    m_files.push_back(file);
}

auto graphite::rsrc::manager::files() const -> std::vector<std::shared_ptr<file>>
{
    return m_files;
}

// MARK: - Resource Look Up

auto graphite::rsrc::manager::find(const std::string& type, const int64_t& id) const -> std::weak_ptr<graphite::rsrc::resource>
{
    for (auto i = m_files.rbegin(); i != m_files.rend(); ++i) {
        auto res = (*i)->find(type, id);
        if (!res.expired()) {
            return res;
        }
    }
    return std::weak_ptr<graphite::rsrc::resource>();
}

auto graphite::rsrc::manager::get_type(const std::string &type) const -> std::vector<std::weak_ptr<rsrc::type>>
{
    std::vector<std::weak_ptr<rsrc::type>> v;
    for (auto i = m_files.rbegin(); i != m_files.rend(); ++i) {
        v.emplace_back((*i)->type_container(type));
    }
    return v;
}