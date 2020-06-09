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
#include "OpenGLX11Window.h"
#include "OpenGLRenderer.h"
#include "OpenGLFunctions.h"
#include "aeongames/LogLevel.h"
#include "aeongames/MemoryPool.h" ///<- This is here just for the literals

namespace AeonGames
{
    GLXContext CreateGLXContext ( Display* display, GLXContext share_context );
    OpenGLX11Window::OpenGLX11Window ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        OpenGLWindow { aOpenGLRenderer, aX, aY, aWidth, aHeight, aFullScreen }
    {
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

    void OpenGLX11Window::MakeCurrent()
    {
        if ( !glXMakeCurrent ( mDisplay, reinterpret_cast<::Window> ( mWindowId ), mGLXContext ) )
        {
            std::cout << LogLevel ( LogLevel::Warning ) <<
                      "glxMakeCurrent Failed." << std::endl;
        }
    }

    void OpenGLX11Window::SwapBuffers()
    {
        glXSwapBuffers ( mDisplay,
                         reinterpret_cast<::Window> ( mWindowId ) );
    }

    void OpenGLX11Window::Initialize()
    {
        mGLXContext = CreateGLXContext ( mDisplay, reinterpret_cast<GLXContext> ( mOpenGLRenderer.GetOpenGLContext() ) );
        XSetErrorHandler ( [] ( Display * display, XErrorEvent * error_event ) -> int
        {
            char error_string[1024];
            XGetErrorText ( display, error_event->error_code, error_string, 1024 );
            std::cout << LogLevel::Error << "Error Code " << static_cast<int> ( error_event->error_code ) << " " << error_string << std::endl;
            return 0;
        } );
        if ( !glXMakeCurrent (  mDisplay,
                                mWindowId,
                                mGLXContext ) )
        {
            XSetErrorHandler ( nullptr );
            throw std::runtime_error ( "glXMakeCurrent call Failed." );
        }
        XSetErrorHandler ( nullptr );
    }

    void OpenGLX11Window::Finalize()
    {
        glXMakeCurrent (  mDisplay,
                          None,
                          mGLXContext );
        OPENGL_CHECK_ERROR_NO_THROW;
    }
}
#endif
