// Copyright (c) 2025 Kirill Gavrilov
// SPDX-License-Identifier: MIT
//
// Adapted from gkv311/occt-samples-qt. See licenses/occt-samples-qt-MIT.txt.

#ifndef OCCTQTTOOLS_H
#define OCCTQTTOOLS_H

#include <Aspect_WindowInputListener.hxx>
#include <Quantity_Color.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QColor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <Standard_WarningsRestore.hxx>

class OpenGl_Caps;
class V3d_View;

class OcctQtTools
{
public:
    static Quantity_Color qtColorToOcct(const QColor &color);
    static void qtGlCapsFromSurfaceFormat(OpenGl_Caps &caps, const QSurfaceFormat &format);

    static bool qtHandleMouseEvent(Aspect_WindowInputListener &listener,
                                   const Handle(V3d_View) &view,
                                   const QMouseEvent *event);
    static bool qtHandleWheelEvent(Aspect_WindowInputListener &listener,
                                   const Handle(V3d_View) &view,
                                   const QWheelEvent *event);

    static Aspect_VKeyMouse qtMouseButtons2VKeys(Qt::MouseButtons buttons);
    static Aspect_VKeyFlags qtMouseModifiers2VKeys(Qt::KeyboardModifiers modifiers);
    static Aspect_VKey qtKey2VKey(int key);
};

#endif // OCCTQTTOOLS_H
