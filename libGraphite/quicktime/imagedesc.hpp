//
// Created by Tom Hancocks on 26/03/2021.
//

#if !defined(GRAPHITE_IMAGEDESC_HPP)
#define GRAPHITE_IMAGEDESC_HPP

#include <memory>
#include "libGraphite/data/reader.hpp"
#include "libGraphite/quickdraw/clut.hpp"
#include "libGraphite/quickdraw/internal/surface.hpp"

namespace graphite::qt {

    struct imagedesc
    {
    private:
        int32_t m_length { 0 };
        uint32_t m_compressor { 0 };
        uint32_t m_version { 0 };
        int16_t m_width { 0 };
        int16_t m_height { 0 };
        int32_t m_data_size { 0 };
        int16_t m_depth { 0 };
        int32_t m_data_offset { 0 };
        std::shared_ptr<qd::clut> m_clut { nullptr };
        std::shared_ptr<qd::surface> m_surface { nullptr };

        auto read_image_data(data::reader& reader) -> void;
    public:
        explicit imagedesc(data::reader& reader);

        [[nodiscard]] auto length() const -> int32_t;
        [[nodiscard]] auto compressor() const -> uint32_t;
        [[nodiscard]] auto version() const -> uint32_t;
        [[nodiscard]] auto width() const -> int16_t;
        [[nodiscard]] auto height() const -> int16_t;
        [[nodiscard]] auto data_size() const -> int32_t;
        [[nodiscard]] auto depth() const -> int16_t;
        [[nodiscard]] auto data_offset() const -> int32_t;
        [[nodiscard]] auto clut() const -> std::shared_ptr<qd::clut>;
        [[nodiscard]] auto surface() const -> std::shared_ptr<qd::surface>;
    };

}

#endif //GRAPHITE_IMAGEDESC_HPP
