//
// Created by Tom Hancocks on 24/03/2020.
//

#if !defined(GRAPHITE_STRING_HPP)
#define GRAPHITE_STRING_HPP

#include <string>
#include "libGraphite/data/data.hpp"

namespace graphite { namespace resources {

    struct string
    {
    private:
        int64_t m_id;
        std::string m_name;
        std::string m_str;
        data::data m_data;

    public:
        string(std::string str, std::shared_ptr<data::data> data, int64_t id = 0, const std::string name = "");

        static auto load_resource(int64_t id) -> std::shared_ptr<string>;

        auto value() const -> std::string;
        auto data() const -> data::data;

        auto set_string(const std::string& str) -> void;
        auto set_data(data::data data) -> void;
    };

}};


#endif //GRAPHITE_STRING_HPP
