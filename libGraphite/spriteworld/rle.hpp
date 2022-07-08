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

#pragma once

#include <string>
#include <vector>
#include "libGraphite/quickdraw/support/surface.hpp"
#include "libGraphite/quickdraw/type/rect.hpp"

namespace graphite::spriteworld
{
    template<std::size_t Width>
    struct rle
    {
    public:
        static_assert(Width == 16 || Width == 32);

        static auto type_code() -> std::string;

    public:
        rle() = default;

        rle(const quickdraw::size<std::int16_t>& size, std::uint16_t frame_count)
            : m_id(0), m_name("RLE"), m_frame_size(size), m_frame_count(frame_count), m_bpp(Width), m_palette_id(0)
        {
            _create_surface();
        }

        explicit rle(const data::block& data, rsrc::resource::identifier id = 0, const std::string& name = "")
            : m_id(id), m_name(name)
        {
            data::reader reader(&data);
            _decode(reader);
        }

        explicit rle(data::reader& reader)
        {
            _decode(reader);
        }

        ~rle() = default;

        auto surface() -> quickdraw::surface&
        {
            return m_surface;
        }

        [[nodiscard]] auto frames() const -> std::vector<quickdraw::rect<std::int16_t>>
        {
            return {};
        }

        [[nodiscard]] auto frame_count() const -> std::size_t
        {
            return m_frame_count;
        }

        [[nodiscard]] auto frame_rect(std::uint32_t idx) const -> quickdraw::rect<std::int16_t>
        {
            return _frame_rect(idx);
        }

        [[nodiscard]] auto frame_surface(std::uint32_t idx) const -> quickdraw::surface
        {
            return _frame_surface(idx);
        }

        auto write_frame(std::uint32_t frame, const quickdraw::surface& surface) -> void
        {
            _write_frame(frame, surface);
        }

        auto encode(data::writer& writer) -> void
        {
            return _encode(writer);
        }

        auto data() -> data::block
        {
            data::writer writer;
            _encode(writer);
            return std::move(*const_cast<data::block *>(writer.data()));
        }

    private:
        enum class pixel_type { type1, type2 };

        enum class opcode : std::uint8_t
        {
            eof = 0x00,
            line_start = 0x01,
            pixel_data = 0x02,
            transparent_run = 0x03,
            pixel_run = 0x04,
        };

        rsrc::resource::identifier m_id { 0 };
        std::string m_name;
        std::vector<quickdraw::rect<std::int16_t>> m_frames;
        quickdraw::surface m_surface;
        quickdraw::size<std::int16_t> m_frame_size { 0 };
        quickdraw::size<std::int16_t> m_grid_size { 0 };
        std::uint16_t m_frame_count { 0 };
        std::uint16_t m_bpp { 0 };
        std::uint16_t m_palette_id { 0 };

        auto _create_surface() -> void;

        [[nodiscard]] auto _frame_rect(std::uint32_t idx) const -> quickdraw::rect<std::int16_t>;
        [[nodiscard]] auto _frame_surface(std::uint32_t idx) const -> quickdraw::surface;
        auto _write_frame(std::uint32_t frame, const quickdraw::surface& surface) -> void;

        auto _encode(data::writer& writer) -> void;
        auto _decode(data::reader& reader) -> void;

        [[nodiscard]] auto _surface_offset(std::int32_t frame, std::int32_t offset) -> std::uint64_t;

        auto _write_pixel(std::uint32_t pixel, std::uint8_t mask, std::uint64_t offset) -> void;
        auto _write_pixel(std::uint64_t pixel, std::uint8_t mask, std::uint64_t offset, enum pixel_type type) -> void;
    };

    template<> auto rle<16>::type_code() -> std::string { return "rlëD"; }
    template<> auto rle<32>::type_code() -> std::string { return "rlëX"; }
}
