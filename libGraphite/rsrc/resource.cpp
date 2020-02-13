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

graphite::rsrc::resource::resource(int64_t id, const std::string& name)
	: m_id(id), m_name(name)
{
	
}

graphite::rsrc::resource::resource(
	int64_t id, 
	std::weak_ptr<graphite::rsrc::type> type, 
	const std::string& name, 
	std::shared_ptr<graphite::data::data> data
)
	: m_id(id), m_name(name), m_type(type), m_data(data)
{
	
}

// MARK: - Resource Metadata Accessors

int64_t graphite::rsrc::resource::id() const
{
	return m_id;
}

void graphite::rsrc::resource::set_id(int64_t id)
{
	m_id = id;
}

std::string graphite::rsrc::resource::name() const
{
	return m_name;
}

void graphite::rsrc::resource::set_name(const std::string& name)
{
	m_name = name;
}

// MARK: - Resource Type

std::weak_ptr<graphite::rsrc::type> graphite::rsrc::resource::type() const
{
	return m_type;
}

void graphite::rsrc::resource::set_type(std::weak_ptr<graphite::rsrc::type> type)
{
	m_type = type;
}

std::string graphite::rsrc::resource::type_code() const
{
	if (auto type = m_type.lock()) {
		return type->code();
	}
	else {
		return "????";
	}
}


// MARK: - Data

std::shared_ptr<graphite::data::data> graphite::rsrc::resource::data()
{
	return m_data;
}

// MARK: - 

void graphite::rsrc::resource::set_data_offset(std::size_t offset)
{
	m_data_offset = offset;
}

std::size_t graphite::rsrc::resource::data_offset() const
{
	return m_data_offset;
}
