//
// Created by Tom Hancocks on 20/02/2020.
//

#include <stdexcept>
#include "libGraphite/quickdraw/internal/packbits.hpp"
#include <stdexcept>

auto graphite::qd::packbits::decode(std::vector<uint8_t> &out_data, const std::vector<uint8_t>& pack_data, std::size_t value_size) -> std::size_t
{
    std::size_t pos = 0;

    while (pos < pack_data.size()) {
        auto count = pack_data[pos++];
        if (count >= 0 && count < 128) {
            uint16_t run = (1 + count) * value_size;
            if ((pos + run) > pack_data.size()) {
                throw std::runtime_error("Unable to decode packbits.");
            }
            out_data.insert(out_data.end(), std::make_move_iterator(pack_data.begin() + pos), std::make_move_iterator(pack_data.begin() + pos + run));
            pos += run;
        }
        else if (count > 128) {
            uint8_t run = 256 - count + 1;
            for (uint8_t i = 0; i < run; ++i) {
                for (uint8_t j = 0; j < value_size; ++j) {
                    out_data.push_back(pack_data[pos + j]);
                }
            }
            pos += value_size;
        }
        else {
            // No-op
        }
    }

    return out_data.size();
}

auto graphite::qd::packbits::encode(const std::vector<uint8_t>& scanline_bytes) -> std::vector<uint8_t>
{
    std::vector<uint8_t> result;
    std::vector<uint8_t> buffer(128);

    int offset = 0;
    const unsigned long max = scanline_bytes.size() - 1;
    const unsigned long max_minus_1 = max - 1;

    while (offset <= max) {
        // Compressed run
        int run = 1;
        uint8_t replicate = scanline_bytes[offset];
        while (run < 127 && offset < max && scanline_bytes[offset] == scanline_bytes[offset + 1]) {
            offset++;
            run++;
        }

        if (run > 1) {
            offset++;
            result.emplace_back(static_cast<uint8_t>(-(run - 1)));
            result.emplace_back(replicate);
        }

        // Literal run
        run = 0;
        while ((run < 128 && ((offset < max && scanline_bytes[offset] != scanline_bytes[offset + 1])
                              || (offset < max_minus_1 && scanline_bytes[offset] != scanline_bytes[offset + 2])))) {
            buffer[run++] = scanline_bytes[offset++];
        }

        if (offset == max && run > 0 && run < 128) {
            buffer[run++] = scanline_bytes[offset++];
        }

        if (run > 0) {
            result.emplace_back(run - 1);
            result.insert(result.end(), buffer.begin(), buffer.begin() + run);
            buffer = std::vector<uint8_t>(128);
        }

        if (offset == max && (run <= 0 || run >= 128)) {
            result.emplace_back(0);
            result.emplace_back(scanline_bytes[offset++]);
        }
    }

    return result;
}
