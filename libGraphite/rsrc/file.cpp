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

#include <stdexcept>
#include "libGraphite/rsrc/file.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/rsrc/classic.hpp"
#include "libGraphite/rsrc/extended.hpp"
#include "libGraphite/rsrc/rez.hpp"

// MARK: - Construct

graphite::rsrc::file::file(std::string path)
    : m_path(std::move(path))
{
	read(m_path);
}

// MARK: - Accessors

auto graphite::rsrc::file::type_count() const -> std::size_t
{
	return m_types.size();
}

auto graphite::rsrc::file::types() const -> std::vector<std::shared_ptr<type>>
{
    return m_types;
}

auto graphite::rsrc::file::current_format() const -> graphite::rsrc::file::format
{
	return m_format;
}

auto graphite::rsrc::file::name() const -> std::string
{
    std::string out;
    auto path = m_path;

    while (!m_path.empty()) {
        if (path.back() == '.') {
            out.clear();
            path.pop_back();
        }
        else if (path.back() == '/') {
            return out;
        }
        else {
            out.insert(out.begin(), path.back());
            path.pop_back();
        }
    }

    return out;
}

// MARK: - File Reading

auto graphite::rsrc::file::read(const std::string& path) -> void
{
	// Load the file data and prepare to parse the contents of the resource
	// file. We also need to keep hold of the actual internal data.
	auto reader = std::make_shared<graphite::data::reader>(path);
	m_data = reader->get();

	// 1. Determine the file format and validity.
	if (reader->read_quad(0, graphite::data::reader::mode::peek) == 1) {
		m_format = graphite::rsrc::file::format::extended;
	}
    else if (reader->read_long(0, graphite::data::reader::mode::peek) == 'BRGR') {
        m_format = graphite::rsrc::file::format::rez;
    }
	else {
		m_format = graphite::rsrc::file::format::classic;
	}

	// 2. Launch the appropriate parser for the current format of the file.
	switch (m_format) {
		case graphite::rsrc::file::format::classic: {
			m_types = graphite::rsrc::classic::parse(reader);
			break;
		}
		case graphite::rsrc::file::format::extended: {
			m_types = graphite::rsrc::extended::parse(reader);
			break;
		}
		case graphite::rsrc::file::format::rez: {
			m_types = graphite::rsrc::rez::parse(reader);
			break;
		}

		default:
			throw std::runtime_error("Resource File format not currently handled.");
			break;
	}
}

// MARK: - File Writing

auto graphite::rsrc::file::write(const std::string& path, enum graphite::rsrc::file::format fmt) -> void
{
	// Determine the correct location to save to, or throw an error.
	auto write_path = path;
	if (path.empty()) {
		if (m_path.empty()) {
			throw std::runtime_error("Unable to write resource file to disk. No save location provided.");
		}
		write_path = m_path;
	}

	switch (fmt) {
		case graphite::rsrc::file::format::classic: {
			graphite::rsrc::classic::write(write_path, m_types);
			break;
		}
		case graphite::rsrc::file::format::extended: {
			graphite::rsrc::extended::write(write_path, m_types);
			break;
		}
		case graphite::rsrc::file::format::rez: {
			graphite::rsrc::rez::write(write_path, m_types);
			break;
		}
	}
	
}

// MARK: - Resource Managemnet

auto graphite::rsrc::file::add_resource(const std::string &code, const int64_t &id, const std::string &name,
                                        const std::shared_ptr<graphite::data::data> &data,
                                        const std::map<std::string, std::string> &attributes) -> void
{
    // Get the container
    if (auto type = type_container(code, attributes).lock()) {
        // Add the resource...
        auto resource = std::make_shared<graphite::rsrc::resource>(id, type, name, data);
        type->add_resource(resource);
        return;
    }

    throw std::runtime_error("Failed to find or create resource type container for " + code);
}

auto graphite::rsrc::file::type_container(const std::string &code,
                                          const std::map<std::string, std::string> &attributes) -> std::weak_ptr<graphite::rsrc::type>
{
    for (const auto& type : m_types) {
        if (type->code() == code && type->attributes() == attributes) {
            // Note: Not sure if this is the correct solution here (if not the solution will need some further though)t
            // This currently assumes that the attributes are part of the type definition, and that
            // `alph` != `alph:lang=en` != `alph:lang=en:bar=2`
            return type;
        }
    }

    auto type = std::make_shared<graphite::rsrc::type>(code, attributes);
    m_types.push_back(type);
    return type;
}

auto graphite::rsrc::file::find(const std::string& type, const int64_t& id, const std::map<std::string, std::string> &attributes) -> std::weak_ptr<graphite::rsrc::resource>
{
    if (auto container = type_container(type, attributes).lock()) {
        return container->get(id);
    }
    return {};
}

auto graphite::rsrc::file::find(const std::string &type, const std::string &name_prefix,
                                const std::map<std::string, std::string> &attributes) -> std::vector<std::shared_ptr<resource>>
{
    if (auto container = type_container(type, attributes).lock()) {
        return container->get(name_prefix);
    }
    return {};
}
