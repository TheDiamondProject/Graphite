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

#include <limits>
#include <stdexcept>
#include "libGraphite/hints.hpp"
#include "libGraphite/rsrc/classic.hpp"
#include "libGraphite/encoding/macroman/macroman.hpp"

// MARK: - Parsing / Reading

auto graphite::rsrc::classic::parse(const std::shared_ptr<graphite::data::reader>& reader) -> std::vector<std::shared_ptr<graphite::rsrc::type>>
{
	// 1. Resource File preamble, 
	auto data_offset = reader->read_long();
	auto map_offset = reader->read_long();
	auto data_length = reader->read_long();
	auto map_length = reader->read_long();

	// Before proceeding any further, we need to verify that the resource file is valid.
	// We can do this in two ways. We can check the corresponding second header/preamble
	// at the start of the resource map, and we can check the lengths provided equal the
	// size of the file.
    if (data_offset == 0 || map_offset == 0 || map_length == 0) {
        throw std::runtime_error("[Classic Resource File] Invalid Preamble.");
    }

	auto rsrc_size = data_offset + data_length + map_length;
	if (map_offset != data_offset + data_length) {
		throw std::runtime_error("[Classic Resource File] ResourceMap starts at the unexpected location.");
	}

	if (rsrc_size > reader->size()) {
		throw std::runtime_error("[Classic Resource File] ResourceFile has unexpected length.");
	}

	// Now move to the start of the resource map, and verify the contents of the preamble.
	reader->set_position(map_offset);

    auto data_offset2 = reader->read_long();
    auto map_offset2 = reader->read_long();
    auto data_length2 = reader->read_long();
    auto map_length2 = reader->read_long();

    // Ignore second preamble if all zero, as this can happen sometimes.
    if (data_offset2 != 0 || map_offset2 != 0 || data_length2 != 0 || map_length2 != 0) {
        if (data_offset2 != data_offset) {
            throw std::runtime_error("[Classic Resource File] Second Preamble 'data_offset' mismatch.");
        }

        if (map_offset2 != map_offset) {
            throw std::runtime_error("[Classic Resource File] Second Preamble 'map_offset' mismatch.");
        }

        if (data_length2 != data_length) {
            throw std::runtime_error("[Classic Resource File] Second Preamble 'data_length' mismatch.");
        }

        if (map_length2 != map_length) {
            throw std::runtime_error("[Classic Resource File] Second Preamble 'map_length' mismatch.");
        }
    }

	// 2. Now that the preamble is parsed and verified, parse the contents
	// of the ResourceMap. The first two fields are used by the actual Resource Manager in
	// the Classic Macintosh OS, but are not used by this implementation.
	GRAPHITE_UNUSED auto next_map = reader->read_long();
	GRAPHITE_UNUSED auto reference = reader->read_short();

	// Start to read the actual content of the resource map. This is the content that
	// we actually care about. The first field is the flags/attributes of the resource
	// fork.
	GRAPHITE_UNUSED auto flags = static_cast<graphite::rsrc::file::flags>(reader->read_short());

	// The next fields are the offsets of the type list and the name list.
	auto type_list_offset = static_cast<uint64_t>(reader->read_short());
	auto name_list_offset = static_cast<uint64_t>(reader->read_short());

	// 3. Parse the list of Resource Types.
	reader->set_position(map_offset + type_list_offset);
	auto type_count = static_cast<uint16_t>(reader->read_short() + 1);
	std::vector<std::shared_ptr<graphite::rsrc::type>> types;

	for (auto type_idx = 0; type_idx < type_count; ++type_idx) {
		auto code = reader->read_cstr(4);
		auto count = static_cast<uint16_t>(reader->read_short() + 1);
		auto first_resource_offset = static_cast<uint64_t>(reader->read_short());

		auto type = std::make_shared<graphite::rsrc::type>(code);

		// 4. Parse the list of Resources for the current resource type.
		reader->save_position();
		reader->set_position(map_offset + type_list_offset + first_resource_offset);

		for (auto res_idx = 0; res_idx < count; ++res_idx) {
			auto id = static_cast<int64_t>(reader->read_signed_short());
			auto name_offset = reader->read_short();
            GRAPHITE_UNUSED auto flags = reader->read_byte();
			auto resource_data_offset = reader->read_triple();
            GRAPHITE_UNUSED auto handle = reader->read_long();

			// 5. Parse out of the name of the resource.
			std::string name;
			if (name_offset != std::numeric_limits<uint16_t>::max()) {
				reader->save_position();
				reader->set_position(map_offset + name_list_offset + name_offset);
				name = reader->read_pstr();
				reader->restore_position();
			}

			// 6. Create a data slice for the resource's data.
			reader->save_position();
			reader->set_position(data_offset + resource_data_offset);
			auto data_size = reader->read_long();
			auto slice = reader->read_data(data_size);
			reader->restore_position();

			// 7. Construct a new resource instance, and add it to the type.
			auto resource = std::make_shared<graphite::rsrc::resource>(id, type, name, slice);
			type->add_resource(resource);
		}

		reader->restore_position();

		// 8. Save the resource type into the list of types and return it.
		types.push_back(type);
	}

	return types;
}

// MARK: - Writing

auto graphite::rsrc::classic::write(const std::string& path, const std::vector<std::shared_ptr<graphite::rsrc::type>>& types) -> void
{
	auto writer = std::make_shared<graphite::data::writer>();

	// 1. Begin setting up the preamble.
	uint32_t data_offset = 256;
	uint32_t map_offset = 0;
	uint32_t data_length = 0;
	uint32_t map_length = 0;

	writer->write_long(data_offset);
	writer->write_long(map_offset);
	writer->write_long(data_length);
	writer->write_long(map_length);
	writer->pad_to_size(data_offset);

	// 2. Iterate through all of the resources and write their data blobs to the file data.
    // When doing this we need to record the starting points of each resources data, as
    uint16_t resource_count = 0;
    for (const auto& type : types) {
        resource_count += type->count();
        
        for (const auto& resource : type->resources()) {
            // Get the data for the resource and determine its size.
            auto data = resource->data();
            auto size = data->size();
            resource->set_data_offset(writer->size() - data_offset);
            writer->write_long(static_cast<uint32_t>(size));
            writer->write_data(data);
        }
    }
    
    // 3. Start writing the ResourceMap. This consists of several characteristics,
    // The first of which is a secondary preamble. We can now calculate the map_offset and
    // the data_length, but we're still waiting on the map_length. For now, write these values
    // as zeros.
    map_offset = static_cast<uint32_t>(writer->size());
    data_length = map_offset - data_offset;
    
    writer->write_long(data_offset);
    writer->write_long(map_offset);
    writer->write_long(data_length);
    writer->write_long(map_length);
    
    // The next six bytes are used by the MacOS ResourceManager and thus not important to
    // us.
    writer->write_byte(0x00, 6);
    
    // 4. We're now writing the primary map information, which includes flags, and offsets for
    // the type list and the name list. We can calculate where each of these will be.
    const uint16_t resource_type_length = 8;
    const uint16_t resource_length = 12;
    uint16_t type_list_offset = 28; // The type list is 28 bytes from the start of the resource map.
    uint16_t name_list_offset = type_list_offset + (types.size() * resource_type_length) + (resource_count * resource_length) + sizeof(uint16_t);
    
    writer->write_short(0x0000);
    writer->write_short(type_list_offset);
    writer->write_short(name_list_offset);

    // Now moving on to actually writing each of the type descriptors into the data.
    uint16_t resource_offset = sizeof(uint16_t) + (types.size() * resource_type_length);
    writer->write_short(types.size() - 1);
    for (const auto& type : types) {
        // We need to ensure that the type code is 4 characters -- otherwise this file will be
        // massively corrupt when produced.
        auto mac_roman = graphite::encoding::mac_roman::from_utf8(type->code());
        if (mac_roman.size() != 4) {
            throw std::runtime_error("Attempted to write invalid type code to Resource File '" + type->code() + "'");
        }
        writer->write_bytes(mac_roman);
        writer->write_short(type->count() - 1);
        writer->write_short(resource_offset);
        
        resource_offset += type->count() * resource_length;
    }
    
    // 5. Now we're writing the actual resource headers.
    uint16_t name_offset = 0;
    uint16_t name_len = 0;
    for (const auto& type : types) {
        for (const auto& resource : type->resources()) {
            
            auto id = resource->id();
            if (id < std::numeric_limits<int16_t>::min() || id > std::numeric_limits<int16_t>::max()) {
                throw std::runtime_error("Attempted to write resource id outside of valid range.");
            }
            writer->write_signed_short(static_cast<int16_t>(id));
            
            // The name is actually stored in the name list, and the resource stores an offset
            // to that name. If no name is assigned to the resource then the offset is encoded as
            // 0xFFFF.
            if (resource->name().empty()) {
                writer->write_short(0xFFFF);
            }
            else {
                if (name_offset + name_len >= 0xFFFF) {
                    throw std::runtime_error("Attempted to write name offset exceeding maximum value.");
                }
                name_offset += name_len;
                writer->write_short(name_offset);
                
                // Convert the name to MacRoman so that we can get the length of it when encoded.
                auto mac_roman = graphite::encoding::mac_roman::from_utf8(resource->name());
                name_len = mac_roman.size() + 1;
                if (name_len > 0x100) {
                    name_len = 0x100;
                }
            }
            
            // Write the resource attributes - these are currently hard coded as nothing.
            writer->write_byte(0x00);
            
            // The data offset is a 3 byte (24-bit) value. This means the hi-byte needs discarding
            // and then a swap performing.
            auto offset = static_cast<uint32_t>(resource->data_offset());
            if (offset > 0xFFFFFF) {
                throw std::runtime_error("Attempted to write resource file exceeding maximum size.");
            }
            writer->write_byte((offset >> 16) & 0xFF);
            writer->write_byte((offset >>  8) & 0xFF);
            writer->write_byte((offset >>  0) & 0xFF);
            
            // Finally this is a reserved field for use by the ResourceManager.
            writer->write_long(0x00000000);
            
        }
    }
    
    // 6. Finally we write out each of the resource names, and calculate the map length.
    name_offset = 0;
    for (const auto& type : types) {
        for (const auto& resource : type->resources()) {
            if (resource->name().empty()) {
                continue;
            }
            
            auto mac_roman = graphite::encoding::mac_roman::from_utf8(resource->name());
            if (mac_roman.size() >= 0x100) {
                mac_roman.resize(0xFF);
            }
            name_offset += mac_roman.size() + 1;
            
            writer->write_byte(static_cast<uint8_t>(mac_roman.size()));
            writer->write_bytes(mac_roman);
        }
    }
    // Even if the data fits the spec, the resource manager will still not read files larger than 16MB
    if (writer->size() > 0xFFFFFF) {
        throw std::runtime_error("Attempted to write resource file exceeding maximum size.");
    }
    map_length = static_cast<uint32_t>(writer->size() - map_offset);

    // 7. Fix the preamble values.
    writer->set_position(0);
    writer->write_long(data_offset);
    writer->write_long(map_offset);
    writer->write_long(data_length);
    writer->write_long(map_length);
    
    writer->set_position(map_offset);
    writer->write_long(data_offset);
    writer->write_long(map_offset);
    writer->write_long(data_length);
    writer->write_long(map_length);
    
	// Finish by writing the contents of the Resource File to disk.
	writer->save(path);
}
