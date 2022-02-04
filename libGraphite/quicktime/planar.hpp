//
// Created by Tom Hancocks on 5/02/2022.
//

#if !defined(GRAPHITE_PHOTOSHOP_HPP)
#define GRAPHITE_PHOTOSHOP_HPP

#include <memory>
#include "libGraphite/data/reader.hpp"
#include "libGraphite/quicktime/imagedesc.hpp"
#include "libGraphite/quickdraw/internal/surface.hpp"

namespace graphite::qt {

    struct planar
    {
    public:
        static auto decode(const qt::imagedesc& imagedesc, data::reader& reader) -> qd::surface;
    };

}

#endif //GRAPHITE_PHOTOSHOP_HPP
