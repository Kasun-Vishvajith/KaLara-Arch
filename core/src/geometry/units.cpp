#include "kalara/core/geometry/units.hpp"
#include <sstream>
#include <iomanip>

namespace kalara::core::geometry {

double UnitConverter::toDisplay(double value_mm, UnitSystem unit) noexcept {
    switch (unit) {
        case UnitSystem::Millimetres: return value_mm;
        case UnitSystem::Centimetres: return value_mm / 10.0;
        case UnitSystem::Metres:      return value_mm / 1000.0;
        case UnitSystem::Inches:      return value_mm / 25.4;
        case UnitSystem::Feet:        return value_mm / 304.8;
    }
    return value_mm;
}

double UnitConverter::fromDisplay(double displayValue, UnitSystem unit) noexcept {
    switch (unit) {
        case UnitSystem::Millimetres: return displayValue;
        case UnitSystem::Centimetres: return displayValue * 10.0;
        case UnitSystem::Metres:      return displayValue * 1000.0;
        case UnitSystem::Inches:      return displayValue * 25.4;
        case UnitSystem::Feet:        return displayValue * 304.8;
    }
    return displayValue;
}

std::string_view UnitConverter::unitSymbol(UnitSystem unit) noexcept {
    switch (unit) {
        case UnitSystem::Millimetres: return "mm";
        case UnitSystem::Centimetres: return "cm";
        case UnitSystem::Metres:      return "m";
        case UnitSystem::Inches:      return "in";
        case UnitSystem::Feet:        return "ft";
    }
    return "mm";
}

std::string UnitConverter::format(double value_mm, UnitSystem unit, int decimalPlaces) {
    double disp = toDisplay(value_mm, unit);
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(decimalPlaces) << disp << " " << unitSymbol(unit);
    return ss.str();
}

} // namespace kalara::core::geometry
