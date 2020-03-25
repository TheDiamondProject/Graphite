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

namespace graphite { namespace qd {

    struct clut
    {
    public:
        enum flags : uint16_t { pixmap = 0x0000, device = 0x8000 };

    private:
        int64_t m_id;
        std::string m_name;

        uint32_t m_seed;
        enum flags m_flags;
        uint16_t m_size;
        std::vector<std::tuple<uint16_t, qd::color>> m_entries;

        void parse(data::reader& reader);

    public:
        clut();
        clut(std::shared_ptr<graphite::data::data> data, int64_t id = 0, std::string name = "");
        clut(data::reader& reader);

        static std::shared_ptr<clut> load_resource(int64_t id);

        int size() const;

        qd::color at(int index) const;
        qd::color get(int value) const;

    };

}};


#endif //GRAPHITE_CLUT_HPP
