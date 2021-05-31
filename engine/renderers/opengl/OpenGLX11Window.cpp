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
#include <X11/Xlib.h>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/Scene.h"
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
        ::Window root = DefaultRootWindow ( reinterpret_cast<const OpenGLX11Renderer&> ( aOpenGLRenderer ).GetDisplay() );
        XVisualInfo* xvi = glXGetVisualFromFBConfig ( reinterpret_cast<const OpenGLX11Renderer&> ( aOpenGLRenderer ).GetDisplay(),
                           GetGLXConfig ( reinterpret_cast<const OpenGLX11Renderer&> ( aOpenGLRenderer ).GetDisplay() ) );
        mColorMap = XCreateColormap ( reinterpret_cast<const OpenGLX11Renderer&> ( aOpenGLRenderer ).GetDisplay(),
                                      DefaultRootWindow ( reinterpret_cast<const OpenGLX11Renderer&> ( aOpenGLRenderer ).GetDisplay() ), xvi->visual, AllocNone );
        XSetWindowAttributes swa
        {
            .background_pixmap = None,
            .background_pixel  = 0,
            .border_pixel      = 0,
            .event_mask = StructureNotifyMask | KeyPressMask | ExposureMask,
            .colormap = mColorMap,
        };
        mWindowId = XCreateWindow (
                        reinterpret_cast<const OpenGLX11Renderer&> ( aOpenGLRenderer ).GetDisplay(),
                        root,
                        aX, aY,
                        aWidth, aHeight,
                        0,
                        xvi->depth, InputOutput, xvi->visual, CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask, &swa
                    );
        XFree ( xvi );
        SetWindowForId ( reinterpret_cast<void*> ( mWindowId ), this );
        XStoreName ( reinterpret_cast<const OpenGLX11Renderer&> ( aOpenGLRenderer ).GetDisplay(), mWindowId, "AeonGames" );

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
        OpenGLWindow{aOpenGLRenderer}, mWindowId{ reinterpret_cast<::Window> ( aWindowId ) }
    {
        SetWindowForId ( aWindowId, this );
        try
        {
            if ( !MakeCurrent() )
            {
                throw std::runtime_error ( "glXMakeCurrent call Failed." );
            }
            XWindowAttributes xwindowattributes {};
            XGetWindowAttributes ( reinterpret_cast<const OpenGLX11Renderer&> ( aOpenGLRenderer ).GetDisplay(), mWindowId, &xwindowattributes );
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
        return glXMakeCurrent ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), mWindowId, context );
    }

    void OpenGLX11Window::SwapBuffers()
    {
        glXSwapBuffers ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), mWindowId );
    }

    void OpenGLX11Window::Initialize()
    {
    }

    void OpenGLX11Window::Finalize()
    {

        mOpenGLRenderer.MakeCurrent();
        OPENGL_CHECK_ERROR_NO_THROW;
        RemoveWindowForId ( reinterpret_cast<void*> ( mWindowId ) );
        if ( mColorMap )
        {
            XDestroyWindow ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), mWindowId );
            XFreeColormap ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), mColorMap );
        }
    }
    static const int visual_attribs[] =
    {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };

    GLXFBConfig OpenGLX11Window::GetGLXConfig ( Display* display )
    {
        int frame_buffer_config_count{};
        GLXFBConfig *frame_buffer_configs =
            glXChooseFBConfig ( display,
                                DefaultScreen ( display ),
                                visual_attribs, &frame_buffer_config_count );
        if ( !frame_buffer_configs )
        {
            throw std::runtime_error ( "Failed to retrieve a framebuffer config" );
        }

        std::sort ( frame_buffer_configs, frame_buffer_configs + frame_buffer_config_count,
                    [display] ( const GLXFBConfig & a, const GLXFBConfig & b )->bool
        {
            int a_sample_buffers{};
            int a_samples{};
            int b_sample_buffers{};
            int b_samples{};
            glXGetFBConfigAttrib ( display, a, GLX_SAMPLE_BUFFERS, &a_sample_buffers );
            glXGetFBConfigAttrib ( display, a, GLX_SAMPLES, &a_samples  );
            glXGetFBConfigAttrib ( display, b, GLX_SAMPLE_BUFFERS, &b_sample_buffers );
            glXGetFBConfigAttrib ( display, b, GLX_SAMPLES, &b_samples  );
            return a_sample_buffers >= b_sample_buffers && a_samples > b_samples;
        } );
        GLXFBConfig result = frame_buffer_configs[ 0 ];
        XFree ( frame_buffer_configs );
        return result;
    }

    void OpenGLX11Window::Run ( Scene& aScene )
    {
        bool running{true};
        XEvent xevent;
        Atom wm_delete_window = XInternAtom ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), "WM_DELETE_WINDOW", 0 );
        XSetWMProtocols ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), mWindowId, &wm_delete_window, 1 );
        std::chrono::high_resolution_clock::time_point last_time{std::chrono::high_resolution_clock::now() };

        SetScene ( &aScene );
        Show ( true );
        while ( running )
        {
            while ( ( XPending ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay() ) > 0 ) && running )
            {
                XNextEvent ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), &xevent );
                switch ( xevent.type )
                {
                case Expose:
                {
                    // Here is where window resize is required.
                    XWindowAttributes xwa;
                    XGetWindowAttributes ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), mWindowId, &xwa );
                    ResizeViewport ( 0, 0, xwa.width, xwa.height );
                }
                break;
                case KeyPress:
                    //engine.KeyDown ( GetScancode ( XLookupKeysym ( &xEvent.xkey, 0 ) ) );
                    break;
                case KeyRelease:
                    //engine.KeyUp ( GetScancode ( XLookupKeysym ( &xEvent.xkey, 0 ) ) );
                    break;
                case ButtonPress:
                    //engine.ButtonDown ( xEvent.xbutton.button, xEvent.xbutton.x, xEvent.xbutton.y );
                    break;
                case ButtonRelease:
                    //engine.ButtonUp ( xEvent.xbutton.button, xEvent.xbutton.x, xEvent.xbutton.y );
                    break;
                case MotionNotify:
                    //engine.MouseMove ( xEvent.xmotion.x, xEvent.xmotion.y );
                    break;
                case ClientMessage:
                    if ( static_cast<Atom> ( xevent.xclient.data.l[0] ) == wm_delete_window )
                    {
                        running = false;
                    }
                    break;
                case ConfigureNotify:
                    break;
                default:
                    std::cout << LogLevel::Info <<  "Received Event Type: " <<  xevent.type << std::endl;
                    break;
                }
            }
            std::chrono::high_resolution_clock::time_point current_time {std::chrono::high_resolution_clock::now() };
            std::chrono::duration<double> delta{std::chrono::duration_cast<std::chrono::duration<double>> ( current_time - last_time ) };
            aScene.Update ( delta.count() );
            last_time = current_time;
            RenderLoop();
        }
        Show ( false );
        SetScene ( nullptr );
    }

    void OpenGLX11Window::Show ( bool aShow ) const
    {
        if ( aShow )
        {
            XMapWindow ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), mWindowId );
        }
        else
        {
            XUnmapWindow ( reinterpret_cast<const OpenGLX11Renderer&> ( mOpenGLRenderer ).GetDisplay(), mWindowId );
        }
    }

    void OpenGLX11Window::StartRenderTimer() const
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    void OpenGLX11Window::StopRenderTimer() const
    {
        std::cout << __FUNCTION__ << std::endl;
    }
}
#endif
