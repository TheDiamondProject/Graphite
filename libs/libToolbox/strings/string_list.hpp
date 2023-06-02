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
#include <vector>
#include <libData/reader.hpp>
#include <libResource/structure/instance.hpp>

namespace toolbox
{
    struct string_list
    {
    public:
        static auto type_code() -> std::string { return "STR#"; }

        using iterator = std::vector<std::string>::iterator;

    public:
        string_list() = default;
        explicit string_list(const data::block& data, resource::identifier id = 0, const std::string& name = "");

        [[nodiscard]] auto string_count() const -> std::size_t;
        [[nodiscard]] auto at(std::uint32_t idx) const -> std::string;

        auto begin() noexcept -> iterator;
        auto end() noexcept -> iterator;

    private:
        resource::identifier m_id { 0 };
        std::string m_name;
        std::vector<std::string> m_strings;

        auto decode(data::reader& reader) -> void;
    };
}
