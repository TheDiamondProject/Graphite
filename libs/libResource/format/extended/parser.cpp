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
#include <libResource/format/extended/parser.hpp>
#include <libCommon/hints.hpp>
#include <libEncoding/macroman/macroman.hpp>
#include <libResource/structure/type.hpp>
#include <libResource/structure/instance.hpp>

// MARK: - Parsing

auto resource::format::extended::parse(data::reader &reader, file &file) -> bool
{
    std::vector<struct type *> types;

    // 1. Resource Preamble
    if (reader.read_quad(0, data::reader::mode::peek) != 1) {
        return false;
    }

    reader.move(sizeof(std::uint64_t));
    auto data_offset = reader.read_quad();
    auto map_offset = reader.read_quad();
    auto data_length = reader.read_quad();
    auto map_length = reader.read_quad();

    // Verify that the resource file is valid.
    if (data_offset == 0 || map_offset == 0 || map_length == 0) {
        return false;
    }

    auto rsrc_size = data_offset + data_length + map_length;
    if (map_offset != data_offset + data_length) {
        return false;
    }

    if (rsrc_size > reader.size()) {
        return false;
    }

    // Move to the start of the ResourceMap, and verify the preamble contents.
    reader.set_position(map_offset);
    auto data_offset2 = reader.read_quad();
    auto map_offset2 = reader.read_quad();
    auto data_length2 = reader.read_quad();
    auto map_length2 = reader.read_quad();

    if (data_offset2 != data_offset) {
        return false;
    }

    if (map_offset2 != map_offset) {
        return false;
    }

    if (data_length2 != data_length) {
        return false;
    }

    if (map_length2 != map_length) {
        return false;
    }

    const_cast<data::block *>(reader.data())->originates_from_extended_format();

    // 2. Now that the preamble is parsed and verified, parse the contents
    // of the ResourceMap. The first two fields are used by the actual Resource Manager in
    // the Classic Macintosh OS, but are not used by this implementation.
    GRAPHITE_UNUSED auto next_map = reader.read_long();
    GRAPHITE_UNUSED auto reference = reader.read_short();

    // Start to read the actual content of the resource map. This is the content that
    // we actually care about. The first field is the flags/attributes of the resource
    // fork.
    GRAPHITE_UNUSED auto flags = reader.read_short();

    // The next fields are the offsets of the type list and the name list.
    auto type_list_offset = static_cast<std::uint64_t>(reader.read_quad());
    auto name_list_offset = static_cast<std::uint64_t>(reader.read_quad());
    auto attribute_list_offset = static_cast<std::uint64_t>(reader.read_quad());

    // 3. Parse the list of Resource Types.
    reader.set_position(map_offset + type_list_offset);
    auto type_count = reader.read_quad() + 1;

    for (auto type_idx = 0; type_idx < type_count; ++type_idx) {
        auto code = reader.read_cstr(4);
        auto count = reader.read_quad() + 1;
        auto first_resource_offset = reader.read_quad();
        auto attribute_count = reader.read_quad();
        auto attribute_offset = reader.read_quad();

        auto type = new struct type(code);

        // 4. Extract the list of attributes before we create the type, as they are needed for actually building the
        // type.
        reader.save_position();
        if (attribute_count > 0) {
            reader.set_position(attribute_list_offset + attribute_offset);
            for (auto i = 0; i < attribute_count; ++i) {
                type->add_attribute(reader.read_cstr(), reader.read_cstr());
            }
        }

        // 5. Parse the list of resources for the current resource type.
        reader.set_position(map_offset + type_list_offset + first_resource_offset);

        for (auto res_idx = 0; res_idx < count; ++res_idx) {
            auto id = static_cast<identifier>(reader.read_signed_quad());
            auto name_offset = reader.read_quad();
            GRAPHITE_UNUSED auto flags = reader.read_byte();
            auto resource_data_offset = reader.read_quad();
            GRAPHITE_UNUSED auto handle = reader.read_long();

            reader.save_position();
            // 6. Parse the name out of the list of resource names.
            std::string name;
            if (name_offset != std::numeric_limits<std::uint64_t>::max()) {
                reader.set_position(map_offset + name_list_offset + name_offset);
                name = std::move(reader.read_pstr());
            }

            // 6. Create a data slice for the resources data.
            reader.set_position(data_offset + resource_data_offset);
            auto data_size = reader.read_quad();
            auto slice = reader.read_data(data_size);
            reader.restore_position();

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
