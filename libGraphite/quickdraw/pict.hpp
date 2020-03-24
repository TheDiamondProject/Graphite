//
// Created by Tom Hancocks on 20/02/2020.
//

#if !defined(GRAPHITE_PICT_HPP)
#define GRAPHITE_PICT_HPP

#include "libGraphite/quickdraw/internal/surface.hpp"
#include "libGraphite/quickdraw/pixmap.hpp"
#include "libGraphite/data/reader.hpp"

namespace graphite { namespace qd {

    /**
     * The `graphite::qd::pict` class represents a QuickDraw Picture.
     */
    class pict
    {
    public:
        enum opcode
        {
            nop = 0x0000,
            clip_region = 0x0001,
            pack_bits_rect = 0x0098,
            direct_bits_rect = 0x009a,
            eof = 0x00ff,
            def_hilite = 0x001e,
            long_comment = 0x00a1,
            ext_header = 0x0c00,
        };

    private:
        int64_t m_id;
        std::string m_name;
        std::shared_ptr<graphite::qd::surface> m_surface;
        graphite::qd::rect m_frame;
        double m_x_ratio;
        double m_y_ratio;
        std::size_t m_size;

        void parse(graphite::data::reader& pict_reader);
        graphite::qd::rect read_region(graphite::data::reader& pict_reader);
        void read_long_comment(graphite::data::reader& pict_reader);
        void read_direct_bits_rect(graphite::data::reader& pict_reader);
        void read_pack_bits_rect(graphite::data::reader & pict_reader);

    public:
        pict(std::shared_ptr<graphite::data::data> data, int64_t id = 0, std::string name = "");

        static std::shared_ptr<pict> load_resource(int64_t id);

        std::weak_ptr<graphite::qd::surface> image_surface() const;

    };

}}

#endif //GRAPHITE_PICT_HPP
