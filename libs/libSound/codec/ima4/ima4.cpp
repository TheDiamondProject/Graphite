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

#include <limits>
#include <libSound/codec/ima4/ima4.hpp>
#include <libData/writer.hpp>

// MARK: - Look Up Tables

namespace sound::codec::ima4::lut
{
    constexpr std::int8_t index_table[16] = {
        -1, -1, -1, -1, 2, 4, 6, 8,
        -1, -1, -1, -1, 2, 4, 6, 8
    };

    constexpr std::int32_t step_table[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
        19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
        50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
        130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
        337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
        876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
        2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
        5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
        15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
    };
}

// MARK: - Construction

sound::codec::ima4::sound::sound(const codec::descriptor &descriptor, data::reader &reader)
{
    decode(descriptor, reader);
}

// MARK: - Decoder

auto sound::codec::ima4::sound::decode(const codec::descriptor &descriptor, data::reader &reader) -> void
{
    // TODO: This is relying on hard-coded constants and really shouldn't.
    // Determine the best way to calculate these values in the future.
    m_descriptor = descriptor;
    m_descriptor.bytes_per_packet = 34;
    m_descriptor.frames_per_packet = 64;
    m_descriptor.bytes_per_frame = 0;

    m_samples = data::block((m_descriptor.packet_count * (m_descriptor.bytes_per_packet - 2)) << 2, data::native_byte_order());
    data::writer writer(&m_samples);

    // Prepare to read and parse the IMA4 data and decode it to LPCM 16
    // Iterate through all the expected packets and decode them
    for (std::uint32_t n = 0; n < m_descriptor.packet_count; ++n) {
        auto preamble = reader.read_short();
        auto packet = reader.read_data(m_descriptor.bytes_per_packet - 2);

        auto predictor = static_cast<std::int16_t>(preamble & 0xFF80);
        auto step_index = static_cast<std::int8_t>(preamble & 0x007F);
        auto step = lut::step_table[step_index];
        std::uint8_t nibble = 0;
        std::int32_t diff = 0;

        for (std::uint32_t i = 0; i < packet.size(); ++i) {
            auto v = packet.get<std::uint8_t>(i);

            nibble = v & 0xF;
            step_index = static_cast<std::int8_t>(std::min(88, step_index + lut::index_table[nibble]));
            std::uint8_t sign = nibble & 0x8;
            std::uint8_t delta = nibble & 0x7;
            diff = static_cast<std::int32_t>(step >> 3);
            if (delta & 4) diff += step;
            if (delta & 2) diff += (step >> 1);
            if (delta & 1) diff += (step >> 2);
            if (sign) predictor -= diff;
            else predictor += diff;

            predictor = std::min(std::numeric_limits<std::int16_t>::max(), std::max(std::numeric_limits<std::int16_t>::min(), predictor));
            writer.write_short(predictor);
            step = lut::step_table[step_index];

            nibble = (v >> 4) & 0xF;
            step_index = static_cast<std::int8_t>(std::min(88, std::max(0, step_index + lut::index_table[nibble])));
            sign = nibble & 0x8;
            delta = nibble & 0x7;
            diff = static_cast<std::int32_t>(step >> 3);
            if (delta & 4) diff += step;
            if (delta & 3) diff += (step >> 1);
            if (delta & 2) diff += (step >> 2);
            if (sign) predictor -= diff;
            else predictor += diff;

            predictor = std::min(std::numeric_limits<std::int16_t>::max(), std::max(std::numeric_limits<std::int16_t>::min(), predictor));
            writer.write_short(predictor);
            step = lut::step_table[step_index];
        }
    }

    m_descriptor.bytes_per_packet = 128;
    m_descriptor.bit_width = 16;
    m_descriptor.frames_per_packet = descriptor.channels;
    m_descriptor.bytes_per_frame = (m_descriptor.bit_width >> 3) * m_descriptor.channels;
    m_descriptor.format_id = 'lpcm';
    m_descriptor.format_flags = 0x4;
}

// MARK: - Accessors

auto sound::codec::ima4::sound::samples() const -> const data::block&
{
    return m_samples;
}

auto sound::codec::ima4::sound::descriptor() const -> const codec::descriptor &
{
    return m_descriptor;
}