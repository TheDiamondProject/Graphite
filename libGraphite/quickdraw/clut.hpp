//
// Created by Tom Hancocks on 25/03/2020.
//

#if !defined(GRAPHITE_CLUT_HPP)
#define GRAPHITE_CLUT_HPP

#include <string>
#include <vector>
#include <tuple>
#include "libGraphite/quickdraw/internal/color.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/data/writer.hpp"

namespace graphite::qd {

    struct clut
    {
    public:
        enum flags : uint16_t { pixmap = 0x0000, device = 0x8000 };

    private:
        int64_t m_id { 0 };
        std::string m_name;

        uint32_t m_seed { 0 };
        enum flags m_flags { pixmap };
        uint16_t m_size { 0 };
        std::vector<std::tuple<uint16_t, qd::color>> m_entries;

        auto parse(data::reader& reader) -> void;

    public:
        clut() = default;
        explicit clut(std::shared_ptr<graphite::data::data> data, int64_t id = 0, std::string name = "");
        explicit clut(data::reader& reader);

        static auto load_resource(int64_t id) -> std::shared_ptr<clut>;

        [[nodiscard]] auto size() const -> int;

        [[nodiscard]] auto at(int index) const -> qd::color;
        [[nodiscard]] auto get(int value) const -> qd::color;
        auto set(const qd::color& color) -> uint16_t;

        auto write(graphite::data::writer& writer) -> void;
    };

}

#endif //GRAPHITE_CLUT_HPP
