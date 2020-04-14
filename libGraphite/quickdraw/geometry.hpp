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

        point() {}
        point(const int16_t x, const int16_t y) : m_x(x), m_y(y) {};
        point(const point& p) : m_x(p.m_x), m_y(p.m_y) {};

        static auto zero() -> point { return point(); };

        auto x() const -> int16_t { return m_x; };
        auto y() const -> int16_t { return m_y; };

        auto set_x(const int16_t x) -> void { m_x = x; };
        auto set_y(const int16_t y) -> void { m_y = y; };

        static auto read(graphite::data::reader& reader, coding_type type = qd) -> point
        {
            switch (type) {
                case coding_type::qd: {
                    auto y = reader.read_signed_short();
                    auto x = reader.read_signed_short();
                    return point(x, y);
                }
                case coding_type::pict: {
                    return point(reader.read_signed_short(), reader.read_signed_short());
                }
            }
        }

        auto write(graphite::data::writer& writer, coding_type type = qd) -> void
        {
            switch (type) {
                case coding_type::qd: {
                    writer.write_signed_short(m_y);
                    writer.write_signed_short(m_x);
                    break;
                }
                case coding_type::pict: {
                    writer.write_signed_short(m_x);
                    writer.write_signed_short(m_y);
                    break;
                }
            }
        }

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

        fixed_point() {}
        fixed_point(const double x, const double y) : m_x(x), m_y(y) {};
        fixed_point(const fixed_point& p) : m_x(p.m_x), m_y(p.m_y) {};

        static auto zero() -> fixed_point { return fixed_point(); };

        auto x() const -> double { return m_x; };
        auto y() const -> double { return m_y; };

        auto set_x(const double x) -> void { m_x = x; };
        auto set_y(const double y) -> void { m_y = y; };

        static auto read(graphite::data::reader& reader, coding_type type = qd) -> fixed_point
        {
            switch (type) {
                case coding_type::qd: {
                    auto y = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
                    auto x = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
                    return fixed_point(x, y);
                }
            }
        }

        auto write(graphite::data::writer& writer, coding_type type = qd) -> void
        {
            switch (type) {
                case coding_type::qd: {
                    writer.write_signed_long(static_cast<int32_t>(m_y * (1 << 16)));
                    writer.write_signed_long(static_cast<int32_t>(m_x * (1 << 16)));
                    break;
                }
            }
        }

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

        size() {}
        size(const int16_t width, const int16_t height) : m_width(width), m_height(height) {};
        size(const size& s) : m_width(s.m_width), m_height(s.m_height) {};

        static auto zero() -> size { return size(); };

        auto width() const -> int16_t { return m_width; };
        auto height() const -> int16_t { return m_height; };

        auto set_width(const int16_t width) -> void { m_width = width; };
        auto set_height(const int16_t height) -> void { m_height = height; };

        static auto read(graphite::data::reader& reader, coding_type type = qd) -> size
        {
            switch (type) {
                case coding_type::qd: {
                    auto height = reader.read_signed_short();
                    auto width = reader.read_signed_short();
                    return size(width, height);
                }
                case coding_type::pict: {
                    return size(reader.read_signed_short(), reader.read_signed_short());
                }
            }
        }

        auto write(graphite::data::writer& writer, coding_type type = qd) -> void
        {
            switch (type) {
                case coding_type::qd: {
                    writer.write_signed_short(m_height);
                    writer.write_signed_short(m_width);
                    break;
                }
                case coding_type::pict: {
                    writer.write_signed_short(m_width);
                    writer.write_signed_short(m_height);
                    break;
                }
            }
        }
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

        fixed_size() {}
        fixed_size(const double width, const double height) : m_width(width), m_height(height) {};
        fixed_size(const fixed_size& s) : m_width(s.m_width), m_height(s.m_height) {};

        static auto zero() -> fixed_size { return fixed_size(); };

        auto width() const -> double { return m_width; };
        auto height() const -> double { return m_height; };

        auto set_width(const double width) -> void { m_width = width; };
        auto set_height(const double height) -> void { m_height = height; };

        static auto read(graphite::data::reader& reader, coding_type type = qd) -> fixed_size
        {
            switch (type) {
                case coding_type::qd: {
                    auto height = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
                    auto width = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
                    return fixed_size(width, height);
                }
            }
        }

        auto write(graphite::data::writer& writer, coding_type type = qd) -> void
        {
            switch (type) {
                case coding_type::qd: {
                    writer.write_signed_long(static_cast<int32_t>(m_height * (1 << 16)));
                    writer.write_signed_long(static_cast<int32_t>(m_width * (1 << 16)));
                    break;
                }
            }
        }
    };

    // MARK: - Rect

    struct rect
    {
    public:
        enum coding_type { qd, pict };

    private:
        point m_origin { point::zero() };
        size m_size { size::zero() };

    public:

        rect() {};
        rect(const point& origin, const size& size) : m_origin(origin), m_size(size) {};
        rect(const int16_t x, const int16_t y, const int16_t width, const int16_t height) : m_origin(x, y), m_size(width, height) {};
        rect(const rect& r) : m_origin(r.m_origin), m_size(r.m_size) {};

        static auto zero() -> rect { return rect(); };

        auto x() const -> int16_t { return m_origin.x(); };
        auto y() const -> int16_t { return m_origin.y(); };
        auto width() const -> int16_t { return m_size.width(); };
        auto height() const -> int16_t { return m_size.height(); };
        auto origin() const -> point { return m_origin; };
        auto size() const -> size { return m_size; };

        auto set_x(const int16_t x) -> void { m_origin.set_x(x); };
        auto set_y(const int16_t y) -> void { m_origin.set_y(y); };
        auto set_width(const int16_t width) -> void { m_size.set_width(width); };
        auto set_height(const int16_t height) -> void { m_size.set_height(height); };
        auto set_origin(const point& origin) -> void { m_origin = origin; };
        auto set_size(const struct size& size) -> void { m_size = size; };

        static auto read(graphite::data::reader& reader, coding_type type = qd) -> rect
        {
            switch (type) {
                case coding_type::qd: {
                    auto origin = point::read(reader, point::qd);
                    auto opposite = point::read(reader, point::qd);
                    return rect(origin, qd::size(origin.x() + opposite.x(), origin.y() + opposite.y()));
                }
                case coding_type::pict: {
                    return rect(point::read(reader, point::pict), size::read(reader, size::pict));
                }
            }
        }

        auto write(graphite::data::writer& writer, coding_type type = qd) -> void
        {
            switch (type) {
                case coding_type::qd: {
                    m_origin.write(writer, point::qd);
                    m_size.write(writer, size::qd);
                    break;
                }
                case coding_type::pict: {
                    m_origin.write(writer, point::pict);
                    m_size.write(writer, size::pict);
                    break;
                }
            }
        }
    };

    // MARK: - Fixed Rect

    struct fixed_rect
    {
    public:
        enum coding_type { qd };

    private:
        fixed_point m_origin { fixed_point::zero() };
        fixed_size m_size { fixed_size::zero() };

    public:

        fixed_rect() {};
        fixed_rect(const fixed_point& origin, const fixed_size& size) : m_origin(origin), m_size(size) {};
        fixed_rect(const double x, const double y, const double width, const double height) : m_origin(x, y), m_size(width, height) {};
        fixed_rect(const fixed_rect& r) : m_origin(r.m_origin), m_size(r.m_size) {};

        static auto zero() -> fixed_rect { return fixed_rect(); };

        auto x() const -> double { return m_origin.x(); };
        auto y() const -> double { return m_origin.y(); };
        auto width() const -> double { return m_size.width(); };
        auto height() const -> double { return m_size.height(); };
        auto origin() const -> fixed_point { return m_origin; };
        auto size() const -> fixed_size { return m_size; };

        auto set_x(const double x) -> void { m_origin.set_x(x); };
        auto set_y(const double y) -> void { m_origin.set_y(y); };
        auto set_width(const double width) -> void { m_size.set_width(width); };
        auto set_height(const double height) -> void { m_size.set_height(height); };
        auto set_origin(const fixed_point& origin) -> void { m_origin = origin; };
        auto set_size(const fixed_size& size) -> void { m_size = size; };

        static auto read(graphite::data::reader& reader, coding_type type = qd) -> fixed_rect
        {
            switch (type) {
                case coding_type::qd: {
                    auto origin = fixed_point::read(reader, fixed_point::qd);
                    auto opposite = fixed_point::read(reader, fixed_point::qd);
                    return fixed_rect(origin, fixed_size(origin.x() + opposite.x(), origin.y() + opposite.y()));
                }
            }
        }

        auto write(graphite::data::writer& writer, coding_type type = qd) -> void
        {
            switch (type) {
                case coding_type::qd: {
                    m_origin.write(writer, fixed_point::qd);
                    m_size.write(writer, fixed_size::qd);
                    break;
                }
            }
        }
    };


}};

#endif //GRAPHITE_GEOMETRY_HPP
