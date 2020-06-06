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
#ifdef _WIN32
#include <ctime>
#include <ratio>
#include <chrono>
#include <iostream>
#include "aeongames/Platform.h"
#include "aeongames/Window.h"
#include "aeongames/Node.h"
#include "aeongames/Scene.h"

namespace AeonGames
{
    enum
    {
        RENDER_LOOP = 1
    };

    void AeonEngineTimerProc (
        HWND hwnd,
        UINT uMsg,
        UINT_PTR wParam,
        DWORD ms )
    {
        switch ( uMsg )
        {
        case WM_TIMER:
            if ( wParam == RENDER_LOOP )
            {
                Window* window = Window::GetWindowFromId ( hwnd );
                window->RenderLoop();
            }
            break;
        }
    }

    LRESULT CALLBACK AeonEngineWindowProc (
        _In_ HWND   hwnd,
        _In_ UINT   uMsg,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam
    )
    {
        switch ( uMsg )
        {
        case WM_CLOSE:
            ShowWindow ( hwnd, SW_HIDE );
            PostQuitMessage ( 0 );
            break;
        case WM_DESTROY:
            PostQuitMessage ( 0 );
            break;
        case WM_SIZE:
        {
            Window* window = Window::GetWindowFromId ( hwnd );
            if ( window )
            {
                window->ResizeViewport ( 0, 0, LOWORD ( lParam ), HIWORD ( lParam ) );
            }
            return 0;
        }
        default:
            return DefWindowProc ( hwnd, uMsg, wParam, lParam );
        }
        return 0;
    }

    Window::Window ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        mAspectRatio { static_cast<float> ( aWidth ) / static_cast<float> ( aHeight ) }
    {
        DWORD dwExStyle{WS_EX_APPWINDOW | WS_EX_WINDOWEDGE};
        DWORD dwStyle{WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN};
        if ( aFullScreen )
        {
            dwExStyle = WS_EX_APPWINDOW;
            dwStyle = {WS_POPUP};
            DEVMODE device_mode{};
            device_mode.dmSize = sizeof ( DEVMODE );
            if ( EnumDisplaySettingsEx ( nullptr, ENUM_CURRENT_SETTINGS, &device_mode, 0 ) )
            {
                std::cout <<
                          "Position: " << device_mode.dmPosition.x << " " << device_mode.dmPosition.y << std::endl <<
                          "Display Orientation: " << device_mode.dmDisplayOrientation << std::endl <<
                          "Display Flags: " << device_mode.dmDisplayFlags << std::endl <<
                          "Display Frecuency: " << device_mode.dmDisplayFrequency << std::endl <<
                          "Bits Per Pixel: " << device_mode.dmBitsPerPel << std::endl <<
                          "Width: " << device_mode.dmPelsWidth << std::endl <<
                          "Height: " << device_mode.dmPelsHeight << std::endl;
                aX = device_mode.dmPosition.x;
                aY = device_mode.dmPosition.y;
                aWidth = device_mode.dmPelsWidth;
                aHeight = device_mode.dmPelsHeight;
                mAspectRatio = static_cast<float> ( aWidth ) / static_cast<float> ( aHeight );
                ChangeDisplaySettings ( &device_mode, CDS_FULLSCREEN );
            }
        }
        RECT rect = { aX, aY, static_cast<int32_t> ( aWidth ), static_cast<int32_t> ( aHeight ) };
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof ( WNDCLASSEX );
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = ( WNDPROC ) AeonEngineWindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = GetModuleHandle ( nullptr );
        wcex.hIcon = LoadIcon ( nullptr, IDI_WINLOGO );
        wcex.hCursor = LoadCursor ( nullptr, IDC_ARROW );
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = "AeonEngine";
        wcex.hIconSm = nullptr;
        ATOM atom = RegisterClassEx ( &wcex );
        mWindowId = CreateWindowEx ( dwExStyle,
                                     MAKEINTATOM ( atom ), "AeonEngine",
                                     dwStyle,
                                     rect.left, rect.top, // Location
                                     rect.right - rect.left, rect.bottom - rect.top, // Dimensions
                                     nullptr,
                                     nullptr,
                                     GetModuleHandle ( nullptr ),
                                     nullptr );
        WindowMap.emplace ( std::pair<void*, Window*> {mWindowId, this} );
    }
    DLL Window::~Window()
    {
        WindowMap.erase ( static_cast<HWND> ( mWindowId ) );
        ATOM atom = GetClassWord ( static_cast<HWND> ( mWindowId ), GCW_ATOM );
        DestroyWindow ( static_cast<HWND> ( mWindowId ) );
        UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                              0x0ULL +
#endif
                              MAKELONG ( atom, 0 ) ), nullptr );
    }
    void Window::Run ( Scene& aScene )
    {
        MSG msg;
        bool done = false;
        ShowWindow ( static_cast<HWND> ( mWindowId ), SW_SHOW );
        std::chrono::high_resolution_clock::time_point last_time{std::chrono::high_resolution_clock::now() };
        SetScene ( &aScene );
        while ( !done )
        {
            if ( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE ) )
            {
                if ( msg.message == WM_QUIT )
                {
                    done = true;
                }
                else
                {
                    TranslateMessage ( &msg );
                    DispatchMessage ( &msg );
                }
            }
            else
            {
                std::chrono::high_resolution_clock::time_point current_time {std::chrono::high_resolution_clock::now() };
                std::chrono::duration<double> delta{std::chrono::duration_cast<std::chrono::duration<double>> ( current_time - last_time ) };
                aScene.Update ( delta.count() );
                last_time = current_time;
                RenderLoop();
            }
        }
        ShowWindow ( static_cast<HWND> ( mWindowId ), SW_HIDE );
        SetScene ( nullptr );
    }
    void Window::Show ( bool aShow ) const
    {
        ShowWindow ( static_cast<HWND> ( mWindowId ), aShow ? SW_SHOW : SW_HIDE );
    }
    uint32_t Window::GetWidth() const
    {
        RECT rect;
        if ( GetWindowRect ( reinterpret_cast<HWND> ( mWindowId ), &rect ) )
        {
            return rect.right - rect.left;
        }
        return 0;
    }
    uint32_t Window::GetHeight() const
    {
        RECT rect;
        if ( GetWindowRect ( reinterpret_cast<HWND> ( mWindowId ), &rect ) )
        {
            return rect.bottom - rect.top;
        }
        return 0;
    }

    void Window::StartRenderTimer() const
    {
        SetTimer ( reinterpret_cast<HWND> ( mWindowId ), RENDER_LOOP, USER_TIMER_MINIMUM, static_cast<TIMERPROC> ( AeonEngineTimerProc ) );
    }
    void Window::StopRenderTimer() const
    {
        KillTimer ( reinterpret_cast<HWND> ( mWindowId ), RENDER_LOOP );
    }
}
#endif