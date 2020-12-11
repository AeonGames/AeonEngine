/*
Copyright (C) 2019,2020 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLX11Window.h"
#include "OpenGLRenderer.h"
#include "OpenGLX11Renderer.h"
#include "OpenGLFunctions.h"
#include "aeongames/LogLevel.h"
#include "aeongames/MemoryPool.h" ///<- This is here just for the literals

namespace AeonGames
{
    std::ostream &operator<< ( std::ostream &out, const XVisualInfo& aXVisualInfo );
    OpenGLX11Window::OpenGLX11Window ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        OpenGLWindow { aOpenGLRenderer, aX, aY, aWidth, aHeight, aFullScreen }
    {
        ::Window root = DefaultRootWindow ( mDisplay );
        GLXFBConfig config = reinterpret_cast<const OpenGLX11Renderer*> ( &mOpenGLRenderer )->GetGLXFBConfig();
        XVisualInfo* xvisualid = glXGetVisualFromFBConfig ( mDisplay, config );
        Colormap cmap = XCreateColormap ( mDisplay, root, xvisualid->visual, AllocNone );
        XSetWindowAttributes swa
        {
            .background_pixmap = None,
            .background_pixel  = 0,
            .border_pixel      = 0,
            .event_mask = StructureNotifyMask | KeyPressMask | ExposureMask,
            .colormap = cmap,
        };
        mWindowId = XCreateWindow (
                        mDisplay,
                        root,
                        aX, aY,
                        aWidth, aHeight,
                        0,
                        DefaultDepth ( mDisplay, DefaultScreen ( mDisplay ) ), InputOutput, xvisualid->visual, CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask, &swa
                    );
        XFree ( xvisualid );
        SetWindowForId ( reinterpret_cast<void*> ( mWindowId ), this );
        XStoreName ( mDisplay, mWindowId, "AeonGames" );
        try
        {
            Initialize();
            OpenGLWindow::Initialize();
        }
        catch ( ... )
        {
            OpenGLWindow::Finalize();
            Finalize();
            throw;
        }
    }

    OpenGLX11Window::OpenGLX11Window ( const OpenGLRenderer& aOpenGLRenderer, void* aWindowId ) :
        OpenGLWindow{aOpenGLRenderer, aWindowId}
    {
        try
        {
            XWindowAttributes xwindowattributes{};
            XGetWindowAttributes ( mDisplay, mWindowId, &xwindowattributes );
            mOverlay.Initialize ( xwindowattributes.width, xwindowattributes.height, Texture::Format::RGBA, Texture::Type::UNSIGNED_INT_8_8_8_8_REV );
            Initialize();
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
        return glXMakeCurrent ( mDisplay, mWindowId, reinterpret_cast<GLXContext> ( mOpenGLRenderer.GetContext() ) );
    }

    void OpenGLX11Window::SwapBuffers()
    {
        glXSwapBuffers ( mDisplay, mWindowId );
    }

    void OpenGLX11Window::Initialize()
    {
        if ( !MakeCurrent() )
        {
            throw std::runtime_error ( "glXMakeCurrent call Failed." );
        }
    }

    void OpenGLX11Window::Finalize()
    {
        mOpenGLRenderer.MakeCurrent();
        OPENGL_CHECK_ERROR_NO_THROW;
        RemoveWindowForId ( reinterpret_cast<void*> ( mWindowId ) );
        if ( mDisplay != nullptr )
        {
            XDestroyWindow ( mDisplay, mWindowId );
            XCloseDisplay ( mDisplay );
        }
    }
}
#endif
