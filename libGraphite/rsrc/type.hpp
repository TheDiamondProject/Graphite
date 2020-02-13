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
#include "libGraphite/rsrc/resource.hpp"

#if !defined(GRAPHITE_RSRC_TYPE)
#define GRAPHITE_RSRC_TYPE

namespace graphite
{

namespace rsrc
{

/**
 * 
 */
class type
{
public:
	/**
	 * Construct a new a resource type container with the specified
	 * type code.
	 */
	type(const std::string& code);

	/**
	 * Returns the type code of the receiver.
	 */
	std::string code() const;

	/**
	 * Returns a count of the number of resources associated to this type.
	 */
	std::size_t count() const;

	/**
	 * Add a new resource to the receiver.
	 */
	void add_resource(std::shared_ptr<resource> resource);

	/**
	 * Returns an vector containing all of the resources 
	 */
	std::vector<std::shared_ptr<resource>> resources() const;

private:
	std::string m_code;
	std::vector<std::shared_ptr<resource>> m_resources;
};

};

};

#endif