#include "kalara/exporters/json_exporter.hpp"
#include "kalara/architecture/wall.hpp"
#include "kalara/architecture/opening.hpp"
#include "kalara/architecture/room.hpp"
#include "kalara/architecture/annotation.hpp"
#include "kalara/core/json.hpp"
#include <fstream>

namespace kalara::exporters {

using namespace kalara::core;
using namespace kalara::architecture;

static JsonObject pointToJson(const kalara::core::geometry::Point2D& pt) {
    JsonObject obj;
    obj["x"] = pt.x;
    obj["y"] = pt.y;
    return obj;
}

std::string JsonInterchangeExporter::exportProjectToJson(const Project& project, int indent) {
    JsonObject root;
    root["format"] = "KaLara-Interchange-JSON";
    root["schema_version"] = "1.0";
    root["canonical_unit"] = "millimetre";
    root["unit_symbol"] = "mm";

    JsonObject projObj;
    projObj["id"] = project.id.string();
    projObj["name"] = project.name;

    JsonArray sitesArr;
    for (const auto& site : project.sites()) {
        JsonObject siteObj;
        siteObj["id"] = site->id.string();
        siteObj["name"] = site->name;
        siteObj["north_angle_deg"] = site->northAngle_deg;

        // Property Boundary
        JsonArray propBoundary;
        for (const auto& p : site->propertyBoundary) {
            propBoundary.push_back(JsonValue(pointToJson(p)));
        }
        siteObj["property_boundary"] = JsonValue(std::move(propBoundary));

        // Setbacks
        JsonObject setbacksObj;
        setbacksObj["front_mm"] = site->setbacks.front_mm;
        setbacksObj["rear_mm"] = site->setbacks.rear_mm;
        setbacksObj["left_mm"] = site->setbacks.sideLeft_mm;
        setbacksObj["right_mm"] = site->setbacks.sideRight_mm;
        siteObj["setbacks"] = JsonValue(std::move(setbacksObj));

        // Buildings
        JsonArray bldsArr;
        for (const auto& bld : site->buildings()) {
            JsonObject bldObj;
            bldObj["id"] = bld->id.string();
            bldObj["name"] = bld->name;

            JsonArray levelsArr;
            for (const auto& lvl : bld->levels()) {
                JsonObject lvlObj;
                lvlObj["id"] = lvl->id.string();
                lvlObj["name"] = lvl->name;
                lvlObj["elevation_mm"] = lvl->elevation_mm;
                lvlObj["height_mm"] = lvl->height_mm;

                // Walls
                JsonArray wallsArr;
                for (const auto& w : lvl->walls()) {
                    JsonObject wObj;
                    wObj["id"] = w->id.string();
                    wObj["start"] = JsonValue(pointToJson(w->start));
                    wObj["end"] = JsonValue(pointToJson(w->end));
                    wObj["thickness_mm"] = w->thickness_mm;
                    wObj["length_mm"] = w->length_mm();
                    wallsArr.push_back(JsonValue(std::move(wObj)));
                }
                lvlObj["walls"] = JsonValue(std::move(wallsArr));

                // Rooms
                JsonArray roomsArr;
                for (const auto& r : lvl->rooms()) {
                    JsonObject rObj;
                    rObj["id"] = r->id.string();
                    rObj["name"] = r->name;
                    rObj["type"] = std::string(roomTypeName(r->type));
                    JsonArray boundaryArr;
                    for (const auto& bp : r->boundary) {
                        boundaryArr.push_back(JsonValue(pointToJson(bp)));
                    }
                    rObj["boundary"] = JsonValue(std::move(boundaryArr));
                    roomsArr.push_back(JsonValue(std::move(rObj)));
                }
                lvlObj["rooms"] = JsonValue(std::move(roomsArr));

                // Doors
                JsonArray doorsArr;
                for (const auto& d : lvl->doors()) {
                    JsonObject dObj;
                    dObj["id"] = d->id.string();
                    dObj["host_wall_id"] = d->hostWallId.string();
                    dObj["offset_mm"] = d->offsetAlongWall_mm;
                    dObj["width_mm"] = d->width_mm;
                    dObj["height_mm"] = d->height_mm;
                    doorsArr.push_back(JsonValue(std::move(dObj)));
                }
                lvlObj["doors"] = JsonValue(std::move(doorsArr));

                // Windows
                JsonArray winsArr;
                for (const auto& win : lvl->windows()) {
                    JsonObject winObj;
                    winObj["id"] = win->id.string();
                    winObj["host_wall_id"] = win->hostWallId.string();
                    winObj["offset_mm"] = win->offsetAlongWall_mm;
                    winObj["width_mm"] = win->width_mm;
                    winObj["height_mm"] = win->height_mm;
                    winObj["sill_mm"] = win->sillHeight_mm;
                    winsArr.push_back(JsonValue(std::move(winObj)));
                }
                lvlObj["windows"] = JsonValue(std::move(winsArr));

                // Dimensions
                JsonArray dimsArr;
                for (const auto& dim : lvl->dimensions()) {
                    JsonObject dimObj;
                    dimObj["id"] = dim->id.string();
                    dimObj["point1"] = JsonValue(pointToJson(dim->point1));
                    dimObj["point2"] = JsonValue(pointToJson(dim->point2));
                    dimObj["measured_mm"] = dim->measuredDistance_mm();
                    dimsArr.push_back(JsonValue(std::move(dimObj)));
                }
                lvlObj["dimensions"] = JsonValue(std::move(dimsArr));

                levelsArr.push_back(JsonValue(std::move(lvlObj)));
            }
            bldObj["levels"] = JsonValue(std::move(levelsArr));
            bldsArr.push_back(JsonValue(std::move(bldObj)));
        }
        siteObj["buildings"] = JsonValue(std::move(bldsArr));
        sitesArr.push_back(JsonValue(std::move(siteObj)));
    }
    projObj["sites"] = JsonValue(std::move(sitesArr));
    root["project"] = JsonValue(std::move(projObj));

    return JsonValue(std::move(root)).dump(indent);
}

bool JsonInterchangeExporter::exportProjectToFile(const Project& project, const std::string& filePath, int indent) {
    std::string jsonStr = exportProjectToJson(project, indent);
    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return false;
    out << jsonStr;
    return out.good();
}

} // namespace kalara::exporters
