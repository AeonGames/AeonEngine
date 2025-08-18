/*
Copyright (C) 2016,2018,2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Platform.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Frustum.hpp"
#include <cassert>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <regex>
#include <tuple>
#include <shellapi.h>
#include <chrono>
#include "Window.h"

/** Convert a WinMain command line (lpCmdLine) into a regular argc,argv pair.
 * @param aCmdLine Windows API WinMain format command line.
 * @return tuple containing a vector of char* (std::get<0>) and a string
 * containing the argument strings separated each by a null character(std::get<1>).
 * @note The string part is meant only as managed storage for the char* vector,
 * as those point into the string memory, so there should not be a real reason
 * to read it directly.
*/
static std::tuple<std::vector<char*>, const std::string> GetArgs ( char* aCmdLine )
{
    /*Match any whitespace OR any whitespace followed by eol OR eol.*/
    std::regex whitespace{"\\s+|\\s+$|$"};
    std::tuple<std::vector<char*>, std::string>
    /*Replace all matches with a single plain old space character.
        Note: using "\0" instead of " " would be great, but "\0"
        gets converted as per the spec into the empty string rather
        than a size 1 string containing the null character.
        And no, "\0\0" does not work, std::string("\0\0",2) does not either.*/
    result{std::vector<char*>{}, std::regex_replace ( aCmdLine, whitespace, " " ) };

    size_t count{0};
    for ( size_t i = 0; i < std::get<1> ( result ).size(); ++i )
    {
        if ( std::get<1> ( result ) [i] == ' ' )
        {
            std::get<1> ( result ) [i] = '\0';
            ++count;
        }
    }

    if ( count )
    {
        std::get<0> ( result ).reserve ( count );
        std::get<0> ( result ).emplace_back ( std::get<1> ( result ).data() );
        for ( uint32_t i = 0; i < std::get<1> ( result ).size() - 1; ++i )
        {
            if ( std::get<1> ( result ) [i] == '\0' )
            {
                std::get<0> ( result ).emplace_back ( std::get<1> ( result ).data() + ( i + 1 ) );
            }
        }
    }
    return result;
}

extern int Main ( int argc, char *argv[] );

int WINAPI ENTRYPOINT WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    auto args = GetArgs ( lpCmdLine );
    return Main ( static_cast<int> ( std::get<0> ( args ).size() ), std::get<0> ( args ).data() );
}

int ENTRYPOINT main ( int argc, char *argv[] )
{
#ifdef _MSC_VER
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#if 0
    // Send all reports to STDOUT
    _CrtSetReportMode ( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile ( _CRT_WARN, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode ( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile ( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode ( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile ( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
#endif
    // Use _CrtSetBreakAlloc( ) to set breakpoints on allocations.
    //_CrtSetBreakAlloc (1159202);
#endif
    std::ostringstream stream;
    for ( int i = 0; i < argc; ++i )
    {
        stream << argv[i] << ( ( i != argc - 1 ) ? " " : "" );
    }
    std::string command_line = stream.str();
    return WinMain ( GetModuleHandle ( NULL ), NULL, stream.str().data(), 0 );
}

namespace AeonGames
{
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
            Window* window = reinterpret_cast<Window*> ( GetWindowLongPtr ( hwnd, GWLP_USERDATA ) );
            if ( window )
            {
                return window->Resize ( LOWORD ( lParam ), HIWORD ( lParam ) );
            }
            return 0;
        }
        default:
            return DefWindowProc ( hwnd, uMsg, wParam, lParam );
        }
        return 0;
    }

    Window::Window ( const std::string& aRendererName, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen )
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
        SetWindowLongPtr ( static_cast<HWND> ( mWindowId ), GWLP_USERDATA, ( LONG_PTR ) this );
        mRenderer = ConstructRenderer ( aRendererName, mWindowId );
    }

    Window::~Window()
    {
        if ( mRenderer )
        {
            mRenderer->DetachWindow ( this );
        }
        DestroyWindow ( static_cast<HWND> ( mWindowId ) );
    }

    uint32_t Window::Resize ( uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight && mRenderer )
        {
            mRenderer->ResizeViewport ( mWindowId, 0, 0, aWidth, aHeight );
            mAspectRatio = static_cast<float> ( aWidth ) / static_cast<float> ( aHeight );
        }
        return 0;
    }

    void Window::Run ( Scene& aScene )
    {
        MSG msg;
        bool done = false;
        ShowWindow ( static_cast<HWND> ( mWindowId ), SW_SHOW );
        std::chrono::high_resolution_clock::time_point last_time{std::chrono::high_resolution_clock::now() };
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
            else if ( mRenderer )
            {
                std::chrono::high_resolution_clock::time_point current_time {std::chrono::high_resolution_clock::now() };
                std::chrono::duration<double> delta{std::chrono::duration_cast<std::chrono::duration<double >> ( current_time - last_time ) };
                aScene.Update ( delta.count() );
                last_time = current_time;
                if ( const Node* camera = aScene.GetCamera() )
                {
                    mRenderer->SetViewMatrix ( mWindowId, camera->GetGlobalTransform().GetInverted().GetMatrix() );
                    Matrix4x4 projection {};
                    projection.Perspective ( aScene.GetFieldOfView(), mAspectRatio, aScene.GetNear(), aScene.GetFar() );
                    mRenderer->SetProjectionMatrix ( mWindowId, projection );
                }
                mRenderer->BeginRender ( mWindowId );
                aScene.LoopTraverseDFSPreOrder ( [this] ( const Node & aNode )
                {
                    AABB transformed_aabb = aNode.GetGlobalTransform() * aNode.GetAABB();
                    if ( mRenderer->GetFrustum ( mWindowId ).Intersects ( transformed_aabb ) )
                    {
                        // Call Node specific rendering function.
                        aNode.Render ( *mRenderer, mWindowId );
                    }
                } );
                mRenderer->EndRender ( mWindowId );
            }
        }
        ShowWindow ( static_cast<HWND> ( mWindowId ), SW_HIDE );
    }
}
#endif