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
    OpenGLRenderer::OpenGLRenderer() try
    {
        Initialize();
    }
    catch ( ... )
    {
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
        }
    }

    void OpenGLRenderer::EndRender() const
    {
        if ( mDeviceContext != nullptr )
        {
            SwapBuffers ( mDeviceContext );
        }
    }

    bool OpenGLRenderer::RegisterRenderingWindow ( uintptr_t aWindowId )
    {
        mHwnd = reinterpret_cast<HWND> ( aWindowId );
        PIXELFORMATDESCRIPTOR pfd{};
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
                if ( !LoadOpenGLAPI() )
                {
                    std::cout << "Unable to Load OpenGL functions." << std::endl;
                    return false;
                }
                RECT rect;
                GetClientRect ( mHwnd, &rect );
                glViewport ( 0, 0, rect.right, rect.bottom );
                glClearColor ( 0.5f, 0.5f, 0.5f, 0.0f );
                glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    void OpenGLRenderer::UnregisterRenderingWindow ( uintptr_t aWindowId )
    {
        if ( mHwnd && ( mHwnd == reinterpret_cast<HWND> ( aWindowId ) ) )
        {
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
    }

    void OpenGLRenderer::Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const
    {
        if ( aWidth > 0 && aHeight > 0 )
        {
            glViewport ( 0, 0, aWidth, aHeight );
        }
    }

    void OpenGLRenderer::Initialize()
    {
    }
    void OpenGLRenderer::Finalize()
    {
    }
}
