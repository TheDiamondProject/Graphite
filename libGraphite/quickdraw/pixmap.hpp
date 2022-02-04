//
// Created by Tom Hancocks on 19/03/2020.
//

#if !defined(GRAPHITE_PIXMAP_HPP)
#define GRAPHITE_PIXMAP_HPP

#include <cstdint>
#include "libGraphite/data/reader.hpp"
#include "libGraphite/quickdraw/clut.hpp"
#include "libGraphite/quickdraw/geometry.hpp"
#include "libGraphite/quickdraw/internal/surface.hpp"
#include "libGraphite/data/data.hpp"

namespace graphite::qd {

    enum pack_type
    {
        none = 0,
        argb = 1,
        rgb = 2, // Same as none
        packbits_word = 3,
        packbits_component = 4,
    };

    enum pixel_type
    {
        indexed = 0,
        direct = 16,
    };

    class pixmap
    {
    private:
        uint32_t m_base_address { 0 };
        int16_t m_row_bytes { 0 };
        graphite::qd::rect m_bounds { rect::zero() };
        int16_t m_pm_version { 0 };
        enum pack_type m_pack_type { none };
        int32_t m_pack_size { 0 };
        double m_h_res { 72 };
        double m_v_res { 72 };
        enum pixel_type m_pixel_type { indexed };
        int16_t m_pixel_size { 0 };
        int16_t m_cmp_count { 0 };
        int16_t m_cmp_size { 0 };
        uint32_t m_plane_bytes { 0 };
        uint32_t m_pm_table { 0 };
        uint32_t m_pm_extension { 0 };
    public:
        static constexpr int length { 50 };

        pixmap();
        explicit pixmap(qd::rect frame);
        explicit pixmap(std::shared_ptr<data::data> data);

        [[nodiscard]] auto bounds() const -> graphite::qd::rect;
        [[nodiscard]] auto row_bytes() const -> int16_t;
        [[nodiscard]] auto pack_type() const -> enum graphite::qd::pack_type;
        [[nodiscard]] auto pack_size() const -> int16_t;
        [[nodiscard]] auto pixel_type() const -> enum graphite::qd::pixel_type;
        [[nodiscard]] auto pixel_size() const -> int16_t;
        [[nodiscard]] auto cmp_count() const -> int16_t;
        [[nodiscard]] auto cmp_size() const -> int16_t;
        [[nodiscard]] auto plane_bytes() const -> uint32_t;
        [[nodiscard]] auto pm_table() const -> uint32_t;

        auto set_bounds(const graphite::qd::rect& rect) -> void;
        auto set_row_bytes(const int16_t& row_bytes) -> void;
        auto set_pack_type(const graphite::qd::pack_type& pack_type) -> void;
        auto set_pack_size(const int16_t& pack_size) -> void;
        auto set_pixel_type(const graphite::qd::pixel_type& pixel_type) -> void;
        auto set_pixel_size(const int16_t& pixel_size) -> void;
        auto set_cmp_count(const int16_t& cmp_count) -> void;
        auto set_cmp_size(const int16_t& cmp_size) -> void;
        auto set_pm_table(const uint32_t& pm_table) -> void;

        auto build_surface(
            std::shared_ptr<graphite::qd::surface> surface,
            const std::vector<uint8_t>& pixel_data,
            const qd::clut& clut,
            qd::rect destination
        ) -> void;
        auto build_pixel_data(const std::vector<uint16_t>& color_values, uint16_t pixel_size) -> std::shared_ptr<graphite::data::data>;
        auto write(graphite::data::writer& writer) -> void;
    };

}

#endif //GRAPHITE_PIXMAP_HPP
