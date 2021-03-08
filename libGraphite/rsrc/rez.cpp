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

auto graphite::rsrc::rez::parse(const std::shared_ptr<graphite::data::reader>& reader) -> std::vector<std::shared_ptr<graphite::rsrc::type>>
{
    // Read the preamble
    if (reader->read_long() != rez_signature) {
        throw std::runtime_error("[Rez File] Preamble 'signature' mismatch.");
    }
    reader->get()->set_byte_order(graphite::data::data::byte_order::lsb);
    if (reader->read_long() != rez_version) {
        throw std::runtime_error("[Rez File] Preamble 'version' mismatch.");
    }
    auto header_length = reader->read_long();
    
    // Read the header
    reader->move(4); // Unknown value
    auto first_index = reader->read_long();
    auto count = reader->read_long();
    uint32_t expected_header_length = 12 + (count * resource_offset_length) + map_name.size()+1;
    if (header_length != expected_header_length) {
        throw std::runtime_error("[Rez File] Preamble 'header_length' mismatch.");
    }
    
    // Record the offsets
    std::vector<uint64_t> offsets;
    std::vector<uint64_t> sizes;
    for (auto res_idx = 0; res_idx < count; res_idx++) {
        offsets.push_back(static_cast<uint64_t>(reader->read_long()));
        sizes.push_back(static_cast<uint64_t>(reader->read_long()));
        reader->move(4); // Unknown value
    }
    if (reader->read_cstr() != map_name) {
        throw std::runtime_error("[Rez File] Header 'map_name' mismatch.");
    }
    
    // Read the resource map header
    reader->get()->set_byte_order(graphite::data::data::byte_order::msb);
    auto map_offset = offsets.back();
    reader->set_position(map_offset);
    reader->move(4); // Unknown value
    auto type_count = reader->read_long();
    
    // Read the types
    std::vector<std::shared_ptr<graphite::rsrc::type>> types;
    for (auto type_idx = 0; type_idx < type_count; type_idx++) {
        auto code = reader->read_cstr(4);
        auto type_offset = static_cast<int64_t>(reader->read_long());
        auto count = reader->read_long();
        auto type = std::make_shared<graphite::rsrc::type>(code);
        
        reader->save_position();
        reader->set_position(map_offset + type_offset);
        
        // Read the resource info
        for (auto res_idx = 0; res_idx < count; res_idx++) {
            auto index = reader->read_long();
            auto code = reader->read_cstr(4);
            if (code != type->code()) {
                throw std::runtime_error("[Rez File] Resource 'type' mismatch.");
            }
            auto id = static_cast<int64_t>(reader->read_signed_short());
            // The name is padded to 256 bytes - note the end position before reading the cstr
            auto nextOffset = reader->position() + 256;
            auto name = reader->read_cstr();
            
            // Read the resource's data
            reader->set_position(offsets[index-first_index]);
            auto slice = reader->read_data(sizes[index-first_index]);
            reader->set_position(nextOffset);
            
            auto resource = std::make_shared<graphite::rsrc::resource>(id, type, name, slice);
            type->add_resource(resource);
        }
        
        reader->restore_position();
        types.push_back(type);
    }
    
    return types;
}

// MARK: - Writing

auto graphite::rsrc::rez::write(const std::string& path, const std::vector<std::shared_ptr<graphite::rsrc::type>>& types) -> void
{
    auto writer = std::make_shared<graphite::data::writer>();

    // Count up the total number of resources
    uint32_t resource_count = 0;
    for (const auto& type : types) {
        resource_count += type->count();
    }

    // The resource map itself is considered an entry for the offsets in the header
    uint32_t entry_count = resource_count + 1;

    // Calculate header length - this is from the end of the preamble to the start of the resource data
    uint32_t header_length = 12 + (entry_count * resource_offset_length) + map_name.size()+1;

    // Write the preamble
    writer->write_long(rez_signature);
    writer->data()->set_byte_order(graphite::data::data::byte_order::lsb);
    writer->write_long(rez_version);
    writer->write_long(header_length);

    // Calculate the offset to the first resource data
    uint32_t resource_offset = static_cast<uint32_t>(writer->size()) + header_length;

    // Write the header
    uint32_t index = 1; // Index of first resource, starting at 1
    writer->write_long(1); // Unknown value
    writer->write_long(index);
    writer->write_long(entry_count);
    for (const auto& type : types) {
        for (const auto& resource : type->resources()) {
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
    for (const auto& type : types) {
        for (const auto& resource : type->resources()) {
            writer->write_data(resource->data());
        }
    }

    // Write the resource map as big endian
    // Map header
    writer->data()->set_byte_order(graphite::data::data::byte_order::msb);
    writer->write_long(8); // Unknown value
    writer->write_long(type_count);

    // Type counts and offsets
    for (const auto& type : types) {
        auto count = type->count();
        writer->write_cstr(type->code(), 4);
        writer->write_long(type_offset);
        writer->write_long(count);
        type_offset += resource_info_length * count;
    }

    // Info for each resource
    for (const auto& type : types) {
        for (const auto& resource : type->resources()) {
            writer->write_long(index++);
            writer->write_cstr(type->code(), 4);
            writer->write_signed_short(static_cast<int16_t>(resource->id()));
            writer->write_cstr(resource->name(), 256);
        }
    }

    // Finish by writing the contents of the Rez file to disk.
    writer->save(path);
}
