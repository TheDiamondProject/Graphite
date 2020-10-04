//
// Created by Tom Hancocks on 17/07/2020.
//

#if !defined(GRAPHITE_PPAT_HPP)
#define GRAPHITE_PPAT_HPP

#include <string>
#include "libGraphite/quickdraw/internal/surface.hpp"
#include "libGraphite/quickdraw/geometry.hpp"
#include "libGraphite/quickdraw/pixmap.hpp"
#include "libGraphite/quickdraw/clut.hpp"

namespace graphite::qd {

    class ppat
    {
    private:
        int64_t m_id {};
        std::string m_name;
        uint16_t m_pat_type {};
        uint32_t m_pmap_base_addr {};
        uint32_t m_pat_base_addr {};
        qd::pixmap m_pixmap;
        std::shared_ptr<qd::surface> m_surface;
        qd::clut m_clut;

        auto parse(data::reader& reader) -> void;

    public:
        explicit ppat(std::shared_ptr<graphite::data::data> data, int64_t id = 0, std::string name = "");
        explicit ppat(std::shared_ptr<qd::surface> surface);

        static auto load_resource(int64_t id) -> std::shared_ptr<ppat>;

        [[nodiscard]] auto surface() const -> std::weak_ptr<graphite::qd::surface>;
        auto data() -> std::shared_ptr<graphite::data::data>;
    };

}

#endif //GRAPHITE_PPAT_HPP
