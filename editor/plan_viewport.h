#pragma once
#include "render/camera.h"
#include "render/frame_recorder.h"
#include "render/scene.h"
#include "editor/input_system.h"
#include "architecture/wall_authoring.h"
#include <QWidget>
#include <memory>
class QImage;
namespace kalara::editor {
class DocumentSession;
enum class RendererMode { automatic, gpu, painter };
class PlanViewport final : public QWidget {
    Q_OBJECT
public:
    explicit PlanViewport(QWidget* parent=nullptr,RendererMode mode=RendererMode::automatic);
    void setScene(render::Scene2D scene);
    void setSession(DocumentSession* session){session_=session;}
    const render::Scene2D& scene()const{return scene_;}
    render::Camera2D& camera(){return camera_;}
    const render::Camera2D& camera()const{return camera_;}
    RendererMode activeMode()const{return activeMode_;}
    void setRendererMode(RendererMode mode);
    void fitScene();
    void activateSelectTool();void activateWallTool();
    bool wallToolActive()const{return wallMode_;}
    void toggleSnap();void toggleOrtho();void toggleGrid();
    bool snapEnabled()const{return snapOptions_.enabled;}bool orthoEnabled()const{return snapOptions_.ortho;}bool gridSnapEnabled()const{return snapOptions_.grid;}
    bool interactionActive()const{return selecting_;}
    const render::FrameRecorder& frames()const{return frames_;}
    QImage renderImage(QSize dipSize,qreal devicePixelRatio)const;
signals:
    void cameraChanged();
    void inputSettingsChanged();
    void actionRequested(QString actionId);
    void frameRecorded(double milliseconds);
private:
    class Canvas;
    void rebuildCanvas(RendererMode requested);
    void draw(QPainter& painter,const QSizeF& dipSize)const;
    void record(double milliseconds);
    void pointerDown(const PointerEvent& event);void pointerMove(const PointerEvent& event);void pointerUp(const PointerEvent& event);void keyInput(const KeyEvent& event);void cancelInteraction();
    void refreshScene();std::optional<architecture::EntityId> snappedJunction()const;
    friend class Canvas;
    friend class GlCanvas;
    QWidget* canvas_=nullptr;
    render::Scene2D scene_;
    render::Camera2D camera_;
    render::FrameRecorder frames_;
    RendererMode activeMode_=RendererMode::painter;
    DocumentSession* session_=nullptr;
    SelectionQuery selectionQuery_;
    SnapResolver snaps_;
    SnapOptions snapOptions_;
    bool selecting_=false;
    geometry::Point2 selectionStartDip_,selectionCurrentDip_;
    std::optional<SnapCandidate> snap_;
    std::size_t hitCycle_=0;
    bool spaceHeld_=false;
    bool wallMode_=false;
    std::optional<architecture::WallEndpoint> wallAnchor_;
    std::optional<geometry::Point2> wallPreview_;
};
}
