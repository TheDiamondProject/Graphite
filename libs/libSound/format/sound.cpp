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

#include <cstdint>
#include <stdexcept>
#include <libSound/format/sound.hpp>
#include <libSound/codec/descriptor.hpp>
#include <libSound/codec/ima4/ima4.hpp>

// MARK: - Constants / Enumerations

namespace sound::format
{
    enum bitrate : std::uint32_t
    {
        rate_48khz = 0xbb800000,
        rate_44khz = 0xac440000,
        rate_32khz = 0x7d000000,
        rate_22050hz = 0x56220000,
        rate_22khz = 0x56ee8ba3,
        rate_16khz = 0x3e800000,
        rate_11khz = 0x2b7745d1,
        rate_11025hz = 0x2b110000,
        rate_8khz = 0x1f400000,
    };

    enum synthesizer : std::uint16_t
    {
        sampled_sound = 5,
    };

    enum midi : std::uint8_t
    {
        middle_c = 60,
    };

    enum flags : std::uint16_t
    {
        data_offset = 0x8000,
    };

    enum compression : std::int16_t // TODO: Is the name of this enum correct?
    {
        not_compressed = 0,
        fixed_compression = -1,
        variable_compression = -2,
        two_to_one = 1,
        eight_to_three = 2,
        three_to_one = 3,
        six_to_one = 4,
        six_to_one_packet_size = 8,
        three_to_one_packet_size = 16,
    };

    enum sound_format : std::int16_t
    {
        general = 1,
        hypercard = 2,
    };

    enum header : std::uint8_t
    {
        standard = 0x00,
        extended = 0xFF,
        compressed = 0xFE,
    };

    enum command : std::uint16_t
    {
        null_cmd = 0,
        quiet_cmd = 3,
        flush_cmd = 4,
        re_init_cmd = 5,
        wait_cmd = 10,
        pause_cmd = 11,
        resume_cmd = 12,
        call_back_cmd = 13,
        sync_cmd = 14,
        available_cmd = 24,
        version_cmd = 25,
        volume_cmd = 46,   // sound manager 3.0 or later only
        get_volume_cmd = 47,   // sound manager 3.0 or later only
        clock_component_cmd = 50,   // sound manager 3.2.1 or later only
        get_clock_component_cmd = 51,   // sound manager 3.2.1 or later only
        scheduled_sound_cmd = 52,   // sound manager 3.3 or later only
        link_sound_components_cmd = 53,   // sound manager 3.3 or later only
        sound_cmd = 80,
        buffer_cmd = 81,
        rate_multiplier_cmd = 86,
        get_rate_multiplier_cmd = 87,
    };

    enum init_flags : std::int32_t
    {
        init_chan_left = 0x0002, // left stereo channel
        init_chan_right = 0x0003, // right stereo channel
        init_no_interp = 0x0004, // no linear interpolation
        init_no_drop = 0x0008, // no drop-sample conversion
        init_mono = 0x0080, // monophonic channel
        init_stereo = 0x00C0, // stereo channel
        init_m_a_c_e3 = 0x0300, // MACE 3:1
        init_m_a_c_e6 = 0x0400, // MACE 6:1
    };

    enum format_type : std::uint32_t
    {
        sound_format_not_compressed = 0x4E4F4E45, // 'NONE' sound is not compressed
        sound_format_8_bit_offset = 0x72617720, // 'raw ' 8-bit offset binary
        sound_format_16_bit_be = 0x74776F73, // 'twos' 16-bit big endian
        sound_format_16_bit_le = 0x736F7774, // 'sowt' 16-bit little endian
        sound_format_ima4 = 'ima4',
    };
}

// MARK: - Internal Helper Types

namespace sound::format
{
    struct sound_command
    {
        enum command command;
        std::int16_t param1;
        std::int16_t param2;
    };

    struct modifier_reference
    {
        std::uint16_t number;
        std::int32_t init;
    };

    struct sound_list_resource
    {
        std::int16_t format;
        std::int16_t modifier_count;
        struct modifier_reference modifier;
        std::int16_t command_count;
        struct sound_command command;
    };

    struct hypercard_sound_list_resource
    {
        std::int16_t format;
        std::int16_t ref_count;
        std::int16_t command_count;
        struct sound_command command;
    };

    struct sound_header
    {
        std::uint32_t sample_ptr;
        std::uint32_t length;
        std::uint32_t sample_rate_fixed;
        std::uint32_t loop_start;
        std::uint32_t loop_end;
        std::uint8_t encode;
        std::uint8_t base_frequency;
    };

    struct compressed_sound_header
    {
        std::uint32_t frame_count;
        std::int16_t aiff_sample_rate_exp;
        std::uint64_t aiff_sample_rate_man;
        std::uint32_t marker_chunk;
        std::uint32_t format;
        std::uint32_t future_use;
        std::uint32_t state_vars;
        std::uint32_t left_over_samples;
        std::uint16_t compression_id;
        std::uint16_t packet_size;
        std::uint16_t synth_id;
        std::uint16_t sample_size;
    };

    struct extended_sound_header
    {
        std::uint32_t frame_count;
        std::int16_t aiff_sample_rate_exp;
        std::uint64_t aiff_sample_rate_man;
        std::uint32_t marker_chunk;
        std::uint32_t instrument_chunks;
        std::uint32_t aes_recording;
        std::uint16_t sample_size;
    };
}

// MARK: - Construction

resource_core::format::sound::sound(const data::block &data, resource_core::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    data::reader reader(&data);
    decode(reader);
}

resource_core::format::sound::sound(data::reader &reader, resource_core::identifier id, const std::string &name)
    : m_id(id), m_name(name)
{
    decode(reader);
}

resource_core::format::sound::sound(std::uint32_t sample_rate, std::uint8_t sample_bits, const std::vector<std::vector<std::uint32_t>> &sample_data)
{
    m_descriptor.sample_rate = sample_rate;
    m_descriptor.bit_width = sample_bits;

    data::writer writer(data::byte_order::lsb);
    if (sample_bits == 8) {
        for (auto& channel : sample_data) {
            for (auto& frame : channel) {
                writer.write_byte(static_cast<std::uint8_t>(frame));
            }
        }
    }
    else {
        for (auto& channel : sample_data) {
            for (auto& frame : channel) {
                writer.write_short(static_cast<std::uint16_t>(frame));
            }
        }
    }

    m_samples = std::move(*const_cast<data::block *>(writer.data()));
}

sound::format::sound::sound(std::uint32_t sample_rate, std::uint8_t sample_bits, const data::block& sample_data)
{
    m_descriptor.sample_rate = sample_rate;
    m_descriptor.bit_width = sample_bits;
    m_samples = sample_data;
}

// MARK: - Decoding

auto sound::format::sound::decode(data::reader &reader) -> void
{
    auto sound_format = reader.read_signed_short();

    switch (sound_format) {
        case sound_format::general: {
            sound_list_resource list;
            list.format = sound_format;
            list.modifier_count = reader.read_signed_short();
            list.modifier.number = reader.read_signed_short();
            list.modifier.init = reader.read_signed_long();
            list.command_count = reader.read_signed_short();
            list.command.command = static_cast<enum command>(reader.read_short());
            list.command.param1 = reader.read_signed_short();
            list.command.param2 = reader.read_signed_long();

            if (list.modifier_count != 1
             || list.modifier.number != synthesizer::sampled_sound
             || list.command_count != 1
             || list.command.command != (static_cast<std::uint16_t>(flags::data_offset) + static_cast<std::uint16_t>(buffer_cmd)))
            {
                throw std::runtime_error("Only sampled sound synthesizers are supported.");
            }

            break;
        }
        case sound_format::hypercard: {
            hypercard_sound_list_resource list;
            list.format = sound_format;
            list.ref_count = reader.read_signed_short();
            list.command_count = reader.read_signed_short();
            list.command.command = static_cast<enum command>(reader.read_short());
            list.command.param1 = reader.read_signed_short();
            list.command.param2 = reader.read_signed_long();

            if (list.command_count != 1
             || list.command.command != (static_cast<std::uint16_t>(flags::data_offset) + static_cast<std::uint16_t>(buffer_cmd)))
            {
                throw std::runtime_error("Unsupported configuration.");
            }

            break;
        }
        default: {
            throw std::runtime_error("Unrecognised sound format '" + std::to_string(sound_format) + "' in resource: "
                                     + std::to_string(m_id) + ", " + m_name);
        }
    }

    sound_header header;
    header.sample_ptr = reader.read_long();
    header.length = reader.read_long();
    header.sample_rate_fixed = reader.read_long();
    header.loop_start = reader.read_long();
    header.loop_end = reader.read_long();
    header.encode = reader.read_byte();
    header.base_frequency = reader.read_byte();

    switch (header.encode) {
        case header::standard: {
            m_descriptor.format_id = format_type::sound_format_8_bit_offset;
            m_descriptor.channels = 1;
            m_descriptor.packet_count = header.length;
            break;
        }
        case header::extended: {
            extended_sound_header ext;
            ext.frame_count = reader.read_long();
            ext.aiff_sample_rate_exp = reader.read_signed_short();
            ext.aiff_sample_rate_man = reader.read_quad();
            ext.marker_chunk = reader.read_long();
            ext.instrument_chunks = reader.read_long();
            ext.aes_recording = reader.read_long();
            ext.sample_size = reader.read_short();
            reader.move(14);

            m_descriptor.format_id = ext.sample_size == 8 ? format_type::sound_format_8_bit_offset
                                                        : format_type::sound_format_16_bit_be;
            m_descriptor.channels = header.length;
            m_descriptor.packet_count = ext.frame_count;
            break;
        }
        case header::compressed: {
            compressed_sound_header cmp;
            cmp.frame_count = reader.read_long();
            cmp.aiff_sample_rate_exp = reader.read_signed_short();
            cmp.aiff_sample_rate_man = reader.read_quad();
            cmp.marker_chunk = reader.read_long();
            cmp.format = reader.read_long();
            cmp.future_use = reader.read_long();
            cmp.state_vars = reader.read_long();
            cmp.left_over_samples = reader.read_long();
            cmp.compression_id = reader.read_short();
            cmp.packet_size = reader.read_short();
            cmp.synth_id = reader.read_short();
            cmp.sample_size = reader.read_short();

            if (cmp.compression_id == compression::three_to_one) {
                m_descriptor.format_id = 'MAC3';
            }
            else if (cmp.compression_id == compression::six_to_one) {
                m_descriptor.format_id = 'MAC6';
            }
            else {
                m_descriptor.format_id = cmp.format;
            }

            m_descriptor.channels = header.length;
            m_descriptor.packet_count = cmp.frame_count;
            break;
        }
        default: {
            throw std::runtime_error("Invalid header in sound resource.");
        }
    }

    m_descriptor.sample_rate = static_cast<std::uint32_t>(static_cast<double>(header.sample_rate_fixed) * 1.0 / static_cast<double>(1 << 16));

    if (m_descriptor.format_id == format_type::sound_format_8_bit_offset || m_descriptor.format_id == format_type::sound_format_16_bit_be) {
        if (m_descriptor.format_id == format_type::sound_format_8_bit_offset) {
            m_descriptor.bit_width = 8;
        }
        else {
            m_descriptor.bit_width = 16;
            m_descriptor.format_flags = 0x6; // kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian
        }

        m_descriptor.format_id = 'lpcm';
        m_descriptor.bytes_per_frame = (m_descriptor.bit_width >> 3) * m_descriptor.channels;
        m_descriptor.frames_per_packet = 1;
        m_descriptor.bytes_per_packet = m_descriptor.bytes_per_frame * m_descriptor.frames_per_packet;

        auto sample_count = reader.size() - reader.position();
        m_samples = reader.read_data(sample_count);
    }
    else if (m_descriptor.format_id == format_type::sound_format_ima4) {
        // TODO: Do not hard code this, but work out the conversions...
        m_descriptor.format_flags = 0;
        m_descriptor.bytes_per_packet = 34;
        m_descriptor.frames_per_packet = 64;
        m_descriptor.bytes_per_frame = 0;
        m_descriptor.channels = 1;
        m_descriptor.bit_width = 0;

        codec::ima4::sound ima4(m_descriptor, reader);
        m_samples = ima4.samples();
        m_descriptor = ima4.descriptor();
    }
    else {
         throw std::runtime_error("Unrecognised sound format.");
    }
}

// MARK: - Accessors

auto sound::format::sound::samples() const -> const data::block&
{
    return m_samples;
}

auto sound::format::sound::codec_descriptor() const -> const codec::descriptor&
{
    return m_descriptor;
}

auto sound::format::sound::sample_rate() const -> std::uint32_t
{
    return m_descriptor.sample_rate;
}

auto sound::format::sound::channels() const -> std::uint16_t
{
    return m_descriptor.channels;
}

auto sound::format::sound::bit_width() const -> std::uint8_t
{
    return m_descriptor.bit_width;
}

auto sound::format::sound::bytes_per_frame() const -> std::uint32_t
{
    return m_descriptor.bytes_per_frame;
}

auto sound::format::sound::frames_per_packet() const -> std::uint32_t
{
    return m_descriptor.frames_per_packet;
}

auto sound::format::sound::bytes_per_packet() const -> std::uint32_t
{
    return m_descriptor.bytes_per_packet;
}

auto sound::format::sound::packet_count() const -> std::uint32_t
{
    return m_descriptor.packet_count;
}

auto sound::format::sound::format_id() const -> std::uint32_t
{
    return m_descriptor.format_id;
}

auto sound::format::sound::format_flags() const -> std::uint32_t
{
    return m_descriptor.format_flags;
}

// MARK: - Decoding

auto sound::format::sound::data() -> data::block
{
    data::writer writer;
    encode(writer);
    return std::move(*const_cast<data::block*>(writer.data()));
}

auto sound::format::sound::encode(data::writer &writer) -> void
{
    // TODO: Implement this...
}