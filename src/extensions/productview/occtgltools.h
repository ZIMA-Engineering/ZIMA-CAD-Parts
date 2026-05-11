// Copyright (c) 2023 Kirill Gavrilov
// SPDX-License-Identifier: MIT
//
// Adapted from gkv311/occt-samples-qt. See licenses/occt-samples-qt-MIT.txt.

#ifndef OCCTGLTOOLS_H
#define OCCTGLTOOLS_H

#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <V3d_View.hxx>

class OpenGl_Context;

class OcctGlTools
{
public:
    class OcctNeutralWindow : public Aspect_NeutralWindow
    {
    public:
        OcctNeutralWindow() {}

        virtual double DevicePixelRatio() const override { return m_pixelRatio; }
        void SetDevicePixelRatio(double ratio) { m_pixelRatio = ratio; }

    private:
        double m_pixelRatio = 1.0;
    };

    static Handle(OpenGl_Context) GetGlContext(const Handle(V3d_View) &view);
    static bool BindShaderManagerToContext(const Handle(V3d_View) &view);
    static Aspect_Drawable GetGlNativeWindow(Aspect_Drawable nativeWin);
    static bool InitializeGlWindow(const Handle(V3d_View) &view,
                                   Aspect_Drawable nativeWin,
                                   const Graphic3d_Vec2i &size,
                                   double pixelRatio);
    static bool InitializeGlFbo(const Handle(V3d_View) &view);
    static void ResetGlStateBeforeOcct(const Handle(V3d_View) &view);
    static void ResetGlStateAfterOcct(const Handle(V3d_View) &view);
};

#endif // OCCTGLTOOLS_H
