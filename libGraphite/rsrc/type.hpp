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
#include "libGraphite/rsrc/resource.hpp"

#if !defined(GRAPHITE_RSRC_TYPE)
#define GRAPHITE_RSRC_TYPE

namespace graphite { namespace rsrc {

    /**
     *
     */
    class type
    {
    private:
        std::string m_code;
        std::vector<std::shared_ptr<resource>> m_resources;
        std::map<std::string, std::string> m_attributes;

    public:
    	/**
    	 * Construct a new a resource type container with the specified
    	 * type code.
    	 */
    	explicit type(std::string  code, std::map<std::string, std::string>  attributes = {});

    	/**
    	 * Returns the type code of the receiver.
    	 */
    	auto code() const -> std::string;

    	/**
    	 * Returns the attribute map of the receiver.
    	 */
    	 auto attributes() const -> std::map<std::string, std::string>;

    	/**
    	 * Returns the attribute map of the receiver as a string.
    	 */
    	 auto attributes_string() const -> std::string;

    	/**
    	 * Returns a count of the number of resources associated to this type.
    	 */
    	auto count() const -> std::size_t;

    	/**
    	 * Add a new resource to the receiver.
    	 */
    	auto add_resource(std::shared_ptr<resource> resource) -> void;

    	/**
    	 * Returns an vector containing all of the resources
    	 */
    	auto resources() const -> std::vector<std::shared_ptr<resource>>;

    	/**
    	 * Returns the resource with the specified ID.
    	 */
    	auto get(int16_t id) const -> std::weak_ptr<resource>;
    };

}}

#endif