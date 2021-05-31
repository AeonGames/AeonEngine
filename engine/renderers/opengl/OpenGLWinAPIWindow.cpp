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

#ifdef _WIN32
#include "aeongames/MemoryPool.h"
#include "aeongames/Scene.h"
#include "aeongames/ProtobufClasses.h"
#include "OpenGLWinAPIWindow.h"
#include "OpenGLRenderer.h"
#include "OpenGLFunctions.h"

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
                Window* window = CommonWindow::GetWindowFromId ( hwnd );
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

    OpenGLWinAPIWindow::OpenGLWinAPIWindow ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        OpenGLWindow { aOpenGLRenderer, aX, aY, aWidth, aHeight, aFullScreen }
    {
        DWORD dwExStyle{WS_EX_APPWINDOW | WS_EX_WINDOWEDGE};
        DWORD dwStyle{WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN};
        if ( mFullScreen )
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
                mAspectRatio = ( static_cast<float> ( aWidth ) / static_cast<float> ( aHeight ) );
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
        SetWindowForId ( mWindowId, this );

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

    OpenGLWinAPIWindow::OpenGLWinAPIWindow ( const OpenGLRenderer& aOpenGLRenderer, void* aWindowId ) :
        OpenGLWindow{aOpenGLRenderer}, mWindowId{reinterpret_cast<HWND> ( aWindowId ) }
    {
        SetWindowForId ( aWindowId, this );
        try
        {
            Initialize();
            OpenGLWindow::Initialize();
            MakeCurrent();
        }
        catch ( ... )
        {
            OpenGLWindow::Finalize();
            Finalize();
            throw;
        }
    }

    OpenGLWinAPIWindow::~OpenGLWinAPIWindow()
    {
        OpenGLWindow::Finalize();
        Finalize();
        RemoveWindowForId ( mWindowId );
        ATOM atom = GetClassWord ( static_cast<HWND> ( mWindowId ), GCW_ATOM );
        DestroyWindow ( static_cast<HWND> ( mWindowId ) );
        UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                              0x0ULL +
#endif
                              MAKELONG ( atom, 0 ) ), nullptr );
    }

    bool OpenGLWinAPIWindow::MakeCurrent()
    {
        return wglMakeCurrent ( mDeviceContext, reinterpret_cast<HGLRC> ( mOpenGLRenderer.GetContext() ) );
    }

    void OpenGLWinAPIWindow::SwapBuffers()
    {
        ::SwapBuffers ( mDeviceContext );
    }

    void OpenGLWinAPIWindow::Initialize()
    {
        RECT rect{};
        GetWindowRect ( reinterpret_cast<HWND> ( mWindowId ), &rect );
        mDeviceContext = GetDC ( static_cast<HWND> ( mWindowId ) );
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int pf = ChoosePixelFormat ( mDeviceContext, &pfd );
        SetPixelFormat ( mDeviceContext, pf, &pfd );
        MakeCurrent();
        glViewport ( rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLWinAPIWindow::Finalize()
    {
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( !mOpenGLRenderer.MakeCurrent() )
        {
            LPSTR pBuffer = NULL;
            FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                            nullptr, GetLastError(), MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPSTR ) &pBuffer, 0, nullptr );
            if ( pBuffer != nullptr )
            {
                std::cout << pBuffer << std::endl;
                LocalFree ( pBuffer );
            }
        }
        OPENGL_CHECK_ERROR_NO_THROW;
        ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), mDeviceContext );
        mDeviceContext = nullptr;
    }
    void OpenGLWinAPIWindow::Run ( Scene& aScene )
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
        ShowWindow ( mWindowId, SW_HIDE );
        SetScene ( nullptr );
    }
    void OpenGLWinAPIWindow::Show ( bool aShow ) const
    {
        ShowWindow ( static_cast<HWND> ( mWindowId ), aShow ? SW_SHOW : SW_HIDE );
    }
    void OpenGLWinAPIWindow::StartRenderTimer() const
    {
        SetTimer ( reinterpret_cast<HWND> ( mWindowId ), RENDER_LOOP, USER_TIMER_MINIMUM, static_cast<TIMERPROC> ( AeonEngineTimerProc ) );
    }
    void OpenGLWinAPIWindow::StopRenderTimer() const
    {
        KillTimer ( reinterpret_cast<HWND> ( mWindowId ), RENDER_LOOP );
    }
}
#endif
