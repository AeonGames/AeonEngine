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
#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include "OpenGLFunctions.h"
#include "OpenGLRenderer.h"
#include "OpenGLMesh.h"
#include "aeongames/LogLevel.h"

#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include "glcorearb.h"
#include <GL/gl.h>
#include "wglext.h"

namespace AeonGames
{

    WNDPROC OpenGLRenderer::mWindowProc = nullptr;
OpenGLRenderer::OpenGLRenderer() try :
        mInstance ( nullptr ),
                  mHwnd ( nullptr ),
                  mDeviceContext ( nullptr ),
                  mOpenGLContext ( nullptr )
    {
        Initialize();
    }
    catch ( ... )
    {
        FinalizeRenderingWindow();
        Finalize();
        throw;
    }

    OpenGLRenderer::~OpenGLRenderer()
    {
        Finalize();
    }

    void OpenGLRenderer::BeginRender() const
    {
        if ( mDeviceContext != nullptr )
        {
            wglMakeCurrent ( mDeviceContext, mOpenGLContext );
            glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            SwapBuffers ( mDeviceContext );
        }
    }

    void OpenGLRenderer::EndRender() const
    {
        if ( mDeviceContext != nullptr )
        {
            SwapBuffers ( mDeviceContext );
        }
    }

    void OpenGLRenderer::Render ( const std::shared_ptr<Mesh> aMesh ) const
    {
    }

    bool OpenGLRenderer::InitializeRenderingWindow ( HINSTANCE aInstance, HWND aHwnd )
    {
        mInstance = aInstance;
        mHwnd = aHwnd;
        PIXELFORMATDESCRIPTOR pfd;
        PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = nullptr;
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
        mDeviceContext = GetDC ( mHwnd );
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cRedBits = 0;
        pfd.cRedShift = 0;
        pfd.cGreenBits = 0;
        pfd.cGreenShift = 0;
        pfd.cBlueBits = 0;
        pfd.cBlueShift = 0;
        pfd.cAlphaBits = 0;
        pfd.cAlphaShift = 0;
        pfd.cAccumBits = 0;
        pfd.cAccumRedBits = 0;
        pfd.cAccumGreenBits = 0;
        pfd.cAccumBlueBits = 0;
        pfd.cAccumAlphaBits = 0;
        pfd.cDepthBits = 32;
        pfd.cStencilBits = 0;
        pfd.cAuxBuffers = 0;
        pfd.iLayerType = PFD_MAIN_PLANE;
        pfd.bReserved = 0;
        pfd.dwLayerMask = 0;
        pfd.dwVisibleMask = 0;
        pfd.dwDamageMask = 0;
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
                mWindowProc = reinterpret_cast<WNDPROC> ( GetWindowLongPtr ( mHwnd, GWLP_WNDPROC ) );
                SetWindowLongPtr ( mHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR> ( WindowProc ) );
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    void OpenGLRenderer::FinalizeRenderingWindow ()
    {
        if ( mHwnd && mWindowProc )
        {
            SetWindowLongPtr ( mHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR> ( mWindowProc ) );
            mWindowProc = nullptr;
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
    }

    void OpenGLRenderer::Initialize()
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
        wcex.lpszClassName = "AeonEngineTmpWnd";
        wcex.hIconSm = nullptr;
        ATOM atom = RegisterClassEx ( &wcex );
        HWND hWnd = CreateWindowEx ( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
                                     MAKEINTATOM ( atom ), "AeonEngine Temporary Window",
                                     WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                     0, 0, // Location
                                     10, 10, // dimensions
                                     nullptr,
                                     nullptr,
                                     GetModuleHandle ( nullptr ),
                                     nullptr );
        if ( !InitializeRenderingWindow ( GetModuleHandle ( nullptr ), hWnd ) )
        {
            DestroyWindow ( hWnd );
            UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                                  0x0ULL +
#endif
                                  MAKELONG ( atom, 0 ) ), nullptr );
            throw std::runtime_error ( "Unable to Initialize Rendering Window." );
        }

        if ( !LoadOpenGLAPI() )
        {
            throw std::runtime_error ( "Unable to Load OpenGL functions." );
        }

        FinalizeRenderingWindow();
        DestroyWindow ( hWnd );
        UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                              0x0ULL +
#endif
                              MAKELONG ( atom, 0 ) ), nullptr );
    }
    void OpenGLRenderer::Finalize()
    {
    }

    LRESULT OpenGLRenderer::WindowProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        switch ( uMsg )
        {
        case WM_SIZE:
            if ( LOWORD ( lParam ) && HIWORD ( lParam ) )
            {
                glViewport ( 0, 0, LOWORD ( lParam ), HIWORD ( lParam ) );
            }
            break;
        }
        return CallWindowProc ( mWindowProc, hwnd, uMsg, wParam, lParam );
    }
}
