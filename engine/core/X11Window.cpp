/*
Copyright (C) 2017-2020 Rodrigo Jose Hernandez Cordoba

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
#if __unix__
#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <GL/glx.h>
#include <chrono>
#include "aeongames/X11Window.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Scene.h"

namespace AeonGames
{
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

    GLXFBConfig X11Window::GetGLXConfig()
    {
        int frame_buffer_config_count{};
        GLXFBConfig *frame_buffer_configs =
            glXChooseFBConfig ( GetDisplay(),
                                DefaultScreen ( GetDisplay() ),
                                visual_attribs, &frame_buffer_config_count );
        if ( !frame_buffer_configs )
        {
            throw std::runtime_error ( "Failed to retrieve a framebuffer config" );
        }

        std::sort ( frame_buffer_configs, frame_buffer_configs + frame_buffer_config_count,
                    [] ( const GLXFBConfig & a, const GLXFBConfig & b )->bool
        {
            int a_sample_buffers{};
            int a_samples{};
            int b_sample_buffers{};
            int b_samples{};
            glXGetFBConfigAttrib ( GetDisplay(), a, GLX_SAMPLE_BUFFERS, &a_sample_buffers );
            glXGetFBConfigAttrib ( GetDisplay(), a, GLX_SAMPLES, &a_samples  );
            glXGetFBConfigAttrib ( GetDisplay(), b, GLX_SAMPLE_BUFFERS, &b_sample_buffers );
            glXGetFBConfigAttrib ( GetDisplay(), b, GLX_SAMPLES, &b_samples  );
            return a_sample_buffers >= b_sample_buffers && a_samples > b_samples;
        } );
        GLXFBConfig result = frame_buffer_configs[ 0 ];
        XFree ( frame_buffer_configs );
        return result;
    }

    X11Window::X11Window ( void* aWindowId ) :
        mWindowId{ reinterpret_cast<::Window> ( aWindowId ) }
    {
        SetWindowForId ( aWindowId, this );
    }

    X11Window::X11Window ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        CommonWindow ( aX, aY, aWidth, aHeight, aFullScreen )
    {
        ::Window root = DefaultRootWindow ( GetDisplay() );
        XVisualInfo* xvi = glXGetVisualFromFBConfig ( GetDisplay(), GetGLXConfig() );
        mColorMap = XCreateColormap ( GetDisplay(), DefaultRootWindow ( GetDisplay() ), xvi->visual, AllocNone );
        XSetWindowAttributes swa
        {
            .background_pixmap = None,
            .background_pixel  = 0,
            .border_pixel      = 0,
            .event_mask = StructureNotifyMask | KeyPressMask | ExposureMask,
            .colormap = mColorMap,
        };
        mWindowId = XCreateWindow (
                        GetDisplay(),
                        root,
                        aX, aY,
                        aWidth, aHeight,
                        0,
                        xvi->depth, InputOutput, xvi->visual, CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask, &swa
                    );
        XFree ( xvi );
        SetWindowForId ( reinterpret_cast<void*> ( mWindowId ), this );
        XStoreName ( GetDisplay(), mWindowId, "AeonGames" );
    }

    DLL X11Window::~X11Window()
    {
        if ( mColorMap )
        {
            XDestroyWindow ( GetDisplay(), mWindowId );
            XFreeColormap ( GetDisplay(), mColorMap );
        }
    }

    void X11Window::Run ( Scene& aScene )
    {
        bool running{true};
        XEvent xevent;
        Atom wm_delete_window = XInternAtom ( GetDisplay(), "WM_DELETE_WINDOW", 0 );
        XSetWMProtocols ( GetDisplay(), mWindowId, &wm_delete_window, 1 );
        std::chrono::high_resolution_clock::time_point last_time{std::chrono::high_resolution_clock::now() };

        SetScene ( &aScene );
        Show ( true );
        while ( running )
        {
            while ( ( XPending ( GetDisplay() ) > 0 ) && running )
            {
                XNextEvent ( GetDisplay(), &xevent );
                switch ( xevent.type )
                {
                case Expose:
                {
                    // Here is where window resize is required.
                    XWindowAttributes xwa;
                    XGetWindowAttributes ( GetDisplay(), mWindowId, &xwa );
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

    void X11Window::Show ( bool aShow ) const
    {
        if ( aShow )
        {
            XMapWindow ( GetDisplay(), mWindowId );
        }
        else
        {
            XUnmapWindow ( GetDisplay(), mWindowId );
        }
    }

    void X11Window::StartRenderTimer() const
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    void X11Window::StopRenderTimer() const
    {
        std::cout << __FUNCTION__ << std::endl;
    }
}
#endif
