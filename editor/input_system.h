#pragma once
#include "architecture/model.h"
#include "render/camera.h"
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>
namespace kalara::editor {
enum class PointerButton { none, left, middle, right };
struct PointerEvent { geometry::Point2 dip; geometry::Point2 world; PointerButton button=PointerButton::none; bool shift=false,alt=false,space=false; };
struct KeyEvent { int key=0; bool shift=false,control=false,alt=false; bool textInputFocused=false; bool pressed=true; };
class Tool {
public:
    virtual ~Tool()=default;
    virtual std::string id()const=0;
    virtual void enter()=0;
    virtual void pointerDown(const PointerEvent&)=0;
    virtual void pointerMove(const PointerEvent&)=0;
    virtual void pointerUp(const PointerEvent&)=0;
    virtual void key(const KeyEvent&)=0;
    virtual void cancel()=0;
    virtual void exit()=0;
    virtual bool hasDraft()const=0;
};
class ToolController {
public:
    void activate(std::unique_ptr<Tool> tool);
    Tool* active()const{return active_.get();}
    void pointerDown(const PointerEvent& event);void pointerMove(const PointerEvent& event);void pointerUp(const PointerEvent& event);void key(const KeyEvent& event);
    void cancel();void focusLost();void documentChanged();void floorChanged();
private:std::unique_ptr<Tool> active_;
};
enum class HitPriority { grip=0, selectedControl=1, opening=2, libraryInstance=3, annotation=4, wall=5, room=6, underlay=7 };
struct Hit { architecture::EntityId id;HitPriority priority;double distanceDip=0;bool locked=false; };
class SelectionQuery {
public:
    static bool eligible(const architecture::Project& project,const architecture::Entity& entity,bool& locked);
    std::vector<Hit> hitTest(const architecture::Project& project,const render::Camera2D& camera,render::SizeDip viewport,geometry::Point2 cursorDip,const std::set<architecture::EntityId>& selected,double radiusDip=8)const;
    std::set<architecture::EntityId> marquee(const architecture::Project& project,const render::Camera2D& camera,render::SizeDip viewport,geometry::Point2 startDip,geometry::Point2 endDip)const;
    bool movable(const architecture::Project& project,const architecture::EntityId& id)const;
    std::optional<Hit> cycle(const std::vector<Hit>& hits,std::size_t index)const;
};
enum class SnapKind { junction, endpoint, intersection, midpoint, perpendicular, center, alignment, grid, hostProjection, nearest };
struct SnapCandidate { SnapKind kind;geometry::Point2 world;double distanceDip=0;std::string stableKey;std::string label;bool referenceOnly=false; };
struct SnapOptions { bool enabled=true,grid=false,ortho=false,polar=false,allowReference=false;double polarIncrementRadians=0.2617993877991494;double radiusDip=8,hysteresisDip=12,gridMm=100;std::optional<geometry::Point2> origin;std::vector<SnapCandidate> supplemental; };
class SnapResolver {
public:
    std::vector<SnapCandidate> candidates(const architecture::Project& project,const render::Camera2D& camera,render::SizeDip viewport,geometry::Point2 cursorDip,const SnapOptions& options)const;
    std::optional<SnapCandidate> resolve(const architecture::Project& project,const render::Camera2D& camera,render::SizeDip viewport,geometry::Point2 cursorDip,const SnapOptions& options);
    std::optional<SnapCandidate> cycle();
    void clear(){latched_.reset();ranked_.clear();cycleIndex_=0;}
private:
    static geometry::Point2 constrain(geometry::Point2 world,const SnapOptions& options);
    std::optional<SnapCandidate> latched_;std::vector<SnapCandidate> ranked_;std::size_t cycleIndex_=0;
};
enum class NumericField { length, angle, absoluteX, absoluteY, deltaX, deltaY };
struct NumericResult { std::optional<double> canonicalValue;std::string error; };
NumericResult parseNumeric(std::string_view text,NumericField field,geometry::Unit displayUnit=geometry::Unit::mm);
bool routeSingleLetterShortcut(const KeyEvent& event);
}
