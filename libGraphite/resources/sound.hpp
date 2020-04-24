//
// Created by Tom Hancocks on 24/03/2020.
//

#if !defined(GRAPHITE_SOUND_HPP)
#define GRAPHITE_SOUND_HPP

#include <memory>
#include <string>
#include <vector>
#include "libGraphite/data/data.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/data/writer.hpp"

namespace graphite { namespace resources {

    class sound
    {
    private:
        enum format : uint16_t { type1 = 0x0001, type2 = 0x0002 };

        enum command : uint16_t
        {
            null = 0,                   // Do nothing
            quiet = 3,                  // Stop a sound that is playing
            flush = 4,                  // Flush a sound channel
            reinit = 5,                 // Reinitialise a sound channel
            wait = 10,                  // Suspend processing in a channel
            pause = 11,                 // Pause processing in a channel
            resume = 12,                // Resume processing in a channel
            callback = 13,              // Execute a callback procedure
            sync = 14,                  // Synchronise channels
            available = 24,             // See if initialisation options are supported.
            version = 25,               // Determine version
            totalLoad = 26,             // Report total cpu load
            load = 27,                  // Report cpu load for a new channel
            freqDuration = 40,          // Play a note for a duration
            rest = 41,                  // Rest a channel for a duration
            freq = 42,                  // Change pitch of a sound
            amp = 43,                   // Change the amplitude of a sound
            timbre = 44,                // Change the timbre of a sound
            getAmp = 45,                // Get the amplitude of a sound
            volume = 46,                // Set the volume
            getVolume = 47,             // Get the volume
            waveTable = 60,             // Install a wave table as a voice
            play_sound = 80,            // Install a sampled sound as a voice
            buffer = 81,                // Play a sampled sound
            rate = 82,                  // Set the pitch of a sampled sound
            getRate = 83,               // Get the pitch of a sampled sound
        };

        enum data_format : uint16_t {
            squareWaveSynth = 1,        // Square-wave data
            waveTableSynth = 3,         // Wave-table data
            sampledSynth = 5,           // Sampled-sound data
        };

        enum channel_init_option : uint32_t {
            initChanLeft = 0x0002,      // Left stereo channel
            initChanRight = 0x0003,     // Right stereo channel
            waveInitChannel0 = 0x0004,  // Wave-table channel 0
            waveInitChannel1 = 0x0005,  // Wave-table channel 1
            waveInitChannel2 = 0x0006,  // Wave-table channel 2
            waveInitChannel3 = 0x0007,  // Wave-table channel 3
            initMono = 0x0080,          // Monophonic channel
            initStereo = 0x00C0,        // Stereo channel
            initMACE3 = 0x0300,         // 3:1 compression
            initMACE6 = 0x0400,         // 6:1 compression
            initNoInterp = 0x0004,      // No linear interpolation
            initNoDrop = 0x0008,        // No drop-sample conversion
        };

        enum sound_encoding : uint8_t {
            stdSH = 0x00,               // Standard sound header
            extSH = 0xFF,               // Extended sound header
            cmpSH = 0xFE,               // Compressed sound header
        };

        enum compression_id : int16_t {
            variableCompression = -2,   // Variable-ratio compression
            fixedCompression = -1,      // Fixed-ratio compression
            notCompressed = 0,          // Non-compressed samples
            threeToOne = 3,             // 3:1 compressed samples
            sixToOne = 6,               // 6:1 compressed samples
        };

        struct command_record
        {
        public:
            enum command cmd;
            uint16_t param1;
            uint32_t param2;
            bool data_offset_flag;

            command_record(const command cmd, const uint16_t param1, const uint32_t param2, const bool data_offset_flag);
        };

        struct sound_header_record
        {
        public:
            uint32_t data_pointer;      // If nil, samples follow header
            uint32_t length;            // Number of samples in array
            uint32_t sample_rate;       // Sample rate (Fixed)
            uint32_t loop_start;        // Loop start point sample byte number
            uint32_t loop_end;          // Loop end point sample byte number
            enum sound_encoding sample_encoding; // Sample encoding option
            uint8_t base_frequency;     // Base frequency of sample
        };

        struct extended_sound_header_record
        {
        public:
            // The length field in standard_header is interpreted as num_channels
            uint32_t num_frames;        // The number of frames in the sampled-sound data.
                                        // Each frame contains num_channels bytes for 8-bit sound data.
            uint8_t aiff_sample_rate[10]; // The sample rate at which the frames were sampled before compression,
                                          // as expressed in the 80-bit extended data type representation
            uint32_t marker_chunk;      // Pointer to synchronization information. Unused, set to NIL.
            uint32_t instrument_chunks; // Pointer to instrument information.
            uint32_t aes_recording;     // Pointer to information related to audio recording devices.
            uint16_t sample_size;       // The number of bits in each sample frame.
            uint16_t future_use_1;      // Reserved
            uint32_t future_use_2;      // Reserved
            uint32_t future_use_3;      // Reserved
            uint32_t future_use_4;      // Reserved
        };

        struct compressed_sound_header_record
        {
        public:
            // The length field in standard_header is interpreted as num_channels
            uint32_t num_frames;        // The number of frames in the sampled-sound data.
                                        // Each frame contains num_channels bytes for 8-bit sound data.
            uint8_t aiff_sample_rate[10]; // The sample rate at which the frames were sampled before compression,
                                          // as expressed in the 80-bit extended data type representation
            uint32_t marker_chunk;      // Pointer to synchronization information. Unused, set to NIL.
            uint32_t format;            // OSType, e.g. 'MAC3' for MACE3:1
            uint32_t future_use_2;      // Reserved
            uint32_t state_vars;        // Pointer to StateBlock
            uint32_t leftover_samples;  // Pointer to LeftOverBlock
            enum compression_id compression_id; // The compression algorithm used on the samples in the compressed sound header.
            uint16_t packet_size;       // The size, in bits, of the smallest element that a given expansion algorithm can work with.
            uint16_t snth_id;           // This field is unused. You should set it to 0.
            uint16_t sample_size;       // The size of the sample before it was compressed.
        };

        // Basic resource information
        int16_t m_id;
        std::string m_name;

        // Sound information
        uint16_t m_ref_count;
        uint32_t m_sample_rate_int;
        uint16_t m_sample_rate_frac;
        uint8_t m_sample_bits;
        std::vector<std::vector<uint32_t>> m_sample_data;

        auto parse(graphite::data::reader& snd_reader) -> void;
        auto encode(graphite::data::writer& snd_writer) -> void;

    public:
        sound(std::shared_ptr<graphite::data::data> data, int64_t id = 0, std::string name = "");
        sound(uint32_t sample_rate, uint8_t sample_bits, std::vector<std::vector<uint32_t>> sample_data);

        static auto load_resource(int64_t id) -> std::shared_ptr<sound>;

        auto sample_bits() -> uint8_t;
        auto sample_rate() -> uint32_t;
        auto samples() -> std::vector<std::vector<uint32_t>>;

        auto data() -> std::shared_ptr<graphite::data::data>;
    };

}};

#endif
