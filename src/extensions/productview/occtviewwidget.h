#ifndef OCCTVIEWWIDGET_H
#define OCCTVIEWWIDGET_H

#ifdef _MSC_VER
#include <cmath>
#endif

#include <vector>

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_ViewController.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QOpenGLWidget>
#include <Standard_WarningsRestore.hxx>

#include "occtimportworker.h"

class OcctViewWidget : public QOpenGLWidget, public AIS_ViewController
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

protected:
    void initializeGL() override;
    void paintGL() override;
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void createOcctViewer();
    void displayImportedModel();
    void applyDisplayMode(int mode);
    void updateView();
    void handleViewRedraw(const Handle(AIS_InteractiveContext) &context,
                          const Handle(V3d_View) &view) override;

    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    std::vector<Handle(AIS_InteractiveObject)> m_objects;
    OcctImportResultPtr m_pendingResult;
    int m_displayMode;
    bool m_viewInitialized;
    bool m_loadedModel;
};

#endif // OCCTVIEWWIDGET_H
