#pragma once
#include "core/geometry.h"
namespace kalara::render {
struct SizeDip { double width=1,height=1; };
class Camera2D {
public:
    geometry::Point2 center{0,0};
    double pixelsPerMm=0.1;
    geometry::Point2 worldToDip(geometry::Point2 world,SizeDip viewport)const;
    geometry::Point2 dipToWorld(geometry::Point2 dip,SizeDip viewport)const;
    void zoomAt(geometry::Point2 anchorDip,double factor,SizeDip viewport);
    void panDip(geometry::Vector2 delta);
    void fit(geometry::AABB2 bounds,SizeDip viewport,double marginDip=48);
    geometry::AABB2 visibleWorld(SizeDip viewport)const;
};
}
