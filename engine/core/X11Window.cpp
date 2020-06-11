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
#include "aeongames/X11Window.h"

namespace AeonGames
{
    X11Window::X11Window ( void* aWindowId ) : mWindowId{ reinterpret_cast<::Window> ( aWindowId ) }
    {
        SetWindowForId ( aWindowId, this );
    }

    X11Window::X11Window ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen )
    {
        mDisplay = XOpenDisplay ( nullptr );
        ::Window root = DefaultRootWindow ( mDisplay );
        XVisualInfo vi{};
        if ( !XMatchVisualInfo ( mDisplay, XDefaultScreen ( mDisplay ), 32, TrueColor, &vi ) )
        {
            throw std::runtime_error ( "No appropriate visual found" );
        }
        Colormap cmap = XCreateColormap ( mDisplay, root, vi.visual, AllocNone );
        XSetWindowAttributes swa{.event_mask = ExposureMask | KeyPressMask, .colormap = cmap};
        mWindowId = XCreateWindow (
                        mDisplay,
                        root,
                        aX, aY,
                        aWidth, aHeight,
                        0,
                        vi.depth, InputOutput, vi.visual, CWColormap | CWEventMask, &swa
                    );
        XMapWindow ( mDisplay, mWindowId );
    }

    DLL X11Window::~X11Window()
    {
        RemoveWindowForId ( reinterpret_cast<void*> ( mWindowId ) );
        if ( mDisplay != nullptr )
        {
            XDestroyWindow ( mDisplay, mWindowId );
            XCloseDisplay ( mDisplay );
        }
    }

    void X11Window::Run ( Scene& aScene )
    {
    }

    void X11Window::Show ( bool aShow ) const
    {
    }

    void X11Window::StartRenderTimer() const
    {
    }

    void X11Window::StopRenderTimer() const
    {
    }
}
#endif
