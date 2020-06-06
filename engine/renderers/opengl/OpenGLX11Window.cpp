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

    OpenGLWindow::OpenGLWindow ( const OpenGLRenderer&  aOpenGLRenderer, void* aWindowId ) :
        Window{aWindowId},
        mOpenGLRenderer{ aOpenGLRenderer },
        mOverlay{},
        mMemoryPoolBuffer{aOpenGLRenderer, static_cast<GLsizei> ( 8_mb ) }
    {
        int x_return, y_return;
        ::Window root_return;
        unsigned int width_return, height_return;
        unsigned int border_width_return;
        unsigned int depth_return;
        XGetGeometry ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                       reinterpret_cast<::Window> ( mWindowId ),
                       &root_return, &x_return, &y_return, &width_return,
                       &height_return, &border_width_return, &depth_return );
        mOverlay.Initialize ( width_return, height_return, Texture::Format::RGBA, Texture::Type::UNSIGNED_INT_8_8_8_8_REV );
    }

    OpenGLX11Window::~OpenGLX11Window()
    {
        OpenGLWindow::Finalize();
        Finalize();
    }

    void OpenGLX11Window::MakeCurrent()
    {
        if ( !glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ), reinterpret_cast<::Window> ( mWindowId ), static_cast<GLXContext> ( mOpenGLRenderer.GetOpenGLContext() ) ) )
        {
            std::cout << LogLevel ( LogLevel::Warning ) <<
                      "glxMakeCurrent Failed." << std::endl;
        }
    }

    void OpenGLX11Window::SwapBuffers()
    {
        glXSwapBuffers ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ) );
    }

    void OpenGLX11Window::Initialize()
    {
        XSetErrorHandler ( [] ( Display * display, XErrorEvent * error_event ) -> int
        {
            char error_string[1024];
            XGetErrorText ( display, error_event->error_code, error_string, 1024 );
            std::cout << LogLevel::Error << "Error Code " << static_cast<int> ( error_event->error_code ) << " " << error_string << std::endl;
            return 0;
        } );
        if ( !glXMakeCurrent (  static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                                reinterpret_cast<::Window> ( mWindowId ),
                                static_cast<GLXContext> ( mOpenGLRenderer.GetOpenGLContext() ) ) )
        {
            XSetErrorHandler ( nullptr );
            throw std::runtime_error ( "glXMakeCurrent call Failed." );
        }
        XSetErrorHandler ( nullptr );
    }

    void OpenGLX11Window::Finalize()
    {
        glXMakeCurrent (  static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                          reinterpret_cast<::Window> ( mWindowId ),
                          static_cast<GLXContext> ( mOpenGLRenderer.GetOpenGLContext() ) );
        OPENGL_CHECK_ERROR_NO_THROW;
    }
}
#endif
