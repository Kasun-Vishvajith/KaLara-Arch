#pragma once
#include "kalara/architecture/entity_id.hpp"
#include "kalara/architecture/metadata.hpp"
#include "kalara/architecture/semantics.hpp"
#include "kalara/architecture/site.hpp"
#include "kalara/core/geometry/units.hpp"
#include <string>
#include <vector>
#include <memory>

namespace kalara::architecture {

/// Root Architectural Project entity (Section 6).
class Project {
public:
    EntityId id;
    std::string name;
    kalara::core::geometry::UnitSystem displayUnit = kalara::core::geometry::UnitSystem::Millimetres;
    Metadata metadata;

    Project() : id(EntityId::generate("proj")), name("Untitled Project") {
        // A project has at least one default site by default
        addSite("Site 1");
    }

    explicit Project(std::string projectName)
        : id(EntityId::generate("proj")), name(std::move(projectName)) {
        addSite("Site 1");
    }

    [[nodiscard]] SemanticType semanticType() const noexcept { return SemanticType::Project; }

    Site& addSite(std::string siteName = "Site 1") {
        m_sites.emplace_back(std::make_unique<Site>(std::move(siteName)));
        return *m_sites.back();
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Site>>& sites() const noexcept {
        return m_sites;
    }

    [[nodiscard]] Site* findSite(const EntityId& siteId) const noexcept {
        for (const auto& s : m_sites) {
            if (s->id == siteId) return s.get();
        }
        return nullptr;
    }

    [[nodiscard]] Site* defaultSite() const noexcept {
        return m_sites.empty() ? nullptr : m_sites.front().get();
    }

private:
    std::vector<std::unique_ptr<Site>> m_sites;
};

} // namespace kalara::architecture
