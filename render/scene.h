#pragma once
#include "architecture/model.h"
#include <string>
#include <vector>
namespace kalara::render {
struct Color { unsigned char r=0,g=0,b=0,a=255; auto operator<=>(const Color&) const = default; };
struct LinePrimitive { geometry::Point2 a,b; double widthMm=0.25; Color color; std::uint64_t sourceRevision=0; };
struct FillPrimitive { geometry::Polygon2 polygon; Color color; std::uint64_t sourceRevision=0; };
struct TextPrimitive { geometry::Point2 anchor; std::string text; double heightMm=2.5; Color color; };
struct Scene2D { std::vector<LinePrimitive> lines; std::vector<FillPrimitive> fills; std::vector<TextPrimitive> texts; std::optional<geometry::AABB2> bounds; };
struct ViewSpec { geometry::AABB2 worldBounds; std::optional<architecture::EntityId> floorId; };
class SceneIndex {
public:
    void rebuild(const architecture::Project& snapshot);
    std::vector<architecture::EntityId> query(geometry::AABB2 worldBounds,std::optional<architecture::EntityId> floorId={})const;
private:
    struct Entry { architecture::EntityId id; architecture::EntityId floorId; geometry::AABB2 bounds; };
    std::vector<Entry> entries_;
};
class SceneBuilder {
public:
    Scene2D build(const architecture::Project& snapshot,const ViewSpec& view) const;
};
}
