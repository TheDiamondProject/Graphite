//
// Created by Tom Hancocks on 24/03/2020.
//

#pragma once

#include <string>
#include "libGraphite/data/reader.hpp"
#include "libGraphite/rsrc/resource.hpp"

namespace graphite::toolbox
{

    struct string
    {
    public:
        static auto type_code() -> std::string { return "STR "; }

    public:
        string() = default;
        explicit string(const data::block &data, rsrc::resource::identifier id = 0, const std::string& name = "");

        [[nodiscard]] auto value() const -> const std::string&;
        [[nodiscard]] auto data() const -> const data::block&;

    private:
        rsrc::resource::identifier m_id {};
        std::string m_name;
        std::string m_str;
        data::block m_data;

        auto decode(data::reader& reader) -> void;
    };

}
