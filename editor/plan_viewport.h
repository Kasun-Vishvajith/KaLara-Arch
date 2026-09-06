#pragma once
#include "render/camera.h"
#include "render/frame_recorder.h"
#include "render/scene.h"
#include <QWidget>
#include <memory>
class QImage;
namespace kalara::editor {
enum class RendererMode { automatic, gpu, painter };
class PlanViewport final : public QWidget {
    Q_OBJECT
public:
    explicit PlanViewport(QWidget* parent=nullptr,RendererMode mode=RendererMode::automatic);
    void setScene(render::Scene2D scene);
    const render::Scene2D& scene()const{return scene_;}
    render::Camera2D& camera(){return camera_;}
    const render::Camera2D& camera()const{return camera_;}
    RendererMode activeMode()const{return activeMode_;}
    void setRendererMode(RendererMode mode);
    void fitScene();
    const render::FrameRecorder& frames()const{return frames_;}
    QImage renderImage(QSize dipSize,qreal devicePixelRatio)const;
signals:
    void cameraChanged();
    void frameRecorded(double milliseconds);
private:
    class Canvas;
    void rebuildCanvas(RendererMode requested);
    void draw(QPainter& painter,const QSizeF& dipSize)const;
    void record(double milliseconds);
    friend class Canvas;
    friend class GlCanvas;
    QWidget* canvas_=nullptr;
    render::Scene2D scene_;
    render::Camera2D camera_;
    render::FrameRecorder frames_;
    RendererMode activeMode_=RendererMode::painter;
};
}
