/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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

#include <iostream>
#include "GameWindow.h"

namespace AeonGames
{

#ifdef CLOCK_MONOTONIC_COARSE
    static const clockid_t kClockId = CLOCK_MONOTONIC_COARSE;
#else
    static const clockid_t kClockId = CLOCK_REALTIME;
#endif

GameWindow::GameWindow ( AeonEngine& aAeonEngine ) try :
        mAeonEngine ( aAeonEngine ),
                    mDisplay ( nullptr ),
                    mWindow ( 0 )
    {
        Initialize();
    }
    catch ( ... )
    {
        Finalize();
        throw;
    }

    void GameWindow::Initialize ()
    {
        mDisplay = XOpenDisplay ( 0 );
        if ( !mDisplay )
        {
            throw std::runtime_error ( "Could not open display" );
        }

        int number_of_visuals = 0;
        XVisualInfo visual_template{};
        visual_template.screen = DefaultScreen ( mDisplay );
        XVisualInfo* visual_list = XGetVisualInfo ( mDisplay, VisualScreenMask, &visual_template, &number_of_visuals );
        // Pick best Visual
        for ( int i = 0; i < number_of_visuals; ++i )
        {
            printf ( "  %3d: visual 0x%lx class %d (%s) depth %d\n",
                     i,
                     visual_list[i].visualid,
                     visual_list[i].c_class,
                     visual_list[i].c_class == TrueColor ? "TrueColor" : "unknown",
                     visual_list[i].depth );
        }
        XFree ( visual_list );

        XVisualInfo visual_info {};
        //if ( !XMatchVisualInfo ( mDisplay, XDefaultScreen ( mDisplay ), 32, TrueColor, &visual_info ) )
        if ( !XMatchVisualInfo ( mDisplay, XDefaultScreen ( mDisplay ), 24, TrueColor, &visual_info ) )
        {
            throw std::runtime_error ( "No such visual\n" );
        }


        XSetWindowAttributes set_window_attributes;

        set_window_attributes.colormap = XCreateColormap ( mDisplay,
                                         RootWindow ( mDisplay, visual_info.screen ),
                                         visual_info.visual, AllocNone );
        set_window_attributes.background_pixmap = None ;
        set_window_attributes.border_pixel      = 0;
        set_window_attributes.event_mask        = StructureNotifyMask;

        mWindow = XCreateWindow ( mDisplay, RootWindow ( mDisplay, visual_info.screen ),
                                  0, 0, 800, 600, 0, visual_info.depth, InputOutput,
                                  visual_info.visual,
                                  CWBorderPixel | CWColormap | CWEventMask, &set_window_attributes );
        if ( !mWindow )
        {
            throw std::runtime_error ( "Failed to create window." );
        }

        XStoreName ( mDisplay, mWindow, "AeonEngine" );

        XSelectInput ( mDisplay, mWindow,
                       ExposureMask |
                       KeyPressMask |
                       KeyReleaseMask |
                       ButtonPressMask |
                       ButtonReleaseMask |
                       PointerMotionMask |
                       StructureNotifyMask
#if 0
                       | ResizeRedirectMask
#endif
                     );

        mWMDeleteWindow = XInternAtom ( mDisplay, "WM_DELETE_WINDOW", 0 );
        XSetWMProtocols ( mDisplay, mWindow, &mWMDeleteWindow, 1 );
        mAeonEngine.InitializeRenderingWindow ( mDisplay, mWindow );
        XMapWindow ( mDisplay, mWindow );

        XSync ( mDisplay, False );
    }

    int GameWindow::Run()
    {
        bool running = true;
        XEvent xEvent;
        timespec current_time;
        clock_gettime ( kClockId, &current_time );
        timespec last_time = current_time;
        double delta;
        while ( running )
        {
            clock_gettime ( kClockId, &current_time );
            delta = static_cast<float> ( current_time.tv_sec - last_time.tv_sec )   +
                    static_cast<float> ( current_time.tv_nsec - last_time.tv_nsec ) * 1e-9;
            if ( delta > 1e-1 )
            {
                delta = 1.0f / 30.0f;
            }
            mAeonEngine.Step ( delta );
            last_time = current_time;
            if ( ( XPending ( mDisplay ) > 0 ) && running )
            {
                XNextEvent ( mDisplay, &xEvent );
                if ( xEvent.xany.window == mWindow )
                {
                    switch ( xEvent.type )
                    {
                    case Expose:
                        break;
                    case KeyPress:
                        break;
                    case KeyRelease:
                        break;
                    case ButtonPress:
                        break;
                    case ButtonRelease:
                        break;
                    case ConfigureNotify:
                        break;
                    case MotionNotify:
                        break;
                    case ResizeRequest:
                        break;
                    case ClientMessage:
                        if ( static_cast<Atom> ( xEvent.xclient.data.l[0] ) == mWMDeleteWindow )
                        {
                            running = false;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        return EXIT_SUCCESS;
    }

    void GameWindow::Finalize()
    {
        mAeonEngine.FinalizeRenderingWindow();
        if ( mWindow != 0 )
        {
            XWindowAttributes x_window_attributes {};
            XGetWindowAttributes ( mDisplay, mWindow, &x_window_attributes );
            XFreeColormap ( mDisplay, x_window_attributes.colormap );
            XDestroyWindow ( mDisplay, mWindow );
            mWindow = 0;
        }
        if ( mDisplay != nullptr )
        {
            XCloseDisplay ( mDisplay );
            mDisplay = nullptr;
        }
    }

    GameWindow::~GameWindow()
    {
        Finalize();
    }
}
