// Copyright (c) 2022 Tom Hancocks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdexcept>
#include "libGraphite/compression/packbits.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/data/writer.hpp"

// MARK: - Decompression

auto graphite::compression::packbits::decompress(const data::block &compressed, std::size_t value_size) -> data::block
{
    data::block decompressed_data;
    data::reader reader(&compressed);
    data::writer writer(&decompressed_data);

    while (!reader.eof()) {
        auto count = reader.read_byte();
        if (count >= 0 && count < 128) {
            std::uint16_t run = (1 + count) * value_size;
            if ((reader.position() + run) > reader.size()) {
                throw std::runtime_error("Unable to decode packbits");
            }
            auto data = std::move(reader.read_data(run));
            writer.write_data(&data);
        }
        else if (count >= 128) {
            std::uint8_t run = 256 - count + 1;
            for (std::uint8_t i = 0; i < run; ++i) {
                for (std::uint8_t j = 0; j < value_size; ++j) {
                    writer.write_byte(reader.read_byte(j, data::reader::mode::peek));
                }
            }
            reader.move(value_size);
        }
        else {
            // No Operation
        }
    }

    return std::move(decompressed_data);
}

// MARK: - Compression

auto graphite::compression::packbits::compress(const data::block &uncompressed) -> data::block
{
    data::block result;
    data::block buffer(128);
    data::writer writer(&result);

    auto offset = 0;
    const auto max = uncompressed.size() - 1;
    const auto max_minus_1 = max - 1;

    while (offset <= max) {
        // Compressed run
        auto run = 1;
        auto replicate = uncompressed.get<std::uint8_t>(offset);
        while (run < 127 && offset < max && uncompressed.get<std::uint8_t>(offset) == uncompressed.get<std::uint8_t>(offset + 1)) {
            ++offset;
            ++run;
        }

        if (run > 1) {
            ++offset;
            writer.write_byte(static_cast<std::uint8_t>(-(run - 1)));
            writer.write_byte(replicate);
        }

        // Literal run
        run = 0;
        while (run < 128 && ((offset < max && uncompressed.get<std::uint8_t>(offset) != uncompressed.get<std::uint8_t>(offset + 1))
                             || (offset < max_minus_1 && uncompressed.get<std::uint8_t>(offset) != uncompressed.get<std::uint8_t>(offset + 2))))
        {
            buffer.set(uncompressed.get<std::uint8_t>(offset++), 1, run++);
        }

        if (offset == max && run > 0 && run < 128) {
            buffer.set(uncompressed.get<std::uint8_t>(offset++), 1, run++);
        }

        if (run > 0) {
            auto sliced = std::move(buffer.slice(0, run));
            writer.write_byte(run - 1);
            writer.write_data(&sliced);
            buffer.clear();
        }

        if (offset == max && (run <= 0 || run >= 128)) {
            writer.write_byte(0);
            writer.write_byte(uncompressed.get<std::uint8_t>(offset++));
        }
    }

    return std::move(result);
}