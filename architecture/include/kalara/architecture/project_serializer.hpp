#pragma once
#include "kalara/architecture/project.hpp"
#include "kalara/core/json.hpp"
#include <string>
#include <string_view>
#include <memory>
#include <filesystem>
#include <optional>

namespace kalara::architecture {

/// Native project file format constants (Section 10 & Step 15).
inline constexpr std::string_view KLA_FORMAT_NAME = "KaLaraArchNative";
inline constexpr std::string_view KLA_FILE_EXTENSION = ".kla";
inline constexpr std::string_view KLA_FORMAT_VERSION = "1.0.0";
inline constexpr int KLA_CURRENT_SCHEMA_VERSION = 1;

/// Structured project serialization, native .kla file representation,
/// schema/version metadata, and migration foundation (Step 15 & Rule 4).
class ProjectSerializer {
public:
    /// Serialize an architectural Project model into a JsonObject DOM.
    static core::JsonObject serializeProject(const Project& project);

    /// Deserialize a JsonObject DOM into a Project model.
    static std::unique_ptr<Project> deserializeProject(const core::JsonObject& root, std::string* errorOut = nullptr);

    /// Serialize Project directly to formatted JSON string.
    static std::string serializeToString(const Project& project, int indent = 2);

    /// Deserialize Project directly from JSON string.
    static std::unique_ptr<Project> deserializeFromString(std::string_view jsonStr, std::string* errorOut = nullptr);

    /// Save Project to native `.kla` file on disk.
    static bool saveToFile(const Project& project, const std::filesystem::path& path, int indent = 2);

    /// Load Project from native `.kla` file on disk.
    static std::unique_ptr<Project> loadFromFile(const std::filesystem::path& path, std::string* errorOut = nullptr);

    /// Migration foundation: upgrade an older schema version to current (Step 15).
    static bool migrateSchema(core::JsonObject& root, int fromVersion, int toVersion, std::string* errorOut = nullptr);

    /// Verify semantic equivalence between two projects (Acceptance Criteria Step 15: state -> save -> load -> semantically equivalent state).
    static bool areSemanticallyEquivalent(const Project& a, const Project& b, double tol_mm = 1.0);
};

} // namespace kalara::architecture
