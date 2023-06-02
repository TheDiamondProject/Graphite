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

#include <limits>
#include <vector>
#include <libResourceCore/format/rez/parser.hpp>
#include <libCommon/hints.hpp>
#include <libEncoding/macroman/macroman.hpp>
#include <libResourceCore/structure/type.hpp>
#include <libResourceCore/structure/instance.hpp>

// MARK: - Constants

namespace resource_core::format::rez::constants
{
    const std::string map_name = "resource.map";
    constexpr std::uint32_t signature = 'BRGR';
    constexpr std::uint32_t version = 1;
    constexpr std::uint32_t resource_offset_length = 12;
    constexpr std::uint32_t map_header_length = 8;
    constexpr std::uint32_t type_info_length = 12;
    constexpr std::uint32_t resource_info_length = 266;
};

// MARK: - Parsing

auto resource_core::format::rez::parse(data::reader &reader, file &file) -> bool
{
    std::vector<struct type *> types;

    // 1. Read the preamble
    if (reader.read_long() != constants::signature) {
        reader.set_position(0);
        return false;
    }

    reader.change_byte_order(data::byte_order::lsb);
    if (reader.read_long() != constants::version) {
        reader.set_position(0);
        return false;
    }

    // 2. Read the header
    auto header_length = reader.read_long();
    reader.move(4); // Skip over an unknown value.

    auto first_index = reader.read_long();
    auto count = reader.read_long();
    auto expected_header_length = 12 + (count * constants::resource_offset_length) + constants::map_name.size() + 1;
    if (header_length != expected_header_length) {
        reader.set_position(0);
        return false;
    }

    // 3. Record the offsets
    std::vector<std::uint64_t> offsets;
    std::vector<std::uint64_t> sizes;
    for (auto res_idx = 0; res_idx < count; ++res_idx) {
        offsets.emplace_back(static_cast<std::uint64_t>(reader.read_long()));
        sizes.push_back(static_cast<std::uint64_t>(reader.read_long()));
        reader.move(4); // Skip over an unknown value.
    }

    if (reader.read_cstr() != constants::map_name) {
        reader.set_position(0);
        return false;
    }

    // 4. Read the resource map header.
    reader.change_byte_order(data::byte_order::msb);
    auto map_offset = offsets.back();
    reader.set_position(map_offset);
    reader.move(4); // Skip over an unknown value.
    auto type_count = reader.read_long();

    // 5. Read the resource types.
    for (auto type_idx = 0; type_idx < type_count; ++type_idx) {
        auto code = reader.read_cstr(4);
        auto type_offset = static_cast<std::int64_t>(reader.read_long());
        auto count = reader.read_long();

        auto type = new struct type(code);
        reader.save_position();
        reader.set_position(map_offset + type_offset);

        // 6. Read the resource info.
        for (auto res_idx = 0; res_idx < count; ++res_idx) {
            auto index = reader.read_long();
            if (code != reader.read_cstr(4)) {
                reader.set_position(0);
                return false;
            }

            auto id = static_cast<resource_core::identifier>(reader.read_signed_short());
            auto next_offset = reader.position() + 256;
            auto name = reader.read_cstr();

            reader.set_position(offsets[index - first_index]);
            auto slice = reader.read_data(sizes[index - first_index]);
            reader.set_position(next_offset);

            // 7. Construct a new resource instance and add it to the type.
            auto resource = new struct instance(type, id, name, std::move(slice));
            type->add_resource(resource);
        }

        reader.restore_position();
        types.emplace_back(type);
    }

    file.add_types(types);
    return true;
}
