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

#include "libGraphite/toolbox/dialog_item_list.hpp"
#include "libGraphite/data/reader.hpp"

// MARK: - Construction

graphite::toolbox::dialog_item_list::dialog_item_list(const data::block &data, rsrc::resource::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::toolbox::dialog_item_list::dialog_item_list(data::reader &reader)
{
    decode(reader);
}

// MARK: - Encoder

auto graphite::toolbox::dialog_item_list::encode(data::writer &writer) -> void
{
    writer.write_short(m_items.size() - 1);

    for (const auto& item : m_items) {
        writer.write_long(0);

        auto frame = item.frame;
        frame.encode(writer, quickdraw::coding_type::macintosh);

        writer.write_enum(item.type);
        writer.write_pstr(item.info);

        if (writer.position() % 2 == 1) {
            writer.write_byte(0);
        }
    }
}

auto graphite::toolbox::dialog_item_list::data() -> data::block
{
    data::writer writer;
    encode(writer);
    return std::move(*const_cast<data::block *>(writer.data()));
}

// MARK: - Decoder

auto graphite::toolbox::dialog_item_list::decode(data::reader &reader) -> void
{
    auto count = reader.read_short();

    for (auto i = 0; i <= count; ++i) {
        reader.move(4);

        struct item item;
        item.frame = quickdraw::rect<std::int16_t>::read(reader, quickdraw::coding_type::macintosh);
        item.type = static_cast<item_type>(reader.read_byte());
        item.info = reader.read_pstr();

        if (reader.position() % 2 == 1) {
            reader.move();
        }

        m_items.emplace_back(std::move(item));
    }
}