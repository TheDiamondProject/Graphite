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

#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/rsrc/type.hpp"

// MARK: - Constructor

graphite::rsrc::resource::resource(int64_t id, std::string name)
	: m_id(id), m_name(std::move(name))
{
	
}

graphite::rsrc::resource::resource(
	int64_t id, 
	std::weak_ptr<graphite::rsrc::type> type, 
	std::string name,
	std::shared_ptr<graphite::data::data> data
)
	: m_id(id), m_name(std::move(name)), m_type(std::move(type)), m_data(std::move(data))
{
	
}

// MARK: - Resource Metadata Accessors

auto graphite::rsrc::resource::id() const -> int64_t
{
	return m_id;
}

auto graphite::rsrc::resource::set_id(int64_t id) -> void
{
	m_id = id;
}

auto graphite::rsrc::resource::name() const -> std::string
{
	return m_name;
}

auto graphite::rsrc::resource::set_name(const std::string& name) -> void
{
	m_name = name;
}

// MARK: - Resource Type

auto graphite::rsrc::resource::type() const -> std::weak_ptr<graphite::rsrc::type>
{
	return m_type;
}

auto graphite::rsrc::resource::set_type(const std::weak_ptr<graphite::rsrc::type>& type) -> void
{
	m_type = type;
}

auto graphite::rsrc::resource::type_code() const -> std::string
{
	if (auto type = m_type.lock()) {
		return type->code();
	}
	else {
		return "????";
	}
}


// MARK: - Data

auto graphite::rsrc::resource::data() -> std::shared_ptr<graphite::data::data>
{
	return m_data;
}

auto graphite::rsrc::resource::set_data(const std::shared_ptr<graphite::data::data>& data) -> void
{
	m_data = data;
}

// MARK: - 

auto graphite::rsrc::resource::set_data_offset(const std::size_t& offset) -> void
{
	m_data_offset = offset;
}

auto graphite::rsrc::resource::data_offset() const -> std::size_t
{
	return m_data_offset;
}
