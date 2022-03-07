//
// Created by Tom Hancocks on 26/03/2021.
//

#if !defined(GRAPHITE_ANIMATION_HPP)
#define GRAPHITE_ANIMATION_HPP

#include <memory>
#include "libGraphite/data/reader.hpp"
#include "libGraphite/quicktime/imagedesc.hpp"
#include "libGraphite/quickdraw/internal/surface.hpp"

namespace graphite::qt {

    struct animation
    {
    public:
        static auto decode(const qt::imagedesc& imagedesc, data::reader& reader) -> qd::surface;
    };

}

#endif //GRAPHITE_ANIMATION_HPP
