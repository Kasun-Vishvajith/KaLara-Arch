#include "kalara/architecture/project_serializer.hpp"
#include "kalara/core/logging.hpp"
#include <chrono>
#include <cmath>

namespace kalara::architecture {

namespace {

using namespace kalara::core;

// --- Helper JSON Conversion Functions ---

JsonValue serializePoint(const kalara::core::geometry::Point2D& pt) {
    JsonArray arr;
    arr.emplace_back(pt.x);
    arr.emplace_back(pt.y);
    return JsonValue(std::move(arr));
}

kalara::core::geometry::Point2D deserializePoint(const JsonValue& val) {
    if (!val.isArray() || val.asArray().size() < 2) {
        return kalara::core::geometry::Point2D{0.0, 0.0};
    }
    const auto& arr = val.asArray();
    return kalara::core::geometry::Point2D{arr[0].asDouble(), arr[1].asDouble()};
}

JsonValue serializePolygon(const std::vector<kalara::core::geometry::Point2D>& poly) {
    JsonArray arr;
    for (const auto& pt : poly) {
        arr.push_back(serializePoint(pt));
    }
    return JsonValue(std::move(arr));
}

std::vector<kalara::core::geometry::Point2D> deserializePolygon(const JsonValue& val) {
    std::vector<kalara::core::geometry::Point2D> poly;
    if (!val.isArray()) return poly;
    for (const auto& item : val.asArray()) {
        poly.push_back(deserializePoint(item));
    }
    return poly;
}

JsonValue serializeMetadata(const Metadata& meta) {
    JsonObject obj;
    for (const auto& [k, v] : meta.all()) {
        if (std::holds_alternative<std::string>(v)) obj[k] = std::get<std::string>(v);
        else if (std::holds_alternative<double>(v)) obj[k] = std::get<double>(v);
        else if (std::holds_alternative<int64_t>(v)) obj[k] = std::get<int64_t>(v);
        else if (std::holds_alternative<bool>(v)) obj[k] = std::get<bool>(v);
    }
    return JsonValue(std::move(obj));
}

void deserializeMetadata(const JsonValue& val, Metadata& meta) {
    if (!val.isObject()) return;
    for (const auto& [k, v] : val.asObject()) {
        if (v.isBool()) meta.set(k, v.asBool());
        else if (v.isInt()) meta.set(k, v.asInt());
        else if (v.isDouble()) meta.set(k, v.asDouble());
        else if (v.isString()) meta.set(k, v.asString());
    }
}

// --- Wall Serialization ---

JsonValue serializeWall(const Wall& w) {
    JsonObject obj;
    obj["id"] = w.id.string();
    obj["name"] = w.name;
    obj["start"] = serializePoint(w.start);
    obj["end"] = serializePoint(w.end);
    obj["thickness_mm"] = w.thickness_mm;
    obj["height_mm"] = w.height_mm;
    obj["metadata"] = serializeMetadata(w.metadata);
    return JsonValue(std::move(obj));
}

std::unique_ptr<Wall> deserializeWall(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    auto start = deserializePoint(val["start"]);
    auto end = deserializePoint(val["end"]);
    double thickness = val["thickness_mm"].asDouble(200.0);
    double height = val["height_mm"].asDouble(3000.0);

    auto w = std::make_unique<Wall>(start, end, thickness, height);
    w->id = EntityId(val["id"].asString(EntityId::generate("wall").string()));
    w->name = val["name"].asString("Wall");
    deserializeMetadata(val["metadata"], w->metadata);
    return w;
}

// --- Openings Serialization ---

JsonValue serializeDoor(const Door& d) {
    JsonObject obj;
    obj["id"] = d.id.string();
    obj["name"] = d.name;
    obj["host_wall_id"] = d.hostWallId.string();
    obj["offset_mm"] = d.offsetAlongWall_mm;
    obj["width_mm"] = d.width_mm;
    obj["height_mm"] = d.height_mm;
    obj["sill_height_mm"] = d.sillHeight_mm;
    obj["swing"] = static_cast<int>(d.swing);
    obj["metadata"] = serializeMetadata(d.metadata);
    return JsonValue(std::move(obj));
}

std::unique_ptr<Door> deserializeDoor(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    EntityId host(val["host_wall_id"].asString());
    double offset = val["offset_mm"].asDouble(1000.0);
    double width = val["width_mm"].asDouble(900.0);
    double height = val["height_mm"].asDouble(2100.0);
    auto swing = static_cast<DoorSwing>(val["swing"].asInt(static_cast<int>(DoorSwing::LeftInswing)));

    auto d = std::make_unique<Door>(host, offset, width, height, swing);
    d->id = EntityId(val["id"].asString(EntityId::generate("door").string()));
    d->name = val["name"].asString("Single Door");
    d->sillHeight_mm = val["sill_height_mm"].asDouble(0.0);
    deserializeMetadata(val["metadata"], d->metadata);
    return d;
}

JsonValue serializeWindow(const Window& w) {
    JsonObject obj;
    obj["id"] = w.id.string();
    obj["name"] = w.name;
    obj["host_wall_id"] = w.hostWallId.string();
    obj["offset_mm"] = w.offsetAlongWall_mm;
    obj["width_mm"] = w.width_mm;
    obj["height_mm"] = w.height_mm;
    obj["sill_height_mm"] = w.sillHeight_mm;
    obj["window_type"] = static_cast<int>(w.windowType);
    obj["metadata"] = serializeMetadata(w.metadata);
    return JsonValue(std::move(obj));
}

std::unique_ptr<Window> deserializeWindow(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    EntityId host(val["host_wall_id"].asString());
    double offset = val["offset_mm"].asDouble(1500.0);
    double width = val["width_mm"].asDouble(1200.0);
    double height = val["height_mm"].asDouble(1500.0);
    double sill = val["sill_height_mm"].asDouble(900.0);
    auto type = static_cast<WindowType>(val["window_type"].asInt(static_cast<int>(WindowType::Casement)));

    auto w = std::make_unique<Window>(host, offset, width, height, sill, type);
    w->id = EntityId(val["id"].asString(EntityId::generate("win").string()));
    w->name = val["name"].asString("Standard Window");
    deserializeMetadata(val["metadata"], w->metadata);
    return w;
}

// --- Room Serialization ---

JsonValue serializeRoom(const Room& r) {
    JsonObject obj;
    obj["id"] = r.id.string();
    obj["name"] = r.name;
    obj["type"] = static_cast<int>(r.type);
    obj["boundary"] = serializePolygon(r.boundary);
    if (!r.boundaryWallIds.empty()) {
        JsonArray bWalls;
        for (const auto& wid : r.boundaryWallIds) {
            bWalls.emplace_back(wid.string());
        }
        obj["boundary_wall_ids"] = JsonValue(std::move(bWalls));
    }
    obj["metadata"] = serializeMetadata(r.metadata);
    return JsonValue(std::move(obj));
}

std::unique_ptr<Room> deserializeRoom(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    std::string name = val["name"].asString("Living Room");
    auto type = static_cast<RoomType>(val["type"].asInt(static_cast<int>(RoomType::LivingRoom)));
    auto poly = deserializePolygon(val["boundary"]);

    auto r = std::make_unique<Room>(std::move(name), type, std::move(poly));
    r->id = EntityId(val["id"].asString(EntityId::generate("room").string()));
    if (val["boundary_wall_ids"].isArray()) {
        for (const auto& widVal : val["boundary_wall_ids"].asArray()) {
            r->boundaryWallIds.emplace_back(widVal.asString());
        }
    }
    deserializeMetadata(val["metadata"], r->metadata);
    return r;
}

// --- Annotation Serialization ---

JsonValue serializeDimension(const Dimension& d) {
    JsonObject obj;
    obj["id"] = d.id.string();
    obj["point1"] = serializePoint(d.point1);
    obj["point2"] = serializePoint(d.point2);
    obj["offset_mm"] = d.offsetDistance_mm;
    if (d.referencedEntityId.has_value()) {
        obj["referenced_entity_id"] = d.referencedEntityId.value().string();
    }
    obj["metadata"] = serializeMetadata(d.metadata);
    return JsonValue(std::move(obj));
}

std::unique_ptr<Dimension> deserializeDimension(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    auto p1 = deserializePoint(val["point1"]);
    auto p2 = deserializePoint(val["point2"]);
    double offset = val["offset_mm"].asDouble(500.0);

    auto d = std::make_unique<Dimension>(p1, p2, offset);
    d->id = EntityId(val["id"].asString(EntityId::generate("dim").string()));
    if (val.contains("referenced_entity_id") && !val["referenced_entity_id"].asString().empty()) {
        d->referencedEntityId = EntityId(val["referenced_entity_id"].asString());
    }
    deserializeMetadata(val["metadata"], d->metadata);
    return d;
}

JsonValue serializeNote(const NoteAnnotation& n) {
    JsonObject obj;
    obj["id"] = n.id.string();
    obj["position"] = serializePoint(n.position);
    obj["text"] = n.text;
    obj["metadata"] = serializeMetadata(n.metadata);
    return JsonValue(std::move(obj));
}

std::unique_ptr<NoteAnnotation> deserializeNote(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    auto pos = deserializePoint(val["position"]);
    std::string text = val["text"].asString();

    auto n = std::make_unique<NoteAnnotation>(pos, std::move(text));
    n->id = EntityId(val["id"].asString(EntityId::generate("note").string()));
    deserializeMetadata(val["metadata"], n->metadata);
    return n;
}

// --- Library Instance Serialization ---

JsonValue serializeLibraryInstance(const LibraryInstance& inst) {
    JsonObject obj;
    obj["id"] = inst.id.string();
    obj["item_id"] = inst.itemId;
    obj["name"] = inst.name;
    obj["position"] = serializePoint(inst.position);
    obj["rotation_deg"] = inst.rotation.degrees();
    obj["width_mm"] = inst.width_mm;
    obj["length_mm"] = inst.length_mm;
    obj["clearance_f_mm"] = inst.clearance.front_mm;
    obj["clearance_r_mm"] = inst.clearance.rear_mm;
    obj["clearance_l_mm"] = inst.clearance.left_mm;
    obj["clearance_rt_mm"] = inst.clearance.right_mm;
    obj["semantic_type"] = static_cast<int>(inst.semanticType);
    obj["metadata"] = serializeMetadata(inst.metadata);
    return JsonValue(std::move(obj));
}

std::unique_ptr<LibraryInstance> deserializeLibraryInstance(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    LibraryItem item;
    item.id = val["item_id"].asString();
    item.name = val["name"].asString();
    item.width_mm = val["width_mm"].asDouble(1000.0);
    item.length_mm = val["length_mm"].asDouble(1000.0);
    item.clearance.front_mm = val["clearance_f_mm"].asDouble(0.0);
    item.clearance.rear_mm = val["clearance_r_mm"].asDouble(0.0);
    item.clearance.left_mm = val["clearance_l_mm"].asDouble(0.0);
    item.clearance.right_mm = val["clearance_rt_mm"].asDouble(0.0);
    item.semanticType = static_cast<SemanticType>(val["semantic_type"].asInt(static_cast<int>(SemanticType::Furniture)));

    auto pos = deserializePoint(val["position"]);
    auto rot = kalara::core::geometry::Angle::fromDegrees(val["rotation_deg"].asDouble(0.0));

    auto inst = std::make_unique<LibraryInstance>(item, pos, rot);
    inst->id = EntityId(val["id"].asString(EntityId::generate("lib_inst").string()));
    deserializeMetadata(val["metadata"], inst->metadata);
    return inst;
}

// --- Roof Serialization ---

JsonValue serializeRoof(const Roof& r) {
    JsonObject obj;
    obj["id"] = r.id.string();
    obj["name"] = r.name;
    obj["type"] = static_cast<int>(r.type);
    obj["wall_footprint"] = serializePolygon(r.wallFootprint);
    obj["pitch_deg"] = r.pitch_deg;
    obj["overhang_mm"] = r.overhang_mm;
    obj["fascia_height_mm"] = r.fasciaHeight_mm;
    obj["metadata"] = serializeMetadata(r.metadata);
    return JsonValue(std::move(obj));
}

std::unique_ptr<Roof> deserializeRoof(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    std::string name = val["name"].asString("Roof");
    auto type = static_cast<RoofType>(val["type"].asInt(static_cast<int>(RoofType::Gable)));
    auto poly = deserializePolygon(val["wall_footprint"]);
    double pitch = val["pitch_deg"].asDouble(22.5);
    double overhang = val["overhang_mm"].asDouble(600.0);

    auto r = std::make_unique<Roof>(std::move(name), type, std::move(poly), pitch, overhang);
    r->id = EntityId(val["id"].asString(EntityId::generate("roof").string()));
    r->fasciaHeight_mm = val["fascia_height_mm"].asDouble(200.0);
    deserializeMetadata(val["metadata"], r->metadata);
    return r;
}

// --- Constraint Serialization ---

JsonValue serializeConstraint(const Constraint& c) {
    JsonObject obj;
    obj["id"] = c.id.string();
    obj["name"] = c.name;
    obj["type"] = static_cast<int>(c.type);
    obj["severity"] = static_cast<int>(c.severity);
    JsonArray targets;
    for (const auto& tid : c.targetEntityIds) {
        targets.emplace_back(tid.string());
    }
    obj["targets"] = JsonValue(std::move(targets));
    obj["target_value"] = c.targetValue;
    obj["metadata"] = serializeMetadata(c.metadata);
    return JsonValue(std::move(obj));
}

std::unique_ptr<Constraint> deserializeConstraint(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    std::string name = val["name"].asString();
    auto type = static_cast<ConstraintType>(val["type"].asInt(static_cast<int>(ConstraintType::Coincidence)));
    auto severity = static_cast<ConstraintSeverity>(val["severity"].asInt(static_cast<int>(ConstraintSeverity::Hard)));
    std::vector<EntityId> targets;
    if (val["targets"].isArray()) {
        for (const auto& tid : val["targets"].asArray()) {
            targets.emplace_back(tid.asString());
        }
    }
    double value = val["target_value"].asDouble(0.0);

    auto c = std::make_unique<Constraint>(std::move(name), type, severity, std::move(targets), value);
    c->id = EntityId(val["id"].asString(EntityId::generate("cst").string()));
    deserializeMetadata(val["metadata"], c->metadata);
    return c;
}

// --- Level Serialization ---

JsonValue serializeLevel(const Level& lvl) {
    JsonObject obj;
    obj["id"] = lvl.id.string();
    obj["name"] = lvl.name;
    obj["type"] = static_cast<int>(lvl.type);
    obj["elevation_mm"] = lvl.elevation_mm;
    obj["height_mm"] = lvl.height_mm;
    obj["visible"] = lvl.visible;
    obj["locked"] = lvl.locked;
    if (lvl.underlayLevelId.has_value()) {
        obj["underlay_level_id"] = lvl.underlayLevelId.value().string();
    }
    obj["underlay_opacity"] = lvl.underlayOpacity;
    obj["metadata"] = serializeMetadata(lvl.metadata);

    // Walls
    JsonArray wallsArr;
    for (const auto& w : lvl.walls()) wallsArr.push_back(serializeWall(*w));
    obj["walls"] = JsonValue(std::move(wallsArr));

    // Openings
    JsonArray doorsArr;
    for (const auto& d : lvl.doors()) doorsArr.push_back(serializeDoor(*d));
    obj["doors"] = JsonValue(std::move(doorsArr));

    JsonArray windowsArr;
    for (const auto& w : lvl.windows()) windowsArr.push_back(serializeWindow(*w));
    obj["windows"] = JsonValue(std::move(windowsArr));

    // Rooms
    JsonArray roomsArr;
    for (const auto& r : lvl.rooms()) roomsArr.push_back(serializeRoom(*r));
    obj["rooms"] = JsonValue(std::move(roomsArr));

    // Annotations
    JsonArray dimsArr;
    for (const auto& d : lvl.dimensions()) dimsArr.push_back(serializeDimension(*d));
    obj["dimensions"] = JsonValue(std::move(dimsArr));

    JsonArray notesArr;
    for (const auto& n : lvl.notes()) notesArr.push_back(serializeNote(*n));
    obj["notes"] = JsonValue(std::move(notesArr));

    // Library Instances
    JsonArray instArr;
    for (const auto& inst : lvl.libraryInstances()) instArr.push_back(serializeLibraryInstance(*inst));
    obj["library_instances"] = JsonValue(std::move(instArr));

    // Roofs
    JsonArray roofsArr;
    for (const auto& r : lvl.roofs()) roofsArr.push_back(serializeRoof(*r));
    obj["roofs"] = JsonValue(std::move(roofsArr));

    // Constraints
    JsonArray cstArray;
    for (const auto& c : lvl.constraints()) cstArray.push_back(serializeConstraint(*c));
    obj["constraints"] = JsonValue(std::move(cstArray));

    return JsonValue(std::move(obj));
}

std::unique_ptr<Level> deserializeLevel(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    std::string name = val["name"].asString("Level");
    double elev = val["elevation_mm"].asDouble(0.0);
    double height = val["height_mm"].asDouble(3000.0);
    auto type = static_cast<LevelType>(val["type"].asInt(static_cast<int>(LevelType::GroundFloor)));

    auto lvl = std::make_unique<Level>(std::move(name), elev, height, type);
    lvl->id = EntityId(val["id"].asString(EntityId::generate("lvl").string()));
    lvl->visible = val["visible"].asBool(true);
    lvl->locked = val["locked"].asBool(false);
    if (val.contains("underlay_level_id") && !val["underlay_level_id"].asString().empty()) {
        lvl->underlayLevelId = EntityId(val["underlay_level_id"].asString());
    }
    lvl->underlayOpacity = val["underlay_opacity"].asDouble(0.4);
    deserializeMetadata(val["metadata"], lvl->metadata);

    // Walls
    if (val["walls"].isArray()) {
        for (const auto& wVal : val["walls"].asArray()) {
            if (auto w = deserializeWall(wVal)) {
                auto& added = lvl->addWall(w->start, w->end, w->thickness_mm);
                added.id = w->id;
                added.name = w->name;
                added.height_mm = w->height_mm;
                added.metadata = w->metadata;
            }
        }
    }

    // Openings
    if (val["doors"].isArray()) {
        for (const auto& dVal : val["doors"].asArray()) {
            if (auto d = deserializeDoor(dVal)) {
                auto& added = lvl->addDoor(d->hostWallId, d->offsetAlongWall_mm, d->width_mm, d->height_mm, d->swing);
                added.id = d->id;
                added.name = d->name;
                added.sillHeight_mm = d->sillHeight_mm;
                added.metadata = d->metadata;
            }
        }
    }

    if (val["windows"].isArray()) {
        for (const auto& wVal : val["windows"].asArray()) {
            if (auto w = deserializeWindow(wVal)) {
                auto& added = lvl->addWindow(w->hostWallId, w->offsetAlongWall_mm, w->width_mm, w->height_mm, w->sillHeight_mm, w->windowType);
                added.id = w->id;
                added.name = w->name;
                added.metadata = w->metadata;
            }
        }
    }

    // Rooms
    if (val["rooms"].isArray()) {
        for (const auto& rVal : val["rooms"].asArray()) {
            if (auto r = deserializeRoom(rVal)) {
                auto& added = lvl->addRoom(r->name, r->type, r->boundary);
                added.id = r->id;
                added.boundaryWallIds = r->boundaryWallIds;
                added.metadata = r->metadata;
            }
        }
    }

    // Annotations
    if (val["dimensions"].isArray()) {
        for (const auto& dVal : val["dimensions"].asArray()) {
            if (auto d = deserializeDimension(dVal)) {
                auto& added = lvl->addDimension(d->point1, d->point2, d->offsetDistance_mm);
                added.id = d->id;
                added.referencedEntityId = d->referencedEntityId;
                added.metadata = d->metadata;
            }
        }
    }

    if (val["notes"].isArray()) {
        for (const auto& nVal : val["notes"].asArray()) {
            if (auto n = deserializeNote(nVal)) {
                auto& added = lvl->addNote(n->position, n->text);
                added.id = n->id;
                added.metadata = n->metadata;
            }
        }
    }

    // Library Instances
    if (val["library_instances"].isArray()) {
        for (const auto& instVal : val["library_instances"].asArray()) {
            if (auto inst = deserializeLibraryInstance(instVal)) {
                LibraryItem item;
                item.id = inst->itemId;
                item.name = inst->name;
                item.width_mm = inst->width_mm;
                item.length_mm = inst->length_mm;
                item.clearance = inst->clearance;
                item.semanticType = inst->semanticType;

                auto& added = lvl->addLibraryInstance(item, inst->position, inst->rotation);
                added.id = inst->id;
                added.metadata = inst->metadata;
            }
        }
    }

    // Roofs
    if (val["roofs"].isArray()) {
        for (const auto& rVal : val["roofs"].asArray()) {
            if (auto r = deserializeRoof(rVal)) {
                auto& added = lvl->addRoof(r->name, r->type, r->wallFootprint, r->pitch_deg, r->overhang_mm);
                added.id = r->id;
                added.fasciaHeight_mm = r->fasciaHeight_mm;
                added.metadata = r->metadata;
            }
        }
    }

    // Constraints
    if (val["constraints"].isArray()) {
        for (const auto& cVal : val["constraints"].asArray()) {
            if (auto c = deserializeConstraint(cVal)) {
                auto& added = lvl->addConstraint(c->name, c->type, c->severity, c->targetEntityIds, c->targetValue);
                added.id = c->id;
                added.metadata = c->metadata;
            }
        }
    }

    return lvl;
}

// --- Building Serialization ---

JsonValue serializeBuilding(const Building& bld) {
    JsonObject obj;
    obj["id"] = bld.id.string();
    obj["name"] = bld.name;
    obj["metadata"] = serializeMetadata(bld.metadata);

    if (bld.activeLevel()) {
        obj["active_level_id"] = bld.activeLevel()->id.string();
    }

    JsonArray levelsArr;
    for (const auto& lvl : bld.levels()) {
        levelsArr.push_back(serializeLevel(*lvl));
    }
    obj["levels"] = JsonValue(std::move(levelsArr));

    return JsonValue(std::move(obj));
}

std::unique_ptr<Building> deserializeBuilding(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    std::string name = val["name"].asString("Main Building");
    auto bld = std::make_unique<Building>(std::move(name));
    bld->id = EntityId(val["id"].asString(EntityId::generate("bld").string()));
    deserializeMetadata(val["metadata"], bld->metadata);

    while (!bld->levels().empty()) {
        if (!bld->removeLevel(bld->levels().front()->id)) break;
    }

    if (val["levels"].isArray()) {
        for (const auto& lvlVal : val["levels"].asArray()) {
            if (auto lvl = deserializeLevel(lvlVal)) {
                auto& added = bld->addLevel(lvl->name, lvl->elevation_mm, lvl->height_mm, lvl->type);
                added.id = lvl->id;
                added.visible = lvl->visible;
                added.locked = lvl->locked;
                added.underlayLevelId = lvl->underlayLevelId;
                added.underlayOpacity = lvl->underlayOpacity;
                added.metadata = lvl->metadata;

                for (const auto& w : lvl->walls()) {
                    auto& nw = added.addWall(w->start, w->end, w->thickness_mm);
                    nw.id = w->id;
                    nw.name = w->name;
                    nw.height_mm = w->height_mm;
                    nw.metadata = w->metadata;
                }
                for (const auto& d : lvl->doors()) {
                    auto& nd = added.addDoor(d->hostWallId, d->offsetAlongWall_mm, d->width_mm, d->height_mm, d->swing);
                    nd.id = d->id;
                    nd.name = d->name;
                    nd.sillHeight_mm = d->sillHeight_mm;
                    nd.metadata = d->metadata;
                }
                for (const auto& w : lvl->windows()) {
                    auto& nw = added.addWindow(w->hostWallId, w->offsetAlongWall_mm, w->width_mm, w->height_mm, w->sillHeight_mm, w->windowType);
                    nw.id = w->id;
                    nw.name = w->name;
                    nw.metadata = w->metadata;
                }
                for (const auto& r : lvl->rooms()) {
                    auto& nr = added.addRoom(r->name, r->type, r->boundary);
                    nr.id = r->id;
                    nr.boundaryWallIds = r->boundaryWallIds;
                    nr.metadata = r->metadata;
                }
                for (const auto& dim : lvl->dimensions()) {
                    auto& nd = added.addDimension(dim->point1, dim->point2, dim->offsetDistance_mm);
                    nd.id = dim->id;
                    nd.referencedEntityId = dim->referencedEntityId;
                    nd.metadata = dim->metadata;
                }
                for (const auto& n : lvl->notes()) {
                    auto& nn = added.addNote(n->position, n->text);
                    nn.id = n->id;
                    nn.metadata = n->metadata;
                }
                for (const auto& inst : lvl->libraryInstances()) {
                    LibraryItem item;
                    item.id = inst->itemId;
                    item.name = inst->name;
                    item.width_mm = inst->width_mm;
                    item.length_mm = inst->length_mm;
                    item.clearance = inst->clearance;
                    item.semanticType = inst->semanticType;

                    auto& ni = added.addLibraryInstance(item, inst->position, inst->rotation);
                    ni.id = inst->id;
                    ni.metadata = inst->metadata;
                }
                for (const auto& rf : lvl->roofs()) {
                    auto& nrf = added.addRoof(rf->name, rf->type, rf->wallFootprint, rf->pitch_deg, rf->overhang_mm);
                    nrf.id = rf->id;
                    nrf.fasciaHeight_mm = rf->fasciaHeight_mm;
                    nrf.metadata = rf->metadata;
                }
                for (const auto& c : lvl->constraints()) {
                    auto& nc = added.addConstraint(c->name, c->type, c->severity, c->targetEntityIds, c->targetValue);
                    nc.id = c->id;
                    nc.metadata = c->metadata;
                }
            }
        }
    }

    if (val.contains("active_level_id") && !val["active_level_id"].asString().empty()) {
        bld->setActiveLevel(EntityId(val["active_level_id"].asString()));
    }

    return bld;
}

// --- Site Serialization ---

JsonValue serializeSite(const Site& s) {
    JsonObject obj;
    obj["id"] = s.id.string();
    obj["name"] = s.name;
    obj["metadata"] = serializeMetadata(s.metadata);
    obj["property_boundary"] = serializePolygon(s.propertyBoundary);

    JsonObject setbacks;
    setbacks["front_mm"] = s.setbacks.front_mm;
    setbacks["rear_mm"] = s.setbacks.rear_mm;
    setbacks["side_left_mm"] = s.setbacks.sideLeft_mm;
    setbacks["side_right_mm"] = s.setbacks.sideRight_mm;
    obj["setbacks"] = JsonValue(std::move(setbacks));

    obj["north_angle_deg"] = s.northAngle_deg;
    obj["north_arrow_position"] = serializePoint(s.northArrowPosition);

    // Spot Elevations
    JsonArray spots;
    for (const auto& sp : s.spotElevations) {
        JsonObject spObj;
        spObj["pos"] = serializePoint(sp.position);
        spObj["elevation_mm"] = sp.elevation_mm;
        spObj["label"] = sp.label;
        spots.push_back(JsonValue(std::move(spObj)));
    }
    obj["spot_elevations"] = JsonValue(std::move(spots));

    // Roads
    JsonArray roads;
    for (const auto& r : s.roads()) {
        JsonObject rObj;
        rObj["id"] = r->id.string();
        rObj["name"] = r->name;
        rObj["centerline_start"] = serializePoint(r->centerline.start);
        rObj["centerline_end"] = serializePoint(r->centerline.end);
        rObj["width_mm"] = r->width_mm;
        roads.push_back(JsonValue(std::move(rObj)));
    }
    obj["roads"] = JsonValue(std::move(roads));

    // Entrances
    JsonArray entrances;
    for (const auto& e : s.entrances()) {
        JsonObject eObj;
        eObj["id"] = e->id.string();
        eObj["name"] = e->name;
        eObj["type"] = static_cast<int>(e->type);
        eObj["position"] = serializePoint(e->position);
        eObj["width_mm"] = e->width_mm;
        eObj["orientation_deg"] = e->orientation.degrees();
        entrances.push_back(JsonValue(std::move(eObj)));
    }
    obj["entrances"] = JsonValue(std::move(entrances));

    // Parking Zones
    JsonArray parking;
    for (const auto& p : s.parkingZones()) {
        JsonObject pObj;
        pObj["id"] = p->id.string();
        pObj["name"] = p->name;
        pObj["position"] = serializePoint(p->position);
        pObj["stall_count"] = static_cast<int64_t>(p->stallCount);
        pObj["angle_deg"] = p->angle.degrees();
        pObj["stall_width_mm"] = p->stallWidth_mm;
        pObj["stall_length_mm"] = p->stallLength_mm;
        pObj["is_accessible"] = p->isAccessible;
        parking.push_back(JsonValue(std::move(pObj)));
    }
    obj["parking_zones"] = JsonValue(std::move(parking));

    // Landscape Zones
    JsonArray landscape;
    for (const auto& l : s.landscapeZones()) {
        JsonObject lObj;
        lObj["id"] = l->id.string();
        lObj["name"] = l->name;
        lObj["type"] = static_cast<int>(l->type);
        lObj["boundary"] = serializePolygon(l->boundary);
        landscape.push_back(JsonValue(std::move(lObj)));
    }
    obj["landscape_zones"] = JsonValue(std::move(landscape));

    // Outdoor Elements
    JsonArray outdoor;
    for (const auto& o : s.outdoorElements()) {
        JsonObject oObj;
        oObj["id"] = o->id.string();
        oObj["name"] = o->name;
        oObj["type"] = static_cast<int>(o->type);
        oObj["boundary"] = serializePolygon(o->boundary);
        oObj["depth_mm"] = o->depth_mm;
        oObj["coping_surround_mm"] = o->copingSurround_mm;
        outdoor.push_back(JsonValue(std::move(oObj)));
    }
    obj["outdoor_elements"] = JsonValue(std::move(outdoor));

    // Buildings
    JsonArray bldArr;
    for (const auto& b : s.buildings()) {
        bldArr.push_back(serializeBuilding(*b));
    }
    obj["buildings"] = JsonValue(std::move(bldArr));

    return JsonValue(std::move(obj));
}

std::unique_ptr<Site> deserializeSite(const JsonValue& val) {
    if (!val.isObject()) return nullptr;
    std::string name = val["name"].asString("Default Site");
    auto s = std::make_unique<Site>(std::move(name));
    s->id = EntityId(val["id"].asString(EntityId::generate("site").string()));
    deserializeMetadata(val["metadata"], s->metadata);

    s->propertyBoundary = deserializePolygon(val["property_boundary"]);
    if (val["setbacks"].isObject()) {
        const auto& sb = val["setbacks"];
        s->setbacks.front_mm = sb["front_mm"].asDouble(5000.0);
        s->setbacks.rear_mm = sb["rear_mm"].asDouble(3000.0);
        s->setbacks.sideLeft_mm = sb["side_left_mm"].asDouble(2000.0);
        s->setbacks.sideRight_mm = sb["side_right_mm"].asDouble(2000.0);
    }
    s->northAngle_deg = val["north_angle_deg"].asDouble(0.0);
    s->northArrowPosition = deserializePoint(val["north_arrow_position"]);

    // Spot Elevations
    if (val["spot_elevations"].isArray()) {
        for (const auto& spVal : val["spot_elevations"].asArray()) {
            s->addSpotElevation(deserializePoint(spVal["pos"]), spVal["elevation_mm"].asDouble(0.0), spVal["label"].asString());
        }
    }

    // Roads
    if (val["roads"].isArray()) {
        for (const auto& rVal : val["roads"].asArray()) {
            auto start = deserializePoint(rVal["centerline_start"]);
            auto end = deserializePoint(rVal["centerline_end"]);
            kalara::core::geometry::Segment2D seg{start, end};
            double width = rVal["width_mm"].asDouble(10000.0);
            auto& r = s->addRoad(rVal["name"].asString(), seg, width);
            r.id = EntityId(rVal["id"].asString(EntityId::generate("road").string()));
        }
    }

    // Entrances
    if (val["entrances"].isArray()) {
        for (const auto& eVal : val["entrances"].asArray()) {
            auto type = static_cast<SiteEntranceType>(eVal["type"].asInt(static_cast<int>(SiteEntranceType::VehicularDriveway)));
            auto pos = deserializePoint(eVal["position"]);
            double w = eVal["width_mm"].asDouble(3600.0);
            auto rot = kalara::core::geometry::Angle::fromDegrees(eVal["orientation_deg"].asDouble(0.0));
            auto& e = s->addEntrance(eVal["name"].asString(), type, pos, w, rot);
            e.id = EntityId(eVal["id"].asString(EntityId::generate("ent_site").string()));
        }
    }

    // Parking Zones
    if (val["parking_zones"].isArray()) {
        for (const auto& pVal : val["parking_zones"].asArray()) {
            auto pos = deserializePoint(pVal["position"]);
            size_t count = static_cast<size_t>(pVal["stall_count"].asInt(2));
            auto rot = kalara::core::geometry::Angle::fromDegrees(pVal["angle_deg"].asDouble(0.0));
            double sw = pVal["stall_width_mm"].asDouble(2500.0);
            double sl = pVal["stall_length_mm"].asDouble(5000.0);
            bool acc = pVal["is_accessible"].asBool(false);
            auto& p = s->addParkingZone(pVal["name"].asString(), pos, count, rot, sw, sl, acc);
            p.id = EntityId(pVal["id"].asString(EntityId::generate("prk").string()));
        }
    }

    // Landscape Zones
    if (val["landscape_zones"].isArray()) {
        for (const auto& lVal : val["landscape_zones"].asArray()) {
            auto type = static_cast<LandscapeType>(lVal["type"].asInt(static_cast<int>(LandscapeType::Lawn)));
            auto poly = deserializePolygon(lVal["boundary"]);
            auto& l = s->addLandscapeZone(lVal["name"].asString(), type, std::move(poly));
            l.id = EntityId(lVal["id"].asString(EntityId::generate("lnd").string()));
        }
    }

    // Outdoor Elements
    if (val["outdoor_elements"].isArray()) {
        for (const auto& oVal : val["outdoor_elements"].asArray()) {
            auto type = static_cast<OutdoorElementType>(oVal["type"].asInt(static_cast<int>(OutdoorElementType::SwimmingPool)));
            auto poly = deserializePolygon(oVal["boundary"]);
            double depth = oVal["depth_mm"].asDouble(1500.0);
            double coping = oVal["coping_surround_mm"].asDouble(400.0);
            auto& o = s->addOutdoorElement(oVal["name"].asString(), type, std::move(poly), depth, coping);
            o.id = EntityId(oVal["id"].asString(EntityId::generate("out").string()));
        }
    }

    // Buildings
    if (val["buildings"].isArray()) {
        for (const auto& bVal : val["buildings"].asArray()) {
            if (auto b = deserializeBuilding(bVal)) {
                auto& addedBld = s->addBuilding(b->name);
                addedBld.id = b->id;
                addedBld.metadata = b->metadata;

                while (!addedBld.levels().empty()) {
                    if (!addedBld.removeLevel(addedBld.levels().front()->id)) break;
                }

                for (const auto& lvl : b->levels()) {
                    auto& addedLvl = addedBld.addLevel(lvl->name, lvl->elevation_mm, lvl->height_mm, lvl->type);
                    addedLvl.id = lvl->id;
                    addedLvl.visible = lvl->visible;
                    addedLvl.locked = lvl->locked;
                    addedLvl.underlayLevelId = lvl->underlayLevelId;
                    addedLvl.underlayOpacity = lvl->underlayOpacity;
                    addedLvl.metadata = lvl->metadata;

                    for (const auto& w : lvl->walls()) {
                        auto& nw = addedLvl.addWall(w->start, w->end, w->thickness_mm);
                        nw.id = w->id;
                        nw.name = w->name;
                        nw.height_mm = w->height_mm;
                        nw.metadata = w->metadata;
                    }
                    for (const auto& d : lvl->doors()) {
                        auto& nd = addedLvl.addDoor(d->hostWallId, d->offsetAlongWall_mm, d->width_mm, d->height_mm, d->swing);
                        nd.id = d->id;
                        nd.name = d->name;
                        nd.sillHeight_mm = d->sillHeight_mm;
                        nd.metadata = d->metadata;
                    }
                    for (const auto& w : lvl->windows()) {
                        auto& nw = addedLvl.addWindow(w->hostWallId, w->offsetAlongWall_mm, w->width_mm, w->height_mm, w->sillHeight_mm, w->windowType);
                        nw.id = w->id;
                        nw.name = w->name;
                        nw.metadata = w->metadata;
                    }
                    for (const auto& r : lvl->rooms()) {
                        auto& nr = addedLvl.addRoom(r->name, r->type, r->boundary);
                        nr.id = r->id;
                        nr.boundaryWallIds = r->boundaryWallIds;
                        nr.metadata = r->metadata;
                    }
                    for (const auto& dim : lvl->dimensions()) {
                        auto& nd = addedLvl.addDimension(dim->point1, dim->point2, dim->offsetDistance_mm);
                        nd.id = dim->id;
                        nd.referencedEntityId = dim->referencedEntityId;
                        nd.metadata = dim->metadata;
                    }
                    for (const auto& n : lvl->notes()) {
                        auto& nn = addedLvl.addNote(n->position, n->text);
                        nn.id = n->id;
                        nn.metadata = n->metadata;
                    }
                    for (const auto& inst : lvl->libraryInstances()) {
                        LibraryItem item;
                        item.id = inst->itemId;
                        item.name = inst->name;
                        item.width_mm = inst->width_mm;
                        item.length_mm = inst->length_mm;
                        item.clearance = inst->clearance;
                        item.semanticType = inst->semanticType;

                        auto& ni = addedLvl.addLibraryInstance(item, inst->position, inst->rotation);
                        ni.id = inst->id;
                        ni.metadata = inst->metadata;
                    }
                    for (const auto& rf : lvl->roofs()) {
                        auto& nrf = addedLvl.addRoof(rf->name, rf->type, rf->wallFootprint, rf->pitch_deg, rf->overhang_mm);
                        nrf.id = rf->id;
                        nrf.fasciaHeight_mm = rf->fasciaHeight_mm;
                        nrf.metadata = rf->metadata;
                    }
                    for (const auto& c : lvl->constraints()) {
                        auto& nc = addedLvl.addConstraint(c->name, c->type, c->severity, c->targetEntityIds, c->targetValue);
                        nc.id = c->id;
                        nc.metadata = c->metadata;
                    }
                }

                if (b->activeLevel()) {
                    addedBld.setActiveLevel(b->activeLevel()->id);
                }
            }
        }
    }

    return s;
}

} // namespace

// --- ProjectSerializer Public Methods ---

JsonObject ProjectSerializer::serializeProject(const Project& project) {
    JsonObject root;

    // Header & Format Metadata (Rule 4 & Step 15)
    root["format"] = std::string(KLA_FORMAT_NAME);
    root["format_version"] = std::string(KLA_FORMAT_VERSION);
    root["schema_version"] = KLA_CURRENT_SCHEMA_VERSION;
    root["generator"] = "KaLara Arch v0.1.0";

    auto now = std::chrono::system_clock::now();
    auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    root["timestamp"] = static_cast<int64_t>(nowTimeT);

    // Project State
    JsonObject projObj;
    projObj["id"] = project.id.string();
    projObj["name"] = project.name;
    projObj["display_unit"] = static_cast<int>(project.displayUnit);
    projObj["metadata"] = serializeMetadata(project.metadata);

    // Sites
    JsonArray sitesArr;
    for (const auto& site : project.sites()) {
        sitesArr.push_back(serializeSite(*site));
    }
    projObj["sites"] = JsonValue(std::move(sitesArr));

    root["project"] = JsonValue(std::move(projObj));
    return root;
}

std::unique_ptr<Project> ProjectSerializer::deserializeProject(const JsonObject& root, std::string* errorOut) {
    // 1. Verify Format Signature
    auto itFormat = root.find("format");
    if (itFormat == root.end() || itFormat->second.asString() != KLA_FORMAT_NAME) {
        if (errorOut) *errorOut = "Invalid or missing file format signature (expected 'KaLaraArchNative')";
        return nullptr;
    }

    // 2. Schema Version & Migration Guard (Step 15)
    auto itSchema = root.find("schema_version");
    int schemaVer = itSchema != root.end() ? static_cast<int>(itSchema->second.asInt(1)) : 1;

    if (schemaVer > KLA_CURRENT_SCHEMA_VERSION) {
        if (errorOut) {
            *errorOut = "File was created by a newer version of KaLara Arch (schema version " +
                        std::to_string(schemaVer) + " > supported " + std::to_string(KLA_CURRENT_SCHEMA_VERSION) +
                        "). Please update your application.";
        }
        return nullptr;
    }

    JsonObject workingRoot = root;
    if (schemaVer < KLA_CURRENT_SCHEMA_VERSION) {
        if (!migrateSchema(workingRoot, schemaVer, KLA_CURRENT_SCHEMA_VERSION, errorOut)) {
            return nullptr;
        }
    }

    // 3. Extract Project Data
    auto itProj = workingRoot.find("project");
    if (itProj == workingRoot.end() || !itProj->second.isObject()) {
        if (errorOut) *errorOut = "Missing 'project' object in project data";
        return nullptr;
    }

    const auto& pObj = itProj->second;
    std::string projName = pObj["name"].asString("Untitled Project");
    auto project = std::make_unique<Project>(std::move(projName));
    project->id = EntityId(pObj["id"].asString(EntityId::generate("proj").string()));
    project->displayUnit = static_cast<kalara::core::geometry::UnitSystem>(pObj["display_unit"].asInt(0));
    deserializeMetadata(pObj["metadata"], project->metadata);

    if (pObj["sites"].isArray()) {
        auto cleanProj = std::unique_ptr<Project>(new Project());
        cleanProj->id = project->id;
        cleanProj->name = project->name;
        cleanProj->displayUnit = project->displayUnit;
        cleanProj->metadata = project->metadata;

        const auto& sitesArr = pObj["sites"].asArray();
        if (!sitesArr.empty()) {
            for (size_t i = 0; i < sitesArr.size(); ++i) {
                auto s = deserializeSite(sitesArr[i]);
                if (!s) continue;
                Site* targetSite = nullptr;
                if (i == 0 && !cleanProj->sites().empty()) {
                    targetSite = cleanProj->defaultSite();
                    targetSite->id = s->id;
                    targetSite->name = s->name;
                    targetSite->metadata = s->metadata;
                    targetSite->propertyBoundary = s->propertyBoundary;
                    targetSite->setbacks = s->setbacks;
                    targetSite->northAngle_deg = s->northAngle_deg;
                    targetSite->northArrowPosition = s->northArrowPosition;
                    targetSite->spotElevations = s->spotElevations;

                    while (!targetSite->buildings().empty()) {
                        targetSite->removeBuilding(targetSite->buildings().front()->id);
                    }
                } else {
                    targetSite = &cleanProj->addSite(s->name);
                    targetSite->id = s->id;
                    targetSite->metadata = s->metadata;
                    targetSite->propertyBoundary = s->propertyBoundary;
                    targetSite->setbacks = s->setbacks;
                    targetSite->northAngle_deg = s->northAngle_deg;
                    targetSite->northArrowPosition = s->northArrowPosition;
                    targetSite->spotElevations = s->spotElevations;
                    while (!targetSite->buildings().empty()) {
                        targetSite->removeBuilding(targetSite->buildings().front()->id);
                    }
                }

                for (const auto& r : s->roads()) {
                    auto& nr = targetSite->addRoad(r->name, r->centerline, r->width_mm);
                    nr.id = r->id;
                }
                for (const auto& e : s->entrances()) {
                    auto& ne = targetSite->addEntrance(e->name, e->type, e->position, e->width_mm, e->orientation);
                    ne.id = e->id;
                }
                for (const auto& p : s->parkingZones()) {
                    auto& np = targetSite->addParkingZone(p->name, p->position, p->stallCount, p->angle, p->stallWidth_mm, p->stallLength_mm, p->isAccessible);
                    np.id = p->id;
                }
                for (const auto& l : s->landscapeZones()) {
                    auto& nl = targetSite->addLandscapeZone(l->name, l->type, l->boundary);
                    nl.id = l->id;
                }
                for (const auto& o : s->outdoorElements()) {
                    auto& no = targetSite->addOutdoorElement(o->name, o->type, o->boundary, o->depth_mm, o->copingSurround_mm);
                    no.id = o->id;
                }

                for (const auto& b : s->buildings()) {
                    auto& nb = targetSite->addBuilding(b->name);
                    nb.id = b->id;
                    nb.metadata = b->metadata;
                    while (!nb.levels().empty()) {
                        nb.removeLevel(nb.levels().front()->id);
                    }
                    for (const auto& lvl : b->levels()) {
                        auto& nlvl = nb.addLevel(lvl->name, lvl->elevation_mm, lvl->height_mm, lvl->type);
                        nlvl.id = lvl->id;
                        nlvl.visible = lvl->visible;
                        nlvl.locked = lvl->locked;
                        nlvl.underlayLevelId = lvl->underlayLevelId;
                        nlvl.underlayOpacity = lvl->underlayOpacity;
                        nlvl.metadata = lvl->metadata;

                        for (const auto& w : lvl->walls()) {
                            auto& nw = nlvl.addWall(w->start, w->end, w->thickness_mm);
                            nw.id = w->id;
                            nw.name = w->name;
                            nw.height_mm = w->height_mm;
                            nw.metadata = w->metadata;
                        }
                        for (const auto& d : lvl->doors()) {
                            auto& nd = nlvl.addDoor(d->hostWallId, d->offsetAlongWall_mm, d->width_mm, d->height_mm, d->swing);
                            nd.id = d->id;
                            nd.name = d->name;
                            nd.sillHeight_mm = d->sillHeight_mm;
                            nd.metadata = d->metadata;
                        }
                        for (const auto& w : lvl->windows()) {
                            auto& nw = nlvl.addWindow(w->hostWallId, w->offsetAlongWall_mm, w->width_mm, w->height_mm, w->sillHeight_mm, w->windowType);
                            nw.id = w->id;
                            nw.name = w->name;
                            nw.metadata = w->metadata;
                        }
                        for (const auto& r : lvl->rooms()) {
                            auto& nr = nlvl.addRoom(r->name, r->type, r->boundary);
                            nr.id = r->id;
                            nr.boundaryWallIds = r->boundaryWallIds;
                            nr.metadata = r->metadata;
                        }
                        for (const auto& dim : lvl->dimensions()) {
                            auto& nd = nlvl.addDimension(dim->point1, dim->point2, dim->offsetDistance_mm);
                            nd.id = dim->id;
                            nd.referencedEntityId = dim->referencedEntityId;
                            nd.metadata = dim->metadata;
                        }
                        for (const auto& n : lvl->notes()) {
                            auto& nn = nlvl.addNote(n->position, n->text);
                            nn.id = n->id;
                            nn.metadata = n->metadata;
                        }
                        for (const auto& inst : lvl->libraryInstances()) {
                            LibraryItem item;
                            item.id = inst->itemId;
                            item.name = inst->name;
                            item.width_mm = inst->width_mm;
                            item.length_mm = inst->length_mm;
                            item.clearance = inst->clearance;
                            item.semanticType = inst->semanticType;

                            auto& ni = nlvl.addLibraryInstance(item, inst->position, inst->rotation);
                            ni.id = inst->id;
                            ni.metadata = inst->metadata;
                        }
                        for (const auto& rf : lvl->roofs()) {
                            auto& nrf = nlvl.addRoof(rf->name, rf->type, rf->wallFootprint, rf->pitch_deg, rf->overhang_mm);
                            nrf.id = rf->id;
                            nrf.fasciaHeight_mm = rf->fasciaHeight_mm;
                            nrf.metadata = rf->metadata;
                        }
                        for (const auto& c : lvl->constraints()) {
                            auto& nc = nlvl.addConstraint(c->name, c->type, c->severity, c->targetEntityIds, c->targetValue);
                            nc.id = c->id;
                            nc.metadata = c->metadata;
                        }
                    }
                    if (b->activeLevel()) {
                        nb.setActiveLevel(b->activeLevel()->id);
                    }
                }
            }
        }
        return cleanProj;
    }

    return project;
}

std::string ProjectSerializer::serializeToString(const Project& project, int indent) {
    JsonObject root = serializeProject(project);
    return JsonValue(std::move(root)).dump(indent);
}

std::unique_ptr<Project> ProjectSerializer::deserializeFromString(std::string_view jsonStr, std::string* errorOut) {
    auto parsed = JsonValue::parse(jsonStr, errorOut);
    if (!parsed.has_value() || !parsed->isObject()) {
        if (errorOut && errorOut->empty()) *errorOut = "Failed to parse JSON content";
        return nullptr;
    }
    return deserializeProject(parsed->asObject(), errorOut);
}

bool ProjectSerializer::saveToFile(const Project& project, const std::filesystem::path& path, int indent) {
    JsonObject root = serializeProject(project);
    return JsonValue::saveToFile(JsonValue(std::move(root)), path, indent);
}

std::unique_ptr<Project> ProjectSerializer::loadFromFile(const std::filesystem::path& path, std::string* errorOut) {
    auto parsed = JsonValue::loadFromFile(path, errorOut);
    if (!parsed.has_value() || !parsed->isObject()) {
        if (errorOut && errorOut->empty()) *errorOut = "Failed to load/parse file: " + path.string();
        return nullptr;
    }
    return deserializeProject(parsed->asObject(), errorOut);
}

bool ProjectSerializer::migrateSchema(JsonObject& root, int fromVersion, int toVersion, std::string* /*errorOut*/) {
    for (int v = fromVersion; v < toVersion; ++v) {
        Logger::info("Migrating project schema from v" + std::to_string(v) + " to v" + std::to_string(v + 1));
    }
    root["schema_version"] = toVersion;
    return true;
}

bool ProjectSerializer::areSemanticallyEquivalent(const Project& a, const Project& b, double tol_mm) {
    if (a.name != b.name) return false;
    if (a.displayUnit != b.displayUnit) return false;
    if (a.sites().size() != b.sites().size()) return false;

    for (size_t s = 0; s < a.sites().size(); ++s) {
        const auto& siteA = *a.sites()[s];
        const auto& siteB = *b.sites()[s];
        if (siteA.name != siteB.name) return false;
        if (siteA.propertyBoundary.size() != siteB.propertyBoundary.size()) return false;
        for (size_t i = 0; i < siteA.propertyBoundary.size(); ++i) {
            if (std::abs(siteA.propertyBoundary[i].x - siteB.propertyBoundary[i].x) > tol_mm ||
                std::abs(siteA.propertyBoundary[i].y - siteB.propertyBoundary[i].y) > tol_mm) {
                return false;
            }
        }

        if (siteA.buildings().size() != siteB.buildings().size()) return false;
        for (size_t bg = 0; bg < siteA.buildings().size(); ++bg) {
            const auto& bldA = *siteA.buildings()[bg];
            const auto& bldB = *siteB.buildings()[bg];
            if (bldA.name != bldB.name) return false;
            if (bldA.levels().size() != bldB.levels().size()) return false;

            for (size_t lv = 0; lv < bldA.levels().size(); ++lv) {
                const auto& lvlA = *bldA.levels()[lv];
                const auto& lvlB = *bldB.levels()[lv];
                if (lvlA.name != lvlB.name) return false;
                if (std::abs(lvlA.elevation_mm - lvlB.elevation_mm) > tol_mm) return false;
                if (std::abs(lvlA.height_mm - lvlB.height_mm) > tol_mm) return false;

                // Walls
                if (lvlA.walls().size() != lvlB.walls().size()) return false;
                for (size_t w = 0; w < lvlA.walls().size(); ++w) {
                    const auto& wa = *lvlA.walls()[w];
                    const auto& wb = *lvlB.walls()[w];
                    if (std::abs(wa.start.x - wb.start.x) > tol_mm || std::abs(wa.start.y - wb.start.y) > tol_mm) return false;
                    if (std::abs(wa.end.x - wb.end.x) > tol_mm || std::abs(wa.end.y - wb.end.y) > tol_mm) return false;
                    if (std::abs(wa.thickness_mm - wb.thickness_mm) > tol_mm) return false;
                }

                // Doors & Windows
                if (lvlA.doors().size() != lvlB.doors().size()) return false;
                if (lvlA.windows().size() != lvlB.windows().size()) return false;

                // Rooms
                if (lvlA.rooms().size() != lvlB.rooms().size()) return false;

                // Annotations
                if (lvlA.dimensions().size() != lvlB.dimensions().size()) return false;
                if (lvlA.notes().size() != lvlB.notes().size()) return false;

                // Library instances
                if (lvlA.libraryInstances().size() != lvlB.libraryInstances().size()) return false;

                // Roofs
                if (lvlA.roofs().size() != lvlB.roofs().size()) return false;

                // Constraints
                if (lvlA.constraints().size() != lvlB.constraints().size()) return false;
            }
        }
    }

    return true;
}

} // namespace kalara::architecture
