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

#pragma once

#include <string>
#include <type_traits>

namespace graphite::rsrc
{
    struct attribute
    {
    public:
        typedef std::uint64_t hash;

        static auto hash_for_name(const std::string& name) -> hash;

    public:
        attribute(const std::string& name, const std::string& value);

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        attribute(const std::string& name, T value) : m_name(name), m_value(std::to_string(value)) {}

        [[nodiscard]] auto hash_value() const -> hash;
        [[nodiscard]] auto name() const -> const std::string&;
        [[nodiscard]] auto string_value() const -> const std::string&;

        template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
        [[nodiscard]] auto value() const -> T
        {
            return std::to_integer<T>(m_value);
        }

    private:
        std::string m_name;
        std::string m_value;
    };
}

