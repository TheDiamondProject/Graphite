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

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "libGraphite/rsrc/file.hpp"

#if !defined(GRAPHITE_RSRC_MANAGER)
#define GRAPHITE_RSRC_MANAGER

namespace graphite { namespace rsrc {

    /**
     * The Manager is a shared object within an application that is keeps track of
     * all loaded resources.
     *
     * Resources are loaded in through a resource file and then placed into the
     * resource manager to be "managed" if a newly imported resource conflicts with
     * an existing resource, the existing resource will be replaced by the newer
     */
    class manager
    {
    private:
        std::vector<std::shared_ptr<file>> m_files;
        manager();

    public:
    	manager(const manager&) = delete;
        manager& operator=(const manager &) = delete;
        manager(manager &&) = delete;
        manager & operator=(manager &&) = delete;

        /**
         * The Resource Manager is a singleton due to the "resource space"
         * of a process being shared. All resource files loaded into a process
         * occupy the same space and are able to override each other, depending
         * on load order.
         */
        static auto shared_manager() -> manager&;

        /**
         * Import the specified file into the Manager. This will trigger a parsing
         * of the file if it hasn't already occurred.
         */
        auto import_file(std::shared_ptr<file> file) -> void;

        /**
         *
         */
        auto files() const -> std::vector<std::shared_ptr<file>>;

        /**
         * Attempt to get the resource of the specified type and id.
         */
        auto find(const std::string& type, const int64_t& id, const std::map<std::string, std::string>& attributes = {}) const -> std::weak_ptr<resource>;

        /**
         * Returns a list of type containers for the specified type code.
         */
        auto get_type(const std::string& type, const std::map<std::string, std::string>& attributes = {}) const -> std::vector<std::weak_ptr<rsrc::type>>;
    };

}}

#endif