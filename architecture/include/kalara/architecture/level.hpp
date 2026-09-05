#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include <string>

namespace kalara::architecture {

/// Architectural Level / Story (Principle 5).
/// Stores canonical elevation in millimetres (mm).
class Level {
public:
    EntityId id;
    std::string name;
    double elevation_mm = 0.0;
    double height_mm = 3000.0; // Default floor-to-floor height 3000 mm
    Metadata metadata;

    Level() : id(EntityId::generate("lvl")), name("Ground Floor") {}
    Level(std::string levelName, double elevation, double height = 3000.0)
        : id(EntityId::generate("lvl")),
          name(std::move(levelName)),
          elevation_mm(elevation),
          height_mm(height) {}

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Level; }
};

} // namespace kalara::architecture
