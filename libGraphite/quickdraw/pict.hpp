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
            pen_size = 0x0007,
            pen_mode = 0x0008,
            pen_pattern = 0x0009,
            fill_pattern = 0x000a,
            origin = 0x000c,
            rgb_fg_color = 0x001a,
            rgb_bg_color = 0x001b,
            hilite_mode = 0x001c,
            hilite_color = 0x001d,
            def_hilite = 0x001e,
            op_color = 0x001f,
            line = 0x0020,
            line_from = 0x0021,
            short_line = 0x0022,
            short_line_from = 0x0023,
            frame_rect = 0x0030,
            paint_rect = 0x0031,
            erase_rect = 0x0032,
            invert_rect = 0x0033,
            fill_rect = 0x0034,
            frame_same_rect = 0x0038,
            paint_same_rect = 0x0039,
            erase_same_rect = 0x003a,
            invert_same_rect = 0x003b,
            fill_same_rect = 0x003c,
            bits_rect = 0x0090,
            bits_region = 0x0091,
            pack_bits_rect = 0x0098,
            pack_bits_region = 0x0099,
            direct_bits_rect = 0x009a,
            direct_bits_region = 0x009b,
            frame_region = 0x0080,
            paint_region = 0x0081,
            erase_region = 0x0082,
            invert_region = 0x0083,
            fill_region = 0x0084,
            short_comment = 0x00a0,
            long_comment = 0x00a1,
            eof = 0x00ff,
            ext_header = 0x0c00,
            compressed_quicktime = 0x8200,
            uncompressed_quicktime = 0x8201,
        };

    private:
        int64_t m_id {};
        std::string m_name;
        std::shared_ptr<graphite::qd::surface> m_surface;
        graphite::qd::rect m_frame;
        uint32_t m_format {};
        double m_x_ratio {};
        double m_y_ratio {};

        auto parse(graphite::data::reader& pict_reader) -> void;
        auto read_region(graphite::data::reader& pict_reader) const -> graphite::qd::rect;
        auto read_long_comment(graphite::data::reader& pict_reader) -> void;
        auto read_direct_bits_rect(graphite::data::reader& pict_reader, bool region) -> void;
        auto read_indirect_bits_rect(graphite::data::reader& pict_reader, bool packed, bool region) -> void;
        auto read_compressed_quicktime(graphite::data::reader & pict_reader) -> void;
        auto read_uncompressed_quicktime(graphite::data::reader & pict_reader) -> void;
        auto read_image_description(graphite::data::reader & pict_reader) -> void;

        auto encode(graphite::data::writer& pict_encoder, bool rgb555) -> void;
        auto encode_header(graphite::data::writer& pict_encoder) -> void;
        auto encode_def_hilite(graphite::data::writer& pict_encoder) -> void;
        auto encode_clip_region(graphite::data::writer& pict_encoder) -> void;
        auto encode_direct_bits_rect(graphite::data::writer& pict_encoder, bool rgb555) -> void;

    public:
        explicit pict(std::shared_ptr<graphite::data::data> data, int64_t id = 0, std::string name = "");
        explicit pict(std::shared_ptr<graphite::qd::surface> surface);

        static auto load_resource(int64_t id) -> std::shared_ptr<pict>;
        static auto from_surface(std::shared_ptr<graphite::qd::surface> surface) -> std::shared_ptr<pict>;

        [[nodiscard]] auto image_surface() const -> std::weak_ptr<graphite::qd::surface>;
        [[nodiscard]] auto format() const -> uint32_t;

        auto data(bool rgb555 = false) -> std::shared_ptr<graphite::data::data>;
    };

}

#endif //GRAPHITE_PICT_HPP
