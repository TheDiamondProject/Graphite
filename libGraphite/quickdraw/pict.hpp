//
// Created by Tom Hancocks on 20/02/2020.
//

#if !defined(GRAPHITE_PICT_HPP)
#define GRAPHITE_PICT_HPP

#include <libGraphite/data/writer.hpp>
#include "libGraphite/quickdraw/internal/surface.hpp"
#include "libGraphite/quickdraw/pixmap.hpp"
#include "libGraphite/data/reader.hpp"

namespace graphite::qd {

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
            bits_rect = 0x0090,
            pack_bits_rect = 0x0098,
            direct_bits_rect = 0x009a,
            eof = 0x00ff,
            def_hilite = 0x001e,
            op_color = 0x001f,
            short_comment = 0x00a0,
            long_comment = 0x00a1,
            ext_header = 0x0c00,
            compressed_quicktime = 0x8200,
            uncompressed_quicktime = 0x8201,
        };

    private:
        int64_t m_id {};
        std::string m_name;
        std::shared_ptr<graphite::qd::surface> m_surface;
        graphite::qd::rect m_frame;
        double m_x_ratio {};
        double m_y_ratio {};
        std::size_t m_size;

        auto parse(graphite::data::reader& pict_reader) -> void;
        auto read_region(graphite::data::reader& pict_reader) const -> graphite::qd::rect;
        auto read_long_comment(graphite::data::reader& pict_reader) -> void;
        auto read_direct_bits_rect(graphite::data::reader& pict_reader) -> void;
        auto read_indirect_bits_rect(graphite::data::reader& pict_reader, bool packed) -> void;
        auto read_compressed_quicktime(graphite::data::reader & pict_reader) -> void;
        auto read_uncompressed_quicktime(graphite::data::reader & pict_reader) -> void;
        auto read_image_description(graphite::data::reader & pict_reader) -> void;

        auto encode(graphite::data::writer& pict_encoder) -> void;
        auto encode_header(graphite::data::writer& pict_encoder) -> void;
        auto encode_def_hilite(graphite::data::writer& pict_encoder) -> void;
        auto encode_clip_region(graphite::data::writer& pict_encoder) -> void;
        auto encode_direct_bits_rect(graphite::data::writer& pict_encoder) -> void;

    public:
        explicit pict(std::shared_ptr<graphite::data::data> data, int64_t id = 0, std::string name = "");
        explicit pict(std::shared_ptr<graphite::qd::surface> surface);

        static auto load_resource(int64_t id) -> std::shared_ptr<pict>;
        static auto from_surface(std::shared_ptr<graphite::qd::surface> surface) -> std::shared_ptr<pict>;

        [[nodiscard]] auto image_surface() const -> std::weak_ptr<graphite::qd::surface>;

        auto data() -> std::shared_ptr<graphite::data::data>;
    };

}

#endif //GRAPHITE_PICT_HPP
