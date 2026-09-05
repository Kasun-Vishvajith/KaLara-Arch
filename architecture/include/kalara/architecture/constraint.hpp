#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include <string>
#include <vector>
#include <string_view>
#include <memory>

namespace kalara::architecture {

/// Hard constraint (must never be violated) vs Soft preference (Rule 9).
enum class ConstraintSeverity {
    Hard,
    Soft
};

/// Specific types of architectural constraints (Rule 8).
enum class ConstraintType {
    Coincidence,     // Endpoints or vertices must coincide
    Parallelism,     // Elements must remain parallel
    Perpendicularity,// Elements must remain perpendicular (90 degrees)
    FixedDistance,   // Exact distance between entities in mm
    MinimumDistance, // Minimum clearance distance in mm (e.g. setback, passage)
    Containment,     // Element must remain inside another (e.g. pool inside site)
    HostAttachment   // Opening hosted on wall body
};

constexpr std::string_view constraintSeverityName(ConstraintSeverity sev) noexcept {
    return (sev == ConstraintSeverity::Hard) ? "HARD" : "SOFT";
}

constexpr std::string_view constraintTypeName(ConstraintType type) noexcept {
    switch (type) {
        case ConstraintType::Coincidence:      return "Coincidence";
        case ConstraintType::Parallelism:      return "Parallelism";
        case ConstraintType::Perpendicularity: return "Perpendicularity";
        case ConstraintType::FixedDistance:    return "FixedDistance";
        case ConstraintType::MinimumDistance:  return "MinimumDistance";
        case ConstraintType::Containment:      return "Containment";
        case ConstraintType::HostAttachment:   return "HostAttachment";
    }
    return "Unknown";
}

/// Conflict report record when a constraint evaluation fails.
struct ConstraintConflict {
    EntityId constraintId;
    ConstraintSeverity severity;
    ConstraintType type;
    std::string description;
    double deviationValue = 0.0; // Measurable deviation (e.g. mm or radians)
};

/// Architectural Constraint entity as first-class data (Rule 8 & 9).
class Constraint {
public:
    EntityId id;
    std::string name;
    ConstraintType type = ConstraintType::Coincidence;
    ConstraintSeverity severity = ConstraintSeverity::Hard;
    std::vector<EntityId> targetEntityIds;
    double targetValue = 0.0; // Distance in mm or angle in radians
    Metadata metadata;

    Constraint() : id(EntityId::generate("cst")) {}

    Constraint(std::string constraintName, ConstraintType cType, ConstraintSeverity cSev,
               std::vector<EntityId> targets, double value = 0.0)
        : id(EntityId::generate("cst")),
          name(std::move(constraintName)),
          type(cType),
          severity(cSev),
          targetEntityIds(std::move(targets)),
          targetValue(value) {}
};

} // namespace kalara::architecture
