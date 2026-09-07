#pragma once
#include "runtime/command_service.h"
#include <optional>

namespace kalara::architecture {
struct WallSpec {
    geometry::Length thickness{200};
    std::string wallType{"Exterior 200"};
    ReferenceLine referenceLine=ReferenceLine::center;
};
struct WallEndpoint {
    geometry::Point2 position;
    std::optional<EntityId> junctionId;
};
class WallAuthoring {
public:
    WallAuthoring(runtime::ProjectStore& store,runtime::CommandService& commands):store_(store),commands_(commands){}
    runtime::CommitResult addSegment(const EntityId& floorId,const EntityId& layerId,WallEndpoint start,WallEndpoint end,const WallSpec& spec={});
    runtime::CommitResult addRectangle(const EntityId& floorId,const EntityId& layerId,geometry::Point2 first,geometry::Point2 opposite,const WallSpec& spec={});
    runtime::CommitResult moveJunction(const EntityId& junctionId,geometry::Point2 position);
    runtime::CommitResult translateWall(const EntityId& wallId,geometry::Vector2 delta);
    runtime::CommitResult changeThickness(const EntityId& wallId,geometry::Length thickness,ReferenceLine preserve,std::string wallType={});
private:
    runtime::CommitResult apply(runtime::CommandRequest request);
    runtime::CommitResult failure(std::string code,std::string message)const;
    runtime::ProjectStore& store_;runtime::CommandService& commands_;
};
}
