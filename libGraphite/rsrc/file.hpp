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
#include "libGraphite/data/data.hpp"
#include "libGraphite/rsrc/type.hpp"

#if !defined(GRAPHITE_RSRC_FILE)
#define GRAPHITE_RSRC_FILE

namespace graphite
{
  
namespace rsrc
{

/**
 * The `graphite::rsrc::file` represents a ResourceFork file.
 */
class file
{
public:

    enum flags : uint8_t { };

public:
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

public:
    /**
     * Construct a new blank `graphite::rsrc::file`.
     */
    file() = default;
    
    /**
     * Construct a new `graphite::rsrc::file` by loading the contents of the specified
     * file.
     */
    file(const std::string& path);

    /**
     * Read and parse the contents of the resource file at the specified location.
     * Warning: This will destroy all existing information in the resource file.
     */
    void read(const std::string& path);

    /**
     * Write the contents of the the resource file to disk. If no location is specified,
     * then it will use the original read path (if it exists).
     */
    void write(const std::string& path = "", enum format fmt = classic);

    /**
     * Returns the number of types contained in the resource file.
     */
    std::size_t type_count() const;

    /**
     * Reports the current format of the resource file.
     */
    format current_format() const;

    
private:
    std::string m_path { "" };
    std::vector<std::shared_ptr<type>> m_types { nullptr };
    std::shared_ptr<graphite::data::data> m_data { nullptr };
    format m_format { classic };


    /**
     * Parse the contents of the resource file as a _classic_ resource file.
     */
    void parse_classic();

};

};

};

#endif
