#include "architecture/model.h"
#include "persistence/json_codec.h"
#include "persistence/migration.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>
using namespace kalara; using namespace architecture; using namespace geometry; using namespace persistence;
namespace {
int checks=0;void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);} 
EntityHeader h(const char* id,EntityType type,const char* owner,std::optional<EntityId> layer={}){return {EntityId(id),type,EntityId(owner),std::move(layer),1,7};}
Project fixture(bool reverse=false){
    Project project(EntityId("project-fixture"));project.title="KaLara café Ω";project.revision=7;project.display={LengthDisplayUnit::mm,2};project.targetAreaSquareMm={IntentState::undecided,{}};project.extensions={{"example.vendor.note","preserve me; never execute"}};
    EntityId floor("floor-ground"),layer("layer-walls"),a("junction-a"),b("junction-b"),wall("wall-a"),opening("door-a"),building("building-a"),site("site-a");
    std::vector<Entity> items;
    items.emplace_back(Site{h("site-a",EntityType::site,"project-fixture"),std::nullopt,Angle(0.2)});
    items.emplace_back(Building{h("building-a",EntityType::building,"project-fixture"),"Main",{floor}});
    items.emplace_back(Floor{h("floor-ground",EntityType::floor,"building-a"),"Ground",0,std::nullopt,{a,b,wall,opening}});
    items.emplace_back(Layer{h("layer-walls",EntityType::layer,"project-fixture"),"Walls",true,false,true});
    items.emplace_back(Junction{h("junction-a",EntityType::junction,"floor-ground",layer),floor,{0,0}});
    items.emplace_back(Junction{h("junction-b",EntityType::junction,"floor-ground",layer),floor,{6000.125,0}});
    items.emplace_back(Wall{h("wall-a",EntityType::wall,"floor-ground",layer),floor,a,b,Length(200),ReferenceLine::center,SideConvention::tangentLeftRight,"generic-200"});
    items.emplace_back(Opening{h("door-a",EntityType::opening,"floor-ground",layer),floor,wall,OpeningKind::door,Length(2100.25),Length(900),OpeningAnchor::distanceFromStart,HingeEnd::end,SwingSide::right,DoorPattern::singleSwing,WindowPattern::fixed,Length(0),Length(2100)});
    if(reverse)std::reverse(items.begin(),items.end());for(auto& item:items)project.entities.emplace(header(item).id,std::move(item));return project;
}
bool failureHas(const DecodeResult& result,const std::string& code){if(auto* failure=std::get_if<CodecFailure>(&result))for(const auto& diagnostic:failure->diagnostics)if(diagnostic.code==code)return true;return false;}
}
int main(int argc,char** argv){try{
    auto first=fixture(false),second=fixture(true);check(validate(first).empty(),"valid fixture");const auto encoded=encodeJson(first);check(encoded==encodeJson(second),"deterministic ordering");check(encoded.find("6000.125")!=std::string::npos,"double precision retained");
    auto decoded=decodeJson(encoded);check(std::holds_alternative<Project>(decoded),"round trip decoded");check(semanticallyEqual(first,std::get<Project>(decoded)),"round trip semantic equality and IDs");check(std::get<Project>(decoded).targetAreaSquareMm.state==IntentState::undecided&&!std::get<Project>(decoded).targetAreaSquareMm.value,"undecided distinct from zero");check(std::get<Project>(decoded).extensions.at("example.vendor.note")=="preserve me; never execute","extensions preserved");
    QFile schema(QStringLiteral(KALARA_SOURCE_DIR "/persistence/schema/project.schema.json"));check(schema.open(QIODevice::ReadOnly),"schema file opens");QJsonParseError schemaError;const auto schemaDocument=QJsonDocument::fromJson(schema.readAll(),&schemaError);check(schemaError.error==QJsonParseError::NoError&&schemaDocument.isObject(),"schema is executable JSON");check(schemaDocument.object().value("$schema").toString().contains("2020-12"),"schema draft declared");
    auto known=first;known.targetAreaSquareMm={IntentState::known,0.0};auto knownDecoded=decodeJson(encodeJson(known));check(std::get<Project>(knownDecoded).targetAreaSquareMm.value==0.0,"known zero preserved");
    auto omitted=first;omitted.targetAreaSquareMm={IntentState::omitted,{}};check(std::get<Project>(decodeJson(encodeJson(omitted))).targetAreaSquareMm.state==IntentState::omitted,"omitted preserved");
    auto missing=encoded;const auto needle=std::string("junction-b\"");missing.replace(missing.find(needle),needle.size(),"missing-node\"");check(failureHas(decodeJson(missing),"reference.missing"),"missing reference rejected");
    auto wrong=encoded;const auto wallLayer=std::string("\"layerId\": \"layer-walls\"");wrong.replace(wrong.find(wallLayer),wallLayer.size(),"\"layerId\": \"floor-ground\"");check(failureHas(decodeJson(wrong),"reference.type"),"wrong typed reference rejected");
    auto unsupported=encoded;const auto version=std::string("\"schemaVersion\": 1");unsupported.replace(unsupported.find(version),version.size(),"\"schemaVersion\": 99");auto unsupportedResult=migrateAndDecode(unsupported);check(failureHas(unsupportedResult,"schema.unsupported"),"unsupported version rejected");
    check(failureHas(decodeJson("{ broken"),"json.parse"),"malformed JSON rejected");
    auto extra=encoded;extra.insert(extra.find("{\n")+2,"    \"surprise\": true,\n");check(failureHas(decodeJson(extra),"schema.additional_property"),"unknown root property rejected");
    auto wrongBoolean=encoded;const auto booleanNeedle=std::string("\"visible\": true");wrongBoolean.replace(wrongBoolean.find(booleanNeedle),booleanNeedle.size(),"\"visible\": 1");check(failureHas(decodeJson(wrongBoolean),"schema.boolean"),"wrong boolean type rejected");
    auto duplicate=encoded;auto rootEnd=duplicate.rfind("  ]");auto firstStart=duplicate.find("    {");auto firstEnd=duplicate.find("    },",firstStart);duplicate.insert(rootEnd,","+duplicate.substr(firstStart,firstEnd-firstStart+5));check(failureHas(decodeJson(duplicate),"entity.duplicate"),"duplicate ID rejected");
    auto invalid=fixture();auto& wall=std::get<Wall>(invalid.entities.at(EntityId("wall-a")));wall.endJunctionId=wall.startJunctionId;check(!validate(invalid).empty(),"same wall nodes rejected");
    auto generated=EntityId::generate();check(generated.str().size()==36&&generated!=EntityId::generate(),"opaque generated IDs");
    if(argc==2){std::filesystem::create_directories(argv[1]);auto write=[&](const char* name,const std::string& text){std::ofstream file(std::filesystem::path(argv[1])/name,std::ios::binary);file<<text;check(file.good(),"fixture write");};write("project-v1-valid.json",encoded);write("project-v1-missing-reference.json",missing);write("project-v99-unsupported.json",unsupported);}
    std::cout<<"model-json-v1: "<<checks<<" checks PASS; bytes="<<encoded.size()<<"; stable IDs and undecided/zero/omitted preserved\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
