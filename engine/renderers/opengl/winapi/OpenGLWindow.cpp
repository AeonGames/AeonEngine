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

#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <cassert>
#include "glcorearb.h"
#include <GL/gl.h>
#include "wglext.h"
#include "OpenGLWindow.h"
#include "aeongames/AeonEngine.h"
#include <exception>
#include <stdexcept>

namespace AeonGames
{
    ATOM OpenGLWindow::mWindowClassType = 0;

    OpenGLWindow::OpenGLWindow ( AeonEngine& aAeonEngine )
try:
        mAeonEngine ( aAeonEngine )
    {
        Initialize();
    }
    catch ( ... )
    {
        Finalize();
        throw;
    }

    OpenGLWindow::~OpenGLWindow()
    {
        Finalize();
    }

    int OpenGLWindow::Run()
    {
//      auto suzanne_mesh = mAeonEngine.GetMesh("game/meshes/Suzanne.msh");
        ShowWindow ( mHwnd, SW_SHOW );
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

    HWND OpenGLWindow::GetWindowHandle() const
    {
        return mHwnd;
    }

    HDC OpenGLWindow::GetDeviceContext() const
    {
        return mDeviceContext;
    }

    HGLRC OpenGLWindow::GetOpenGLContext() const
    {
        return mOpenGLContext;
    }

    void OpenGLWindow::Initialize()
    {
        if ( !mRefCount )
        {
            WNDCLASSEX wcex;
            wcex.cbSize = sizeof ( WNDCLASSEX );
            wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            wcex.lpfnWndProc = ( WNDPROC ) DefWindowProc;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = 0;
            wcex.hInstance = GetModuleHandle ( nullptr );
            wcex.hIcon = LoadIcon ( nullptr, IDI_WINLOGO );
            wcex.hCursor = LoadCursor ( nullptr, IDC_ARROW );
            wcex.hbrBackground = nullptr;
            wcex.lpszMenuName = nullptr;
            wcex.lpszClassName = "AeonEngine OpenGL Window";
            wcex.hIconSm = nullptr;
            mWindowClassType = RegisterClassEx ( &wcex );
        }

        RECT rect = { 0, 0, 800, 600 };
        if ( !AdjustWindowRectEx ( &rect, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE ) )
        {
            char message[1024] = { 0 };
            if ( ( FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, message, sizeof ( message ) - 1, nullptr ) ) == 0 )
            {
                throw std::runtime_error ( "FormatMessage Failed." );
            }
            throw std::runtime_error ( message );
        }

        mHwnd = CreateWindowEx ( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
                                 MAKEINTATOM ( mWindowClassType ), "AeonEngine OpenGL Window",
                                 WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                 0, 0, // Location
                                 rect.right - rect.left, rect.bottom - rect.top, // dimensions
                                 nullptr,
                                 nullptr,
                                 GetModuleHandle ( nullptr ),
                                 nullptr );

        PIXELFORMATDESCRIPTOR pfd {};
        PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = nullptr;
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
        mDeviceContext = GetDC ( mHwnd );
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int pf = ChoosePixelFormat ( mDeviceContext, &pfd );
        SetPixelFormat ( mDeviceContext, pf, &pfd );
        mOpenGLContext = wglCreateContext ( mDeviceContext );
        wglMakeCurrent ( mDeviceContext, mOpenGLContext );

        //---OpenGL 3.2 Context---//
        wglGetExtensionsStringARB = ( PFNWGLGETEXTENSIONSSTRINGARBPROC ) wglGetProcAddress ( "wglGetExtensionsStringARB" );
        if ( wglGetExtensionsStringARB != nullptr )
        {
            if ( strstr ( wglGetExtensionsStringARB ( mDeviceContext ), "WGL_ARB_create_context" ) != nullptr )
            {
                const int ctxAttribs[] =
                {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                    WGL_CONTEXT_PROFILE_MASK_ARB,
                    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    0
                };

                wglCreateContextAttribsARB = ( PFNWGLCREATECONTEXTATTRIBSARBPROC ) wglGetProcAddress ( "wglCreateContextAttribsARB" );
                wglMakeCurrent ( mDeviceContext, nullptr );
                wglDeleteContext ( mOpenGLContext );
                mOpenGLContext = wglCreateContextAttribsARB ( mDeviceContext, nullptr, ctxAttribs );
                wglMakeCurrent ( mDeviceContext, mOpenGLContext );
                RECT rect;
                GetClientRect ( mHwnd, &rect );
                glViewport ( 0, 0, rect.right, rect.bottom );
                glClearColor ( 0.5f, 0.5f, 0.5f, 0.0f );
            }
        }
        SetLastError ( 0 );
        if ( ( ( SetWindowLongPtr ( mHwnd, GWLP_USERDATA, ( LONG_PTR ) this ) ) == 0 ) && ( GetLastError() != 0 ) )
        {
            char message[1024] = { 0 };
            if ( ( FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, message, sizeof ( message ) - 1, nullptr ) ) == 0 )
            {
                throw std::runtime_error ( "FormatMessage Failed." );
            }
            throw std::runtime_error ( message );
        }
        mRefCount++;
    }

    void OpenGLWindow::Finalize()
    {
        if ( mHwnd )
        {
            DestroyWindow ( mHwnd );
            mHwnd = nullptr;
        }
        if ( mDeviceContext != nullptr )
        {
            wglMakeCurrent ( mDeviceContext, nullptr );
            ReleaseDC ( mHwnd, mDeviceContext );
            mDeviceContext = nullptr;
        }
        if ( mOpenGLContext )
        {
            wglDeleteContext ( mOpenGLContext );
            mOpenGLContext = nullptr;
        }
        if ( mRefCount == 1 )
        {
            UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                                  0x0ULL +
#endif
                                  MAKELONG ( mWindowClassType, 0 ) ), nullptr );
        }
        mRefCount = ( mRefCount > 0 ) ? mRefCount - 1 : mRefCount;
    }

    LRESULT OpenGLWindow::WindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        LRESULT lresult = 0;
        OpenGLWindow* window_ptr = ( ( OpenGLWindow* ) GetWindowLongPtr ( hwnd, GWLP_USERDATA ) );
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

    LRESULT OpenGLWindow::OnSize ( WPARAM type, WORD aWidth, WORD aHeight )
    {
        if ( aWidth && aHeight )
        {
            glViewport ( 0, 0, aWidth, aHeight );
        }
        return 0;
    }

    LRESULT OpenGLWindow::OnPaint()
    {
        RECT rect;
        PAINTSTRUCT paint;
        if ( GetUpdateRect ( mHwnd, &rect, FALSE ) )
        {
            BeginPaint ( mHwnd, &paint );
            EndPaint ( mHwnd, &paint );
        }
        return 0;
    }

    void OpenGLWindow::RenderLoop()
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

        if ( mDeviceContext != nullptr )
        {
            wglMakeCurrent ( mDeviceContext, mOpenGLContext );
            glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            mAeonEngine.Step ( delta );
            SwapBuffers ( mDeviceContext );
        }

        last_time = this_time;
    }
}