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

#if !defined(GRAPHITE_RSRC_RESOURCE)
#define GRAPHITE_RSRC_RESOURCE

namespace graphite { namespace rsrc {

    class type;

    /**
     * The `graphite::rsrc::resource` class represents a single resource within
     * a resource file.
     */
    class resource
    {
    private:
        int64_t m_id;
        std::weak_ptr<graphite::rsrc::type> m_type;
        std::string m_name;
        std::shared_ptr<graphite::data::data> m_data;
        std::size_t m_data_offset { 0 };

    public:
    	/**
    	 * Construct a new resource instance, without an explicit type or
    	 * data.
    	 */
    	resource(int64_t id, const std::string& name);

    	/**
    	 * Construct a new resource instance with the specified type, and data.
    	 */
    	resource(int64_t id, std::weak_ptr<graphite::rsrc::type> type, const std::string& name, std::shared_ptr<graphite::data::data> data);

    	/**
    	 * Returns the ID of the resource.
    	 */
    	auto id() const -> int64_t;

    	/**
    	 * Set the ID of the resource.
    	 */
    	auto set_id(int64_t id) -> void;

    	/**
    	 * Returns the name of the resource.
    	 */
    	auto name() const -> std::string;

    	/**
    	 * Set the name of the resource.
    	 */
    	auto set_name(const std::string& name) -> void;

    	/**
    	 * Returns the type container of the resource.
    	 */
    	auto type() const -> std::weak_ptr<graphite::rsrc::type>;

    	/**
    	 * Set the type container of the resource.
    	 */
    	auto set_type(std::weak_ptr<graphite::rsrc::type> type) -> void;

    	/**
    	 * Returns the type code of the resource.
    	 */
    	auto type_code() const -> std::string;

    	/**
    	 * Returns a shared pointer to the contained data.
    	 */
    	auto data() -> std::shared_ptr<graphite::data::data>;

    	/**
    	 * Store the location of the data within the resource file.
    	 */
    	auto set_data_offset(std::size_t offset) -> void;

    	/**
    	 * The location of the data within the resource file.
    	 */
    	auto data_offset() const -> std::size_t;
    };

}}

#endif
