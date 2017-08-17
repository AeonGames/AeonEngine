/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "OpenGLWindow.h"
#include "OpenGLRenderer.h"
#include "OpenGLModel.h"
#include "OpenGLFunctions.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>

namespace AeonGames
{
    OpenGLWindow::OpenGLWindow ( void* aWindowId, const std::shared_ptr<const OpenGLRenderer> aOpenGLRenderer ) :
        mOpenGLRenderer ( aOpenGLRenderer ), mWindowId ( aWindowId )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    OpenGLWindow::~OpenGLWindow()
    {
        Finalize();
    }

    void* OpenGLWindow::GetWindowId() const
    {
        return mWindowId;
    }

    void OpenGLWindow::ResizeViewport ( uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight )
        {
#ifdef WIN32
            HDC hdc = GetDC ( reinterpret_cast<HWND> ( mWindowId ) );
            wglMakeCurrent ( hdc, reinterpret_cast<HGLRC> ( mOpenGLRenderer->GetOpenGLContext() ) );
            OPENGL_CHECK_ERROR_NO_THROW;
            ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), hdc );
#else
            glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                             reinterpret_cast<::Window> ( mWindowId ),
                             static_cast<GLXContext> ( mOpenGLRenderer->GetOpenGLContext() ) );
            OPENGL_CHECK_ERROR_NO_THROW;
#endif
            glViewport ( 0, 0, aWidth, aHeight );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLWindow::BeginRender() const
    {
#ifdef _WIN32
        HDC hdc = GetDC ( reinterpret_cast<HWND> ( mWindowId ) );
        wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLRenderer->GetOpenGLContext() ) );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), hdc );
#else
        glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ),
                         static_cast<GLXContext> ( mOpenGLRenderer->GetOpenGLContext() ) );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#endif
    }

    void OpenGLWindow::Render ( const std::shared_ptr<RenderModel> aModel, size_t aAnimationIndex, float aTime ) const
    {
        static_cast<OpenGLModel*> ( aModel.get() )->Render ( aAnimationIndex, aTime );
    }

    void OpenGLWindow::EndRender() const
    {
#if _WIN32
        HDC hdc = GetDC ( reinterpret_cast<HWND> ( mWindowId ) );
        wglMakeCurrent ( hdc, reinterpret_cast<HGLRC> ( mOpenGLRenderer->GetOpenGLContext() ) );
        SwapBuffers ( hdc );
        ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), hdc );
#else
        glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ),
                         static_cast<GLXContext> ( mOpenGLRenderer->GetOpenGLContext() ) );
        glXSwapBuffers ( static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ) );
#endif
    }

    void OpenGLWindow::Initialize()
    {
        if ( !mOpenGLRenderer )
        {
            throw std::runtime_error ( "Pointer to OpenGL Renderer is nullptr." );
        }
#if _WIN32
        HDC hdc = GetDC ( static_cast<HWND> ( mWindowId ) );
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int pf = ChoosePixelFormat ( hdc, &pfd );
        SetPixelFormat ( hdc, pf, &pfd );
        wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLRenderer->GetOpenGLContext() ) );
        ReleaseDC ( static_cast<HWND> ( mWindowId ), hdc );
        RECT rect;
        GetClientRect ( static_cast<HWND> ( mWindowId ), &rect );
        glViewport ( 0, 0, rect.right, rect.bottom );
        OPENGL_CHECK_ERROR_THROW;
#else
        if ( !glXMakeCurrent (  static_cast<Display*> ( mOpenGLRenderer->GetWindowId() ),
                                reinterpret_cast<::Window> ( mWindowId ),
                                static_cast<GLXContext> ( mOpenGLRenderer->GetOpenGLContext() ) ) )
        {
            throw std::runtime_error ( "Failed to make OpenGL current to XWindow." );
        }
#endif
        glClearColor ( 0.5f, 0.5f, 0.5f, 1.0f );
        OPENGL_CHECK_ERROR_NO_THROW;
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_BLEND );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDepthFunc ( GL_LESS );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_NO_THROW;
        glCullFace ( GL_BACK );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_CULL_FACE );
        OPENGL_CHECK_ERROR_NO_THROW;
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::Finalize()
    {
    }
}
