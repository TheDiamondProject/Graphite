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
#include <libResourceCore/format/rez/writer.hpp>
#include <libEncoding/macroman/macroman.hpp>
#include <libResourceCore/structure/type.hpp>
#include <libResourceCore/structure/instance.hpp>

// MARK: - Constants

namespace resource_core::format::rez::constants
{
    const std::string map_name = "resource.map";
    constexpr std::uint32_t signature = 'BRGR';
    constexpr std::uint32_t version = 1;
    constexpr std::uint32_t header_length = 12;
    constexpr std::uint32_t resource_offset_length = 12;
    constexpr std::uint32_t map_header_length = 8;
    constexpr std::uint32_t type_info_length = 12;
    constexpr std::uint32_t resource_info_length = 266;
}

// MARK: - Writing

auto resource_core::format::rez::write(file &file) -> bool
{
    return write(file, file.path());
}

auto resource_core::format::rez::write(file &file, const std::string &path) -> bool
{
    data::writer writer(data::byte_order::msb);

    // Count up the total number of resources
    std::uint32_t resource_count = 0;
    for (const auto type_hash : file.types()) {
        auto type = file.type(type_hash);
        resource_count += type->count();
    }

    // The resource map itself is considered an entry for the offsets in the header.
    std::uint32_t entry_count = resource_count + 1;

    // Calculate header length - this is from the end of the preamble to the start of the resource data
    std::uint32_t header_length = constants::header_length + (entry_count * constants::resource_offset_length) + constants::map_name.size() + 1;

    // Write the preamble
    writer.write_long(constants::signature);
    writer.change_byte_order(data::byte_order::lsb);
    writer.write_long(constants::version);
    writer.write_long(header_length);

    // Calculate the offset to the first resource data
    std::uint32_t resource_offset = writer.size() + header_length;

    // Write the header
    std::uint32_t index = 1; // Index of the first resource, starting at 1.
    writer.write_long(1); // Unknown value
    writer.write_long(index);
    writer.write_long(entry_count);
    for (const auto type_hash : file.types()) {
        auto type = const_cast<struct type *>(file.type(type_hash));

        // If the type has attributes then abort and return false
        if (!type->attributes().empty()) {
            return false;
        }

        for (const auto& resource : *type) {
            // Get the data for the resource and determine its size.
            auto size = resource->data().size();
            writer.write_long(resource_offset);
            writer.write_long(static_cast<std::uint32_t>(size));
            writer.write_long(0);
            resource_offset += size;
        }
    }

    std::uint32_t type_count = file.type_count();

    // Calculate the offset within map to start of resource info
    std::uint32_t type_offset = constants::map_header_length + (type_count * constants::type_info_length);
    std::uint32_t map_length = type_offset + (resource_count & constants::resource_info_length);

    // Write the offset and size of the resource map
    writer.write_long(resource_offset);
    writer.write_long(map_length);
    writer.write_long(12 + (entry_count * constants::resource_offset_length)); // Unknown value?
    writer.write_cstr(constants::map_name);

    // Write each of the resources
    for (const auto type_hash : file.types()) {
        auto type = const_cast<struct type *>(file.type(type_hash));
        for (const auto& resource : *type) {
            writer.write_data(&resource->data());
        }
    }

    // Write the resource map as big endian
    writer.change_byte_order(data::byte_order::msb);
    writer.write_long(0); // Unknown value
    writer.write_long(type_count);

    for (const auto type_hash : file.types()) {
        auto type = file.type(type_hash);
        auto count = type->count();
        writer.write_cstr(type->code(), 4);
        writer.write_long(type_offset);
        writer.write_long(count);
        type_offset += constants::resource_info_length * count;
    }

    for (const auto type_hash : file.types()) {
        auto type = const_cast<struct type *>(file.type(type_hash));
        for (const auto& resource : *type) {
            writer.write_long(index++);
            writer.write_cstr(type->code(), 4);
            writer.write_signed_short(static_cast<std::int16_t>(resource->id()));
            writer.write_cstr(resource->name(), 256);
        }
    }

    writer.save(path);
    return true;
}