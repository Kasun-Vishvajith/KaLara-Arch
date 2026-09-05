#pragma once
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/angle.hpp"
#include <cmath>

namespace kalara::core::geometry {

/// 2D Affine Transform (3x3 matrix for translation, rotation, and scaling in mm).
/// [ m00  m01  m02 ]
/// [ m10  m11  m12 ]
/// [  0    0    1  ]
class Transform2D {
public:
    double m00 = 1.0, m01 = 0.0, m02 = 0.0;
    double m10 = 0.0, m11 = 1.0, m12 = 0.0;

    constexpr Transform2D() noexcept = default;
    constexpr Transform2D(double a00, double a01, double a02,
                          double a10, double a11, double a12) noexcept
        : m00(a00), m01(a01), m02(a02),
          m10(a10), m11(a11), m12(a12) {}

    static Transform2D identity() noexcept {
        return Transform2D();
    }

    static Transform2D translation(double tx_mm, double ty_mm) noexcept {
        return Transform2D(1.0, 0.0, tx_mm,
                           0.0, 1.0, ty_mm);
    }

    static Transform2D translation(const Vector2D& v) noexcept {
        return translation(v.dx, v.dy);
    }

    static Transform2D rotation(Angle angle) noexcept {
        double c = std::cos(angle.radians());
        double s = std::sin(angle.radians());
        return Transform2D(c, -s, 0.0,
                           s,  c, 0.0);
    }

    static Transform2D rotationAround(Angle angle, const Point2D& center) noexcept {
        return translation(center.x, center.y) * rotation(angle) * translation(-center.x, -center.y);
    }

    static Transform2D scaling(double sx, double sy) noexcept {
        return Transform2D(sx,  0.0, 0.0,
                           0.0, sy,  0.0);
    }

    Point2D map(const Point2D& p) const noexcept {
        return Point2D(
            m00 * p.x + m01 * p.y + m02,
            m10 * p.x + m11 * p.y + m12
        );
    }

    Vector2D map(const Vector2D& v) const noexcept {
        return Vector2D(
            m00 * v.dx + m01 * v.dy,
            m10 * v.dx + m11 * v.dy
        );
    }

    Transform2D operator*(const Transform2D& o) const noexcept {
        return Transform2D(
            m00 * o.m00 + m01 * o.m10,
            m00 * o.m01 + m01 * o.m11,
            m00 * o.m02 + m01 * o.m12 + m02,

            m10 * o.m00 + m11 * o.m10,
            m10 * o.m01 + m11 * o.m11,
            m10 * o.m02 + m11 * o.m12 + m12
        );
    }

    /// Inverse transform. Returns identity if determinant is zero.
    Transform2D inverted() const noexcept {
        double det = m00 * m11 - m01 * m10;
        if (std::abs(det) <= Tolerances::LinearEpsilon_mm * Tolerances::LinearEpsilon_mm) {
            return identity();
        }
        double invDet = 1.0 / det;
        return Transform2D(
             m11 * invDet,
            -m01 * invDet,
            (m01 * m12 - m02 * m11) * invDet,

            -m10 * invDet,
             m00 * invDet,
            (m02 * m10 - m00 * m12) * invDet
        );
    }
};

} // namespace kalara::core::geometry
