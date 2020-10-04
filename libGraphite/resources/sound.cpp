//
// Created by Tom Hancocks on 24/03/2020.
//

#include "libGraphite/resources/sound.hpp"
#include "libGraphite/rsrc/manager.hpp"
#include "libGraphite/data/reader.hpp"
#import <algorithm>

// MARK: - IMA4 Decoding

// Refer to https://wiki.multimedia.cx/index.php?title=IMA_ADPCM,
// http://www.cs.columbia.edu/~hgs/audio/dvi/IMA_ADPCM.pdf

inline int32_t ima_step_index(int32_t index, int8_t nibble)
{
    static int32_t ima_index_table[16] = {
        -1, -1, -1, -1, 2, 4, 6, 8,
        -1, -1, -1, -1, 2, 4, 6, 8
    };

    return std::min(std::max(0, index + ima_index_table[nibble]), 88);
}

inline int32_t ima_predictor(int32_t predictor, int8_t nibble, int32_t index)
{
    static int32_t ima_step_table[89] = {
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

    int32_t diff = 0;
    auto stepsize = ima_step_table[index];

    if (nibble & 4) diff += stepsize;
    if (nibble & 2) diff += stepsize >> 1;
    if (nibble & 1) diff += stepsize >> 2;
    diff += stepsize >> 3;
    if (nibble & 8) diff = -diff;

    return std::min(std::max(-32768, predictor + diff), 32767);
}

// MARK: - Constructor

graphite::resources::sound::sound(std::shared_ptr<data::data> data, int64_t id, std::string name)
    : m_ref_count(0), m_name(std::move(name)), m_id(id)
{
    // Setup a reader for the snd data, and then parse it.
    data::reader snd_reader(std::move(data));
    parse(snd_reader);
}

graphite::resources::sound::sound(uint32_t sample_rate, uint8_t sample_bits, std::vector<std::vector<uint32_t>> sample_data)
    : m_ref_count(0), m_name("Sound"), m_id(0), m_sample_rate_int(sample_rate), m_sample_rate_frac(0), m_sample_bits(sample_bits), m_sample_data(std::move(sample_data))
{

}

auto graphite::resources::sound::load_resource(int64_t id) -> std::shared_ptr<graphite::resources::sound>
{
    if (auto snd_res = graphite::rsrc::manager::shared_manager().find("snd ", id).lock()) {
        return std::make_shared<resources::sound>(snd_res->data(), id, snd_res->name());
    }
    return nullptr;
}

// MARK: - Accessors

auto graphite::resources::sound::sample_bits() const -> uint8_t
{
    return m_sample_bits;
}

auto graphite::resources::sound::sample_rate() const -> uint32_t
{
    return m_sample_rate_int;
}

auto graphite::resources::sound::samples() -> std::vector<std::vector<uint32_t>>
{
    return m_sample_data;
}

auto graphite::resources::sound::data() -> std::shared_ptr<graphite::data::data>
{
    auto data = std::make_shared<graphite::data::data>();
    graphite::data::writer writer(data);
    encode(writer);
    return data;
}

// MARK: - Parsing/Reading

auto graphite::resources::sound::parse(graphite::data::reader& snd_reader) -> void
{
    // Save the position because our buffer commands reference data by offset from the record start
    auto reader_pos = snd_reader.position();

    auto snd_format = static_cast<format>(snd_reader.read_short());

    uint16_t channel_init_option = 0;
    if (snd_format == type1) {
        // We only support sampled sounds; validate the format 1 sound type now
        auto num_data_formats = snd_reader.read_short();
        if (num_data_formats != 1) {
            throw std::runtime_error("Encountered an incompatble snd format: " + std::to_string(num_data_formats) +
                                     " formats, 1 expected, " + m_name);
        }

        auto data_format_id = snd_reader.read_short();
        if (data_format_id != sampledSynth) {
            throw std::runtime_error("Encountered an incompatble snd format: format " + std::to_string(data_format_id) +
                                     ", 5 expected, " + m_name);
        }

        channel_init_option = snd_reader.read_long();
    }
    else if (snd_format == type2) {
        m_ref_count = snd_reader.read_short();
    }
    else {
        throw std::runtime_error("Encountered an incompatble snd format: " + std::to_string(snd_format) + ", " + m_name);
    }

    // Read command array
    auto num_commands = snd_reader.read_short();
    std::vector<command_record> commands;
    for (auto i = 0; i < num_commands; i++) {
        auto cmd = snd_reader.read_short();
        commands.emplace_back(static_cast<command>(cmd & 0x7FFF), snd_reader.read_short(), snd_reader.read_long(), cmd & 0x8000);
    }

    // We only support sounds with a single buffer command -- validate that now
    if (commands.size() != 1 || commands[0].cmd != buffer) {
        throw std::runtime_error("Encountered an incompatble snd format: " + std::to_string(commands.size()) +
                                " commands, first command " + std::to_string(static_cast<unsigned int>(commands[0].cmd)) +
                                ", " + m_name);
    }

    // Move the reader to the buffer command's data offset
    snd_reader.set_position(reader_pos + commands[0].param2);

    sound_header_record std_header {};

    std_header.data_pointer = snd_reader.read_long();
    std_header.length = snd_reader.read_long();
    std_header.sample_rate = snd_reader.read_long();
    std_header.loop_start = snd_reader.read_long();
    std_header.loop_end = snd_reader.read_long();
    std_header.sample_encoding = static_cast<sound_encoding>(snd_reader.read_byte());
    std_header.base_frequency = snd_reader.read_byte();

    m_sample_rate_int = std_header.sample_rate >> 16;
    m_sample_rate_frac = std_header.sample_rate & 0xFFFF;

    if (std_header.sample_encoding == extSH) {
        extended_sound_header_record ext_header {};
        ext_header.num_frames = snd_reader.read_long();
        // skip aiff_sample_rate, marker_chunk, instrument_chunks, aes_recording
        snd_reader.move(22);
        ext_header.sample_size = snd_reader.read_short();
        // skip future_use values
        snd_reader.move(14);

        m_sample_bits = ext_header.sample_size;
        // Resize the data vector to fit the channel/frame count
        m_sample_data.resize(std_header.length, std::vector<uint32_t>(ext_header.num_frames));

        // Raw sound data follows, channels interleaved
        for (auto f = 0; f < ext_header.num_frames; f++) {
            for (auto c = 0; c < std_header.length; c++) {
                m_sample_data[c][f] = ext_header.sample_size == 8 ? snd_reader.read_byte() : snd_reader.read_short();
            }
        }
    }
    else if (std_header.sample_encoding == cmpSH) {
        compressed_sound_header_record cmp_header {};
        cmp_header.num_frames = snd_reader.read_long();
        // skip aiff_sample_rate, marker_chunk
        snd_reader.move(14);
        cmp_header.format = snd_reader.read_long();
        // skip future_use_2, state_vars, leftover_samples
        snd_reader.move(12);
        cmp_header.compression_id = static_cast<compression_id>(snd_reader.read_signed_short());
        cmp_header.packet_size = snd_reader.read_short();
        // skip snth_id
        snd_reader.move(2);
        cmp_header.sample_size = snd_reader.read_short();

        // We only support fixed ima4 compression
        if (cmp_header.compression_id != fixedCompression || cmp_header.format != 0x696D6134) {
            throw std::runtime_error("Encountered an incompatble snd format: " + std::to_string(cmp_header.compression_id) +
                                     " compression ID, format " + std::to_string(cmp_header.format) +
                                     ", expecting fixed ima4 compression, " + m_name);
        }

        m_sample_bits = 16;
         // Resize the data vector to fit the channel count; sample count is frame count * 64
        m_sample_data.resize(std_header.length, std::vector<uint32_t>(cmp_header.num_frames * 64));

        // Iterate over frames and expand into samples
        for (auto f = 0; f < cmp_header.num_frames; f++) {
            for (auto c = 0; c < std_header.length; c++) {
                // Apple IMA4 parameters described in TN1081:
                // https://www.fenestrated.net/mirrors/Apple%20Technotes%20(As%20of%202002)/tn/tn1081.html
                auto preamble = snd_reader.read_short();
                int32_t predictor = static_cast<int16_t>(preamble & 0xFF80);
                int32_t step_index = preamble & 0x007F;

                for (auto i = 0; i < 32; i++) {
                    uint8_t byte = snd_reader.read_byte();
                    uint8_t lower_nibble = byte & 0x0F;
                    uint8_t upper_nibble = byte >> 4;

                    predictor = ima_predictor(predictor, lower_nibble, step_index);
                    step_index = ima_step_index(step_index, lower_nibble);
                    m_sample_data[c][f * 64 + i * 2] = 32768 + predictor;

                    predictor = ima_predictor(predictor, upper_nibble, step_index);
                    step_index = ima_step_index(step_index, upper_nibble);
                    m_sample_data[c][f * 64 + i * 2 + 1] = 32768 + predictor;
                }
            }
        }
    }
    else {
        m_sample_bits = 8;
        // Resize the data vector to fit the channel/frame count
        m_sample_data.resize(1, std::vector<uint32_t>(std_header.length));

        // Raw 8-bit mono sound data follows
        for (auto f = 0; f < std_header.length; f++) {
            m_sample_data[0][f] = snd_reader.read_byte();
        }
    }
}

// MARK: - Encoder / Writing

auto graphite::resources::sound::encode(graphite::data::writer& snd_writer) -> void
{
    // Write the snd header for a format 1 sound
    snd_writer.write_short(static_cast<uint16_t>(type1));
    snd_writer.write_short(1); // num_data_formats
    snd_writer.write_short(static_cast<uint16_t>(sampledSynth)); // first_data_format_id
    snd_writer.write_long(static_cast<uint16_t>(initMono)); // channel_init_option
    snd_writer.write_short(1); // num_commands
    snd_writer.write_short(0x8000 | static_cast<uint16_t>(buffer)); // command: cmd, high bit set to indicate offset
    snd_writer.write_short(0); // command: param1
    snd_writer.write_long(20); // command: param2, offset from start of record to sound data

    auto num_samples = m_sample_data.size() ? m_sample_data[0].size() : 0;

    // Write the standard sampled sound header fields
    snd_writer.write_long(0); // data pointer
    snd_writer.write_long(num_samples); // number of samples
    snd_writer.write_long(m_sample_rate_int << 16 | m_sample_rate_frac); // sample rate (fixed-point)
    snd_writer.write_long(0); // loop start
    snd_writer.write_long(0); // loop end
    snd_writer.write_byte(static_cast<uint8_t>(stdSH)); // encoding option
    snd_writer.write_byte(0); // base frequency

    // Truncate and write the sound data as 8-bit mono
    for (auto f = 0; f < num_samples; f++) {
        snd_writer.write_byte(m_sample_data[0][f] >> (m_sample_bits - 8));
    }
}

// MARK: - Sound record construction

graphite::resources::sound::command_record::command_record(command cmd, uint16_t param1, uint32_t param2, bool data_offset_flag)
    : cmd(cmd), param1(param1), param2(param2), data_offset_flag(data_offset_flag)
{

}
