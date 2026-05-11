// Copyright (c) 2025 Kirill Gavrilov
// SPDX-License-Identifier: MIT
//
// Adapted from gkv311/occt-samples-qt. See licenses/occt-samples-qt-MIT.txt.

#include "occtqttools.h"

#include <Aspect_ScrollDelta.hxx>
#include <OpenGl_Caps.hxx>
#include <Standard_Version.hxx>
#include <V3d_View.hxx>

Quantity_Color OcctQtTools::qtColorToOcct(const QColor &color)
{
    return Quantity_Color(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_sRGB);
}

void OcctQtTools::qtGlCapsFromSurfaceFormat(OpenGl_Caps &caps, const QSurfaceFormat &format)
{
    caps.contextDebug = format.testOption(QSurfaceFormat::DebugContext);
    caps.contextSyncDebug = caps.contextDebug;
    caps.contextCompatible = format.profile() != QSurfaceFormat::CoreProfile;
#if (OCC_VERSION_HEX >= 0x070700)
    caps.buffersDeepColor = format.redBufferSize() == 10
            && format.greenBufferSize() == 10
            && format.blueBufferSize() == 10;
#endif
}

bool OcctQtTools::qtHandleMouseEvent(Aspect_WindowInputListener &listener,
                                     const Handle(V3d_View) &view,
                                     const QMouseEvent *event)
{
    if (view.IsNull() || view->Window().IsNull())
        return false;

    const Graphic3d_Vec2d point2d(event->position().x(), event->position().y());
    const Graphic3d_Vec2i point2i(view->Window()->ConvertPointToBacking(point2d)
                                  + Graphic3d_Vec2d(0.5));
    const Aspect_VKeyMouse buttons = qtMouseButtons2VKeys(event->buttons());
    const Aspect_VKeyFlags flags = qtMouseModifiers2VKeys(event->modifiers());

    if (event->type() == QEvent::MouseMove)
        return listener.UpdateMousePosition(point2i, buttons, flags, false);

    return listener.UpdateMouseButtons(point2i, buttons, flags, false);
}

bool OcctQtTools::qtHandleWheelEvent(Aspect_WindowInputListener &listener,
                                     const Handle(V3d_View) &view,
                                     const QWheelEvent *event)
{
    if (view.IsNull() || view->Window().IsNull())
        return false;

    const Graphic3d_Vec2d point2d(event->position().x(), event->position().y());
    const Graphic3d_Vec2i point2i(view->Window()->ConvertPointToBacking(point2d)
                                  + Graphic3d_Vec2d(0.5));
    return listener.UpdateMouseScroll(Aspect_ScrollDelta(point2i,
                                                         double(event->angleDelta().y()) / 120.0));
}

Aspect_VKeyMouse OcctQtTools::qtMouseButtons2VKeys(Qt::MouseButtons buttons)
{
    Aspect_VKeyMouse keys = Aspect_VKeyMouse_NONE;
    if ((buttons & Qt::LeftButton) != 0)
        keys |= Aspect_VKeyMouse_LeftButton;
    if ((buttons & Qt::MiddleButton) != 0)
        keys |= Aspect_VKeyMouse_MiddleButton;
    if ((buttons & Qt::RightButton) != 0)
        keys |= Aspect_VKeyMouse_RightButton;
    return keys;
}

Aspect_VKeyFlags OcctQtTools::qtMouseModifiers2VKeys(Qt::KeyboardModifiers modifiers)
{
    Aspect_VKeyFlags flags = Aspect_VKeyFlags_NONE;
    if ((modifiers & Qt::ShiftModifier) != 0)
        flags |= Aspect_VKeyFlags_SHIFT;
    if ((modifiers & Qt::ControlModifier) != 0)
        flags |= Aspect_VKeyFlags_CTRL;
    if ((modifiers & Qt::AltModifier) != 0)
        flags |= Aspect_VKeyFlags_ALT;
    return flags;
}

Aspect_VKey OcctQtTools::qtKey2VKey(int key)
{
    switch (key)
    {
    case Qt::Key_F:
        return Aspect_VKey_F;
    default:
        return Aspect_VKey_UNKNOWN;
    }
}
