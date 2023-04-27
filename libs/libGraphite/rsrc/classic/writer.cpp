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

#include "libGraphite/rsrc/classic/writer.hpp"

#include <limits>
#include <vector>
#include "libGraphite/encoding/macroman/macroman.hpp"
#include "libGraphite/rsrc/type.hpp"
#include "libGraphite/rsrc/resource.hpp"

// MARK: - Constants

namespace graphite::rsrc::format::classic::constants::defaults
{
    constexpr uint32_t data_offset = 256;
    constexpr uint32_t map_offset = 0;
    constexpr uint32_t data_length = 0;
    constexpr uint32_t map_length = 0;
}

namespace graphite::rsrc::format::classic::constants
{

    constexpr uint16_t resource_type_length = 8;
    constexpr uint16_t resource_length = 12;
    constexpr uint16_t type_list_offset = 28;
}

// MARK: - Writing

auto graphite::rsrc::format::classic::write(file &file) -> bool
{
    return write(file, file.path());
}

auto graphite::rsrc::format::classic::write(file &file, const std::string &path) -> bool
{
    graphite::data::writer writer(data::byte_order::msb);

    // 1. Begin setting up the preamble
    auto data_offset = constants::defaults::data_offset;
    auto map_offset = constants::defaults::map_offset;
    auto data_length = constants::defaults::data_length;
    auto map_length = constants::defaults::map_length;

    writer.write_long(data_offset);
    writer.write_long(map_offset);
    writer.write_long(data_length);
    writer.write_long(map_length);
    writer.pad_to_size(data_offset);

    // 2. Iterate through all of the resources and write their data blobs to the file data.
    // When doing this we need to record the starting points of each resources data.
    uint16_t resource_count = 0;

    std::vector<struct type *> types(file.type_count());
    auto type_ptr = types.begin();

    for (const auto& type_code : file.types()) {
        auto type = *type_ptr = const_cast<struct type *>(file.type(type_code));
        resource_count += type->count();

        // If the type has attributes then abort and return false
        if (!type->attributes().empty()) {
            return false;
        }

        for (auto& resource : *type) {
            auto data = resource->data();
            auto size = data.size();
            resource->set_data_offset(writer.size() - data_offset);
            writer.write_long(static_cast<uint32_t>(size));
            writer.write_data(&data);
        }

        type_ptr++;
    }

    // 3. Start writing the ResourceMap. This consists of several characteristics, the first of which is a secondary
    // preamble. We can now calculate the map offset and data length, but we're still waiting on the map length. For
    // now, write these values as zero.
    map_offset = static_cast<uint32_t>(writer.size());
    data_length = map_offset - data_offset;
    writer.write_long(data_offset);
    writer.write_long(map_offset);
    writer.write_long(data_length);
    writer.write_long(map_length);

    // The next size bytes are used by the MacOS ResourceManager and thus not important to us.
    writer.write_byte(0x00, 6);

    // 4. We're now writing the primary map information, which includes flags, and offsets for the type list and the
    // name list. We can calculate where each of these will be.
    auto name_list_offset = constants::type_list_offset + (file.type_count() * constants::resource_type_length);
    name_list_offset += (resource_count * constants::resource_length) + sizeof(uint16_t);

    writer.write_short(0x0000);
    writer.write_short(constants::type_list_offset);
    writer.write_short(name_list_offset);

    // Now moving on to actually write each of the type descriptors into the data.
    auto resource_offset = sizeof(uint16_t) + (file.type_count() * constants::resource_type_length);
    writer.write_short(file.type_count() - 1);
    for (const auto type : types) {
        // We need to ensure that the type code is 4 characters -- otherwise this file will be massively corrupt
        // when produced.
        auto mac_roman = encoding::mac_roman::from_utf8(type->code());
        if (mac_roman.size() != 4) {
            return false;
        }
        writer.write_bytes(mac_roman);
        writer.write_short(type->count() - 1);
        writer.write_short(resource_offset);

        resource_offset += type->count() * constants::resource_length;
    }

    // 5. Now we're writing the actual resource headers.
    uint16_t name_offset = 0;
    uint16_t name_len = 0;
    for (const auto type : types) {
        for (const auto& resource : *type) {
            auto id = resource->id();

            if (id < std::numeric_limits<int16_t>::min() || id > std::numeric_limits<int16_t>::max()) {
                return false;
            }
            writer.write_signed_short(static_cast<int16_t>(id));

            // The name is actually stored in the name list, and the resource stores and offset to that name.
            // If no name is assigned to the resource then the offset is encoded as 0xFFFF
            if (resource->name().empty()) {
                writer.write_short(0xFFFF);
            }
            else {
                if (name_offset + name_len >= 0xFFFF) {
                    return false;
                }
                name_offset += name_len;
                writer.write_short(name_offset);

                // Convert the name to MacRoman so that we can get the length of it when encoded.
                auto mac_roman = encoding::mac_roman::from_utf8(resource->name());
                name_len = mac_roman.size() + 1;
                if (name_len > 0x100) {
                    name_len = 0x100;
                }
            }

            // Write the resource attributes - these are currently hard coded as nothing.
            writer.write_byte(0x00);

            // The data offset is a 3 byte (24-bit) value. This means the hi-byte needs discarding and then a swap
            // performing.
            auto offset = static_cast<uint32_t>(resource->data_offset());
            if (offset > 0xFFFFFF) {
                return false;
            }
            writer.write_triple(offset);

            // Finally this is a reserved field for use by the ResourceManager.
            writer.write_long(0x0000'0000);
        }
    }

    // 6. Finally we write out each of the resource names, and calculate the map length.
    name_offset = 0;
    for (const auto type : types) {
        for (const auto& resource : *type) {
            if (resource->name().empty()) {
                continue;
            }

            auto mac_roman = encoding::mac_roman::from_utf8(resource->name());
            if (mac_roman.size() >= 0x100) {
                mac_roman.resize(0xFF);
            }
            name_offset += writer.write_pstr(resource->name()) + 1;
        }
    }

    // Even if the data fits the spex, the resource manager will still not read files larger than 16MB.
    if (writer.size() > 0xFFFFFF) {
        return false;
    }
    map_length = static_cast<uint32_t>(writer.size() - map_offset);

    // 7. Fix the preamble values.
    writer.set_position(0);
    writer.write_long(data_offset);
    writer.write_long(map_offset);
    writer.write_long(data_length);
    writer.write_long(map_length);

    writer.set_position(map_offset);
    writer.write_long(data_offset);
    writer.write_long(map_offset);
    writer.write_long(data_length);
    writer.write_long(map_length);

    // Finish by writing the contents of the Resource File to disk.
    writer.save(path);
    return true;
}
