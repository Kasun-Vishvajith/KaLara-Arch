#pragma once
#include "kalara/core/geometry/point_vector.hpp"
#include "kalara/core/geometry/rect.hpp"
#include <algorithm>

namespace kalara::editor {

/// Manages 2D Viewport camera, pan, zoom, and coordinate transformations between
/// Screen (pixels) and World (millimetres).
class ViewportState {
public:
    /// Scale factor: screen pixels per world millimetre.
    /// E.g. scale = 0.1 means 10 mm in world = 1 pixel on screen.
    double scale = 0.1;

    /// World coordinate (in mm) that is centered at the screen origin / viewport center.
    kalara::core::geometry::Point2D panOffset_mm{0.0, 0.0};

    /// Viewport window dimensions in screen pixels.
    int viewportWidth = 1280;
    int viewportHeight = 800;

    /// Zoom limits
    static constexpr double MinScale = 0.001; // Zoomed very far out (1000m view)
    static constexpr double MaxScale = 50.0;  // Zoomed very close in (fine detail)

    /// Convert screen pixel coordinate to world millimetres.
    [[nodiscard]] kalara::core::geometry::Point2D screenToWorld(double screenX, double screenY) const noexcept {
        double cx = viewportWidth * 0.5;
        double cy = viewportHeight * 0.5;
        double worldX = (screenX - cx) / scale + panOffset_mm.x;
        // Architectural CAD Y-axis typically points upwards in world coordinates
        double worldY = -(screenY - cy) / scale + panOffset_mm.y;
        return kalara::core::geometry::Point2D(worldX, worldY);
    }

    /// Convert world millimetres to screen pixel coordinate.
    [[nodiscard]] kalara::core::geometry::Point2D worldToScreen(double worldX, double worldY) const noexcept {
        double cx = viewportWidth * 0.5;
        double cy = viewportHeight * 0.5;
        double screenX = (worldX - panOffset_mm.x) * scale + cx;
        double screenY = -(worldY - panOffset_mm.y) * scale + cy;
        return kalara::core::geometry::Point2D(screenX, screenY);
    }

    [[nodiscard]] kalara::core::geometry::Point2D worldToScreen(const kalara::core::geometry::Point2D& p) const noexcept {
        return worldToScreen(p.x, p.y);
    }

    [[nodiscard]] kalara::core::geometry::Point2D screenToWorld(const kalara::core::geometry::Point2D& p) const noexcept {
        return screenToWorld(p.x, p.y);
    }

    /// Pan by delta in screen pixels.
    void panByScreenDelta(double deltaScreenX, double deltaScreenY) noexcept {
        panOffset_mm.x -= deltaScreenX / scale;
        panOffset_mm.y += deltaScreenY / scale;
    }

    /// Zoom at a specific screen anchor point (e.g. mouse cursor).
    void zoomAtScreenPoint(double factor, double screenX, double screenY) noexcept {
        auto worldBefore = screenToWorld(screenX, screenY);
        scale = std::clamp(scale * factor, MinScale, MaxScale);
        auto worldAfter = screenToWorld(screenX, screenY);

        panOffset_mm.x += (worldBefore.x - worldAfter.x);
        panOffset_mm.y += (worldBefore.y - worldAfter.y);
    }

    /// Visible world rectangle in millimetres.
    [[nodiscard]] kalara::core::geometry::Rect2D visibleWorldBounds() const noexcept {
        auto topLeft = screenToWorld(0.0, 0.0);
        auto bottomRight = screenToWorld(static_cast<double>(viewportWidth), static_cast<double>(viewportHeight));
        return kalara::core::geometry::Rect2D(topLeft, bottomRight);
    }
};

} // namespace kalara::editor
