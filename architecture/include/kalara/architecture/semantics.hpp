#pragma once
#include <string_view>

namespace kalara::architecture {

/// Semantic classification types for entities (Principle 2).
enum class SemanticType {
    Unknown,
    Project,
    Site,
    Building,
    Level,
    PropertyBoundary,
    BuildingFootprint,
    Setback,
    Room,
    Wall,
    Door,
    Window,
    Stair,
    Furniture,
    Fixture,
    Annotation,
    Dimension,
    SiteElement,
    Roof
};

constexpr std::string_view semanticTypeName(SemanticType type) noexcept {
    switch (type) {
        case SemanticType::Unknown:           return "Unknown";
        case SemanticType::Project:           return "Project";
        case SemanticType::Site:              return "Site";
        case SemanticType::Building:          return "Building";
        case SemanticType::Level:             return "Level";
        case SemanticType::PropertyBoundary:  return "PropertyBoundary";
        case SemanticType::BuildingFootprint: return "BuildingFootprint";
        case SemanticType::Setback:           return "Setback";
        case SemanticType::Room:              return "Room";
        case SemanticType::Wall:              return "Wall";
        case SemanticType::Door:              return "Door";
        case SemanticType::Window:            return "Window";
        case SemanticType::Stair:             return "Stair";
        case SemanticType::Furniture:         return "Furniture";
        case SemanticType::Fixture:           return "Fixture";
        case SemanticType::Annotation:        return "Annotation";
        case SemanticType::Dimension:         return "Dimension";
        case SemanticType::SiteElement:       return "SiteElement";
        case SemanticType::Roof:              return "Roof";
    }
    return "Unknown";
}

} // namespace kalara::architecture
