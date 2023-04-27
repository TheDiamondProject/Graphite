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

#include "libGraphite/toolbox/dialog.hpp"
#include "libGraphite/data/reader.hpp"

// MARK: - Construction

graphite::toolbox::dialog::dialog(const data::block &data, rsrc::resource::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

graphite::toolbox::dialog::dialog(graphite::data::reader &reader)
{
    decode(reader);
}

// MARK: - Accessors

auto graphite::toolbox::dialog::bounds() const -> quickdraw::rect<std::int16_t>
{
    return m_bounds;
}

auto graphite::toolbox::dialog::proc_id() const -> std::int16_t
{
    return m_proc_id;
}

auto graphite::toolbox::dialog::visible() const -> bool
{
    return m_visible;
}

auto graphite::toolbox::dialog::go_away() const -> bool
{
    return m_go_away;
}

auto graphite::toolbox::dialog::ref_con() const -> std::int32_t
{
    return m_ref_con;
}

auto graphite::toolbox::dialog::interface_list() const -> rsrc::resource::identifier
{
    return m_ditl_id;
}

auto graphite::toolbox::dialog::auto_position() const -> std::uint16_t
{
    return m_auto_position;
}

auto graphite::toolbox::dialog::title() const -> std::string
{
    return m_title;
}

auto graphite::toolbox::dialog::set_bounds(const quickdraw::rect<std::int16_t>& bounds) -> void
{
    m_bounds = bounds;
}

auto graphite::toolbox::dialog::set_proc_id(std::int16_t id) -> void
{
    m_proc_id = id;
}

auto graphite::toolbox::dialog::set_visible(bool visible) -> void
{
    m_visible = visible;
}

auto graphite::toolbox::dialog::set_go_away(bool go_away) -> void
{
    m_go_away = go_away;
}

auto graphite::toolbox::dialog::set_ref_con(std::int32_t ref_con) -> void
{
    m_ref_con = ref_con;
}

auto graphite::toolbox::dialog::set_interface_list(rsrc::resource::identifier id) -> void
{
    m_ditl_id = id;
}

auto graphite::toolbox::dialog::set_auto_position(std::uint16_t position) -> void
{
    m_auto_position = position;
}

auto graphite::toolbox::dialog::set_title(const std::string& title) -> void
{
    m_title = title;
}

// MARK: - Decoder

auto graphite::toolbox::dialog::decode(data::reader &reader) -> void
{
    m_bounds = quickdraw::rect<std::int16_t>::read(reader, quickdraw::coding_type::macintosh);

    m_proc_id = reader.read_signed_short();
    m_visible = reader.read_short() != 0;
    m_go_away = reader.read_short() != 0;
    m_ref_con = reader.read_signed_long();
    m_ditl_id = reader.read_short();
    m_title = reader.read_pstr();

    if (reader.position() % 2 != 0) {
        reader.move();
    }

    m_auto_position = reader.read_short();
}

// MARK: - Encoder

auto graphite::toolbox::dialog::encode(data::writer &writer) -> void
{

}

auto graphite::toolbox::dialog::data() -> data::block
{
    data::writer writer;
    encode(writer);
    return std::move(*const_cast<data::block *>(writer.data()));
}
