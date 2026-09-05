#pragma once
#include "kalara/core/geometry/tolerances.hpp"
#include <numbers>
#include <cmath>

namespace kalara::core::geometry {

/// Angle representation supporting both degrees and radians,
/// with orthogonal-snapping helpers (Rule 14).
class Angle {
public:
    constexpr Angle() noexcept : m_radians(0.0) {}
    constexpr explicit Angle(double radians) noexcept : m_radians(radians) {}

    static constexpr Angle fromRadians(double rad) noexcept {
        return Angle(rad);
    }

    static constexpr Angle fromDegrees(double deg) noexcept {
        return Angle(deg * (std::numbers::pi / 180.0));
    }

    constexpr double radians() const noexcept { return m_radians; }
    constexpr double degrees() const noexcept { return m_radians * (180.0 / std::numbers::pi); }

    /// Normalize angle to [0, 2*pi)
    Angle normalized() const noexcept {
        constexpr double twoPi = 2.0 * std::numbers::pi;
        double rad = std::fmod(m_radians, twoPi);
        if (rad < 0.0) rad += twoPi;
        return Angle(rad);
    }

    /// Check if angle is orthogonal (0, 90, 180, 270 degrees) within tolerance
    bool isOrthogonal(double eps = Tolerances::AngularEpsilon_rad) const noexcept {
        Angle norm = normalized();
        constexpr double halfPi = std::numbers::pi / 2.0;
        double remainder = std::fmod(norm.radians(), halfPi);
        return (remainder <= eps) || (halfPi - remainder <= eps);
    }

    bool operator==(const Angle& other) const noexcept {
        return Tolerances::equalAngular(m_radians, other.m_radians);
    }

    constexpr Angle operator+(const Angle& other) const noexcept {
        return Angle(m_radians + other.m_radians);
    }

    constexpr Angle operator-(const Angle& other) const noexcept {
        return Angle(m_radians - other.m_radians);
    }

    constexpr Angle operator-() const noexcept {
        return Angle(-m_radians);
    }

private:
    double m_radians;
};

} // namespace kalara::core::geometry
