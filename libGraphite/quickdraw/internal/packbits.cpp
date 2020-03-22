//
// Created by Tom Hancocks on 20/02/2020.
//

#include "libGraphite/quickdraw/internal/packbits.hpp"

std::size_t graphite::qd::packbits::decode(std::vector<uint8_t> &out_data, std::vector<uint8_t> pack_data, std::size_t value_size)
{
    std::size_t pos = 0;

    while (pos < pack_data.size()) {
        auto count = pack_data[pos++];
        if (count >= 0 && count < 128) {
            uint16_t run = (1 + count) * value_size;
            out_data.insert(out_data.end(), std::make_move_iterator(pack_data.begin() + pos), std::make_move_iterator(pack_data.begin() + pos + run));
            pos += run;
        }
        else if (count >= 128) {
            uint8_t run = 256 - count + 1;
            for (uint8_t i = 0; i < run; ++i) {
                out_data.insert(out_data.end(), std::make_move_iterator(pack_data.begin() + pos), std::make_move_iterator(pack_data.begin() + pos + value_size));
            }
            pos += value_size;
        }
        else {
            // No-op
        }
    }

    return out_data.size();
}