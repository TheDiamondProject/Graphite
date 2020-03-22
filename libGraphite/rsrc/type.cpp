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

#include "libGraphite/rsrc/type.hpp"

// MARK: - Constructor

graphite::rsrc::type::type(const std::string& code)
	: m_code(code)
{
	
}

// MARK: - Metadata Accessors

std::string graphite::rsrc::type::code() const
{
	return m_code;
}

// MARK: - Resource Management

std::size_t graphite::rsrc::type::count() const
{
	return m_resources.size();
}

void graphite::rsrc::type::add_resource(std::shared_ptr<graphite::rsrc::resource> resource)
{
	m_resources.push_back(resource);
}

std::vector<std::shared_ptr<graphite::rsrc::resource>> graphite::rsrc::type::resources() const
{
	return m_resources;
}

std::weak_ptr<graphite::rsrc::resource> graphite::rsrc::type::get(int16_t id) const
{
    for (auto r = m_resources.begin(); r != m_resources.end(); ++r) {
        if ((*r)->id() == id) {
            return *r;
        }
    }
    return std::weak_ptr<graphite::rsrc::resource>();
}