/*
Copyright (C) 2019-2021 Rodrigo Jose Hernandez Cordoba

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
#ifdef __unix__
#include <cstring>
#include "aeongames/ProtoBufClasses.h"
#include "OpenGLX11Window.h"
#include "OpenGLRenderer.h"
#include "OpenGLX11Renderer.h"
#include "OpenGLFunctions.h"
#include "aeongames/LogLevel.h"
#include "aeongames/MemoryPool.h" ///<- This is here just for the literals

namespace AeonGames
{
    std::ostream &operator<< ( std::ostream &out, const XVisualInfo& aXVisualInfo );

    OpenGLX11Window::OpenGLX11Window (
        const OpenGLRenderer& aOpenGLRenderer,
        int32_t aX,
        int32_t aY,
        uint32_t aWidth,
        uint32_t aHeight,
        bool aFullScreen ) :
        OpenGLWindow { aOpenGLRenderer, aX, aY, aWidth, aHeight, aFullScreen }
    {
        try
        {
            if ( !MakeCurrent() )
            {
                throw std::runtime_error ( "glXMakeCurrent call Failed." );
            }
            OpenGLWindow::Initialize();
        }
        catch ( ... )
        {
            OpenGLWindow::Finalize();
            OpenGLX11Window::Finalize();
            throw;
        }
    }

    OpenGLX11Window::OpenGLX11Window ( const OpenGLRenderer& aOpenGLRenderer, void* aWindowId ) :
        OpenGLWindow{aOpenGLRenderer, aWindowId}
    {
        try
        {
            if ( !MakeCurrent() )
            {
                throw std::runtime_error ( "glXMakeCurrent call Failed." );
            }
            XWindowAttributes xwindowattributes {};
            XGetWindowAttributes ( GetDisplay(), mWindowId, &xwindowattributes );
            //mOverlay.Resize ( xwindowattributes.width, xwindowattributes.height, nullptr, Texture::Format::RGBA, Texture::Type::UNSIGNED_INT_8_8_8_8_REV );
            OpenGLWindow::Initialize();
        }
        catch ( ... )
        {
            OpenGLWindow::Finalize();
            Finalize();
            throw;
        }
    }

    OpenGLX11Window::~OpenGLX11Window()
    {
        OpenGLWindow::Finalize();
        Finalize();
    }

    bool OpenGLX11Window::MakeCurrent()
    {
        GLXContext context = reinterpret_cast<GLXContext> ( mOpenGLRenderer.GetContext() );
        return glXMakeCurrent ( GetDisplay(), mWindowId, context );
    }

    void OpenGLX11Window::SwapBuffers()
    {
        glXSwapBuffers ( GetDisplay(), mWindowId );
    }

    void OpenGLX11Window::Initialize()
    {
    }

    void OpenGLX11Window::Finalize()
    {
        mOpenGLRenderer.MakeCurrent();
        OPENGL_CHECK_ERROR_NO_THROW;
        RemoveWindowForId ( reinterpret_cast<void*> ( mWindowId ) );
    }
}
#endif
