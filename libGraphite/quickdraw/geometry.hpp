//
// Created by Tom Hancocks on 19/03/2020.
//

#if !defined(GRAPHITE_GEOMETRY_HPP)
#define GRAPHITE_GEOMETRY_HPP

#include "libGraphite/data/reader.hpp"
#include "libGraphite/data/writer.hpp"

namespace graphite { namespace qd {

    // MARK: - Point

    struct point
    {
    public:
        enum coding_type { qd, pict };

    private:
        int16_t m_x { 0 };
        int16_t m_y { 0 };

    public:
        point();
        point(const int16_t x, const int16_t y);
        point(const qd::point& p);

        static auto zero() -> qd::point;

        auto x() const -> int16_t;
        auto y() const -> int16_t;

        auto set_x(const int16_t x) -> void;
        auto set_y(const int16_t y) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> qd::point;
        auto write(graphite::data::writer& writer, coding_type type = qd) -> void;

    };

    // MARK: - Fixed Point

    struct fixed_point
    {
    public:
        enum coding_type { qd };

    private:
        double m_x { 0 };
        double m_y { 0 };

    public:
        fixed_point();
        fixed_point(const double x, const double y);
        fixed_point(const fixed_point& p);

        static auto zero() -> fixed_point;

        auto x() const -> double;
        auto y() const -> double;

        auto set_x(const double x) -> void;
        auto set_y(const double y) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> fixed_point;
        auto write(graphite::data::writer& writer, enum coding_type type = qd) -> void;

    };

    // MARK: - Size

    struct size
    {
    public:
        enum coding_type { qd, pict };

    private:
        int16_t m_width { 0 };
        int16_t m_height { 0 };

    public:
        size();
        size(const int16_t width, const int16_t height);
        size(const size& s);

        static auto zero() -> size;

        auto width() const -> int16_t;
        auto height() const -> int16_t;

        auto set_width(const int16_t width) -> void;
        auto set_height(const int16_t height) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> size;
        auto write(graphite::data::writer& writer, enum coding_type type = qd) -> void;
    };

    // MARK: - Fixed Size

    struct fixed_size
    {
    public:
        enum coding_type { qd };

    private:
        double m_width { 0 };
        double m_height { 0 };

    public:
        fixed_size();
        fixed_size(const double width, const double height);
        fixed_size(const fixed_size& s);

        static auto zero() -> fixed_size;

        auto width() const -> double;
        auto height() const -> double;

        auto set_width(const double width) -> void;
        auto set_height(const double height) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> fixed_size;
        auto write(graphite::data::writer& writer, enum coding_type type = qd) -> void;
    };

    // MARK: - Rect

    struct rect
    {
    public:
        enum coding_type { qd, pict };

    private:
        qd::point m_origin { point::zero() };
        qd::size m_size { size::zero() };

    public:
        rect();
        rect(const qd::point& origin, const qd::size& size);
        rect(const int16_t x, const int16_t y, const int16_t width, const int16_t height);
        rect(const qd::rect& r);

        static auto zero() -> qd::rect;

        auto x() const -> int16_t;
        auto y() const -> int16_t;
        auto width() const -> int16_t;
        auto height() const -> int16_t;
        auto origin() const -> qd::point;
        auto size() const -> qd::size;

        auto set_x(const int16_t x) -> void;
        auto set_y(const int16_t y) -> void;
        auto set_width(const int16_t width) -> void;
        auto set_height(const int16_t height) -> void;
        auto set_origin(const qd::point& origin) -> void;
        auto set_size(const struct qd::size& size) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> qd::rect;
        auto write(graphite::data::writer& writer, enum coding_type type = qd) -> void;
    };

    // MARK: - Fixed Rect

    struct fixed_rect
    {
    public:
        enum coding_type { qd };

    private:
        qd::fixed_point m_origin { fixed_point::zero() };
        qd::fixed_size m_size { fixed_size::zero() };

    public:
        fixed_rect();
        fixed_rect(const qd::fixed_point& origin, const qd::fixed_size& size);
        fixed_rect(const double x, const double y, const double width, const double height);
        fixed_rect(const qd::fixed_rect& r);

        static auto zero() -> qd::fixed_rect;

        auto x() const -> double;
        auto y() const -> double;
        auto width() const -> double;
        auto height() const -> double;
        auto origin() const -> qd::fixed_point;
        auto size() const -> qd::fixed_size;

        auto set_x(const double x) -> void;
        auto set_y(const double y) -> void;
        auto set_width(const double width) -> void;
        auto set_height(const double height) -> void;
        auto set_origin(const qd::fixed_point& origin) -> void;
        auto set_size(const qd::fixed_size& size) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> qd::fixed_rect;
        auto write(graphite::data::writer& writer, enum coding_type type = qd) -> void;
    };


}};

#endif //GRAPHITE_GEOMETRY_HPP
