#ifndef OCCTVIEWWIDGET_H
#define OCCTVIEWWIDGET_H

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPoint>
#include <QQuaternion>
#include <QVector3D>

#include "occtimportworker.h"

class OcctViewWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit OcctViewWidget(QWidget *parent = nullptr);
    ~OcctViewWidget();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void clearScene();
    void setImportedModel(const OcctImportResultPtr &result);
    void fitAll();
    void setIsometricView();
    void setFrontView();
    void setTopView();
    void setRightView();
    void setShadedMode();
    void setWireframeMode();
    bool hasLoadedModel() const;

signals:
    void initializationFailed(const QString &message);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void uploadModel();
    void updateProjection();
    void setViewRotation(const QQuaternion &rotation);

    OcctImportResultPtr m_result;
    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLVertexArrayObject m_vertexArray;
    QMatrix4x4 m_projection;
    QQuaternion m_rotation;
    QVector3D m_center;
    QVector3D m_pan;
    QPoint m_lastMousePosition;
    float m_radius;
    float m_distance;
    int m_vertexCount;
    bool m_initialized;
    bool m_wireframe;
};

#endif // OCCTVIEWWIDGET_H
