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
#include "rsrc/extended.hpp"
#include "encoding/macroman/macroman.hpp"

// MARK: - Parsing / Reading

std::vector<std::shared_ptr<graphite::rsrc::type>> graphite::rsrc::extended::parse(std::shared_ptr<graphite::data::reader> reader)
{
	// 1. Resource File preamble, 
    auto version __attribute__((unused)) = reader->read_quad();
	auto data_offset = reader->read_quad();
	auto map_offset = reader->read_quad();
	auto data_length = reader->read_quad();
	auto map_length = reader->read_quad();

	// Before proceeding any further, we need to verify that the resource file is valid.
	// We can do this in two ways. We can check the corresponding second header/preamble
	// at the start of the resource map, and we can check the lengths provided equal the
	// size of the file.
	auto rsrc_size = data_offset + data_length + map_length;
	if (map_offset != data_offset + data_length) {
		throw std::runtime_error("[Extended Resource File] ResourceMap starts at the unexpected location.");
	}

	if (rsrc_size != reader->size()) {
		throw std::runtime_error("[Extended Resource File] ResourceFile has unexpected length.");
	}

	// Now move to the start of the resource map, and verify the contents of the preamble.
	reader->set_position(map_offset);

	if (reader->read_long() != data_offset) {
		throw std::runtime_error("[Extended Resource File] Second Preamble 'data_offset' mismatch.");
	}

	if (reader->read_long() != map_offset) {
		throw std::runtime_error("[Extended Resource File] Second Preamble 'map_offset' mismatch.");
	}

	if (reader->read_long() != data_length) {
		throw std::runtime_error("[Extended Resource File] Second Preamble 'data_length' mismatch.");
	}

	if (reader->read_long() != map_length) {
		throw std::runtime_error("[Extended Resource File] Second Preamble 'map_length' mismatch.");
	}

	// 2. Now that the preamble is parsed and verified, parse the contents
	// of the ResourceMap. The first two fields are used by the actual Resource Manager in
	// the Classic Macintosh OS, but are not used by this implementation.
	auto next_map __attribute__((unused)) = reader->read_long();
	auto reference __attribute__((unused)) = reader->read_short();

	// Start to read the actual content of the resource map. This is the content that
	// we actually care about. The first field is the flags/attributes of the resource
	// fork.
	auto flags __attribute__((unused)) = static_cast<graphite::rsrc::file::flags>(reader->read_short());

	// The next fields are the offsets of the type list and the name list.
	auto type_list_offset = static_cast<uint64_t>(reader->read_quad());
	auto name_list_offset = static_cast<uint64_t>(reader->read_quad());

	// 3. Parse the list of Resource Types.
	reader->set_position(map_offset + type_list_offset);
	auto type_count = reader->read_quad();
	std::vector<std::shared_ptr<graphite::rsrc::type>> types;

	for (auto type_idx = 0; type_idx <= type_count; ++type_idx) {
		auto code = reader->read_cstr(4);
		auto count = static_cast<uint64_t>(reader->read_quad() + 1);
		auto first_resource_offset = static_cast<uint64_t>(reader->read_quad());

		auto type = std::make_shared<graphite::rsrc::type>(code);

		// 4. Parse the list of Resources for the current resource type.
		reader->save_position();
		reader->set_position(map_offset + type_list_offset + first_resource_offset);

		for (auto res_idx = 0; res_idx < count; ++res_idx) {
			auto id = static_cast<int64_t>(reader->read_signed_quad());
			auto name_offset = reader->read_quad();
			auto flags __attribute__((unused)) = reader->read_byte();
			auto resource_data_offset = reader->read_quad();
			auto handle __attribute__((unused)) = reader->read_long();

			// 5. Parse out of the name of the resource.
			std::string name = "";
			if (name_offset != UINT64_MAX) {
				reader->save_position();
				reader->set_position(map_offset + name_list_offset + name_offset);
				name = reader->read_pstr();
				reader->restore_position();
			}

			// 6. Create a data slice for the resource's data.
			reader->save_position();
			reader->set_position(data_offset + resource_data_offset);
			auto data_size = reader->read_quad();
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

void graphite::rsrc::extended::write(const std::string& path, std::vector<std::shared_ptr<graphite::rsrc::type>> types)
{
	auto writer = std::make_shared<graphite::data::writer>();

	// 1. Begin setting up the preamble.
	uint64_t data_offset = 256;
	uint64_t map_offset = 0;
	uint64_t data_length = 0;
	uint64_t map_length = 0;

    writer->write_quad(1);
	writer->write_quad(data_offset);
	writer->write_quad(map_offset);
	writer->write_quad(data_length);
	writer->write_quad(map_length);
	writer->pad_to_size(data_offset);

	// 2. Iterate through all of the resources and write their data blobs to the file data.
    // When doing this we need to record the starting points of each resources data, as
    uint16_t resource_count = 0;
    for (auto type : types) {
        resource_count += type->count();
        
        for (auto resource : type->resources()) {
            // Get the data for the resource and determine its size.
            auto data = resource->data();
            auto size = data->size();
            resource->set_data_offset(writer->size() - data_offset);
            writer->write_quad(size);
            writer->write_data(data);
        }
    }
    
    // 3. Start writing the ResourceMap. This consists of several characteristics,
    // The first of which is a secondary preamble. We can now calculate the map_offset and
    // the data_length, but we're still waiting on the map_length. For now, write these values
    // as zeros.
    map_offset = writer->size();
    data_length = map_offset - data_offset;
    
    writer->write_quad(data_offset);
    writer->write_quad(map_offset);
    writer->write_quad(data_length);
    writer->write_quad(map_length);
    
    // The next six bytes are reserved.
    writer->write_byte(0x00, 6);
    
    // 4. We're now writing the primary map information, which includes flags, and offsets for
    // the type list and the name list. We can calculate where each of these will be.
    const uint64_t resource_type_length = 20;
    const uint64_t resource_length = 29;
    uint64_t type_list_offset = 56; // The type list is 56 bytes from the start of the resource map.
    uint64_t name_list_offset = type_list_offset + (types.size() * resource_type_length) + (resource_count * resource_length) + sizeof(uint64_t);
    
    writer->write_short(0x0000);
    writer->write_quad(type_list_offset);
    writer->write_quad(name_list_offset);

    // Now moving on to actually writing each of the type descriptors into the data.
    uint16_t resource_offset = sizeof(uint64_t) + (types.size() * resource_type_length);
    writer->write_quad(types.size() - 1);
    for (auto type : types) {
        // We need to ensure that the type code is 4 characters -- otherwise this file will be
        // massively corrupt when produced.
        auto mac_roman = graphite::encoding::mac_roman::from_utf8(type->code());
        if (mac_roman.size() != 4) {
            throw std::runtime_error("Attempted to write invalid type code to Resource File '" + type->code() + "'");
        }
        writer->write_bytes(mac_roman);
        writer->write_quad(type->count() - 1);
        writer->write_quad(resource_offset);
        
        resource_offset += type->count() * resource_length;
    }
    
    // 5. Now we're writing the actual resource headers.
    uint16_t name_offset = 0;
    for (auto type : types) {
        for (auto resource : type->resources()) {
            
            writer->write_signed_quad(resource->id());
            
            // The name is actually stored in the name list, and the resource stores an offset
            // to that name. If no name is assigned to the resource then the offset is encoded as
            // 0xFFFFFFFFFFFFFFFF.
            if (resource->name().empty()) {
                writer->write_quad(UINT64_MAX);
            }
            else {
                // Convert the name to MacRoman so that we can get the length of it when encoded.
                auto mac_roman = graphite::encoding::mac_roman::from_utf8(resource->name());
                uint16_t len = mac_roman.size();
                
                writer->write_quad(name_offset);
                name_offset += (len >= 0x100 ? 0xFF : len) + 1;
            }
            writer->write_byte(0x00); // Resource Attribtues
            writer->write_quad(resource->data_offset());
            
            // Finally this is a reserved field for use by the ResourceManager.
            writer->write_long(0x00000000);
            
        }
    }
    
    // 6. Finally we write out each of the resource names, and calculate the map length.
    name_offset = 0;
    for (auto type : types) {
        for (auto resource : type->resources()) {
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
    map_length = static_cast<uint64_t>(writer->size() - map_offset);

    // 7. Fix the preamble values.
    writer->set_position(sizeof(uint64_t));
    writer->write_quad(data_offset);
    writer->write_quad(map_offset);
    writer->write_quad(data_length);
    writer->write_quad(map_length);
    
    writer->set_position(map_offset);
    writer->write_quad(data_offset);
    writer->write_quad(map_offset);
    writer->write_quad(data_length);
    writer->write_quad(map_length);
    
	// Finish by writing the contents of the Resource File to disk.
	writer->save(path);
}
