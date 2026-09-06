#include "editor/plan_viewport.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QImage>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

namespace kalara::editor {
namespace {
QColor color(render::Color value){return {value.r,value.g,value.b,value.a};}
render::SizeDip toDipSize(const QSizeF& value){return {value.width(),value.height()};}
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
    void mousePressEvent(QMouseEvent* event)override{if(event->button()==Qt::MiddleButton){panning_=true;last_=event->position();setCursor(Qt::ClosedHandCursor);event->accept();}}
    void mouseMoveEvent(QMouseEvent* event)override{if(panning_){const auto delta=event->position()-last_;last_=event->position();owner_.camera_.panDip({delta.x(),delta.y()});update();emit owner_.cameraChanged();event->accept();}}
    void mouseReleaseEvent(QMouseEvent* event)override{if(event->button()==Qt::MiddleButton&&panning_){panning_=false;unsetCursor();event->accept();}}
    PlanViewport& owner_;bool panning_=false;QPointF last_;
};
class GlCanvas final : public QOpenGLWidget {
public:
    GlCanvas(PlanViewport& owner,QWidget* parent):QOpenGLWidget(parent),owner_(owner){setMouseTracking(true);setFocusPolicy(Qt::StrongFocus);setAccessibleName("GPU 2D plan viewport");}
protected:
    void paintGL()override{QElapsedTimer timer;timer.start();QPainter painter(this);owner_.draw(painter,QSizeF(size()));owner_.record(timer.nsecsElapsed()/1000000.0);}
    void wheelEvent(QWheelEvent* event)override{const double steps=event->angleDelta().y()/120.0;owner_.camera().zoomAt({event->position().x(),event->position().y()},std::pow(1.2,steps),toDipSize(QSizeF(this->size())));update();emit owner_.cameraChanged();event->accept();}
    void mousePressEvent(QMouseEvent* event)override{if(event->button()==Qt::MiddleButton){panning_=true;last_=event->position();setCursor(Qt::ClosedHandCursor);event->accept();}}
    void mouseMoveEvent(QMouseEvent* event)override{if(panning_){const auto delta=event->position()-last_;last_=event->position();owner_.camera().panDip({delta.x(),delta.y()});update();emit owner_.cameraChanged();event->accept();}}
    void mouseReleaseEvent(QMouseEvent* event)override{if(event->button()==Qt::MiddleButton&&panning_){panning_=false;unsetCursor();event->accept();}}
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
void PlanViewport::setScene(render::Scene2D scene){scene_=std::move(scene);canvas_->update();}
void PlanViewport::fitScene(){if(scene_.bounds)camera_.fit(*scene_.bounds,toDipSize(QSizeF(canvas_->size())));canvas_->update();emit cameraChanged();}
void PlanViewport::record(double milliseconds){frames_.record(milliseconds);emit frameRecorded(milliseconds);}
void PlanViewport::draw(QPainter& painter,const QSizeF& dipSize)const{
    painter.setRenderHint(QPainter::Antialiasing,true);painter.fillRect(QRectF(QPointF(0,0),dipSize),palette().color(QPalette::Base));const auto viewport=toDipSize(dipSize);
    const auto visible=camera_.visibleWorld(viewport);double grid=10;while(grid*camera_.pixelsPerMm<24)grid*=5;while(grid*camera_.pixelsPerMm>120)grid/=2;
    const bool dark=palette().color(QPalette::Base).lightness()<128;const QColor minorColor=dark?QColor("#394352"):QColor("#D9DFE7");const QColor majorColor=dark?QColor("#526075"):QColor("#BCC6D3");
    const auto x0=std::floor(visible.min.x/grid)*grid,y0=std::floor(visible.min.y/grid)*grid;
    for(double x=x0;x<=visible.max.x;x+=grid){const auto index=static_cast<long long>(std::llround(x/grid));painter.setPen(QPen(index%5?minorColor:majorColor,1));auto a=camera_.worldToDip({x,visible.min.y},viewport),b=camera_.worldToDip({x,visible.max.y},viewport);painter.drawLine(QPointF(a.x,a.y),QPointF(b.x,b.y));}
    for(double y=y0;y<=visible.max.y;y+=grid){const auto index=static_cast<long long>(std::llround(y/grid));painter.setPen(QPen(index%5?minorColor:majorColor,1));auto a=camera_.worldToDip({visible.min.x,y},viewport),b=camera_.worldToDip({visible.max.x,y},viewport);painter.drawLine(QPointF(a.x,a.y),QPointF(b.x,b.y));}
    for(const auto& fill:scene_.fills){QPolygonF polygon;for(auto p:fill.polygon.outer){auto dip=camera_.worldToDip(p,viewport);polygon<<QPointF(dip.x,dip.y);}QPainterPath path;path.addPolygon(polygon);for(const auto& hole:fill.polygon.holes){QPolygonF points;for(auto p:hole){auto dip=camera_.worldToDip(p,viewport);points<<QPointF(dip.x,dip.y);}QPainterPath holePath;holePath.addPolygon(points);path=path.subtracted(holePath);}painter.fillPath(path,color(fill.color));}
    for(const auto& line:scene_.lines){auto a=camera_.worldToDip(line.a,viewport),b=camera_.worldToDip(line.b,viewport);QPen pen(sceneColor(line.color,palette()));pen.setWidthF(std::max(1.0,line.widthMm*camera_.pixelsPerMm));painter.setPen(pen);painter.drawLine(QPointF(a.x,a.y),QPointF(b.x,b.y));}
    for(const auto& text:scene_.texts){auto p=camera_.worldToDip(text.anchor,viewport);QFont font=painter.font();font.setPixelSize(std::max(12,static_cast<int>(std::round(text.heightMm*camera_.pixelsPerMm))));painter.setFont(font);painter.setPen(sceneColor(text.color,palette()));painter.drawText(QPointF(p.x,p.y),QString::fromUtf8(text.text));}
}
QImage PlanViewport::renderImage(QSize dipSize,qreal dpr)const{QImage image(dipSize*dpr,QImage::Format_ARGB32_Premultiplied);image.setDevicePixelRatio(dpr);QPainter painter(&image);draw(painter,QSizeF(dipSize));return image;}
}
