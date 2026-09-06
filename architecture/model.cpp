#include "architecture/model.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace kalara::architecture {
namespace {
bool validId(const std::string& value) {
    if (value.empty() || value.size() > 128) return false;
    return std::all_of(value.begin(), value.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '-' || c == '_' || c == '.';
    });
}
Diagnostic error(std::string code, std::string message, const EntityId* id = nullptr, std::string field = {}) {
    Diagnostic result{std::move(code), Severity::error, std::move(message), {}, std::move(field), "Correct the project data and try again"};
    if (id) {
        // Entity IDs are opaque strings; Diagnostic's legacy numeric field is not used here.
        result.suggestedAction += " (entity " + id->str() + ")";
    }
    return result;
}
bool equalPoint(geometry::Point2 a, geometry::Point2 b) { return a.x == b.x && a.y == b.y; }
bool equalPolygon(const geometry::Polygon2& a, const geometry::Polygon2& b) {
    if (a.outer.size() != b.outer.size() || a.holes.size() != b.holes.size()) return false;
    for (std::size_t i=0;i<a.outer.size();++i) if (!equalPoint(a.outer[i],b.outer[i])) return false;
    for (std::size_t h=0;h<a.holes.size();++h) {
        if (a.holes[h].size()!=b.holes[h].size()) return false;
        for (std::size_t i=0;i<a.holes[h].size();++i) if(!equalPoint(a.holes[h][i],b.holes[h][i])) return false;
    }
    return true;
}
bool equalEntity(const Entity& a, const Entity& b) {
    if (a.index()!=b.index()) return false;
    return std::visit([&](const auto& left) {
        using T=std::decay_t<decltype(left)>;
        const auto& right=std::get<T>(b);
        const auto headerEqual=left.header.id==right.header.id && left.header.type==right.header.type &&
            left.header.ownerId==right.header.ownerId && left.header.layerId==right.header.layerId &&
            left.header.createdRevision==right.header.createdRevision && left.header.modifiedRevision==right.header.modifiedRevision;
        if(!headerEqual)return false;
        if constexpr(std::is_same_v<T,Site>) return left.north.radians==right.north.radians && left.boundary.has_value()==right.boundary.has_value() && (!left.boundary || equalPolygon(*left.boundary,*right.boundary));
        if constexpr(std::is_same_v<T,Building>) return left.name==right.name && left.floorIds==right.floorIds;
        if constexpr(std::is_same_v<T,Floor>) return left.name==right.name && left.order==right.order && left.elevation.has_value()==right.elevation.has_value() && (!left.elevation || left.elevation->mm==right.elevation->mm) && left.entityIds==right.entityIds;
        if constexpr(std::is_same_v<T,Layer>) return left.name==right.name && left.visible==right.visible && left.locked==right.locked && left.printable==right.printable;
        if constexpr(std::is_same_v<T,Junction>) return left.floorId==right.floorId && equalPoint(left.position,right.position);
        if constexpr(std::is_same_v<T,Wall>) return left.floorId==right.floorId && left.startJunctionId==right.startJunctionId && left.endJunctionId==right.endJunctionId && left.thickness.mm==right.thickness.mm && left.referenceLine==right.referenceLine && left.sideConvention==right.sideConvention && left.wallType==right.wallType;
    },a);
}
}
EntityId::EntityId(std::string value):value_(std::move(value)){if(!validId(value_))throw std::invalid_argument("Entity ID is empty or contains unsupported characters");}
EntityId EntityId::generate(){
    std::array<unsigned char,16> bytes{};std::random_device source;for(auto& byte:bytes)byte=static_cast<unsigned char>(source());
    bytes[6]=(bytes[6]&0x0f)|0x40;bytes[8]=(bytes[8]&0x3f)|0x80;
    std::ostringstream out;out<<std::hex<<std::setfill('0');for(std::size_t i=0;i<bytes.size();++i){if(i==4||i==6||i==8||i==10)out<<'-';out<<std::setw(2)<<static_cast<int>(bytes[i]);}
    return EntityId(out.str());
}
Project::Project(EntityId projectId):id(std::move(projectId)){}
const Entity* Project::find(const EntityId& entityId)const{auto it=entities.find(entityId);return it==entities.end()?nullptr:&it->second;}
const EntityHeader& header(const Entity& entity){return std::visit([](const auto& value)->const EntityHeader&{return value.header;},entity);}
std::string_view entityTypeName(EntityType type){switch(type){case EntityType::site:return"site";case EntityType::building:return"building";case EntityType::floor:return"floor";case EntityType::layer:return"layer";case EntityType::junction:return"junction";case EntityType::wall:return"wall";}return"unknown";}
std::vector<Diagnostic> validate(const Project& project){
    std::vector<Diagnostic> errors;
    if(project.schemaVersion!=Project::currentSchemaVersion)errors.push_back(error("schema.unsupported","Unsupported schema version",nullptr,"schemaVersion"));
    if(project.title.empty())errors.push_back(error("project.title.empty","Project title is required",nullptr,"title"));
    if(project.revision>9007199254740991ULL)errors.push_back(error("project.revision.range","Project revision exceeds JSON's exact integer range",nullptr,"revision"));
    if(project.display.lengthDecimals<0||project.display.lengthDecimals>9)errors.push_back(error("display.decimals.range","Length decimals must be between zero and nine",nullptr,"display.lengthDecimals"));
    if(project.targetAreaSquareMm.state==IntentState::known && (!project.targetAreaSquareMm.value || !std::isfinite(*project.targetAreaSquareMm.value) || *project.targetAreaSquareMm.value<0))errors.push_back(error("intent.value.invalid","Known target area requires a finite nonnegative value",nullptr,"intent.targetArea"));
    if(project.targetAreaSquareMm.state!=IntentState::known && project.targetAreaSquareMm.value)errors.push_back(error("intent.value.unexpected","Undecided or omitted intent cannot carry a value",nullptr,"intent.targetArea"));
    auto typed=[&](const EntityId& id,EntityType expected,const EntityId& source,std::string field)->bool{
        const auto* target=project.find(id);if(!target){errors.push_back(error("reference.missing","Referenced entity does not exist",&source,std::move(field)));return false;}
        if(header(*target).type!=expected){errors.push_back(error("reference.type","Referenced entity has the wrong type",&source,std::move(field)));return false;}return true;
    };
    for(const auto& [key,entity]:project.entities){const auto& h=header(entity);
        if(key!=h.id)errors.push_back(error("entity.key_mismatch","Entity map key must match its ID",&h.id,"id"));
        if(h.createdRevision>h.modifiedRevision||h.modifiedRevision>project.revision)errors.push_back(error("entity.revision.invalid","Entity revisions exceed project revision",&h.id,"modifiedRevision"));
        if(h.layerId)typed(*h.layerId,EntityType::layer,h.id,"layerId");
        std::visit([&](const auto& value){using T=std::decay_t<decltype(value)>;
            if constexpr(std::is_same_v<T,Site>){if(value.boundary)if(auto issue=geometry::validate(*value.boundary))errors.push_back(error("site.boundary.invalid",*issue,&h.id,"boundary"));}
            if constexpr(std::is_same_v<T,Building>){if(h.ownerId!=project.id)errors.push_back(error("scope.project","Building owner must be the project",&h.id,"ownerId"));for(const auto& id:value.floorIds){if(typed(id,EntityType::floor,h.id,"floorIds")&&header(*project.find(id)).ownerId!=h.id)errors.push_back(error("scope.owner","Floor has another building owner",&h.id,"floorIds"));}}
            if constexpr(std::is_same_v<T,Floor>){typed(h.ownerId,EntityType::building,h.id,"ownerId");for(const auto& id:value.entityIds){if(const auto* target=project.find(id)){if(header(*target).ownerId!=value.header.id)errors.push_back(error("scope.owner","Floor member has another owner",&h.id,"entityIds"));}else errors.push_back(error("reference.missing","Floor member does not exist",&h.id,"entityIds"));}}
            if constexpr(std::is_same_v<T,Layer>){if(h.ownerId!=project.id)errors.push_back(error("scope.project","Layer owner must be the project",&h.id,"ownerId"));}
            if constexpr(std::is_same_v<T,Junction>){typed(value.floorId,EntityType::floor,h.id,"floorId");if(value.header.ownerId!=value.floorId)errors.push_back(error("scope.floor","Junction owner and floor differ",&h.id,"ownerId"));}
            if constexpr(std::is_same_v<T,Wall>){
                typed(value.floorId,EntityType::floor,h.id,"floorId");const bool a=typed(value.startJunctionId,EntityType::junction,h.id,"startJunctionId"),b=typed(value.endJunctionId,EntityType::junction,h.id,"endJunctionId");
                if(value.startJunctionId==value.endJunctionId)errors.push_back(error("wall.junction.same","Wall junctions must be distinct",&h.id,"endJunctionId"));
                if(!std::isfinite(value.thickness.mm)||value.thickness.mm<=geometry::epsilon(value.thickness.mm))errors.push_back(error("wall.thickness.invalid","Wall thickness must be positive",&h.id,"thicknessMm"));
                if(a&&b){const auto& start=std::get<Junction>(*project.find(value.startJunctionId));const auto& end=std::get<Junction>(*project.find(value.endJunctionId));if(start.floorId!=value.floorId||end.floorId!=value.floorId)errors.push_back(error("wall.scope","Wall and junctions must share a floor",&h.id,"floorId"));else if(geometry::near(start.position,end.position,geometry::distance(start.position,end.position).mm))errors.push_back(error("wall.length.zero","Wall length is numerically zero",&h.id,"endJunctionId"));}
            }
        },entity);
    }
    return errors;
}
bool semanticallyEqual(const Project& a,const Project& b){
    if(a.schemaVersion!=b.schemaVersion||a.id!=b.id||a.title!=b.title||a.revision!=b.revision||a.display.lengthUnit!=b.display.lengthUnit||a.display.lengthDecimals!=b.display.lengthDecimals||a.targetAreaSquareMm.state!=b.targetAreaSquareMm.state||a.targetAreaSquareMm.value!=b.targetAreaSquareMm.value||a.extensions!=b.extensions||a.entities.size()!=b.entities.size())return false;
    auto left=a.entities.begin(),right=b.entities.begin();for(;left!=a.entities.end();++left,++right)if(left->first!=right->first||!equalEntity(left->second,right->second))return false;return true;
}
bool entitySemanticallyEqual(const Entity& first,const Entity& second){return equalEntity(first,second);}
bool semanticContentEqual(const Project& first,const Project& second){
    Project a=first,b=second;a.revision=b.revision=0;
    for(auto& [id,entity]:a.entities){auto& value=const_cast<EntityHeader&>(header(entity));value.createdRevision=value.modifiedRevision=0;}
    for(auto& [id,entity]:b.entities){auto& value=const_cast<EntityHeader&>(header(entity));value.createdRevision=value.modifiedRevision=0;}
    return semanticallyEqual(a,b);
}
}
