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
#ifdef __unix__
#include <ctime>
#include <ratio>
#include <chrono>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include<GL/glx.h>
#include "aeongames/Platform.h"
#include "aeongames/Window.h"
namespace AeonGames
{
    Window::Window ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        mAspectRatio { static_cast<float> ( aWidth ) / static_cast<float> ( aHeight ) }
    {
        static int att[] =
        {
            GLX_RGBA,
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            GLX_DOUBLEBUFFER,
            None
        };
        Display* display = XOpenDisplay ( std::getenv ( "DISPLAY" ) );
        ::Window root = DefaultRootWindow ( display );
        XVisualInfo* vi = glXChooseVisual ( display, 0, att );
        if ( vi == nullptr )
        {
            throw std::runtime_error ( "No appropriate visual found" );
        }
        Colormap cmap = XCreateColormap ( display, root, vi->visual, AllocNone );
        XSetWindowAttributes swa{.event_mask = ExposureMask | KeyPressMask, .colormap = cmap};
        mWindowId = reinterpret_cast<void*> (
                        XCreateWindow (
                            display,
                            root,
                            aX, aY,
                            aWidth, aHeight,
                            0,
                            vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa
                        ) );
        XMapWindow ( display, reinterpret_cast<::Window> ( mWindowId ) );
        XCloseDisplay ( display );
    }

    DLL Window::~Window() = default;

    void Window::Run ( Scene& aScene )
    {
    }
    void Window::Show ( bool aShow ) const
    {
    }
    uint32_t Window::GetWidth() const
    {
        int x_return, y_return;
        ::Window root_return;
        unsigned int width_return, height_return;
        unsigned int border_width_return;
        unsigned int depth_return;
        Display* display = XOpenDisplay ( nullptr );
        XGetGeometry ( display,
                       reinterpret_cast<::Window> ( mWindowId ),
                       &root_return, &x_return, &y_return, &width_return,
                       &height_return, &border_width_return, &depth_return );
        XCloseDisplay ( display );
        return width_return;
    }
    uint32_t Window::GetHeight() const
    {
        int x_return, y_return;
        ::Window root_return;
        unsigned int width_return, height_return;
        unsigned int border_width_return;
        unsigned int depth_return;
        Display* display = XOpenDisplay ( nullptr );
        XGetGeometry ( display,
                       reinterpret_cast<::Window> ( mWindowId ),
                       &root_return, &x_return, &y_return, &width_return,
                       &height_return, &border_width_return, &depth_return );
        XCloseDisplay ( display );
        return height_return;
    }
}
#endif
