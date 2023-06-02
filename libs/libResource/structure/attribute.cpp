// Copyright (c) 2022 Tom Hancocks
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

#include <libResource/structure/attribute.hpp>
#include <libHashing/xxhash/xxhash.hpp>

// MARK: - Construction

resource::attribute::attribute(const std::string &name, const std::string &value)
    : m_name(name), m_value(value)
{
}

// MARK: - Accessors

auto resource::attribute::hash_value() const -> hash
{
    return hash_for_name(m_name);
}

auto resource::attribute::name() const -> const std::string&
{
    return m_name;
}

auto resource::attribute::string_value() const -> const std::string&
{
    return m_value;
}

// MARK: - Helpers

auto resource::attribute::hash_for_name(const std::string& name) -> hash
{
    return hashing::xxh64(name.c_str(), name.size());
}
