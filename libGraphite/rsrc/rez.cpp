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

#include <iostream>
#include "libGraphite/rsrc/rez.hpp"
#include "libGraphite/encoding/macroman/macroman.hpp"

// MARK: - Parsing / Reading

auto graphite::rsrc::rez::parse(std::shared_ptr<graphite::data::reader> reader) -> std::vector<std::shared_ptr<graphite::rsrc::type>>
{
    // TODO: .rez implementation
    return {};
}

// MARK: - Writing

auto graphite::rsrc::rez::write(const std::string& path, std::vector<std::shared_ptr<graphite::rsrc::type>> types) -> void
{
    auto writer = std::make_shared<graphite::data::writer>();
    writer->data()->set_byte_order(graphite::data::data::byte_order::lsb);

    const std::string rez_signature = "BRGR";
    const std::string map_name = "resource.map";
    const uint32_t rez_version = 1;
    const uint32_t resource_offset_length = 12;
    const uint32_t map_header_length = 8;
    const uint32_t type_info_length = 12;
    const uint32_t resource_info_length = 266;

    // Count up the total number of resources
    uint32_t resource_count = 0;
    for (auto type : types) {
        resource_count += type->count();
    }

    // The resource map itself is considered an entry for the offsets in the header
    uint32_t entry_count = resource_count + 1;

    // Calculate header length - this is from the end of the preamble to the start of the resource data
    uint32_t header_length = 12 + (entry_count * resource_offset_length) + map_name.size()+1;

    // Write the preamble
    writer->write_cstr(rez_signature, 4);
    writer->write_long(rez_version);
    writer->write_long(header_length);

    // Calculate the offset to the first resource data
    uint32_t resource_offset = static_cast<uint32_t>(writer->size()) + header_length;

    // Write the header
    uint32_t index = 1; // Index of first resource, starting at 1
    writer->write_long(1); // Unknown value
    writer->write_long(index);
    writer->write_long(entry_count);
    for (auto type : types) {
        for (auto resource : type->resources()) {
            // Get the data for the resource and determine its size.
            auto data = resource->data();
            auto size = data->size();
            writer->write_long(resource_offset);
            writer->write_long(static_cast<uint32_t>(size));
            writer->write_long(0); // Unknown value
            resource_offset += size;
        }
    }

    uint32_t type_count = types.size();
    // Calculate offset within map to start of resource info
    uint32_t type_offset = map_header_length + (type_count * type_info_length);
    uint32_t map_length = type_offset + (resource_count * resource_info_length);
    // Write the offset and size of the resource map
    writer->write_long(resource_offset);
    writer->write_long(map_length);
    writer->write_long(12 + (entry_count * resource_offset_length)); // Unknown value

    // Write the name of the resource map
    writer->write_cstr(map_name);

    // Write each resource
    for (auto type : types) {
        for (auto resource : type->resources()) {
            writer->write_data(resource->data());
        }
    }

    // Write the resource map as big endian
    // Map header
    writer->data()->set_byte_order(graphite::data::data::byte_order::msb);
    writer->write_long(8); // Unknown value
    writer->write_long(type_count);

    // Type counts and offsets
    for (auto type : types) {
        auto count = type->count();
        writer->write_cstr(type->code(), 4);
        writer->write_long(type_offset);
        writer->write_long(count);
        type_offset += resource_info_length * count;
    }

    // Info for each resource
    for (auto type : types) {
        for (auto resource : type->resources()) {
            writer->write_long(index++);
            writer->write_cstr(type->code(), 4);
            writer->write_signed_short(static_cast<int16_t>(resource->id()));
            writer->write_cstr(resource->name(), 256);
        }
    }

    // Finish by writing the contents of the Rez file to disk.
    writer->save(path);
}
