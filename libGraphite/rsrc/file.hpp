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
#include "libGraphite/data/data.hpp"
#include "libGraphite/rsrc/type.hpp"

#if !defined(GRAPHITE_RSRC_FILE)
#define GRAPHITE_RSRC_FILE

namespace graphite::rsrc {

    /**
     * The `graphite::rsrc::file` represents a ResourceFork file.
     */
    class file
    {
    public:

        enum flags : uint8_t { };

        /**
         * Denotes the format of a resource file being represented.
         *
         *  + classic
         *      The classic resource file format is that found in the original Macintosh
         *      operating systems. This is the only format used by the Diamond Project.
         *
         *  + extended
         *      The extended resource file format is extremely similar to the classic
         *      format, but has a number of extensions for modern use. This is primarily
         *      used by Kestrel.
         *
         *  + rez
         *      The rez resource file format was a format that was created for use within
         *      EV Nova for Windows. This is used as a compatibility option for Kestrel.
         */
        enum format { classic, extended, rez };

    private:
        std::string m_path;
        std::vector<std::shared_ptr<type>> m_types;
        std::shared_ptr<graphite::data::data> m_data { nullptr };
        format m_format { classic };

    public:
        /**
         * Construct a new blank `graphite::rsrc::file`.
         */
        file() = default;

        /**
         * Construct a new `graphite::rsrc::file` by loading the contents of the specified
         * file.
         */
        explicit file(std::string path);

        /**
         * Read and parse the contents of the resource file at the specified location.
         * Warning: This will destroy all existing information in the resource file.
         */
        auto read(const std::string& path) -> void;

        /**
         * Write the contents of the the resource file to disk. If no location is specified,
         * then it will use the original read path (if it exists).
         */
        auto write(const std::string& path = "", enum format fmt = classic) -> void;

        /**
         * Returns the name of the file.
         */
        [[nodiscard]] auto name() const -> std::string;

        /**
         * Returns the number of types contained in the resource file.
         */
        [[nodiscard]] auto type_count() const -> std::size_t;
        
        /**
         * Returns the list of all types contained in the resource file.
         */
        [[nodiscard]] auto types() const -> std::vector<std::shared_ptr<type>>;

        /**
         * Reports the current format of the resource file.
         */
        [[nodiscard]] auto current_format() const -> format;

        /**
         * Add a resource into the receiver.
         */
        auto add_resource(const std::string& type,
                          const int64_t& id,
                          const std::string& name,
                          const std::shared_ptr<graphite::data::data>& data,
                          const std::map<std::string, std::string>& attributes = {}) -> void;

        /**
         * Retrieve a type container for the specified type code. If a container
         * does not exist then create one.
         */
        auto type_container(const std::string& code,
                            const std::map<std::string, std::string>& attributes = {}) -> std::weak_ptr<graphite::rsrc::type>;

        /**
         * Attempt to get the resource of the specified type and id.
         */
        auto find(const std::string& type, const int64_t& id, const std::map<std::string, std::string> &attributes) -> std::weak_ptr<resource>;
    };

}

#endif
