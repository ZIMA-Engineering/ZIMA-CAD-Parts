#ifdef _WIN32
#include <windows.h>
#endif

#include "occtviewwidget.h"

#include "occtgltools.h"
#include "occtqttools.h"

#include <AIS_DisplayMode.hxx>
#include <AIS_Triangulation.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Graphic3d_RenderingParams.hxx>
#include <Message.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPalette>
#include <QWheelEvent>
#include <Standard_WarningsRestore.hxx>
#include <V3d_TypeOfOrientation.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs_AISObject.hxx>

namespace {

Aspect_Drawable toAspectDrawable(WId windowId)
{
#ifdef _WIN32
    return reinterpret_cast<Aspect_Drawable>(windowId);
#else
    return static_cast<Aspect_Drawable>(windowId);
#endif
}

}

OcctViewWidget::OcctViewWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_displayMode(AIS_Shaded),
      m_viewInitialized(false),
      m_loadedModel(false)
{
    createOcctViewer();

    setAttribute(Qt::WA_AcceptTouchEvents);
    setMouseTracking(true);
    setBackgroundRole(QPalette::NoRole);
    setFocusPolicy(Qt::StrongFocus);
    setUpdatesEnabled(true);
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
}

OcctViewWidget::~OcctViewWidget()
{
    Handle(Aspect_DisplayConnection) displayConnection;
    if (!m_viewer.IsNull())
        displayConnection = m_viewer->Driver()->GetDisplayConnection();

    if (!m_context.IsNull())
    {
        m_context->RemoveAll(false);
        m_context.Nullify();
    }

    if (!m_view.IsNull())
    {
        m_view->Remove();
        m_view.Nullify();
    }

    m_viewer.Nullify();

    if (m_viewInitialized)
    {
        makeCurrent();
        displayConnection.Nullify();
        doneCurrent();
    }
}

QSize OcctViewWidget::minimumSizeHint() const
{
    return QSize(320, 240);
}

QSize OcctViewWidget::sizeHint() const
{
    return QSize(900, 640);
}

void OcctViewWidget::clearScene()
{
    m_pendingResult.clear();
    m_objects.clear();
    m_loadedModel = false;

    if (!m_context.IsNull())
    {
        m_context->RemoveAll(false);
        if (!m_view.IsNull())
            m_view->Invalidate();
    }

    updateView();
}

void OcctViewWidget::setImportedModel(const OcctImportResultPtr &result)
{
    m_pendingResult = result;
    if (!m_viewInitialized)
        return;

    displayImportedModel();
}

void OcctViewWidget::fitAll()
{
    if (m_view.IsNull() || !m_loadedModel)
        return;

    m_view->FitAll(0.01, false);
    m_view->ZFitAll();
    updateView();
}

void OcctViewWidget::setIsometricView()
{
    if (m_view.IsNull())
        return;

    m_view->SetProj(V3d_XposYnegZpos);
    fitAll();
}

void OcctViewWidget::setFrontView()
{
    if (m_view.IsNull())
        return;

    m_view->SetProj(V3d_Yneg);
    fitAll();
}

void OcctViewWidget::setTopView()
{
    if (m_view.IsNull())
        return;

    m_view->SetProj(V3d_Zpos);
    fitAll();
}

void OcctViewWidget::setRightView()
{
    if (m_view.IsNull())
        return;

    m_view->SetProj(V3d_Xpos);
    fitAll();
}

void OcctViewWidget::setShadedMode()
{
    applyDisplayMode(AIS_Shaded);
}

void OcctViewWidget::setWireframeMode()
{
    applyDisplayMode(AIS_WireFrame);
}

bool OcctViewWidget::hasLoadedModel() const
{
    return m_loadedModel;
}

void OcctViewWidget::initializeGL()
{
    if (m_viewer.IsNull() || m_view.IsNull())
        createOcctViewer();

    Handle(OpenGl_GraphicDriver) driver =
            Handle(OpenGl_GraphicDriver)::DownCast(m_viewer->Driver());
    OcctQtTools::qtGlCapsFromSurfaceFormat(driver->ChangeOptions(), format());

    const Aspect_Drawable nativeWin = toAspectDrawable(effectiveWinId());
    const Graphic3d_Vec2i viewSize(width(), height());
    const bool firstInit = m_view->Window().IsNull();

    if (!OcctGlTools::InitializeGlWindow(m_view, nativeWin, viewSize, devicePixelRatioF()))
        return;

    makeCurrent();
    m_viewInitialized = true;

    if (firstInit && !m_pendingResult.isNull())
        displayImportedModel();
}

void OcctViewWidget::paintGL()
{
    if (m_view.IsNull() || m_view->Window().IsNull())
        return;

    const double oldPixelRatio = m_view->Window()->DevicePixelRatio();
    if (m_view->Window()->NativeHandle()
            != OcctGlTools::GetGlNativeWindow(toAspectDrawable(effectiveWinId()))
            || devicePixelRatioF() != oldPixelRatio)
    {
        initializeGL();
    }

    if (!OcctGlTools::InitializeGlFbo(m_view))
        return;

    if (!OcctGlTools::BindShaderManagerToContext(m_view))
        return;

    OcctGlTools::ResetGlStateBeforeOcct(m_view);
    m_view->InvalidateImmediate();
    AIS_ViewController::FlushViewEvents(m_context, m_view, true);
    OcctGlTools::ResetGlStateAfterOcct(m_view);
}

bool OcctViewWidget::event(QEvent *event)
{
    return QOpenGLWidget::event(event);
}

void OcctViewWidget::keyPressEvent(QKeyEvent *event)
{
    if (OcctQtTools::qtKey2VKey(event->key()) == Aspect_VKey_F)
    {
        fitAll();
        event->accept();
        return;
    }

    QOpenGLWidget::keyPressEvent(event);
}

void OcctViewWidget::mousePressEvent(QMouseEvent *event)
{
    QOpenGLWidget::mousePressEvent(event);
    if (OcctQtTools::qtHandleMouseEvent(*this, m_view, event))
    {
        event->accept();
        updateView();
    }
}

void OcctViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QOpenGLWidget::mouseReleaseEvent(event);
    if (OcctQtTools::qtHandleMouseEvent(*this, m_view, event))
    {
        event->accept();
        updateView();
    }
}

void OcctViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    QOpenGLWidget::mouseMoveEvent(event);
    if (OcctQtTools::qtHandleMouseEvent(*this, m_view, event))
    {
        event->accept();
        updateView();
    }
}

void OcctViewWidget::wheelEvent(QWheelEvent *event)
{
    QOpenGLWidget::wheelEvent(event);
    if (OcctQtTools::qtHandleWheelEvent(*this, m_view, event))
    {
        event->accept();
        updateView();
    }
}

void OcctViewWidget::createOcctViewer()
{
    if (!m_viewer.IsNull())
        return;

    Handle(Aspect_DisplayConnection) displayConnection = new Aspect_DisplayConnection();
    Handle(OpenGl_GraphicDriver) driver = new OpenGl_GraphicDriver(displayConnection, false);
    driver->ChangeOptions().buffersNoSwap = true;
    driver->ChangeOptions().buffersOpaqueAlpha = true;
    driver->ChangeOptions().useSystemBuffer = false;

    m_viewer = new V3d_Viewer(driver);
    m_viewer->SetDefaultBackgroundColor(OcctQtTools::qtColorToOcct(palette().base().color()));
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();

    m_context = new AIS_InteractiveContext(m_viewer);
    m_context->SetDisplayMode(m_displayMode, false);

    m_view = m_viewer->CreateView();
    m_view->SetImmediateUpdate(false);
    m_view->SetBackgroundColor(OcctQtTools::qtColorToOcct(palette().base().color()));
#ifndef __APPLE__
    m_view->ChangeRenderingParams().NbMsaaSamples = 4;
#endif
}

void OcctViewWidget::displayImportedModel()
{
    if (m_pendingResult.isNull()
            || m_context.IsNull())
        return;

    m_context->RemoveAll(false);
    m_objects.clear();
    m_loadedModel = false;

    if (!m_pendingResult->document.IsNull())
    {
        for (Standard_Integer i = 1; i <= m_pendingResult->rootLabels.Length(); ++i)
        {
            Handle(XCAFPrs_AISObject) object =
                    new XCAFPrs_AISObject(m_pendingResult->rootLabels.Value(i));
            m_context->Display(object, false);
            m_context->SetDisplayMode(object, m_displayMode, false);
            m_objects.push_back(object);
        }
    }

    for (const Handle(Poly_Triangulation) &triangulation : m_pendingResult->triangulations)
    {
        if (triangulation.IsNull() || !triangulation->HasGeometry())
            continue;

        Handle(AIS_Triangulation) object = new AIS_Triangulation(triangulation);
        m_context->Display(object, 0, -1, false);
        m_objects.push_back(object);
    }

    m_loadedModel = !m_objects.empty();
    if (m_loadedModel)
    {
        setIsometricView();
        fitAll();
    }

    m_view->Invalidate();
    updateView();
}

void OcctViewWidget::applyDisplayMode(int mode)
{
    m_displayMode = mode;
    if (m_context.IsNull())
        return;

    for (const Handle(AIS_InteractiveObject) &object : m_objects)
    {
        if (!object.IsNull() && Handle(AIS_Triangulation)::DownCast(object).IsNull())
            m_context->SetDisplayMode(object, m_displayMode, false);
    }

    if (!m_view.IsNull())
        m_view->Invalidate();
    updateView();
}

void OcctViewWidget::updateView()
{
    update();
}

void OcctViewWidget::handleViewRedraw(const Handle(AIS_InteractiveContext) &context,
                                      const Handle(V3d_View) &view)
{
    AIS_ViewController::handleViewRedraw(context, view);
    if (myToAskNextFrame)
        updateView();
}
