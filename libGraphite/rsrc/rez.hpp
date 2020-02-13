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
#include "libGraphite/rsrc/file.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/data/writer.hpp"

#if !defined(GRAPHITE_RSRC_REZ)
#define GRAPHITE_RSRC_REZ

namespace graphite
{

namespace rsrc
{

namespace rez
{

/**
 * Parse the specified/provided data object that represents a resource file 
 * into a list of resource types.
 */
std::vector<std::shared_ptr<graphite::rsrc::type>> parse(std::shared_ptr<graphite::data::reader> reader);

/**
 * Build a data object that represents a resource file from the provided list
 * of resource types.
 */
void write(const std::string& path, std::vector<std::shared_ptr<graphite::rsrc::type>> types);

};

};

};

#endif