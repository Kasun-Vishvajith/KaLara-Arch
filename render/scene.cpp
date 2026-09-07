#include "render/scene.h"
#include <algorithm>
#include <cmath>
#include <limits>
namespace kalara::render {
namespace {
bool overlaps(geometry::AABB2 a,geometry::AABB2 b){return a.min.x<=b.max.x&&a.max.x>=b.min.x&&a.min.y<=b.max.y&&a.max.y>=b.min.y;}
geometry::AABB2 box(geometry::Point2 a,geometry::Point2 b,double pad){return {{std::min(a.x,b.x)-pad,std::min(a.y,b.y)-pad},{std::max(a.x,b.x)+pad,std::max(a.y,b.y)+pad}};}
void expand(std::optional<geometry::AABB2>& bounds,geometry::Point2 p){if(!bounds){bounds=geometry::AABB2{p,p};return;}bounds->min.x=std::min(bounds->min.x,p.x);bounds->min.y=std::min(bounds->min.y,p.y);bounds->max.x=std::max(bounds->max.x,p.x);bounds->max.y=std::max(bounds->max.y,p.y);}
struct WallShape{architecture::EntityId id;geometry::Polygon2 polygon;};
bool filled(geometry::Point2 point,const std::vector<WallShape>& shapes){for(const auto& shape:shapes)if(geometry::locate(point,shape.polygon)!=geometry::Location::outside)return true;return false;}
double endpointExtension(const architecture::Project& project,const architecture::Wall& wall,const architecture::EntityId& junctionId){std::vector<geometry::Vector2> rays;double largest=wall.thickness.mm;const auto joint=std::get<architecture::Junction>(*project.find(junctionId)).position;for(const auto& [id,value]:project.entities)if(const auto* candidate=std::get_if<architecture::Wall>(&value);candidate&&candidate->floorId==wall.floorId&&(candidate->startJunctionId==junctionId||candidate->endJunctionId==junctionId)){const auto otherId=candidate->startJunctionId==junctionId?candidate->endJunctionId:candidate->startJunctionId;const auto other=std::get<architecture::Junction>(*project.find(otherId)).position;const auto vector=other-joint;const auto length=geometry::norm(vector);if(length>geometry::epsilon(length))rays.push_back(vector*(1/length));largest=std::max(largest,candidate->thickness.mm);}if(rays.size()<2)return 0;if(rays.size()>2)return largest/2;const double cosine=std::clamp(geometry::dot(rays[0],rays[1]),-1.0,1.0),angle=std::acos(cosine);if(angle<=1e-8)return 4*largest;return std::min(4*largest,(largest/2)/std::tan(angle/2));}
}
Scene2D SceneBuilder::build(const architecture::Project& snapshot,const ViewSpec& view)const{
    Scene2D scene;std::vector<WallShape> shapes;
    SceneIndex index;index.rebuild(snapshot);
    for(const auto& id:index.query(view.worldBounds,view.floorId)){const auto& value=*snapshot.find(id);const auto* wall=std::get_if<architecture::Wall>(&value);if(!wall)continue;
        const auto* startValue=snapshot.find(wall->startJunctionId);const auto* endValue=snapshot.find(wall->endJunctionId);if(!startValue||!endValue)continue;
        const auto start=std::get<architecture::Junction>(*startValue).position,end=std::get<architecture::Junction>(*endValue).position;
        if(!overlaps(box(start,end,wall->thickness.mm/2),view.worldBounds))continue;
        const auto direction=end-start;const auto length=geometry::norm(direction);const auto unit=direction*(1/length);const geometry::Vector2 normal{-unit.y*wall->thickness.mm/2,unit.x*wall->thickness.mm/2};const auto extendedStart=start+unit*-endpointExtension(snapshot,*wall,wall->startJunctionId),extendedEnd=end+unit*endpointExtension(snapshot,*wall,wall->endJunctionId);
        geometry::Polygon2 polygon{{extendedStart+normal*-1,extendedEnd+normal*-1,extendedEnd+normal,extendedStart+normal},{}};
        scene.fills.push_back({polygon,{224,226,230,255},snapshot.revision,id});
        shapes.push_back({id,polygon});
        for(auto p:polygon.outer)expand(scene.bounds,p);
    }
    for(const auto& shape:shapes)for(std::size_t edgeIndex=0;edgeIndex<shape.polygon.outer.size();++edgeIndex){const geometry::Segment2 edge(shape.polygon.outer[edgeIndex],shape.polygon.outer[(edgeIndex+1)%shape.polygon.outer.size()]);std::vector<double> cuts{0,1};for(const auto& other:shapes)for(std::size_t j=0;j<other.polygon.outer.size();++j){const geometry::Segment2 otherEdge(other.polygon.outer[j],other.polygon.outer[(j+1)%other.polygon.outer.size()]);for(const auto point:geometry::intersections(edge,otherEdge)){const auto parameter=geometry::projectionParameter(point,edge);if(parameter>0&&parameter<1)cuts.push_back(parameter);}}std::sort(cuts.begin(),cuts.end());cuts.erase(std::unique(cuts.begin(),cuts.end(),[](double a,double b){return std::abs(a-b)<1e-10;}),cuts.end());const auto vector=edge.b-edge.a;const auto length=geometry::norm(vector);const geometry::Vector2 normal{-vector.y/length,vector.x/length};const double probe=std::max(geometry::epsilon(length)*10,1e-5);for(std::size_t i=1;i<cuts.size();++i){if(cuts[i]-cuts[i-1]<1e-10)continue;const auto a=edge.a+vector*cuts[i-1],b=edge.a+vector*cuts[i],middle=a+(b-a)*.5;if(filled(middle+normal*probe,shapes)==filled(middle+normal*-probe,shapes))continue;bool duplicate=false;for(const auto& line:scene.lines)if((geometry::near(line.a,a,length)&&geometry::near(line.b,b,length))||(geometry::near(line.a,b,length)&&geometry::near(line.b,a,length))){duplicate=true;break;}if(!duplicate)scene.lines.push_back({a,b,0.35,{32,40,51,255},snapshot.revision,shape.id});}}
    return scene;
}
void SceneIndex::rebuild(const architecture::Project& snapshot){entries_.clear();for(const auto& [id,value]:snapshot.entities){const auto* wall=std::get_if<architecture::Wall>(&value);if(!wall)continue;const auto* a=snapshot.find(wall->startJunctionId);const auto* b=snapshot.find(wall->endJunctionId);if(!a||!b)continue;entries_.push_back({id,wall->floorId,box(std::get<architecture::Junction>(*a).position,std::get<architecture::Junction>(*b).position,wall->thickness.mm/2)});}}
std::vector<architecture::EntityId> SceneIndex::query(geometry::AABB2 worldBounds,std::optional<architecture::EntityId> floorId)const{std::vector<architecture::EntityId> result;for(const auto& entry:entries_)if((!floorId||entry.floorId==*floorId)&&overlaps(entry.bounds,worldBounds))result.push_back(entry.id);return result;}
}
