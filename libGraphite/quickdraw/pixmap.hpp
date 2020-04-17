//
// Created by Tom Hancocks on 19/03/2020.
//

#if !defined(GRAPHITE_PIXMAP_HPP)
#define GRAPHITE_PIXMAP_HPP

#include <cstdint>
#include "libGraphite/data/reader.hpp"
#include "libGraphite/quickdraw/geometry.hpp"
#include "libGraphite/data/data.hpp"

namespace graphite { namespace qd {

    enum pixel_format
    {
        unknown = 0,
        monochrome = 0x01,          // 1 bit indexed
        indexed_2 = 0x02,           // 2 bit indexed
        indexed_4 = 0x04,           // 4 bit indexed
        indexed_8 = 0x08,           // 8 bit indexed
        b16_rgb555 = 0x10,          // 16 bit, RGB 555 (Mac)
        true_color = 0x18,          // 24 bit RGB
        true_color_alpha = 0x20,    // 32 bit ARGB
    };

    class pixmap
    {
    private:
        uint32_t m_base_address { 0 };
        int16_t m_row_bytes { 0 };
        graphite::qd::rect m_bounds { rect::zero() };
        int16_t m_pm_version { 0 };
        int16_t m_pack_type { 0 };
        int32_t m_pack_size { 0 };
        double m_h_res { 72 };
        double m_v_res { 72 };
        int16_t m_pixel_type { 0 };
        int16_t m_pixel_size { 0 };
        int16_t m_cmp_count { 0 };
        int16_t m_cmp_size { 0 };
        enum pixel_format m_pixel_format { unknown };
        uint32_t m_pm_table { 0 };
        uint32_t m_pm_extension { 0 };
    public:
        static constexpr int length { 50 };

        pixmap();
        pixmap(qd::rect frame);
        pixmap(std::shared_ptr<data::data> data);

        auto bounds() const -> graphite::qd::rect;
        auto row_bytes() const -> int16_t;
        auto pack_type() const -> int16_t;
        auto pack_size() const -> int16_t;
        auto pixel_type() const -> int16_t;
        auto pixel_size() const -> int16_t;
        auto cmp_count() const -> int16_t;
        auto cmp_size() const -> int16_t;
        auto pixel_format() const -> enum pixel_format;

        auto set_bounds(const graphite::qd::rect rect) -> void;
        auto set_row_bytes(const int16_t row_bytes) -> void;
        auto set_pack_type(const int16_t pack_type) -> void;
        auto set_pack_size(const int16_t pack_size) -> void;
        auto set_pixel_type(const int16_t pixel_type) -> void;
        auto set_pixel_size(const int16_t pixel_size) -> void;
        auto set_cmp_count(const int16_t cmp_count) -> void;
        auto set_cmp_size(const int16_t cmp_size) -> void;

        auto write(graphite::data::writer& writer) -> void;
    };

}};

#endif //GRAPHITE_PIXMAP_HPP
