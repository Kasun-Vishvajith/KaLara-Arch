#include "kalara/architecture/opening.hpp"
#include "kalara/core/geometry/ops.hpp"

namespace kalara::architecture {

kalara::core::geometry::Point2D Opening::centerPoint(const Wall& hostWall) const noexcept {
    auto dir = hostWall.direction();
    return hostWall.start + dir * offsetAlongWall_mm;
}

kalara::core::geometry::Segment2D Opening::openingSegment(const Wall& hostWall) const noexcept {
    auto dir = hostWall.direction();
    auto center = centerPoint(hostWall);
    double halfW = width_mm * 0.5;
    return kalara::core::geometry::Segment2D(center - dir * halfW, center + dir * halfW);
}

std::vector<kalara::core::geometry::Point2D> Opening::openingBox(const Wall& hostWall) const noexcept {
    auto seg = openingSegment(hostWall);
    auto n = hostWall.normal();
    double halfThick = hostWall.thickness_mm * 0.5;

    auto p1 = seg.start + n * halfThick;
    auto p2 = seg.end   + n * halfThick;
    auto p3 = seg.end   - n * halfThick;
    auto p4 = seg.start - n * halfThick;

    return { p1, p2, p3, p4 };
}

bool Opening::containsPoint(const kalara::core::geometry::Point2D& p, const Wall& hostWall) const noexcept {
    auto poly = openingBox(hostWall);
    return kalara::core::geometry::GeometricOps::pointInPolygon(p, poly);
}

kalara::core::geometry::Point2D Door::hingePoint(const Wall& hostWall) const noexcept {
    auto seg = openingSegment(hostWall);
    switch (swing) {
        case DoorSwing::LeftInswing:
        case DoorSwing::LeftOutswing:
            return seg.start;
        case DoorSwing::RightInswing:
        case DoorSwing::RightOutswing:
            return seg.end;
    }
    return seg.start;
}

} // namespace kalara::architecture
