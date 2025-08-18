/*
Copyright (C) 2016,2018-2021,2024,2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include "Window.h"

int Main ( int argc, char *argv[] );

int ENTRYPOINT main ( int argc, char *argv[] )
{
    XSetErrorHandler ( [] ( Display * mDisplay, XErrorEvent * error_event ) -> int
    {
        char error_string[1024];
        XGetErrorText ( mDisplay, error_event->error_code, error_string, 1024 );
        std::cout << AeonGames::LogLevel::Error << error_string << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Error Code " << static_cast<int> ( error_event->error_code ) << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Request Code " << static_cast<int> ( error_event->request_code ) << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Minor Code " << static_cast<int> ( error_event->minor_code ) << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Display " << error_event->display << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Resource Id " << error_event->resourceid << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Serial " << error_event->serial << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Type " << error_event->type << std::endl;
        return 0;
    } );
    return Main ( argc, argv );
}

namespace AeonGames
{
    static int choose_visual_attribs[] =
    {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        None
    };

#if 0
    static const int choose_fb_config_attribs[] =
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

    static GLXFBConfig GetGLXConfig ( Display* display )
    {
        int frame_buffer_config_count{};
        GLXFBConfig *frame_buffer_configs =
            glXChooseFBConfig ( display,
                                DefaultScreen ( display ),
                                choose_fb_config_attribs, &frame_buffer_config_count );
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
#endif

    Window::Window ( const std::string& aRendererName, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        mDisplay{XOpenDisplay ( nullptr ) }
    {
        ::Window root = DefaultRootWindow ( mDisplay );
        XVisualInfo* xvi = glXChooseVisual ( mDisplay, DefaultScreen ( mDisplay ), choose_visual_attribs );
        if ( xvi == nullptr )
        {
            throw std::runtime_error ( "No supported visual reported by glXChooseVisual." );
        }
        mColorMap = XCreateColormap ( mDisplay, root, xvi->visual, AllocNone );
        XSetWindowAttributes swa
        {
            .background_pixmap = None,
            .background_pixel  = 0,
            .border_pixel      = 0,
            .event_mask = StructureNotifyMask | KeyPressMask | ExposureMask,
            .colormap = mColorMap,
        };
        mWindowId = XCreateWindow (
                        mDisplay,
                        root,
                        aX, aY,
                        aWidth, aHeight,
                        0,
                        xvi->depth, InputOutput, xvi->visual, CWBackPixmap | CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &swa
                    );
        XFree ( xvi );
        XStoreName ( mDisplay, mWindowId, "AeonGames" );
        XFlush ( mDisplay );
        mRenderer = ConstructRenderer ( aRendererName, reinterpret_cast<void*> ( mWindowId ) );
    }

    Window::~Window()
    {
        if ( mRenderer )
        {
            mRenderer->DetachWindow ( this );
        }
        if ( mWindowId )
        {
            XDestroyWindow ( mDisplay, mWindowId );
        }
        if ( mColorMap )
        {
            XFreeColormap ( mDisplay, mColorMap );
        }
        XCloseDisplay ( mDisplay );
        mDisplay = None;
    }

    uint32_t Window::Resize ( uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight && mRenderer )
        {
            mRenderer->ResizeViewport ( reinterpret_cast<void*> ( mWindowId ), 0, 0, aWidth, aHeight );
            mAspectRatio = static_cast<float> ( aWidth ) / static_cast<float> ( aHeight );
        }
        return 0;
    }

    void Window::Run ( Scene& aScene )
    {
        bool running{true};
        XEvent xevent;
        Atom wm_delete_window = XInternAtom ( mDisplay, "WM_DELETE_WINDOW", 0 );
        XSetWMProtocols ( mDisplay, mWindowId, &wm_delete_window, 1 );
        std::chrono::high_resolution_clock::time_point last_time{std::chrono::high_resolution_clock::now() };

        XMapWindow ( mDisplay, mWindowId );
        while ( running )
        {
            while ( ( XPending ( mDisplay ) > 0 ) && running )
            {
                XNextEvent ( mDisplay, &xevent );
                switch ( xevent.type )
                {
                case Expose:
                {
                    XWindowAttributes xwa;
                    XGetWindowAttributes ( mDisplay, mWindowId, &xwa );
                    Resize ( xwa.width, xwa.height );
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
            std::chrono::duration<double> delta{std::chrono::duration_cast<std::chrono::duration<double >> ( current_time - last_time ) };
            aScene.Update ( delta.count() );
            last_time = current_time;
            if ( mRenderer )
            {
                if ( const Node * camera = aScene.GetCamera() )
                {
                    mRenderer->SetViewMatrix ( reinterpret_cast<void*> ( mWindowId ), camera->GetGlobalTransform().GetInverted().GetMatrix() );
                    Matrix4x4 projection {};
                    projection.Perspective ( aScene.GetFieldOfView(), mAspectRatio, aScene.GetNear(), aScene.GetFar() );
                    mRenderer->SetProjectionMatrix ( reinterpret_cast<void*> ( mWindowId ), projection );
                }
                mRenderer->BeginRender ( reinterpret_cast<void*> ( mWindowId ) );
                aScene.LoopTraverseDFSPreOrder ( [this] ( const Node & aNode )
                {
                    AABB transformed_aabb = aNode.GetGlobalTransform() * aNode.GetAABB();
                    if ( mRenderer->GetFrustum ( reinterpret_cast<void * > ( mWindowId ) ).Intersects ( transformed_aabb ) )
                    {
                        // Call Node specific rendering function.
                        aNode.Render ( *mRenderer, reinterpret_cast<void*> ( mWindowId ) );
                    }
                } );
                mRenderer->EndRender ( reinterpret_cast<void*> ( mWindowId ) );
            }
        }
        XUnmapWindow ( mDisplay, mWindowId );
    }
}
#endif