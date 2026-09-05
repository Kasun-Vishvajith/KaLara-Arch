#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/project.hpp"
#include "kalara/architecture/site.hpp"
#include "kalara/architecture/building.hpp"
#include "kalara/architecture/level.hpp"
#include "kalara/core/geometry/point_vector.hpp"
#include <string>
#include <vector>
#include <string_view>
#include <optional>
#include <memory>

namespace kalara::runtime {

/// Severity level of a validation diagnostic (Step 14).
enum class ValidationSeverity {
    Info,
    Warning,
    Error
};

constexpr std::string_view validationSeverityName(ValidationSeverity sev) noexcept {
    switch (sev) {
        case ValidationSeverity::Info:    return "Info";
        case ValidationSeverity::Warning: return "Warning";
        case ValidationSeverity::Error:   return "Error";
    }
    return "Diagnostic";
}

/// Category of architectural validation check.
enum class ValidationCategory {
    Geometry,
    Connection,
    Room,
    Clearance,
    SiteAndSetback,
    Constraint
};

constexpr std::string_view validationCategoryName(ValidationCategory cat) noexcept {
    switch (cat) {
        case ValidationCategory::Geometry:       return "Geometry";
        case ValidationCategory::Connection:     return "Connection";
        case ValidationCategory::Room:           return "Room";
        case ValidationCategory::Clearance:      return "Clearance";
        case ValidationCategory::SiteAndSetback: return "Site & Setback";
        case ValidationCategory::Constraint:     return "Constraint";
    }
    return "General";
}

/// Individual architectural validation diagnostic issue.
struct ValidationIssue {
    std::string code;
    ValidationSeverity severity = ValidationSeverity::Warning;
    ValidationCategory category = ValidationCategory::Geometry;
    std::string message;
    std::string suggestedRemedy;
    std::vector<kalara::architecture::EntityId> entityIds;
    std::optional<kalara::core::geometry::Point2D> location;
};

/// Complete report of all validation diagnostics.
class ValidationReport {
public:
    std::vector<ValidationIssue> issues;

    /// Non-negotiable Rule 16 disclaimer: Automated validation is distinct from professional approval.
    static constexpr std::string_view LegalDisclaimer =
        "KaLara Arch automated validation is provided for design assistance and geometric/constraint checking only. "
        "It does not constitute or replace required professional architectural, structural, building code, or authority approval.";

    void addIssue(ValidationIssue issue) {
        issues.emplace_back(std::move(issue));
    }

    [[nodiscard]] bool hasErrors() const noexcept {
        for (const auto& i : issues) {
            if (i.severity == ValidationSeverity::Error) return true;
        }
        return false;
    }

    [[nodiscard]] bool hasWarnings() const noexcept {
        for (const auto& i : issues) {
            if (i.severity == ValidationSeverity::Warning) return true;
        }
        return false;
    }

    [[nodiscard]] size_t errorCount() const noexcept {
        size_t count = 0;
        for (const auto& i : issues) {
            if (i.severity == ValidationSeverity::Error) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t warningCount() const noexcept {
        size_t count = 0;
        for (const auto& i : issues) {
            if (i.severity == ValidationSeverity::Warning) ++count;
        }
        return count;
    }

    [[nodiscard]] size_t infoCount() const noexcept {
        size_t count = 0;
        for (const auto& i : issues) {
            if (i.severity == ValidationSeverity::Info) ++count;
        }
        return count;
    }

    [[nodiscard]] std::vector<ValidationIssue> filterByCategory(ValidationCategory cat) const {
        std::vector<ValidationIssue> res;
        for (const auto& i : issues) {
            if (i.category == cat) res.push_back(i);
        }
        return res;
    }

    [[nodiscard]] std::vector<ValidationIssue> filterBySeverity(ValidationSeverity sev) const {
        std::vector<ValidationIssue> res;
        for (const auto& i : issues) {
            if (i.severity == sev) res.push_back(i);
        }
        return res;
    }
};

/// Architectural Validation Engine (Rule 16 & Step 14).
/// Inspects geometry, connections, room enclosures, clearances, setbacks, and constraints.
class ValidationEngine {
public:
    /// Validate entire project including sites, buildings, and levels.
    static ValidationReport validateProject(const kalara::architecture::Project& project);

    /// Validate a site, its boundaries, setbacks, and contained buildings.
    static ValidationReport validateSite(const kalara::architecture::Site& site);

    /// Validate a single building and its levels (optionally against parent site setbacks).
    static ValidationReport validateBuilding(const kalara::architecture::Building& building,
                                             const kalara::architecture::Site* site = nullptr);

    /// Validate an architectural level (walls, rooms, openings, clearances, roofs).
    static ValidationReport validateLevel(const kalara::architecture::Level& level);

private:
    static void validateGeometry(const kalara::architecture::Level& level, ValidationReport& report);
    static void validateConnections(const kalara::architecture::Level& level, ValidationReport& report);
    static void validateRooms(const kalara::architecture::Level& level, ValidationReport& report);
    static void validateClearances(const kalara::architecture::Level& level, ValidationReport& report);
    static void validateRoofs(const kalara::architecture::Level& level, ValidationReport& report);
    static void validateSiteAndSetbacks(const kalara::architecture::Site& site,
                                       const kalara::architecture::Building& building,
                                       ValidationReport& report);
};

} // namespace kalara::runtime
