//
// Created by Tom Hancocks on 20/02/2020.
//

#if !defined(GRAPHITE_PACKBITS_HPP)
#define GRAPHITE_PACKBITS_HPP

#include <vector>
#include <memory>

namespace graphite { namespace qd {

    struct packbits
    {
    public:
        static std::size_t decode(std::vector<uint8_t> &out_data, std::vector<uint8_t> pack_data, std::size_t value_size);
    };

}}

#endif //GRAPHITE_PACKBITS_HPP
