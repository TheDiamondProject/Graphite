//
// Created by Tom Hancocks on 25/03/2020.
//

#if !defined(GRAPHITE_CICN_HPP)
#define GRAPHITE_CICN_HPP

#include <string>
#include "libGraphite/quickdraw/internal/surface.hpp"
#include "libGraphite/quickdraw/geometry.hpp"
#include "libGraphite/quickdraw/pixmap.hpp"
#include "libGraphite/quickdraw/clut.hpp"

namespace graphite { namespace qd {

    class cicn
    {
    private:
        int64_t m_id;
        std::string m_name;
        qd::pixmap m_pixmap;
        uint32_t m_mask_base_addr;
        uint16_t m_mask_row_bytes;
        qd::rect m_mask_bounds;
        uint32_t m_bmap_base_addr;
        uint16_t m_bmap_row_bytes;
        qd::rect m_bmap_bounds;
        uint32_t m_icon_data;
        std::shared_ptr<qd::surface> m_surface;
        qd::clut m_clut;

        auto parse(data::reader& reader) -> void;

    public:
        cicn(std::shared_ptr<graphite::data::data> data, int64_t id = 0, std::string name = "");

        static auto load_resource(int64_t id) -> std::shared_ptr<cicn>;

        auto surface() const -> std::weak_ptr<graphite::qd::surface>;
        auto data() -> std::shared_ptr<graphite::data::data>;
    };

}};

#endif //GRAPHITE_CICN_HPP
