//
// Created by Tom Hancocks on 20/02/2020.
//

#if !defined(GRAPHITE_PACKBITS_HPP)
#define GRAPHITE_PACKBITS_HPP

#include <vector>
#include <memory>
#include <stdexcept>

namespace graphite::qd {

    struct packbits
    {
    public:
        static auto decode(std::vector<uint8_t> &out_data, const std::vector<uint8_t>& pack_data, std::size_t value_size) -> std::size_t;
        static auto encode(const std::vector<uint8_t>& scanline_bytes) -> std::vector<uint8_t>;
    };

}

#endif //GRAPHITE_PACKBITS_HPP
