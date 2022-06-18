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

#include "libGraphite/rsrc/extended/writer.hpp"

#include <limits>
#include <vector>
#include "libGraphite/encoding/macroman/macroman.hpp"
#include "libGraphite/rsrc/type.hpp"
#include "libGraphite/rsrc/resource.hpp"

// MARK: - Constants

namespace graphite::rsrc::format::extended::constants::defaults
{
    constexpr uint64_t version = 1;
    constexpr uint64_t data_offset = 256;
    constexpr uint64_t map_offset = 0;
    constexpr uint64_t data_length = 0;
    constexpr uint64_t map_length = 0;
}

namespace graphite::rsrc::format::extended::constants
{

    constexpr uint16_t resource_type_length = 36;
    constexpr uint16_t resource_length = 29;
    constexpr uint16_t type_list_offset = 64;
}

// MARK: - Writing

auto graphite::rsrc::format::extended::write(file &file) -> bool
{
    return write(file, file.path());
}

auto graphite::rsrc::format::extended::write(file &file, const std::string &path) -> bool
{
    graphite::data::writer writer(data::byte_order::msb);

    // 1. Begin setting up the preamble
    auto data_offset = constants::defaults::data_offset;
    auto map_offset = constants::defaults::map_offset;
    auto data_length = constants::defaults::data_length;
    auto map_length = constants::defaults::map_length;

    writer.write_quad(constants::defaults::version);
    writer.write_quad(data_offset);
    writer.write_quad(map_offset);
    writer.write_quad(data_length);
    writer.write_quad(map_length);
    writer.pad_to_size(data_offset);

    // 2. Iterate through all of the resources and write their data blobs to the file data.
    // When doing this we need to record the starting points of each resources data.
    uint64_t resource_count = 0;

    std::vector<struct type *> types(file.type_count());
    auto type_ptr = types.begin();

    for (const auto& type_code : file.types()) {
        auto type = *type_ptr = const_cast<struct type *>(file.type(type_code));
        resource_count += type->count();

        for (auto& resource : *type) {
            auto data = resource.data();
            auto size = data.size();
            resource.set_data_offset(writer.size() - data_offset);
            writer.write_quad(size);
            writer.write_data(&data);
        }

        type_ptr++;
    }

    // 3. Start writing the ResourceMap. This consists of several characteristics,
    // The first of which is a secondary preamble. We can now calculate the map_offset and
    // the data_length, but we're still waiting on the map_length. For now, write these values
    // as zeros.
    map_offset = writer.size();
    data_length = map_offset - data_offset;

    writer.write_quad(data_offset);
    writer.write_quad(map_offset);
    writer.write_quad(data_length);
    writer.write_quad(map_length);

    // The next six bytes are reserved.
    writer.write_byte(0, 6);

    // 4. We're now writing the primary map information, which includes flags and offsets for the
    // type list and the name list. We can calculate where each of these will be.
    auto name_list_offset = constants::type_list_offset + (types.size() * constants::resource_type_length);
    name_list_offset += (resource_count * constants::resource_length) + sizeof(uint64_t);
    uint64_t attribute_list_offset_position = 0;

    writer.write_short(0);
    writer.write_quad(constants::type_list_offset);
    writer.write_quad(name_list_offset);

    attribute_list_offset_position = writer.position();
    writer.write_quad(attribute_list_offset_position);

    // No moving on to actually writing each of the type descriptors into the data.
    uint64_t attribute_offset = 0;
    uint64_t resource_offset = sizeof(uint64_t) + (types.size() * constants::resource_type_length);
    writer.write_quad(types.size() - 1);
    for (const auto type : types) {
        // We need to ensure that the type code is 4 characters -- otherwise this file be massively corrupt
        // when produced.
        auto mac_roman = encoding::mac_roman::from_utf8(type->code());
        if (mac_roman.size() != 4) {
            return false;
        }

        writer.write_bytes(mac_roman);
        writer.write_quad(type->count() - 1);
        writer.write_quad(resource_offset);
        writer.write_quad(type->attributes().size());
        writer.write_quad(attribute_offset);

        for (const auto& attribute : type->attributes()) {
            attribute_offset += attribute.second.name().size() + attribute.second.string_value().size() + 2;
        }
        resource_offset += type->count() * constants::resource_length;
    }

    // 5. Now we're writing the actual resource headers.
    uint64_t name_offset = 0;
    for (const auto type : types) {
        for (const auto& resource : *type) {
            writer.write_signed_quad(resource.id());

            // The name is actually stored in the name list, and the resource stores an offset to that name.
            // If no name is assigned to the resource then the offset is encoded as 0xFFFFFFFFFFFFFFFF.
            if (resource.name().empty()) {
                writer.write_quad(std::numeric_limits<uint64_t>::max());
            }
            else {
                // Convert the name to MacRoman so that we can get the length of it when encoded.
                auto mac_roman = encoding::mac_roman::from_utf8(resource.name());
                auto len = mac_roman.size();

                writer.write_quad(name_offset);
                name_offset += (len >= 0x100 ? 0xFF : len) + 1;
            }

            writer.write_byte(0);
            writer.write_quad(resource.data_offset());
            writer.write_long(0);
        }
    }

    // 6. Write out each of the resource names, and calculate the map length.
    name_offset = 0;
    for (const auto type : types) {
        for (const auto& resource : *type) {
            if (resource.name().empty()) {
                continue;
            }

            auto mac_roman = encoding::mac_roman::from_utf8(resource.name());
            if (mac_roman.size() >= 0x100) {
                mac_roman.resize(0xFF);
            }
            name_offset += mac_roman.size() + 1;
            writer.write_byte(static_cast<uint8_t>(mac_roman.size()));
            writer.write_bytes(mac_roman);
        }
    }

    // 7. Finally write out a list of attributes, but make sure the actual location of this attribute list is
    // kept correct.
    auto pos = writer.position();
    writer.set_position(attribute_list_offset_position);
    writer.write_quad(pos);
    writer.set_position(pos);

    attribute_offset = 0;
    for (const auto type : types) {
        const auto& attributes = type->attributes();

        auto initial = writer.position();
        for (const auto& attribute : attributes) {
            writer.write_cstr(attribute.second.name());
            writer.write_cstr(attribute.second.string_value());
        }

        attribute_offset += (writer.position() - initial);
    }
    map_length = static_cast<uint64_t>(writer.size() - map_offset);

    // 8. Fix the preamble sizes.
    writer.set_position(sizeof(uint64_t));
    writer.write_quad(data_offset);
    writer.write_quad(map_offset);
    writer.write_quad(data_length);
    writer.write_quad(map_length);

    writer.set_position(map_offset);
    writer.write_quad(data_offset);
    writer.write_quad(map_offset);
    writer.write_quad(data_length);
    writer.write_quad(map_length);

    // Finish by writing the contents of the resource file to disk.
    writer.save(path, data_offset + data_length + map_length);
    return true;
}
