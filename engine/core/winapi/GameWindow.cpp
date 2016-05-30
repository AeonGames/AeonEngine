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

#include "aeongames/AeonEngine.h"
#include "GameWindow.h"
#include <exception>
#include <stdexcept>
#include <cassert>
#include <cstdint>

namespace AeonGames
{
    ATOM GameWindow::mClassAtom = 0;

    static HINSTANCE gInstance = nullptr;

GameWindow::GameWindow ( AeonEngine& aAeonEngine, LONG aWidth, LONG aHeight ) try :
        mWindowHandle ( nullptr ),
                      mAeonEngine ( aAeonEngine )
    {
        Initialize ( gInstance, aWidth, aHeight );
    }
    catch ( ... )
    {
        Finalize();
        throw;
    }

    GameWindow::~GameWindow()
    {
        Finalize();
    }

    int GameWindow::Run()
    {
        auto suzanne_mesh = mAeonEngine.GetMesh ( "game/meshes/Suzanne.msh" );
        ShowWindow ( mWindowHandle, SW_SHOW );
        MSG msg{};
        while ( msg.message != WM_QUIT )
        {
            if ( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE ) )
            {
                if ( msg.message != WM_QUIT )
                {
                    TranslateMessage ( &msg );
                    DispatchMessage ( &msg );
                }
            }
            else
            {
                RenderLoop();
            }
        }
        assert ( msg.message == WM_QUIT );
        return static_cast<int> ( msg.wParam );
    }

    void GameWindow::Initialize ( HINSTANCE aInstance, LONG aWidth, LONG aHeight )
    {
        if ( mClassAtom == 0 )
        {
            Register ( aInstance );
        }
        RECT rect = { 0, 0, aWidth, aHeight };
        if ( !AdjustWindowRectEx ( &rect, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE ) )
        {
            char message[1024] = { 0 };
            if ( ( FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, message, sizeof ( message ) - 1, nullptr ) ) == 0 )
            {
                throw std::runtime_error ( "FormatMessage Failed." );
            }
            throw std::runtime_error ( message );
        }
        if ( ( mWindowHandle = CreateWindowEx ( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
                                                "AeonEngine", "Aeon Engine",
                                                WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                                0, 0, // Location
                                                rect.right - rect.left, rect.bottom - rect.top, // dimensions
                                                NULL,
                                                NULL,
                                                aInstance,
                                                this ) ) == nullptr )
        {
            char message[1024] = { 0 };
            if ( ( FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, message, sizeof ( message ) - 1, nullptr ) ) == 0 )
            {
                throw std::runtime_error ( "FormatMessage Failed." );
            }
            throw std::runtime_error ( message );
        }

        SetLastError ( 0 );
        if ( ( ( SetWindowLongPtr ( mWindowHandle, GWLP_USERDATA, ( LONG_PTR ) this ) ) == 0 ) && ( GetLastError() != 0 ) )
        {
            char message[1024] = { 0 };
            if ( ( FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, message, sizeof ( message ) - 1, nullptr ) ) == 0 )
            {
                throw std::runtime_error ( "FormatMessage Failed." );
            }
            throw std::runtime_error ( message );
        }
//        mAeonEngine.InitializeRenderingWindow ( aInstance, mWindowHandle );
    }

    void GameWindow::Finalize()
    {
//        mAeonEngine.FinalizeRenderingWindow();
        if ( mWindowHandle != nullptr )
        {
            DestroyWindow ( mWindowHandle );
            mWindowHandle = nullptr;
        }
    }

    void GameWindow::RenderLoop()
    {
        LARGE_INTEGER frequency;
        LARGE_INTEGER this_time;
        QueryPerformanceCounter ( &this_time );
        QueryPerformanceFrequency ( &frequency );
        static LARGE_INTEGER last_time = this_time;
        float delta = static_cast<float> ( this_time.QuadPart - last_time.QuadPart ) / static_cast<float> ( frequency.QuadPart );
        if ( delta > 1e-1f )
        {
            delta = 1.0f / 30.0f;
        }
        mAeonEngine.Step ( delta );
        last_time = this_time;
    }

    void GameWindow::Register ( HINSTANCE hInstance )
    {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof ( WNDCLASSEX );
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = ( WNDPROC ) GameWindow::WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof ( GameWindow* );
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon ( NULL, IDI_WINLOGO );
        wcex.hCursor = LoadCursor ( NULL, IDC_ARROW );
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = "AeonEngine";
        wcex.hIconSm = NULL;
        if ( ( GameWindow::mClassAtom = RegisterClassEx ( &wcex ) ) == 0 )
        {
            char message[1024] = { 0 };
            if ( ( FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, message, sizeof ( message ) - 1, nullptr ) ) == 0 )
            {
                throw std::runtime_error ( "FormatMessage Failed." );
            }
            throw std::runtime_error ( message );
        }
    }

    LRESULT CALLBACK GameWindow::WindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        LRESULT lresult = 0;
        GameWindow* window_ptr = ( ( GameWindow* ) GetWindowLongPtr ( hwnd, GWLP_USERDATA ) );
        uint32_t scancode;
        switch ( uMsg )
        {
        case WM_PAINT:
            lresult = window_ptr->OnPaint();
            break;
        case WM_CLOSE:
            PostQuitMessage ( 0 );
        case WM_SIZE:
            lresult = window_ptr->OnSize ( wParam, LOWORD ( lParam ), HIWORD ( lParam ) );
            break;
        case WM_KEYDOWN:
            if ( ! ( lParam & 0x40000000 ) ) // only if this is not auto repeat
            {
                scancode = ( lParam & 0xff0000 ) >> 16;
                // scan code is at bites 16-23
                //window_ptr->engine->KeyDown ( ( lParam & 0xff0000 ) >> 16 );
                switch ( scancode )
                {
                case 0x11:
                    break;
                }
            }
            lresult = DefWindowProc ( hwnd, uMsg, wParam, lParam );
            break;
        case WM_KEYUP:
            // scan code is at bites 16-23
            //window_ptr->engine->KeyUp ( ( lParam & 0xff0000 ) >> 16 );
            scancode = ( lParam & 0xff0000 ) >> 16;
            switch ( scancode )
            {
            case 0x11:
                break;
            }
            lresult = DefWindowProc ( hwnd, uMsg, wParam, lParam );
            break;
        case WM_MOUSEMOVE:
            break;
        case WM_LBUTTONDOWN:
            break;
        case WM_LBUTTONUP:
            break;
        default:
            lresult = DefWindowProc ( hwnd, uMsg, wParam, lParam );
        }
        return lresult;
    }

    LRESULT GameWindow::OnSize ( WPARAM type, WORD newwidth, WORD newheight )
    {
        return 0;
    }

    LRESULT GameWindow::OnPaint()
    {
        RECT rect;
        PAINTSTRUCT paint;
        if ( GetUpdateRect ( mWindowHandle, &rect, FALSE ) )
        {
            BeginPaint ( mWindowHandle, &paint );
            EndPaint ( mWindowHandle, &paint );
        }
        return 0;
    }
}

BOOL APIENTRY ENTRYPOINT DllMain ( HANDLE hModule, DWORD ul_reason_for_call, LPVOID )
{
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        AeonGames::gInstance = ( HINSTANCE ) hModule;
        break;
    }
    return TRUE;
}
