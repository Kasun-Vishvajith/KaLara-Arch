#pragma once
#include <string>
#include <string_view>

namespace kalara::core::geometry {

/// Supported display unit systems (Principle 3).
/// Canonical internal unit is ALWAYS millimetres (mm).
enum class UnitSystem {
    Millimetres,
    Centimetres,
    Metres,
    Inches,
    Feet
};

class UnitConverter {
public:
    /// Convert canonical millimetres (mm) to display unit value.
    static double toDisplay(double value_mm, UnitSystem unit) noexcept;

    /// Convert display unit value to canonical millimetres (mm).
    static double fromDisplay(double displayValue, UnitSystem unit) noexcept;

    /// Format canonical millimetre value as a display string with unit symbol.
    static std::string format(double value_mm, UnitSystem unit, int decimalPlaces = 2);

    /// Convert unit system enum to symbol string (e.g. "mm", "m", "ft").
    static std::string_view unitSymbol(UnitSystem unit) noexcept;
};

} // namespace kalara::core::geometry
