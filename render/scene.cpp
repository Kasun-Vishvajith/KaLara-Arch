#include "render/scene.h"
#include <algorithm>
#include <limits>
namespace kalara::render {
namespace {
bool overlaps(geometry::AABB2 a,geometry::AABB2 b){return a.min.x<=b.max.x&&a.max.x>=b.min.x&&a.min.y<=b.max.y&&a.max.y>=b.min.y;}
geometry::AABB2 box(geometry::Point2 a,geometry::Point2 b,double pad){return {{std::min(a.x,b.x)-pad,std::min(a.y,b.y)-pad},{std::max(a.x,b.x)+pad,std::max(a.y,b.y)+pad}};}
void expand(std::optional<geometry::AABB2>& bounds,geometry::Point2 p){if(!bounds){bounds=geometry::AABB2{p,p};return;}bounds->min.x=std::min(bounds->min.x,p.x);bounds->min.y=std::min(bounds->min.y,p.y);bounds->max.x=std::max(bounds->max.x,p.x);bounds->max.y=std::max(bounds->max.y,p.y);}
}
Scene2D SceneBuilder::build(const architecture::Project& snapshot,const ViewSpec& view)const{
    Scene2D scene;
    SceneIndex index;index.rebuild(snapshot);
    for(const auto& id:index.query(view.worldBounds,view.floorId)){const auto& value=*snapshot.find(id);const auto* wall=std::get_if<architecture::Wall>(&value);if(!wall)continue;
        const auto* startValue=snapshot.find(wall->startJunctionId);const auto* endValue=snapshot.find(wall->endJunctionId);if(!startValue||!endValue)continue;
        const auto start=std::get<architecture::Junction>(*startValue).position,end=std::get<architecture::Junction>(*endValue).position;
        if(!overlaps(box(start,end,wall->thickness.mm/2),view.worldBounds))continue;
        const auto direction=end-start;const auto length=geometry::norm(direction);const geometry::Vector2 normal{-direction.y/length*wall->thickness.mm/2,direction.x/length*wall->thickness.mm/2};
        geometry::Polygon2 polygon{{start+normal*-1,end+normal*-1,end+normal,start+normal},{}};
        scene.fills.push_back({polygon,{224,226,230,255},snapshot.revision,id});
        scene.lines.push_back({polygon.outer[0],polygon.outer[1],0.35,{32,40,51,255},snapshot.revision,id});scene.lines.push_back({polygon.outer[3],polygon.outer[2],0.35,{32,40,51,255},snapshot.revision,id});
        for(auto p:polygon.outer)expand(scene.bounds,p);
    }
    return scene;
}
void SceneIndex::rebuild(const architecture::Project& snapshot){entries_.clear();for(const auto& [id,value]:snapshot.entities){const auto* wall=std::get_if<architecture::Wall>(&value);if(!wall)continue;const auto* a=snapshot.find(wall->startJunctionId);const auto* b=snapshot.find(wall->endJunctionId);if(!a||!b)continue;entries_.push_back({id,wall->floorId,box(std::get<architecture::Junction>(*a).position,std::get<architecture::Junction>(*b).position,wall->thickness.mm/2)});}}
std::vector<architecture::EntityId> SceneIndex::query(geometry::AABB2 worldBounds,std::optional<architecture::EntityId> floorId)const{std::vector<architecture::EntityId> result;for(const auto& entry:entries_)if((!floorId||entry.floorId==*floorId)&&overlaps(entry.bounds,worldBounds))result.push_back(entry.id);return result;}
}
