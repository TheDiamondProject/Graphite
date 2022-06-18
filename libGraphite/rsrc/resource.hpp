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
#include <cstdint>
#include "libGraphite/data/data.hpp"

namespace graphite::rsrc
{
    struct type;

    struct resource
    {
    public:
        typedef std::int64_t identifier;
        typedef std::uint64_t identifier_hash;
        typedef std::uint64_t name_hash;

        static constexpr resource::identifier default_resource_id { 128 };

    public:
        resource(resource::identifier id = default_resource_id, const std::string& name = "");
        resource(struct type *type, resource::identifier id = default_resource_id, const std::string& name = "", data::block data = {});
        explicit resource(const resource& resource);
        resource(resource&& resource) noexcept;

        ~resource();

        [[nodiscard]] auto id() const -> resource::identifier;
        [[nodiscard]] auto type() const -> struct type *;
        [[nodiscard]] auto name() const -> const std::string&;
        [[nodiscard]] auto type_code() const -> std::string;
        [[nodiscard]] auto data() const -> const data::block&;

        auto set_id(resource::identifier id) -> void;
        auto set_name(const std::string& name) -> void;
        auto set_type(struct type *type) -> void;

        static auto hash(identifier id) -> identifier_hash;
        static auto hash(const std::string& name) -> name_hash;

        auto set_data_offset(std::size_t offset) -> void;
        [[nodiscard]] auto data_offset() const -> std::size_t;

    private:
        resource::identifier m_id { default_resource_id };
        struct type *m_type { nullptr };
        std::string m_name;
        data::block m_data;
        std::size_t m_data_offset { 0 };
    };
}
