//
// Created by Tom Hancocks on 19/03/2020.
//

#if !defined(GRAPHITE_GEOMETRY_HPP)
#define GRAPHITE_GEOMETRY_HPP

#include "libGraphite/data/reader.hpp"
#include "libGraphite/data/writer.hpp"

namespace graphite::qd {

    // MARK: - Point

    struct point
    {
    public:
        enum coding_type { qd, pict };

    private:
        int16_t m_x { 0 };
        int16_t m_y { 0 };

    public:
        point() = default;
        point(int16_t x, int16_t y);
        point(const qd::point& p) = default;

        static auto zero() -> qd::point;

        [[nodiscard]] auto x() const -> int16_t;
        [[nodiscard]] auto y() const -> int16_t;

        auto set_x(const int16_t& x) -> void;
        auto set_y(const int16_t& y) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> qd::point;
        auto write(graphite::data::writer& writer, coding_type type = qd) const -> void;

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
        fixed_point() = default;
        fixed_point(double x, double y);
        fixed_point(const fixed_point& p) = default;

        static auto zero() -> fixed_point;

        [[nodiscard]] auto x() const -> double;
        [[nodiscard]] auto y() const -> double;

        auto set_x(const double& x) -> void;
        auto set_y(const double& y) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> fixed_point;
        auto write(graphite::data::writer& writer, enum coding_type type = qd) const -> void;

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
        size() = default;
        size(int16_t width, int16_t height);
        size(const size& s) = default;

        static auto zero() -> size;

        [[nodiscard]] auto width() const -> int16_t;
        [[nodiscard]] auto height() const -> int16_t;

        auto set_width(const int16_t& width) -> void;
        auto set_height(const int16_t& height) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> size;
        auto write(graphite::data::writer& writer, enum coding_type type = qd) const -> void;
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
        fixed_size() = default;
        fixed_size(double width, double height);
        fixed_size(const fixed_size& s) = default;

        static auto zero() -> fixed_size;

        [[nodiscard]] auto width() const -> double;
        [[nodiscard]] auto height() const -> double;

        auto set_width(const double& width) -> void;
        auto set_height(const double& height) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> fixed_size;
        auto write(graphite::data::writer& writer, enum coding_type type = qd) const -> void;
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
        rect() = default;
        rect(const qd::point& origin, const qd::size& size);
        rect(int16_t x, int16_t y, int16_t width, int16_t height);
        rect(const qd::rect& r) = default;

        static auto zero() -> qd::rect;

        [[nodiscard]] auto x() const -> int16_t;
        [[nodiscard]] auto y() const -> int16_t;
        [[nodiscard]] auto width() const -> int16_t;
        [[nodiscard]] auto height() const -> int16_t;
        [[nodiscard]] auto origin() const -> qd::point;
        [[nodiscard]] auto size() const -> qd::size;

        auto set_x(const int16_t& x) -> void;
        auto set_y(const int16_t& y) -> void;
        auto set_width(const int16_t& width) -> void;
        auto set_height(const int16_t& height) -> void;
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
        fixed_rect() = default;
        fixed_rect(const qd::fixed_point& origin, const qd::fixed_size& size);
        fixed_rect(double x, double y, double width, double height);
        fixed_rect(const qd::fixed_rect& r) = default;

        static auto zero() -> qd::fixed_rect;

       [[nodiscard]] auto x() const -> double;
       [[nodiscard]] auto y() const -> double;
       [[nodiscard]] auto width() const -> double;
       [[nodiscard]] auto height() const -> double;
       [[nodiscard]] auto origin() const -> qd::fixed_point;
       [[nodiscard]] auto size() const -> qd::fixed_size;

        auto set_x(const double& x) -> void;
        auto set_y(const double& y) -> void;
        auto set_width(const double& width) -> void;
        auto set_height(const double& height) -> void;
        auto set_origin(const qd::fixed_point& origin) -> void;
        auto set_size(const qd::fixed_size& size) -> void;

        static auto read(graphite::data::reader& reader, enum coding_type type = qd) -> qd::fixed_rect;
        auto write(graphite::data::writer& writer, enum coding_type type = qd) -> void;
    };

}

#endif //GRAPHITE_GEOMETRY_HPP
