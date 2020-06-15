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
#include <chrono>
#include "aeongames/X11Window.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Scene.h"

namespace AeonGames
{
    X11Window::X11Window ( void* aWindowId ) : mWindowId{ reinterpret_cast<::Window> ( aWindowId ) }
    {
        SetWindowForId ( aWindowId, this );
    }

    X11Window::X11Window ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        CommonWindow ( aX, aY, aWidth, aHeight, aFullScreen )
    {
#if 0
        mDisplay = XOpenDisplay ( nullptr );
        ::Window root = DefaultRootWindow ( mDisplay );
        XVisualInfo xvisualid{};
        if ( !XMatchVisualInfo ( mDisplay, XDefaultScreen ( mDisplay ), DefaultDepth ( mDisplay, DefaultScreen ( mDisplay ) ), TrueColor, &xvisualid ) )
        {
            throw std::runtime_error ( "No appropriate visual found" );
        }
        Colormap cmap = XCreateColormap ( mDisplay, root, xvisualid.visual, AllocNone );
        XSetWindowAttributes swa
        {
            .background_pixmap = None,
            .background_pixel  = 0,
            .border_pixel      = 0,
            .event_mask = StructureNotifyMask | KeyPressMask,
            .colormap = cmap,
        };
        mWindowId = XCreateWindow (
                        mDisplay,
                        root,
                        aX, aY,
                        aWidth, aHeight,
                        0,
                        DefaultDepth ( mDisplay, DefaultScreen ( mDisplay ) ), InputOutput, xvisualid.visual, CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask, &swa
                    );
        SetWindowForId ( reinterpret_cast<void*> ( mWindowId ), this );
        XMapWindow ( mDisplay, mWindowId );
        XStoreName ( mDisplay, mWindowId, "AeonGames" );
#endif
    }

    DLL X11Window::~X11Window() = default;
    void X11Window::Run ( Scene& aScene )
    {
        bool running{true};
        XEvent xevent;
        Atom wm_delete_window = XInternAtom ( mDisplay, "WM_DELETE_WINDOW", 0 );
        XSetWMProtocols ( mDisplay, mWindowId, &wm_delete_window, 1 );
        std::chrono::high_resolution_clock::time_point last_time{std::chrono::high_resolution_clock::now() };

        SetScene ( &aScene );
        Show ( true );
        while ( running )
        {
            while ( ( XPending ( mDisplay ) > 0 ) && running )
            {
                XNextEvent ( mDisplay, &xevent );
                switch ( xevent.type )
                {
                case Expose:
                {
                    // Here is where window resize is required.
                    XWindowAttributes xwa;
                    XGetWindowAttributes ( mDisplay, mWindowId, &xwa );
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
            XMapWindow ( mDisplay, mWindowId );
        }
        else
        {
            XUnmapWindow ( mDisplay, mWindowId );
        }
    }

    void X11Window::StartRenderTimer() const
    {
    }

    void X11Window::StopRenderTimer() const
    {
    }
}
#endif
