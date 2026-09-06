#include "runtime/command_service.h"
#include <algorithm>
#include <set>
#include <stdexcept>

namespace kalara::runtime {
using namespace architecture;
namespace {
Diagnostic diagnostic(std::string code,std::string message){return {std::move(code),Severity::error,std::move(message),{},"","Retry from the current document revision"};}
bool same(const std::optional<Entity>& a,const std::optional<Entity>& b){if(a.has_value()!=b.has_value())return false;return !a||entitySemanticallyEqual(*a,*b);}
void stamp(Entity& entity,std::uint64_t revision,bool created){
    auto& h=const_cast<EntityHeader&>(header(entity));
    if(created)h.createdRevision=revision;
    h.modifiedRevision=revision;
}
}
ProjectStore::ProjectStore(Project project):state_(std::make_shared<const Project>(std::move(project))){if(!validate(*state_).empty())throw std::invalid_argument("ProjectStore requires a valid project");}
CommandService::CommandService(ProjectStore& store,std::size_t historyBudgetBytes):store_(store),budget_(historyBudgetBytes){if(!budget_)throw std::invalid_argument("History budget must be positive");savedState_=*store.snapshot();}
CommitFailure CommandService::fail(std::string code,std::string message)const{return {{diagnostic(std::move(code),std::move(message))}};}
PrepareResult CommandService::prepare(const CommandRequest& request)const{
    const auto snapshot=store_.snapshot();
    if(request.projectId!=snapshot->id)return PrepareFailure{{diagnostic("command.project","Command targets another project")}};
    if(request.expectedRevision!=snapshot->revision)return PrepareFailure{{diagnostic("command.stale","Command request uses a stale revision")}};
    if(request.commandId.empty()||request.label.empty())return PrepareFailure{{diagnostic("command.metadata","Command ID and label are required")}};
    ChangePlan plan;plan.projectId_=request.projectId;plan.baseRevision_=request.expectedRevision;plan.commandId_=request.commandId;plan.label_=request.label;plan.scope_=request.scope;plan.policySummary_=request.policySummary;
    std::set<EntityId> seen;
    for(const auto& mutation:request.mutations){
        if(!seen.insert(mutation.id).second)return PrepareFailure{{diagnostic("command.duplicate_target","A command cannot mutate an entity twice")}};
        if(mutation.replacement&&header(*mutation.replacement).id!=mutation.id)return PrepareFailure{{diagnostic("command.id_mismatch","Replacement entity ID differs from its mutation ID")}};
        std::optional<Entity> before;if(const auto* current=snapshot->find(mutation.id))before=*current;
        if(same(before,mutation.replacement))continue;
        plan.deltas_.push_back({mutation.id,std::move(before),mutation.replacement});
    }
    return plan;
}
CommitResult CommandService::commit(const ChangePlan& plan){
    const auto snapshot=store_.snapshot();
    if(plan.projectId()!=snapshot->id)return fail("commit.project","Change plan targets another project");
    if(plan.baseRevision()!=snapshot->revision)return fail("commit.stale","Change plan is stale and was not applied");
    if(plan.deltas().empty())return CommitSuccess{snapshot->revision,0,{},true};
    Project candidate=*snapshot;const auto revision=snapshot->revision+1;std::size_t applied=0;
    try{
        for(const auto& delta:plan.deltas()){
            if(failAfter_&&applied==*failAfter_)throw std::runtime_error("injected failure");
            auto current=candidate.entities.find(delta.id);
            std::optional<Entity> actual=current==candidate.entities.end()?std::nullopt:std::optional<Entity>(current->second);
            if(!same(actual,delta.before))return fail("commit.base_changed","Entity baseline differs from the prepared plan");
            if(delta.after){Entity next=*delta.after;stamp(next,revision,!delta.before);candidate.entities.insert_or_assign(delta.id,std::move(next));}
            else candidate.entities.erase(delta.id);
            ++applied;
        }
        if(failAfter_&&applied==*failAfter_)throw std::runtime_error("injected failure");
        candidate.revision=revision;
        for(const auto& hook:hooks_){auto issues=hook(candidate,plan.deltas());for(auto& issue:issues)if(issue.severity==Severity::error)return CommitFailure{std::move(issues)};}
        if(auto issues=validate(candidate);!issues.empty())return CommitFailure{std::move(issues)};
    }catch(const std::exception& error){return fail("commit.injected_failure",error.what());}
    while(history_.size()>cursor_){retainedBytes_-=history_.back().bytes;history_.pop_back();}
    const auto id=nextHistoryId_++;HistoryEntry entry{id,plan.label(),plan.deltas(),estimate(plan.deltas())};retainedBytes_+=entry.bytes;history_.push_back(std::move(entry));++cursor_;
    store_.state_=std::make_shared<const Project>(std::move(candidate));prune();publish(id,plan.label(),plan.deltas());
    std::vector<EntityId> ids;for(const auto& delta:plan.deltas())ids.push_back(delta.id);return CommitSuccess{revision,id,std::move(ids),false};
}
CommitResult CommandService::applyHistory(const HistoryEntry& entry,bool forward){
    const auto snapshot=store_.snapshot();Project candidate=*snapshot;const auto revision=snapshot->revision+1;
    for(const auto& delta:entry.deltas){const auto& value=forward?delta.after:delta.before;if(value){Entity restored=*value;stamp(restored,revision,false);candidate.entities.insert_or_assign(delta.id,std::move(restored));}else candidate.entities.erase(delta.id);}
    candidate.revision=revision;if(auto issues=validate(candidate);!issues.empty())return CommitFailure{std::move(issues)};
    store_.state_=std::make_shared<const Project>(std::move(candidate));publish(entry.id,forward?"Redo "+entry.label:"Undo "+entry.label,entry.deltas);
    std::vector<EntityId> ids;for(const auto& delta:entry.deltas)ids.push_back(delta.id);return CommitSuccess{revision,entry.id,std::move(ids),false};
}
CommitResult CommandService::undo(){if(!cursor_)return fail("history.empty","Nothing to undo");auto result=applyHistory(history_[cursor_-1],false);if(std::holds_alternative<CommitSuccess>(result))--cursor_;return result;}
CommitResult CommandService::redo(){if(cursor_==history_.size())return fail("history.empty","Nothing to redo");auto result=applyHistory(history_[cursor_],true);if(std::holds_alternative<CommitSuccess>(result))++cursor_;return result;}
void CommandService::addDependencyHook(DependencyHook hook){hooks_.push_back(std::move(hook));}
void CommandService::markSaved(std::uint64_t revision){if(store_.snapshot()->revision!=revision)throw std::invalid_argument("Only the current committed revision can be marked saved");savedState_=*store_.snapshot();}
bool CommandService::dirty()const{return !savedState_||!semanticContentEqual(*savedState_,*store_.snapshot());}
std::size_t CommandService::estimate(const std::vector<EntityDelta>& deltas){std::size_t bytes=sizeof(HistoryEntry);for(const auto& delta:deltas)bytes+=sizeof(delta)+delta.id.str().size()+(delta.before?sizeof(Entity):0)+(delta.after?sizeof(Entity):0);return bytes;}
void CommandService::publish(std::uint64_t historyId,const std::string& label,const std::vector<EntityDelta>& deltas){if(!eventSink_)return;CommittedEvent event{store_.snapshot()->revision,historyId,label,{}};for(const auto& delta:deltas)event.changedIds.push_back(delta.id);eventSink_(event);}
void CommandService::prune(){while(retainedBytes_>budget_&&history_.size()>1&&cursor_>0){retainedBoundary_=history_.front().id;retainedBytes_-=history_.front().bytes;history_.erase(history_.begin());--cursor_;}}
}
