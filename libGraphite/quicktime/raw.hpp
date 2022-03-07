//
// Created by Tom Hancocks on 5/02/2022.
//

#if !defined(GRAPHITE_RAW_HPP)
#define GRAPHITE_RAW_HPP

#include <memory>
#include "libGraphite/data/reader.hpp"
#include "libGraphite/quicktime/imagedesc.hpp"
#include "libGraphite/quickdraw/internal/surface.hpp"

namespace graphite::qt {

    struct raw
    {
    public:
        static auto decode(const qt::imagedesc& imagedesc, data::reader& reader) -> qd::surface;
    };

}

#endif //GRAPHITE_RAW_HPP
