#include "editor/session.h"
#include <utility>
namespace kalara::editor {
DocumentSession::DocumentSession(QString name, QObject* parent)
    : QObject(parent), title(std::move(name)) {
    architecture::Project project(architecture::EntityId::generate());
    project.title=title.toUtf8().toStdString();
    const auto projectId=project.id;
    architecture::EntityHeader buildingHeader{architecture::EntityId("building"),architecture::EntityType::building,projectId,std::nullopt,0,0};
    architecture::EntityHeader floorHeader{activeFloorId_,architecture::EntityType::floor,buildingHeader.id,std::nullopt,0,0};
    architecture::EntityHeader layerHeader{wallLayerId_,architecture::EntityType::layer,projectId,std::nullopt,0,0};
    project.entities.emplace(buildingHeader.id,architecture::Building{buildingHeader,"Building",{activeFloorId_}});
    project.entities.emplace(floorHeader.id,architecture::Floor{floorHeader,"Ground Floor",0,std::nullopt,{}});
    project.entities.emplace(layerHeader.id,architecture::Layer{layerHeader,"Walls",true,false,true});
    projectStore_=std::make_unique<runtime::ProjectStore>(std::move(project));
    commands_=std::make_unique<runtime::CommandService>(*projectStore_);
    walls_=std::make_unique<architecture::WallAuthoring>(*projectStore_,*commands_);
}
void DocumentSession::select(std::set<architecture::EntityId> ids) {
    if (ids == selection_) return;
    selection_ = std::move(ids);
    emit selectionChanged();
}
}
