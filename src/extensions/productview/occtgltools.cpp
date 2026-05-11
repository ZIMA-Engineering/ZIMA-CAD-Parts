// Copyright (c) 2023 Kirill Gavrilov
// SPDX-License-Identifier: MIT
//
// Adapted from gkv311/occt-samples-qt. See licenses/occt-samples-qt-MIT.txt.

#ifdef _WIN32
#include <windows.h>
#endif

#include "occtgltools.h"

#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_GlCore20.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OpenGl_ShaderManager.hxx>
#include <OpenGl_View.hxx>
#include <OpenGl_Window.hxx>
#include <Message.hxx>
#include <Standard_Version.hxx>

class OcctQtFrameBuffer : public OpenGl_FrameBuffer
{
    DEFINE_STANDARD_RTTI_INLINE(OcctQtFrameBuffer, OpenGl_FrameBuffer)

public:
    OcctQtFrameBuffer() {}

    virtual void BindBuffer(const Handle(OpenGl_Context) &glContext) override
    {
        OpenGl_FrameBuffer::BindBuffer(glContext);
        glContext->SetFrameBufferSRGB(true, false);
    }

    virtual void BindDrawBuffer(const Handle(OpenGl_Context) &glContext) override
    {
        OpenGl_FrameBuffer::BindDrawBuffer(glContext);
        glContext->SetFrameBufferSRGB(true, false);
    }

    virtual void BindReadBuffer(const Handle(OpenGl_Context) &glContext) override
    {
        OpenGl_FrameBuffer::BindReadBuffer(glContext);
    }
};

Handle(OpenGl_Context) OcctGlTools::GetGlContext(const Handle(V3d_View) &view)
{
    if (view.IsNull() || view->View().IsNull())
        return Handle(OpenGl_Context)();

    Handle(OpenGl_View) glView = Handle(OpenGl_View)::DownCast(view->View());
    if (glView.IsNull() || glView->GlWindow().IsNull())
        return Handle(OpenGl_Context)();

    return glView->GlWindow()->GetGlContext();
}

bool OcctGlTools::BindShaderManagerToContext(const Handle(V3d_View) &view)
{
    Handle(OpenGl_Context) glContext = GetGlContext(view);
    if (glContext.IsNull() || glContext->ShaderManager().IsNull())
        return false;

    glContext->ShaderManager()->SetContext(glContext.operator->());
    return true;
}

Aspect_Drawable OcctGlTools::GetGlNativeWindow(Aspect_Drawable nativeWin)
{
#ifdef _WIN32
    HDC wglDeviceContext = wglGetCurrentDC();
    HWND wglWindow = WindowFromDC(wglDeviceContext);
    return reinterpret_cast<Aspect_Drawable>(wglWindow);
#else
    return nativeWin;
#endif
}

bool OcctGlTools::InitializeGlWindow(const Handle(V3d_View) &view,
                                     Aspect_Drawable nativeWin,
                                     const Graphic3d_Vec2i &size,
                                     double pixelRatio)
{
    const Aspect_Drawable actualNativeWin = GetGlNativeWindow(nativeWin);

    Handle(OpenGl_GraphicDriver) driver =
            Handle(OpenGl_GraphicDriver)::DownCast(view->Viewer()->Driver());
    Handle(OpenGl_Context) glContext = new OpenGl_Context();
    if (!glContext->Init(!driver->Options().contextCompatible))
    {
        Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
        return false;
    }

    Handle(OcctNeutralWindow) window = Handle(OcctNeutralWindow)::DownCast(view->Window());
    if (window.IsNull())
    {
        window = new OcctNeutralWindow();
        window->SetVirtual(true);
    }

    window->SetNativeHandle(actualNativeWin);
    window->SetSize(size.x(), size.y());
    window->SetDevicePixelRatio(pixelRatio);
    view->SetWindow(window, glContext->RenderingContext());
    BindShaderManagerToContext(view);
    view->MustBeResized();
    view->Invalidate();

#if (OCC_VERSION_HEX >= 0x070700)
    for (const Handle(V3d_View) &subview : view->Subviews())
    {
        subview->MustBeResized();
        subview->Invalidate();
    }
#endif

    return true;
}

bool OcctGlTools::InitializeGlFbo(const Handle(V3d_View) &view)
{
    Handle(OpenGl_Context) glContext = GetGlContext(view);
    if (glContext.IsNull() || glContext->ShaderManager().IsNull())
        return false;

    glContext->ShaderManager()->SetContext(glContext.operator->());

    Handle(OcctQtFrameBuffer) defaultFbo =
            Handle(OcctQtFrameBuffer)::DownCast(glContext->DefaultFrameBuffer());
    if (defaultFbo.IsNull())
        defaultFbo = new OcctQtFrameBuffer();

    if (!defaultFbo->InitWrapper(glContext))
    {
        defaultFbo.Nullify();
        Message::DefaultMessenger()->Send("Default FBO wrapper creation failed", Message_Fail);
        return false;
    }

    glContext->SetDefaultFrameBuffer(Handle(OpenGl_FrameBuffer)());

    Graphic3d_Vec2i oldViewSize;
    const Graphic3d_Vec2i newViewSize = defaultFbo->GetVPSize();
    Handle(OcctNeutralWindow) window = Handle(OcctNeutralWindow)::DownCast(view->Window());
    window->Size(oldViewSize.x(), oldViewSize.y());
    if (newViewSize != oldViewSize)
    {
        window->SetSize(newViewSize.x(), newViewSize.y());
        view->MustBeResized();
        view->Invalidate();

#if (OCC_VERSION_HEX >= 0x070700)
        for (const Handle(V3d_View) &subview : view->Subviews())
        {
            subview->MustBeResized();
            subview->Invalidate();
            defaultFbo->SetupViewport(glContext);
        }
#endif
    }

    glContext->SetDefaultFrameBuffer(defaultFbo);
    return true;
}

void OcctGlTools::ResetGlStateBeforeOcct(const Handle(V3d_View) &view)
{
    Handle(OpenGl_Context) glContext = GetGlContext(view);
    if (glContext.IsNull())
        return;

    if (glContext->core20fwd != nullptr)
        glContext->core20fwd->glUseProgram(0);

    glContext->core11fwd->glBindTexture(GL_TEXTURE_2D, 0);
    glContext->core11fwd->glDisable(GL_BLEND);

#ifndef HAVE_GLES2
    if (glContext->core11ffp != nullptr)
    {
        glContext->core11fwd->glDisable(GL_ALPHA_TEST);
        glContext->core11fwd->glDisable(GL_TEXTURE_2D);
    }
#endif
}

void OcctGlTools::ResetGlStateAfterOcct(const Handle(V3d_View) &view)
{
    Handle(OpenGl_Context) glContext = GetGlContext(view);
    if (glContext.IsNull())
        return;

    glContext->core11fwd->glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glContext->core11fwd->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    if (glContext->core15fwd != nullptr)
        glContext->core15fwd->glActiveTexture(GL_TEXTURE0);
}
