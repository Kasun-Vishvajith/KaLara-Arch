#include "render/camera.h"
#include <algorithm>
#include <stdexcept>
namespace kalara::render {
geometry::Point2 Camera2D::worldToDip(geometry::Point2 p,SizeDip size)const{return {(p.x-center.x)*pixelsPerMm+size.width/2,(center.y-p.y)*pixelsPerMm+size.height/2};}
geometry::Point2 Camera2D::dipToWorld(geometry::Point2 p,SizeDip size)const{return {center.x+(p.x-size.width/2)/pixelsPerMm,center.y-(p.y-size.height/2)/pixelsPerMm};}
void Camera2D::zoomAt(geometry::Point2 anchor,double factor,SizeDip size){if(!std::isfinite(factor)||factor<=0)throw std::invalid_argument("Zoom factor must be positive");const auto before=dipToWorld(anchor,size);pixelsPerMm=std::clamp(pixelsPerMm*factor,0.00001,1000.0);const auto after=dipToWorld(anchor,size);center=center+(before-after);}
void Camera2D::panDip(geometry::Vector2 delta){center=center+geometry::Vector2{-delta.x/pixelsPerMm,delta.y/pixelsPerMm};}
void Camera2D::fit(geometry::AABB2 bounds,SizeDip size,double margin){const double availableW=std::max(1.0,size.width-2*margin),availableH=std::max(1.0,size.height-2*margin);const double width=std::max(bounds.max.x-bounds.min.x,geometry::epsilon(bounds.max.x-bounds.min.x)),height=std::max(bounds.max.y-bounds.min.y,geometry::epsilon(bounds.max.y-bounds.min.y));pixelsPerMm=std::clamp(std::min(availableW/width,availableH/height),0.00001,1000.0);center={(bounds.min.x+bounds.max.x)/2,(bounds.min.y+bounds.max.y)/2};}
geometry::AABB2 Camera2D::visibleWorld(SizeDip size)const{return {dipToWorld({0,size.height},size),dipToWorld({size.width,0},size)};}
}
