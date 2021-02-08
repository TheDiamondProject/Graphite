//
// Created by tomhancocks on 18/04/2020.
//

#include "libGraphite/quickdraw/geometry.hpp"

// MARK: - Point

graphite::qd::point::point(int16_t x, int16_t y)
    : m_x(x), m_y(y)
{

}

auto graphite::qd::point::zero() -> qd::point
{
    return { 0, 0 };
}

auto graphite::qd::point::x() const -> int16_t
{
    return m_x;
}

auto graphite::qd::point::y() const -> int16_t
{
    return m_y;
}

auto graphite::qd::point::set_x(const int16_t& x) -> void
{
    m_x = x;
}

auto graphite::qd::point::set_y(const int16_t& y) -> void
{
    m_y = y;
}

auto graphite::qd::point::read(graphite::data::reader& reader, enum coding_type type) -> qd::point
{
    switch (type) {
        case coding_type::qd: {
            auto y = reader.read_signed_short();
            auto x = reader.read_signed_short();
            return qd::point(x, y);
        }
        case coding_type::pict: {
            auto x = reader.read_signed_short();
            auto y = reader.read_signed_short();
            return qd::point(x, y);
        }
    }
}

auto graphite::qd::point::write(graphite::data::writer& writer, enum coding_type type) const -> void
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

// MARK: - Fixed Point

graphite::qd::fixed_point::fixed_point(double x, double y)
    : m_x(x), m_y(y)
{

};

auto graphite::qd::fixed_point::zero() -> qd::fixed_point
{
    return { 0, 0 };
};

auto graphite::qd::fixed_point::x() const -> double
{
    return m_x;
};

auto graphite::qd::fixed_point::y() const -> double
{
    return m_y;
};

auto graphite::qd::fixed_point::set_x(const double& x) -> void
{
    m_x = x;
};

auto graphite::qd::fixed_point::set_y(const double& y) -> void
{
    m_y = y;
};

auto graphite::qd::fixed_point::read(graphite::data::reader& reader, enum coding_type type) -> qd::fixed_point
{
    switch (type) {
        case coding_type::qd: {
            auto y = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
            auto x = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
            return qd::fixed_point(x, y);
        }
    }
}

auto graphite::qd::fixed_point::write(graphite::data::writer& writer, enum coding_type type) const -> void
{
    switch (type) {
        case coding_type::qd: {
            writer.write_signed_long(static_cast<int32_t>(m_y * (1 << 16)));
            writer.write_signed_long(static_cast<int32_t>(m_x * (1 << 16)));
            break;
        }
    }
}

// MARK: - Size

graphite::qd::size::size(int16_t width, int16_t height)
    : m_width(width), m_height(height)
{

};

auto graphite::qd::size::zero() -> qd::size
{
    return size();
};

auto graphite::qd::size::width() const -> int16_t
{
    return m_width;
};

auto graphite::qd::size::height() const -> int16_t
{
    return m_height;
};

auto graphite::qd::size::set_width(const int16_t& width) -> void
{
    m_width = width;
};

auto graphite::qd::size::set_height(const int16_t& height) -> void
{
    m_height = height;
};

auto graphite::qd::size::read(graphite::data::reader& reader, enum coding_type type) -> qd::size
{
    switch (type) {
        case coding_type::qd: {
            auto height = reader.read_signed_short();
            auto width = reader.read_signed_short();
            return qd::size(width, height);
        }
        case coding_type::pict: {
            auto width = reader.read_signed_short();
            auto height = reader.read_signed_short();
            return qd::size(width, height);
        }
    }
}

auto graphite::qd::size::write(graphite::data::writer& writer, enum coding_type type) const -> void
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

// MARK: - Fixed Size

graphite::qd::fixed_size::fixed_size(double width, double height)
    : m_width(width), m_height(height)
{

}

auto graphite::qd::fixed_size::zero() -> qd::fixed_size
{
    return { 0, 0 };
}

auto graphite::qd::fixed_size::width() const -> double
{
    return m_width;
}

auto graphite::qd::fixed_size::height() const -> double
{
    return m_height;
}

auto graphite::qd::fixed_size::set_width(const double& width) -> void
{
    m_width = width;
}

auto graphite::qd::fixed_size::set_height(const double& height) -> void
{
    m_height = height;
}

auto graphite::qd::fixed_size::read(graphite::data::reader& reader, enum coding_type type) -> qd::fixed_size
{
    switch (type) {
        case coding_type::qd: {
            auto height = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
            auto width = static_cast<double>(reader.read_signed_long() / static_cast<double>(1 << 16));
            return qd::fixed_size(width, height);
        }
    }
}

auto graphite::qd::fixed_size::write(graphite::data::writer& writer, enum coding_type type) const -> void
{
    switch (type) {
        case coding_type::qd: {
            writer.write_signed_long(static_cast<int32_t>(m_height * (1 << 16)));
            writer.write_signed_long(static_cast<int32_t>(m_width * (1 << 16)));
            break;
        }
    }
}

// MARK: - Rect

graphite::qd::rect::rect(const point& origin, const qd::size& sz)
    : m_origin(origin), m_size(sz)
{

}

graphite::qd::rect::rect(int16_t x, int16_t y, int16_t width, int16_t height)
    : m_origin(x, y), m_size(width, height)
{

}

auto graphite::qd::rect::zero() -> qd::rect
{
    return { 0, 0, 0, 0 };
}

auto graphite::qd::rect::x() const -> int16_t
{
    return m_origin.x();
}

auto graphite::qd::rect::y() const -> int16_t
{
    return m_origin.y();
}

auto graphite::qd::rect::width() const -> int16_t
{
    return m_size.width();
}

auto graphite::qd::rect::height() const -> int16_t
{
    return m_size.height();
}

auto graphite::qd::rect::origin() const -> qd::point
{
    return m_origin;
}

auto graphite::qd::rect::size() const -> qd::size
{
    return m_size;
}

auto graphite::qd::rect::set_x(const int16_t& x) -> void
{
    m_origin.set_x(x);
}

auto graphite::qd::rect::set_y(const int16_t& y) -> void
{
    m_origin.set_y(y);
}

auto graphite::qd::rect::set_width(const int16_t& width) -> void
{
    m_size.set_width(width);
}

auto graphite::qd::rect::set_height(const int16_t& height) -> void
{
    m_size.set_height(height);
}

auto graphite::qd::rect::set_origin(const qd::point& origin) -> void
{
    m_origin = origin;
}

auto graphite::qd::rect::set_size(const struct qd::size& size) -> void
{
    m_size = size;
}

auto graphite::qd::rect::read(graphite::data::reader& reader, enum coding_type type) -> qd::rect
{
    switch (type) {
        case coding_type::qd: {
            auto origin = qd::point::read(reader, point::qd);
            auto opposite = qd::point::read(reader, point::qd);
            return qd::rect(origin, qd::size(opposite.x() - origin.x(), opposite.y() - origin.y()));
        }
        case coding_type::pict: {
            auto origin = point::read(reader, point::pict);
            auto sz = size::read(reader, size::pict);
            return qd::rect(origin, sz);
        }
    }
}

auto graphite::qd::rect::write(graphite::data::writer& writer, enum coding_type type) -> void
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

// MARK: - Fixed Rect

graphite::qd::fixed_rect::fixed_rect(const qd::fixed_point& origin, const qd::fixed_size& size)
    : m_origin(origin), m_size(size)
{

}

graphite::qd::fixed_rect::fixed_rect(double x, double y, double width, double height)
    : m_origin(x, y), m_size(width, height)
{

}

auto graphite::qd::fixed_rect::zero() -> qd::fixed_rect
{
    return { 0, 0, 0, 0 };
}

auto graphite::qd::fixed_rect::x() const -> double
{
    return m_origin.x();
}

auto graphite::qd::fixed_rect::y() const -> double
{
    return m_origin.y();
}

auto graphite::qd::fixed_rect::width() const -> double
{
    return m_size.width();
}

auto graphite::qd::fixed_rect::height() const -> double
{
    return m_size.height();
}

auto graphite::qd::fixed_rect::origin() const -> qd::fixed_point
{
    return m_origin;
}

auto graphite::qd::fixed_rect::size() const -> qd::fixed_size
{
    return m_size;
}

auto graphite::qd::fixed_rect::set_x(const double& x) -> void
{
    m_origin.set_x(x);
}

auto graphite::qd::fixed_rect::set_y(const double& y) -> void
{
    m_origin.set_y(y);
}

auto graphite::qd::fixed_rect::set_width(const double& width) -> void
{
    m_size.set_width(width);
}

auto graphite::qd::fixed_rect::set_height(const double& height) -> void
{
    m_size.set_height(height);
}

auto graphite::qd::fixed_rect::set_origin(const qd::fixed_point& origin) -> void
{
    m_origin = origin;
}

auto graphite::qd::fixed_rect::set_size(const qd::fixed_size& size) -> void
{
    m_size = size;
}

auto graphite::qd::fixed_rect::read(graphite::data::reader& reader, enum coding_type type) -> qd::fixed_rect
{
    switch (type) {
        case coding_type::qd: {
            auto origin = fixed_point::read(reader, fixed_point::qd);
            auto opposite = fixed_point::read(reader, fixed_point::qd);
            return qd::fixed_rect(origin, fixed_size(origin.x() + opposite.x(), origin.y() + opposite.y()));
        }
    }
}

auto graphite::qd::fixed_rect::write(graphite::data::writer& writer, enum coding_type type) -> void
{
    switch (type) {
        case coding_type::qd: {
            m_origin.write(writer, fixed_point::qd);
            m_size.write(writer, fixed_size::qd);
            break;
        }
    }
}
