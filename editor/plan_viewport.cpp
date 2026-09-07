#include "editor/plan_viewport.h"
#include "editor/session.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFocusEvent>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace kalara::editor {
namespace {
QColor color(render::Color value){return {value.r,value.g,value.b,value.a};}
render::SizeDip toDipSize(const QSizeF& value){return {value.width(),value.height()};}
PointerButton button(Qt::MouseButton value){if(value==Qt::LeftButton)return PointerButton::left;if(value==Qt::MiddleButton)return PointerButton::middle;if(value==Qt::RightButton)return PointerButton::right;return PointerButton::none;}
PointerEvent pointer(const QMouseEvent& event,const render::Camera2D& camera,const QSize& widgetSize){geometry::Point2 dip(event.position().x(),event.position().y());return {dip,camera.dipToWorld(dip,toDipSize(QSizeF(widgetSize))),button(event.button()),event.modifiers().testFlag(Qt::ShiftModifier),event.modifiers().testFlag(Qt::AltModifier),false};}
KeyEvent key(const QKeyEvent& event,bool pressed){return {event.key(),event.modifiers().testFlag(Qt::ShiftModifier),event.modifiers().testFlag(Qt::ControlModifier),event.modifiers().testFlag(Qt::AltModifier),false,pressed};}
QColor sceneColor(render::Color value,const QPalette& palette){
    if(value==render::Color{32,40,51,255}&&palette.color(QPalette::Base).lightness()<128)return palette.color(QPalette::Text);
    return color(value);
}
}
class PlanViewport::Canvas : public QWidget {
public:
    Canvas(PlanViewport& owner,QWidget* parent):QWidget(parent),owner_(owner){setMouseTracking(true);setFocusPolicy(Qt::StrongFocus);setAccessibleName("2D plan viewport");}
protected:
    void paintEvent(QPaintEvent*)override{QElapsedTimer timer;timer.start();QPainter painter(this);owner_.draw(painter,QSizeF(size()));owner_.record(timer.nsecsElapsed()/1000000.0);}
    void wheelEvent(QWheelEvent* event)override{const double steps=event->angleDelta().y()/120.0;owner_.camera_.zoomAt({event->position().x(),event->position().y()},std::pow(1.2,steps),toDipSize(QSizeF(this->size())));update();emit owner_.cameraChanged();event->accept();}
    void mousePressEvent(QMouseEvent* event)override{if(event->button()==Qt::MiddleButton||(event->button()==Qt::LeftButton&&owner_.spaceHeld_)){panning_=true;last_=event->position();setCursor(Qt::ClosedHandCursor);event->accept();return;}owner_.pointerDown(pointer(*event,owner_.camera_,size()));event->accept();}
    void mouseMoveEvent(QMouseEvent* event)override{if(panning_){const auto delta=event->position()-last_;last_=event->position();owner_.camera_.panDip({delta.x(),delta.y()});update();emit owner_.cameraChanged();event->accept();return;}owner_.pointerMove(pointer(*event,owner_.camera_,size()));event->accept();}
    void mouseReleaseEvent(QMouseEvent* event)override{if(panning_&&(event->button()==Qt::MiddleButton||event->button()==Qt::LeftButton)){panning_=false;unsetCursor();event->accept();return;}owner_.pointerUp(pointer(*event,owner_.camera_,size()));event->accept();}
    void keyPressEvent(QKeyEvent* event)override{owner_.keyInput(key(*event,true));event->accept();}void keyReleaseEvent(QKeyEvent* event)override{owner_.keyInput(key(*event,false));event->accept();}void focusOutEvent(QFocusEvent* event)override{owner_.cancelInteraction();QWidget::focusOutEvent(event);}
    PlanViewport& owner_;bool panning_=false;QPointF last_;
};
class GlCanvas final : public QOpenGLWidget {
public:
    GlCanvas(PlanViewport& owner,QWidget* parent):QOpenGLWidget(parent),owner_(owner){setMouseTracking(true);setFocusPolicy(Qt::StrongFocus);setAccessibleName("GPU 2D plan viewport");}
protected:
    void paintGL()override{QElapsedTimer timer;timer.start();QPainter painter(this);owner_.draw(painter,QSizeF(size()));owner_.record(timer.nsecsElapsed()/1000000.0);}
    void wheelEvent(QWheelEvent* event)override{const double steps=event->angleDelta().y()/120.0;owner_.camera().zoomAt({event->position().x(),event->position().y()},std::pow(1.2,steps),toDipSize(QSizeF(this->size())));update();emit owner_.cameraChanged();event->accept();}
    void mousePressEvent(QMouseEvent* event)override{if(event->button()==Qt::MiddleButton||(event->button()==Qt::LeftButton&&owner_.spaceHeld_)){panning_=true;last_=event->position();setCursor(Qt::ClosedHandCursor);event->accept();return;}owner_.pointerDown(pointer(*event,owner_.camera(),size()));event->accept();}
    void mouseMoveEvent(QMouseEvent* event)override{if(panning_){const auto delta=event->position()-last_;last_=event->position();owner_.camera().panDip({delta.x(),delta.y()});update();emit owner_.cameraChanged();event->accept();return;}owner_.pointerMove(pointer(*event,owner_.camera(),size()));event->accept();}
    void mouseReleaseEvent(QMouseEvent* event)override{if(panning_&&(event->button()==Qt::MiddleButton||event->button()==Qt::LeftButton)){panning_=false;unsetCursor();event->accept();return;}owner_.pointerUp(pointer(*event,owner_.camera(),size()));event->accept();}
    void keyPressEvent(QKeyEvent* event)override{owner_.keyInput(key(*event,true));event->accept();}void keyReleaseEvent(QKeyEvent* event)override{owner_.keyInput(key(*event,false));event->accept();}void focusOutEvent(QFocusEvent* event)override{owner_.cancelInteraction();QOpenGLWidget::focusOutEvent(event);}
private:
    PlanViewport& owner_;bool panning_=false;QPointF last_;
};
PlanViewport::PlanViewport(QWidget* parent,RendererMode mode):QWidget(parent),frames_(7200){setObjectName("planViewport");auto* layout=new QVBoxLayout(this);layout->setContentsMargins(0,0,0,0);rebuildCanvas(mode);}
void PlanViewport::rebuildCanvas(RendererMode requested){
    if(canvas_){layout()->removeWidget(canvas_);delete canvas_;canvas_=nullptr;}
    const bool offscreen=QApplication::platformName()=="offscreen"||QApplication::platformName()=="minimal";
    activeMode_=requested==RendererMode::automatic?(offscreen?RendererMode::painter:RendererMode::gpu):requested;
    canvas_=activeMode_==RendererMode::gpu?static_cast<QWidget*>(new GlCanvas(*this,this)):static_cast<QWidget*>(new Canvas(*this,this));
    layout()->addWidget(canvas_);canvas_->show();
}
void PlanViewport::setRendererMode(RendererMode mode){rebuildCanvas(mode);canvas_->update();emit cameraChanged();}
void PlanViewport::toggleSnap(){snapOptions_.enabled=!snapOptions_.enabled;if(!snapOptions_.enabled){snaps_.clear();snap_.reset();}canvas_->update();emit inputSettingsChanged();}
void PlanViewport::toggleOrtho(){snapOptions_.ortho=!snapOptions_.ortho;canvas_->update();emit inputSettingsChanged();}
void PlanViewport::toggleGrid(){snapOptions_.grid=!snapOptions_.grid;canvas_->update();emit inputSettingsChanged();}
void PlanViewport::setScene(render::Scene2D scene){scene_=std::move(scene);canvas_->update();}
void PlanViewport::refreshScene(){if(!session_)return;const render::SceneBuilder builder;setScene(builder.build(*session_->projectStore().snapshot(),{{{-10000000,-10000000},{10000000,10000000}},session_->activeFloorId()}));}
void PlanViewport::activateSelectTool(){wallMode_=false;wallAnchor_.reset();wallPreview_.reset();cancelInteraction();canvas_->setFocus();}
void PlanViewport::activateWallTool(){cancelInteraction();wallMode_=true;wallAnchor_.reset();wallPreview_.reset();canvas_->setFocus();}
std::string PlanViewport::commitWallNumeric(std::string_view input){if(!session_||!wallMode_||!wallAnchor_||!wallPreview_)return "Anchor a wall and point in its direction first";std::string text(input);geometry::Point2 endpoint;const auto comma=text.find(',');if(comma!=std::string::npos){const bool relative=!text.empty()&&text.front()=='@';const auto x=parseNumeric(std::string_view(text).substr(relative?1:0,comma-(relative?1:0)),relative?NumericField::deltaX:NumericField::absoluteX);const auto y=parseNumeric(std::string_view(text).substr(comma+1),relative?NumericField::deltaY:NumericField::absoluteY);if(!x.canonicalValue||!y.canonicalValue)return x.error.empty()?y.error:x.error;endpoint=relative?wallAnchor_->position+geometry::Vector2{*x.canonicalValue,*y.canonicalValue}:geometry::Point2{*x.canonicalValue,*y.canonicalValue};}else{const auto value=parseNumeric(text,NumericField::length);if(!value.canonicalValue||*value.canonicalValue<=0)return value.error.empty()?"Length must be positive":value.error;const auto delta=*wallPreview_-wallAnchor_->position;const auto length=geometry::norm(delta);if(length<=geometry::epsilon(length))return "Point in the segment direction first";endpoint=wallAnchor_->position+delta*(*value.canonicalValue/length);}auto result=session_->walls().addSegment(session_->activeFloorId(),session_->wallLayerId(),*wallAnchor_,{endpoint,{}});if(const auto* failure=std::get_if<runtime::CommitFailure>(&result))return failure->diagnostics.empty()?"Wall command failed":failure->diagnostics.front().message;refreshScene();wallAnchor_=architecture::WallEndpoint{endpoint,session_->walls().lastEndJunction()};wallPreview_=endpoint;return{};}
void PlanViewport::fitScene(){if(scene_.bounds)camera_.fit(*scene_.bounds,toDipSize(QSizeF(canvas_->size())));canvas_->update();emit cameraChanged();}
void PlanViewport::record(double milliseconds){frames_.record(milliseconds);emit frameRecorded(milliseconds);}
std::optional<architecture::EntityId> PlanViewport::snappedJunction()const{if(!snap_||snap_->stableKey.rfind("junction:",0)!=0)return{};return architecture::EntityId(snap_->stableKey.substr(9));}
void PlanViewport::pointerDown(const PointerEvent& event){if(!session_||event.button!=PointerButton::left)return;if(wallMode_)return;selecting_=true;selectionStartDip_=selectionCurrentDip_=event.dip;session_->tools().pointerDown(event);canvas_->update();}
void PlanViewport::pointerMove(const PointerEvent& event){if(session_)session_->tools().pointerMove(event);if(selecting_)selectionCurrentDip_=event.dip;if(session_)snap_=snaps_.resolve(*session_->projectStore().snapshot(),camera_,toDipSize(QSizeF(canvas_->size())),event.dip,snapOptions_);if(wallMode_)wallPreview_=snap_?snap_->world:event.world;canvas_->update();}
void PlanViewport::pointerUp(const PointerEvent& event){if(!session_||event.button!=PointerButton::left)return;if(wallMode_){const auto point=snap_?snap_->world:event.world;const auto junction=snappedJunction();if(!wallAnchor_){wallAnchor_=architecture::WallEndpoint{point,junction};wallPreview_=point;canvas_->update();return;}if(!geometry::near(wallAnchor_->position,point,geometry::distance(wallAnchor_->position,point).mm)){auto result=session_->walls().addSegment(session_->activeFloorId(),session_->wallLayerId(),*wallAnchor_,{point,junction});if(std::holds_alternative<runtime::CommitSuccess>(result)){refreshScene();wallAnchor_=architecture::WallEndpoint{point,session_->walls().lastEndJunction()};}}canvas_->update();return;}session_->tools().pointerUp(event);if(!selecting_)return;selectionCurrentDip_=event.dip;auto selected=session_->selection();const double movement=std::hypot(selectionCurrentDip_.x-selectionStartDip_.x,selectionCurrentDip_.y-selectionStartDip_.y);if(movement<4){auto hits=selectionQuery_.hitTest(*session_->projectStore().snapshot(),camera_,toDipSize(QSizeF(canvas_->size())),event.dip,selected);if(!event.alt)hitCycle_=0;auto hit=selectionQuery_.cycle(hits,event.alt?hitCycle_++:0);if(!event.shift)selected.clear();if(hit){if(event.shift&&selected.contains(hit->id))selected.erase(hit->id);else selected.insert(hit->id);}}else{auto found=selectionQuery_.marquee(*session_->projectStore().snapshot(),camera_,toDipSize(QSizeF(canvas_->size())),selectionStartDip_,selectionCurrentDip_);if(!event.shift)selected=std::move(found);else for(const auto& id:found){if(selected.contains(id))selected.erase(id);else selected.insert(id);}}session_->select(std::move(selected));selecting_=false;canvas_->update();}
void PlanViewport::keyInput(const KeyEvent& event){if(event.key==Qt::Key_Space){spaceHeld_=event.pressed;return;}if(!event.pressed)return;if(event.key==Qt::Key_Escape){wallAnchor_.reset();wallPreview_.reset();cancelInteraction();return;}if(event.key==Qt::Key_Tab)snap_=snaps_.cycle();else if(routeSingleLetterShortcut(event)&&event.key==Qt::Key_V)emit actionRequested("tool.select");else if(routeSingleLetterShortcut(event)&&event.key==Qt::Key_W)emit actionRequested("tool.wall");if(session_)session_->tools().key(event);canvas_->update();}
void PlanViewport::cancelInteraction(){selecting_=false;spaceHeld_=false;snap_.reset();snaps_.clear();if(session_)session_->tools().cancel();if(canvas_)canvas_->update();}
void PlanViewport::draw(QPainter& painter,const QSizeF& dipSize)const{
    painter.setRenderHint(QPainter::Antialiasing,true);painter.fillRect(QRectF(QPointF(0,0),dipSize),palette().color(QPalette::Base));const auto viewport=toDipSize(dipSize);
    const auto visible=camera_.visibleWorld(viewport);double grid=10;while(grid*camera_.pixelsPerMm<24)grid*=5;while(grid*camera_.pixelsPerMm>120)grid/=2;
    const bool dark=palette().color(QPalette::Base).lightness()<128;const QColor minorColor=dark?QColor("#394352"):QColor("#D9DFE7");const QColor majorColor=dark?QColor("#526075"):QColor("#BCC6D3");
    const auto x0=std::floor(visible.min.x/grid)*grid,y0=std::floor(visible.min.y/grid)*grid;
    for(double x=x0;x<=visible.max.x;x+=grid){const auto index=static_cast<long long>(std::llround(x/grid));painter.setPen(QPen(index%5?minorColor:majorColor,1));auto a=camera_.worldToDip({x,visible.min.y},viewport),b=camera_.worldToDip({x,visible.max.y},viewport);painter.drawLine(QPointF(a.x,a.y),QPointF(b.x,b.y));}
    for(double y=y0;y<=visible.max.y;y+=grid){const auto index=static_cast<long long>(std::llround(y/grid));painter.setPen(QPen(index%5?minorColor:majorColor,1));auto a=camera_.worldToDip({visible.min.x,y},viewport),b=camera_.worldToDip({visible.max.x,y},viewport);painter.drawLine(QPointF(a.x,a.y),QPointF(b.x,b.y));}
    for(const auto& fill:scene_.fills){QPolygonF polygon;for(auto p:fill.polygon.outer){auto dip=camera_.worldToDip(p,viewport);polygon<<QPointF(dip.x,dip.y);}QPainterPath path;path.addPolygon(polygon);for(const auto& hole:fill.polygon.holes){QPolygonF points;for(auto p:hole){auto dip=camera_.worldToDip(p,viewport);points<<QPointF(dip.x,dip.y);}QPainterPath holePath;holePath.addPolygon(points);path=path.subtracted(holePath);}painter.fillPath(path,color(fill.color));}
    for(const auto& line:scene_.lines){auto a=camera_.worldToDip(line.a,viewport),b=camera_.worldToDip(line.b,viewport);const bool selected=session_&&line.sourceId&&session_->selection().contains(*line.sourceId);QPen pen(selected?QColor("#2563EB"):sceneColor(line.color,palette()));pen.setWidthF(std::max(selected?2.0:1.0,line.widthMm*camera_.pixelsPerMm));painter.setPen(pen);painter.drawLine(QPointF(a.x,a.y),QPointF(b.x,b.y));}
    for(const auto& text:scene_.texts){auto p=camera_.worldToDip(text.anchor,viewport);QFont font=painter.font();font.setPixelSize(std::max(12,static_cast<int>(std::round(text.heightMm*camera_.pixelsPerMm))));painter.setFont(font);painter.setPen(sceneColor(text.color,palette()));painter.drawText(QPointF(p.x,p.y),QString::fromUtf8(text.text));}
    if(wallMode_&&wallAnchor_&&wallPreview_){const auto a=camera_.worldToDip(wallAnchor_->position,viewport),b=camera_.worldToDip(*wallPreview_,viewport);painter.setPen(QPen(QColor("#2563EB"),2,Qt::DashLine));painter.drawLine(QPointF(a.x,a.y),QPointF(b.x,b.y));const auto delta=*wallPreview_-wallAnchor_->position;const auto length=geometry::norm(delta),angle=std::atan2(delta.y,delta.x)*180/std::numbers::pi;painter.drawText(QPointF((a.x+b.x)/2+8,(a.y+b.y)/2-8),QString("%1 mm · %2°").arg(length,0,'f',1).arg(angle,0,'f',1));}
    if(selecting_){QPen pen(QColor("#2563EB"),1,selectionCurrentDip_.x<selectionStartDip_.x?Qt::DashLine:Qt::SolidLine);painter.setPen(pen);painter.setBrush(QColor(37,99,235,28));painter.drawRect(QRectF(QPointF(selectionStartDip_.x,selectionStartDip_.y),QPointF(selectionCurrentDip_.x,selectionCurrentDip_.y)).normalized());}
    if(snap_){const auto p=camera_.worldToDip(snap_->world,viewport);painter.setPen(QPen(QColor("#2563EB"),2));painter.setBrush(Qt::NoBrush);painter.drawEllipse(QPointF(p.x,p.y),5,5);painter.drawText(QPointF(p.x+9,p.y-9),QString::fromStdString(snap_->label));}
}
QImage PlanViewport::renderImage(QSize dipSize,qreal dpr)const{QImage image(dipSize*dpr,QImage::Format_ARGB32_Premultiplied);image.setDevicePixelRatio(dpr);QPainter painter(&image);draw(painter,QSizeF(dipSize));return image;}
}
