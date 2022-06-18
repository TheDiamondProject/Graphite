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

#pragma once

#include <string>
#include "libGraphite/data/data.hpp"
#include "libGraphite/data/writer.hpp"
#include "libGraphite/data/reader.hpp"
#include "libGraphite/rsrc/resource.hpp"
#include "libGraphite/sound/codec/descriptor.hpp"

namespace graphite::sound_manager
{
    struct sound
    {
    public:
        static auto type_code() -> std::string { return "snd "; }

    public:
        explicit sound(const data::block& data, rsrc::resource::identifier id = 0, const std::string& name = "");

        [[nodiscard]] auto samples() const -> const data::block&;
        [[nodiscard]] auto codec_descriptor() const -> const codec::descriptor&;

        [[nodiscard]] auto sample_rate() const -> std::uint32_t;
        [[nodiscard]] auto channels() const -> std::uint16_t;
        [[nodiscard]] auto bit_width() const -> std::uint8_t;
        [[nodiscard]] auto bytes_per_frame() const -> std::uint32_t;
        [[nodiscard]] auto frames_per_packet() const -> std::uint32_t;
        [[nodiscard]] auto bytes_per_packet() const -> std::uint32_t;
        [[nodiscard]] auto packet_count() const -> std::uint32_t;
        [[nodiscard]] auto format_id() const -> std::uint32_t;
        [[nodiscard]] auto format_flags() const -> std::uint32_t;

    private:
        rsrc::resource::identifier m_id {};
        std::string m_name;
        codec::descriptor m_descriptor;
        data::block m_samples;

        auto decode(data::reader& reader) -> void;
    };
}

