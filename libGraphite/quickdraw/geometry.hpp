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

        static point zero() { return point(); };

        int16_t x() const { return m_x; };
        int16_t y() const { return m_y; };

        void set_x(const int16_t x) { m_x = x; };
        void set_y(const int16_t y) { m_y = y; };

        static point read(graphite::data::reader& reader, coding_type type = qd)
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

        void write(graphite::data::writer& writer, coding_type type = qd)
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

        static fixed_point zero() { return fixed_point(); };

        double x() const { return m_x; };
        double y() const { return m_y; };

        void set_x(const double x) { m_x = x; };
        void set_y(const double y) { m_y = y; };

        static fixed_point read(graphite::data::reader& reader, coding_type type = qd)
        {
            switch (type) {
                case coding_type::qd: {
                    auto y = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
                    auto x = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
                    return fixed_point(x, y);
                }
            }
        }

        void write(graphite::data::writer& writer, coding_type type = qd)
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

        static size zero() { return size(); };

        int16_t width() const { return m_width; };
        int16_t height() const { return m_height; };

        void set_width(const int16_t width) { m_width = width; };
        void set_height(const int16_t height) { m_height = height; };

        static size read(graphite::data::reader& reader, coding_type type = qd)
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

        void write(graphite::data::writer& writer, coding_type type = qd)
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

        static fixed_size zero() { return fixed_size(); };

        double width() const { return m_width; };
        double height() const { return m_height; };

        void set_width(const double width) { m_width = width; };
        void set_height(const double height) { m_height = height; };

        static fixed_size read(graphite::data::reader& reader, coding_type type = qd)
        {
            switch (type) {
                case coding_type::qd: {
                    auto height = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
                    auto width = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
                    return fixed_size(width, height);
                }
            }
        }

        void write(graphite::data::writer& writer, coding_type type = qd)
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

        static rect zero() { return rect(); };

        int16_t x() const { return m_origin.x(); };
        int16_t y() const { return m_origin.y(); };
        int16_t width() const { return m_size.width(); };
        int16_t height() const { return m_size.height(); };
        point origin() const { return m_origin; };
        size size() const { return m_size; };

        void set_x(const int16_t x) { m_origin.set_x(x); };
        void set_y(const int16_t y) { m_origin.set_y(y); };
        void set_width(const int16_t width) { m_size.set_width(width); };
        void set_height(const int16_t height) { m_size.set_height(height); };
        void set_origin(const point& origin) { m_origin = origin; };
        void set_size(const struct size& size) { m_size = size; };

        static rect read(graphite::data::reader& reader, coding_type type = qd)
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

        void write(graphite::data::writer& writer, coding_type type = qd)
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

        static fixed_rect zero() { return fixed_rect(); };

        double x() const { return m_origin.x(); };
        double y() const { return m_origin.y(); };
        double width() const { return m_size.width(); };
        double height() const { return m_size.height(); };
        fixed_point origin() const { return m_origin; };
        fixed_size size() const { return m_size; };

        void set_x(const double x) { m_origin.set_x(x); };
        void set_y(const double y) { m_origin.set_y(y); };
        void set_width(const double width) { m_size.set_width(width); };
        void set_height(const double height) { m_size.set_height(height); };
        void set_origin(const fixed_point& origin) { m_origin = origin; };
        void set_size(const fixed_size& size) { m_size = size; };

        static fixed_rect read(graphite::data::reader& reader, coding_type type = qd)
        {
            switch (type) {
                case coding_type::qd: {
                    auto origin = fixed_point::read(reader, fixed_point::qd);
                    auto opposite = fixed_point::read(reader, fixed_point::qd);
                    return fixed_rect(origin, fixed_size(origin.x() + opposite.x(), origin.y() + opposite.y()));
                }
            }
        }

        void write(graphite::data::writer& writer, coding_type type = qd)
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
