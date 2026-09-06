#pragma once
#include "core/diagnostic.h"
#include "core/geometry.h"
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace kalara::architecture {
class EntityId {
public:
    explicit EntityId(std::string value);
    static EntityId generate();
    const std::string& str() const { return value_; }
    auto operator<=>(const EntityId&) const = default;
private:
    std::string value_;
};
enum class EntityType { site, building, floor, layer, junction, wall };
struct EntityHeader {
    EntityId id;
    EntityType type;
    EntityId ownerId;
    std::optional<EntityId> layerId;
    std::uint64_t createdRevision = 0;
    std::uint64_t modifiedRevision = 0;
};
struct Site { EntityHeader header; std::optional<geometry::Polygon2> boundary; geometry::Angle north; };
struct Building { EntityHeader header; std::string name; std::vector<EntityId> floorIds; };
struct Floor { EntityHeader header; std::string name; int order = 0; std::optional<geometry::Length> elevation; std::vector<EntityId> entityIds; };
struct Layer { EntityHeader header; std::string name; bool visible = true; bool locked = false; bool printable = true; };
struct Junction { EntityHeader header; EntityId floorId; geometry::Point2 position; };
enum class ReferenceLine { center, left, right };
enum class SideConvention { tangentLeftRight };
struct Wall {
    EntityHeader header;
    EntityId floorId;
    EntityId startJunctionId;
    EntityId endJunctionId;
    geometry::Length thickness;
    ReferenceLine referenceLine = ReferenceLine::center;
    SideConvention sideConvention = SideConvention::tangentLeftRight;
    std::string wallType;
};
using Entity = std::variant<Site, Building, Floor, Layer, Junction, Wall>;
enum class IntentState { undecided, known, omitted };
template<class T> struct IntentValue { IntentState state = IntentState::undecided; std::optional<T> value; };
enum class LengthDisplayUnit { mm, cm, m, inch, foot };
struct DisplaySettings { LengthDisplayUnit lengthUnit = LengthDisplayUnit::mm; int lengthDecimals = 0; };
struct Project {
    static constexpr int currentSchemaVersion = 1;
    int schemaVersion = currentSchemaVersion;
    EntityId id;
    std::string title;
    std::uint64_t revision = 0;
    DisplaySettings display;
    IntentValue<double> targetAreaSquareMm;
    std::map<std::string, std::string> extensions;
    std::map<EntityId, Entity> entities;
    explicit Project(EntityId id);
    const Entity* find(const EntityId& id) const;
};
const EntityHeader& header(const Entity& entity);
std::string_view entityTypeName(EntityType type);
std::vector<Diagnostic> validate(const Project& project);
bool semanticallyEqual(const Project& first, const Project& second);
bool entitySemanticallyEqual(const Entity& first, const Entity& second);
// Ignores project/entity revision metadata while comparing persistent content.
bool semanticContentEqual(const Project& first, const Project& second);
}
