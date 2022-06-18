//
// Created by Tom Hancocks on 24/03/2020.
//

#if !defined(GRAPHITE_RLE_HPP)
#define GRAPHITE_RLE_HPP

#include <memory>
#include "libGraphite/quickdraw/internal/surface.hpp"
#include "libGraphite/quickdraw/geometry.hpp"

namespace graphite::qd {

    class rle
    {
    private:
        enum opcode : uint8_t
        {
            eof = 0x00,
            line_start = 0x01,
            pixel_data = 0x02,
            transparent_run = 0x03,
            pixel_run = 0x04,
        };

        int64_t m_id {};
        std::string m_name;
        std::vector<qd::rect> m_frames;
        std::shared_ptr<qd::surface> m_surface;
        qd::size m_frame_size;
        qd::size m_grid_size;
        uint16_t m_frame_count {};
        uint16_t m_bpp {};
        uint16_t m_palette_id {};

        auto parse(data::reader &reader) -> void;
        [[nodiscard]] auto surface_offset(int32_t frame, int32_t line) const -> uint64_t;
        auto write_pixel(uint16_t pixel, uint8_t mask, uint64_t offset) -> void;
        auto write_pixel_variant1(uint32_t pixel, uint8_t mask, uint64_t offset) -> void;
        auto write_pixel_variant2(uint32_t pixel, uint8_t mask, uint64_t offset) -> void;

        auto encode(graphite::data::writer& writer) -> void;

    public:
        explicit rle(std::shared_ptr<data::data> data, int64_t id = 0, std::string name = "");
        rle(qd::size frame_size, uint16_t frame_count);

        static auto load_resource(int64_t id) -> std::shared_ptr<rle>;

        [[nodiscard]] auto surface() const -> std::weak_ptr<qd::surface>;
        [[nodiscard]] auto frames() const -> std::vector<qd::rect>;

        [[nodiscard]] auto frame_count() const -> int;
        [[nodiscard]] auto frame_rect(int frame) const -> qd::rect;
        [[nodiscard]] auto frame_surface(int frame) const -> std::shared_ptr<qd::surface>;
        auto write_frame(int frame, const std::shared_ptr<qd::surface>& surface) -> void;

        auto data() -> std::shared_ptr<graphite::data::data>;
    };

}

#endif //GRAPHITE_RLE_HPP
