#pragma once
#include <cmath>

namespace kalara::core::geometry {

/// Tolerances for architectural geometry calculations in millimetres.
struct Tolerances {
    /// Nominal precision target (1 mm per AGENTS.md Principle 4).
    static constexpr double NominalPrecision_mm = 1.0;

    /// Epsilon for linear point coincidence / floating-point calculations.
    static constexpr double LinearEpsilon_mm = 1e-4;

    /// Epsilon for angular comparisons (in radians, ~0.001 degrees).
    static constexpr double AngularEpsilon_rad = 1e-5;

    /// Check if two double-precision values are equal within linear epsilon.
    static inline bool equalLinear(double a, double b, double eps = LinearEpsilon_mm) noexcept {
        return std::abs(a - b) <= eps;
    }

    /// Check if two double-precision values are equal within angular epsilon.
    static inline bool equalAngular(double a, double b, double eps = AngularEpsilon_rad) noexcept {
        return std::abs(a - b) <= eps;
    }
};

} // namespace kalara::core::geometry
