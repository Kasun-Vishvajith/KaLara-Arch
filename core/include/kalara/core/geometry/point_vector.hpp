#pragma once
#include "kalara/core/geometry/tolerances.hpp"
#include "kalara/core/geometry/angle.hpp"
#include <cmath>

namespace kalara::core::geometry {

class Vector2D;

/// 2D Point in millimetres (canonical internal unit).
class Point2D {
public:
    double x = 0.0;
    double y = 0.0;

    constexpr Point2D() noexcept = default;
    constexpr Point2D(double x_mm, double y_mm) noexcept : x(x_mm), y(y_mm) {}

    double distanceTo(const Point2D& other) const noexcept {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::hypot(dx, dy);
    }

    double distanceSquaredTo(const Point2D& other) const noexcept {
        double dx = x - other.x;
        double dy = y - other.y;
        return dx * dx + dy * dy;
    }

    bool coincidesWith(const Point2D& other, double eps = Tolerances::LinearEpsilon_mm) const noexcept {
        return distanceSquaredTo(other) <= (eps * eps);
    }

    bool operator==(const Point2D& other) const noexcept {
        return Tolerances::equalLinear(x, other.x) && Tolerances::equalLinear(y, other.y);
    }

    Point2D operator+(const Vector2D& v) const noexcept;
    Point2D operator-(const Vector2D& v) const noexcept;
    Vector2D operator-(const Point2D& other) const noexcept;
    Vector2D toVector() const noexcept;
};

/// 2D Vector in millimetres.
class Vector2D {
public:
    double dx = 0.0;
    double dy = 0.0;

    constexpr Vector2D() noexcept = default;
    constexpr Vector2D(double dx_mm, double dy_mm) noexcept : dx(dx_mm), dy(dy_mm) {}

    double length() const noexcept {
        return std::hypot(dx, dy);
    }

    double lengthSquared() const noexcept {
        return dx * dx + dy * dy;
    }

    Vector2D normalized() const noexcept {
        double len = length();
        if (len <= Tolerances::LinearEpsilon_mm) {
            return Vector2D(0.0, 0.0);
        }
        return Vector2D(dx / len, dy / len);
    }

    /// Perpendicular vector (rotated 90 degrees CCW).
    Vector2D perpendicular() const noexcept {
        return Vector2D(-dy, dx);
    }

    double dot(const Vector2D& other) const noexcept {
        return dx * other.dx + dy * other.dy;
    }

    /// 2D cross-product equivalent (z-component).
    double cross(const Vector2D& other) const noexcept {
        return dx * other.dy - dy * other.dx;
    }

    Angle angle() const noexcept {
        return Angle(std::atan2(dy, dx)).normalized();
    }

    Angle angleTo(const Vector2D& other) const noexcept {
        double d = dot(other);
        double c = cross(other);
        return Angle(std::atan2(c, d)).normalized();
    }

    constexpr Vector2D operator+(const Vector2D& other) const noexcept {
        return Vector2D(dx + other.dx, dy + other.dy);
    }

    constexpr Vector2D operator-(const Vector2D& other) const noexcept {
        return Vector2D(dx - other.dx, dy - other.dy);
    }

    constexpr Vector2D operator*(double scalar) const noexcept {
        return Vector2D(dx * scalar, dy * scalar);
    }

    constexpr Vector2D operator/(double scalar) const noexcept {
        return Vector2D(dx / scalar, dy / scalar);
    }

    bool operator==(const Vector2D& other) const noexcept {
        return Tolerances::equalLinear(dx, other.dx) && Tolerances::equalLinear(dy, other.dy);
    }
};

inline Point2D Point2D::operator+(const Vector2D& v) const noexcept {
    return Point2D(x + v.dx, y + v.dy);
}

inline Point2D Point2D::operator-(const Vector2D& v) const noexcept {
    return Point2D(x - v.dx, y - v.dy);
}

inline Vector2D Point2D::operator-(const Point2D& other) const noexcept {
    return Vector2D(x - other.x, y - other.y);
}

inline Vector2D Point2D::toVector() const noexcept {
    return Vector2D(x, y);
}

} // namespace kalara::core::geometry
