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


#include "libGraphite/data/data.hpp"

// MARK: - Constructor

graphite::data::data::data(enum graphite::data::data::byte_order bo)
    : m_bo(bo), m_data(std::make_shared<std::vector<char>>(0)), m_start(0), m_size(0)
{
    
}

graphite::data::data::data(std::size_t capacity, enum graphite::data::data::byte_order bo)
    : m_bo(bo), m_data(std::make_shared<std::vector<char>>(0)), m_start(0), m_size(capacity)
{
    
}

graphite::data::data::data(std::shared_ptr<std::vector<char>> bytes, std::size_t size, std::size_t start, enum data::byte_order bo)
    : m_bo(bo), m_data(bytes), m_size(size), m_start(start)
{
   	
}

// MARK: - Offset Calculations

auto graphite::data::data::relative_offset(int64_t offset) const -> int64_t
{
	if (offset > m_size || offset < 0) {
		throw std::logic_error("Attempted to calculate an offset that went beyond the boundaries of the data slice.");
	}
	return m_start + offset;
}

// MARK: - Size

auto graphite::data::data::size() const -> std::size_t
{
    if (m_size > 0) {
        return m_size;
    }
    else if (m_data != nullptr) {
        return m_data->size();
    }
    else {
        return 0;
    }
}

auto graphite::data::data::start() const -> std::size_t
{
    return m_start;
}

auto graphite::data::data::current_byte_order() const -> enum graphite::data::data::byte_order
{
    return m_bo;
}

auto graphite::data::data::set_byte_order(enum data::byte_order bo) -> void
{
    m_bo = bo;
}


// MARK: - Plumbing

auto graphite::data::data::get() -> std::shared_ptr<std::vector<char>>
{
    return m_data;
}

auto graphite::data::data::at(std::size_t offset) const -> char
{
	return m_data->at(relative_offset(offset));
}

