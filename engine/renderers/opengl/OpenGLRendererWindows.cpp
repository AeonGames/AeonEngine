/*
Copyright (C) 2016-2017 Rodrigo Jose Hernandez Cordoba

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
#include <algorithm>
#include "math/3DMath.h"
#include "OpenGLFunctions.h"
#include "OpenGLRenderer.h"
#include "OpenGLMesh.h"
#include "OpenGLPipeline.h"
#include "OpenGLTexture.h"
#include "OpenGLModel.h"
#include "aeongames/LogLevel.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/Model.h"

namespace AeonGames
{
    static PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsString = nullptr;
    static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs = nullptr;
    const int ContextAttribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    bool OpenGLRenderer::AddRenderingWindow ( void* aWindowId )
    {
        /**@todo Should a window wrapper be created? */
        /**@todo Should each window own a renderer instead of the renderer managing the windows? */
        mWindowRegistry.emplace_back ( aWindowId );
        HDC hdc = GetDC ( static_cast<HWND> ( mWindowRegistry.back() ) );
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int pf = ChoosePixelFormat ( hdc, &pfd );
        SetPixelFormat ( hdc, pf, &pfd );
        wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLContext ) );
        ReleaseDC ( static_cast<HWND> ( mWindowRegistry.back() ), hdc );
        RECT rect;
        GetClientRect ( static_cast<HWND> ( mWindowRegistry.back() ), &rect );
        glViewport ( 0, 0, rect.right, rect.bottom );
        OPENGL_CHECK_ERROR_NO_THROW;
        return true;
    }

    void OpenGLRenderer::Initialize()
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
        wcex.hInstance = GetModuleHandle ( NULL );
        wcex.hIcon = LoadIcon ( NULL, IDI_WINLOGO );
        wcex.hCursor = LoadCursor ( NULL, IDC_ARROW );
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = "glUnitTest";
        wcex.hIconSm = NULL;
        ATOM atom = RegisterClassEx ( &wcex );
        mWindowId = CreateWindowEx ( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
                                     MAKEINTATOM ( atom ), "OpenGL Unit Testing Window",
                                     WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                     0, 0, // Location
                                     rect.right - rect.left, rect.bottom - rect.top, // dimensions
                                     NULL,
                                     NULL,
                                     GetModuleHandle ( NULL ),
                                     NULL );
        HDC hdc = GetDC ( static_cast<HWND> ( mWindowId ) );
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
        int pf = ChoosePixelFormat ( hdc, &pfd );
        SetPixelFormat ( hdc, pf, &pfd );

        // Create OpenGL Context
        mOpenGLContext = wglCreateContext ( hdc );
        wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLContext ) );

        // Get newer functions if needed
        if ( !wglGetExtensionsString )
        {
            if ( ! ( wglGetExtensionsString = ( PFNWGLGETEXTENSIONSSTRINGARBPROC ) wglGetProcAddress ( "wglGetExtensionsStringARB" ) ) )
            {
                wglMakeCurrent ( hdc, nullptr );
                wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
                mOpenGLContext = nullptr;
                ReleaseDC ( static_cast<HWND> ( mWindowId ), hdc );
                DestroyWindow ( static_cast<HWND> ( mWindowId ) );
                throw std::runtime_error ( "Failed retrieving a pointer to wglGetExtensionsString" );
            }
        }
        if ( !wglCreateContextAttribs )
        {
            if ( ! ( wglCreateContextAttribs = ( PFNWGLCREATECONTEXTATTRIBSARBPROC ) wglGetProcAddress ( "wglCreateContextAttribsARB" ) ) )
            {
                wglMakeCurrent ( hdc, nullptr );
                wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
                mOpenGLContext = nullptr;
                ReleaseDC ( static_cast<HWND> ( mWindowId ), hdc );
                DestroyWindow ( static_cast<HWND> ( mWindowId ) );
                throw std::runtime_error ( "Failed retrieving a pointer to wglCreateContextAttribsARB" );
            }
        }
        if ( strstr ( wglGetExtensionsString ( hdc ), "WGL_ARB_create_context" ) != nullptr )
        {
            wglMakeCurrent ( hdc, nullptr );
            wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
            if ( ! ( mOpenGLContext = wglCreateContextAttribs ( hdc, nullptr /* change to use local context */, ContextAttribs ) ) )
            {
                ReleaseDC ( static_cast<HWND> ( mWindowId ), hdc );
                DestroyWindow ( static_cast<HWND> ( mWindowId ) );
                throw std::runtime_error ( "wglCreateContextAttribs Failed" );
            }
        }
        else
        {
            wglMakeCurrent ( hdc, nullptr );
            wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
            mOpenGLContext = nullptr;
            ReleaseDC ( static_cast<HWND> ( mWindowId ), hdc );
            DestroyWindow ( static_cast<HWND> ( mWindowId ) );
            throw std::runtime_error ( "WGL_ARB_create_context is not available" );
        }

        // Get New OpenGL API
        if ( !wglMakeCurrent ( hdc, reinterpret_cast<HGLRC> ( mOpenGLContext ) ) )
        {
            std::cout << "wglMakeCurrent Failed. Error: " << GetLastError() << std::endl;
        }
        if ( !LoadOpenGLAPI() )
        {
            std::cout << "Unable to Load OpenGL functions." << std::endl;
        }

        // Initialize OpenGL Objects
        glGenBuffers ( 1, &mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferData ( GL_UNIFORM_BUFFER, sizeof ( mMatrices ),
                       mMatrices, GL_DYNAMIC_DRAW );
        OPENGL_CHECK_ERROR_NO_THROW;

        glClearColor ( 0.5f, 0.5f, 0.5f, 0.0f );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_BLEND );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDepthFunc ( GL_LESS );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_NO_THROW;
        glCullFace ( GL_BACK );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_CULL_FACE );
        OPENGL_CHECK_ERROR_NO_THROW;
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLRenderer::Finalize()
    {
        HDC hdc = GetDC ( static_cast<HWND> ( mWindowId ) );
        wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLContext ) );
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( glIsBuffer ( mMatricesBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteBuffers ( 1, &mMatricesBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mMatricesBuffer = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;

        // Force model shared pointers to drop their references.
        /// @todo Re-evaluate the need for a ModelMap.
        mModelMap.clear();

        ATOM atom = GetClassWord ( static_cast<HWND> ( mWindowId ), GCW_ATOM );
        wglMakeCurrent ( hdc, nullptr );
        wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
        mOpenGLContext = nullptr;
        ReleaseDC ( static_cast<HWND> ( mWindowId ), hdc );
        DestroyWindow ( static_cast<HWND> ( mWindowId ) );
        UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                              0x0ULL +
#endif
                              MAKELONG ( atom, 0 ) ), NULL );
    }
}
