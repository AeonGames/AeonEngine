/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/LogLevel.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/Model.h"
#include "aeongames/Matrix4x4.h"
#include "OpenGLFunctions.h"
#include "OpenGLWinAPIRenderer.h"
#include "OpenGLPipeline.h"
#include "OpenGLTexture.h"
#include "OpenGLWindow.h"
#include "math/3DMath.h"

namespace AeonGames
{
    static PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsString = nullptr;
    static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs = nullptr;
    const int ContextAttribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    void OpenGLWinAPIRenderer::Initialize()
    {
        // Initialize Internal Window
        PIXELFORMATDESCRIPTOR pfd;
        RECT rect = { 0, 0, 10, 10 };
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
        wcex.lpszClassName = "AeonEngineOpenGLInternalWindow";
        wcex.hIconSm = nullptr;
        ATOM atom = RegisterClassEx ( &wcex );
        mWindow = CreateWindowEx ( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
                                   MAKEINTATOM ( atom ), "AeonEngine OpenGL Internal Window",
                                   WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                   0, 0, // Location
                                   rect.right - rect.left, rect.bottom - rect.top, // dimensions
                                   nullptr,
                                   nullptr,
                                   GetModuleHandle ( nullptr ),
                                   nullptr );
        mDeviceContext = GetDC ( static_cast<HWND> ( mWindow ) );
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
        int pf = ChoosePixelFormat ( static_cast<HDC> ( mDeviceContext ), &pfd );
        SetPixelFormat ( static_cast<HDC> ( mDeviceContext ), pf, &pfd );

        // Create OpenGL Context
        mGLContext = wglCreateContext ( static_cast<HDC> ( mDeviceContext ) );
        wglMakeCurrent ( static_cast<HDC> ( mDeviceContext ), static_cast<HGLRC> ( mGLContext ) );

        // Get newer functions if needed
        if ( !wglGetExtensionsString )
        {
            if ( ! ( wglGetExtensionsString = ( PFNWGLGETEXTENSIONSSTRINGARBPROC ) wglGetProcAddress ( "wglGetExtensionsStringARB" ) ) )
            {
                wglDeleteContext ( static_cast<HGLRC> ( mGLContext ) );
                mGLContext = nullptr;
                ReleaseDC ( static_cast<HWND> ( mWindow ), static_cast<HDC> ( mDeviceContext ) );
                DestroyWindow ( static_cast<HWND> ( mWindow ) );
                throw std::runtime_error ( "Failed retrieving a pointer to wglGetExtensionsString" );
            }
        }
        if ( !wglCreateContextAttribs )
        {
            if ( ! ( wglCreateContextAttribs = ( PFNWGLCREATECONTEXTATTRIBSARBPROC ) wglGetProcAddress ( "wglCreateContextAttribsARB" ) ) )
            {
                wglDeleteContext ( static_cast<HGLRC> ( mGLContext ) );
                mGLContext = nullptr;
                ReleaseDC ( static_cast<HWND> ( mWindow ), reinterpret_cast<HDC> ( mDeviceContext ) );
                DestroyWindow ( static_cast<HWND> ( mWindow ) );
                throw std::runtime_error ( "Failed retrieving a pointer to wglCreateContextAttribsARB" );
            }
        }
        if ( strstr ( wglGetExtensionsString ( reinterpret_cast<HDC> ( mDeviceContext ) ), "WGL_ARB_create_context" ) != nullptr )
        {
            wglDeleteContext ( static_cast<HGLRC> ( mGLContext ) );
            if ( ! ( mGLContext = wglCreateContextAttribs ( reinterpret_cast<HDC> ( mDeviceContext ), nullptr, ContextAttribs ) ) )
            {
                ReleaseDC ( static_cast<HWND> ( mWindow ), reinterpret_cast<HDC> ( mDeviceContext ) );
                DestroyWindow ( static_cast<HWND> ( mWindow ) );
                throw std::runtime_error ( "wglCreateContextAttribs Failed" );
            }
        }
        else
        {
            wglDeleteContext ( static_cast<HGLRC> ( mGLContext ) );
            mGLContext = nullptr;
            ReleaseDC ( static_cast<HWND> ( mWindow ), static_cast<HDC> ( mDeviceContext ) );
            DestroyWindow ( static_cast<HWND> ( mWindow ) );
            throw std::runtime_error ( "WGL_ARB_create_context is not available" );
        }
        // Make OpenGL Context current.
        if ( !wglMakeCurrent ( static_cast<HDC> ( mDeviceContext ), reinterpret_cast<HGLRC> ( mGLContext ) ) )
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
    }

    void OpenGLWinAPIRenderer::Finalize()
    {
        if ( wglMakeCurrent ( reinterpret_cast<HDC> ( mDeviceContext ), NULL ) != TRUE )
        {
            std::cout << LogLevel::Error << "wglMakeCurrent failed." << std::endl;
        }
        ATOM atom = GetClassWord ( static_cast<HWND> ( mWindow ), GCW_ATOM );
        if ( wglDeleteContext ( static_cast<HGLRC> ( mGLContext ) ) != TRUE )
        {
            std::cout << LogLevel::Error << "wglDeleteContext failed." << std::endl;
        }
        mGLContext = nullptr;
        ReleaseDC ( static_cast<HWND> ( mWindow ), reinterpret_cast<HDC> ( mDeviceContext ) );
        DestroyWindow ( static_cast<HWND> ( mWindow ) );
        UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                              0x0ULL +
#endif
                              MAKELONG ( atom, 0 ) ), nullptr );
    }
    OpenGLWinAPIRenderer::OpenGLWinAPIRenderer()
    {
        try
        {
            Initialize();
            if ( !LoadOpenGLAPI() )
            {
                throw std::runtime_error ( "Unable to Load OpenGL functions." );
            }
            InitializeOverlay();
        }
        catch ( ... )
        {
            FinalizeOverlay();
            Finalize();
            throw;
        }
    }
    OpenGLWinAPIRenderer::~OpenGLWinAPIRenderer()
    {
        FinalizeOverlay();
        Finalize();
    }

    bool OpenGLWinAPIRenderer::MakeCurrent () const
    {
        return wglMakeCurrent ( mDeviceContext, mGLContext );
    }

    void* OpenGLWinAPIRenderer::GetContext() const
    {
        return mGLContext;
    }
}
#endif
