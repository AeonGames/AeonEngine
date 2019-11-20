/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLWinWindow.h"
#include "OpenGLRenderer.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    OpenGLWinWindow::OpenGLWinWindow ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        OpenGLWindow { aOpenGLRenderer, aX, aY, aWidth, aHeight, aFullScreen }
    {
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

    OpenGLWinWindow::OpenGLWinWindow ( const OpenGLRenderer& aOpenGLRenderer, void* aWindowId ) :
        OpenGLWindow{aOpenGLRenderer, aWindowId}
    {
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

    OpenGLWinWindow::~OpenGLWinWindow()
    {
        OpenGLWindow::Finalize();
        Finalize();
    }

    void OpenGLWinWindow::MakeCurrent()
    {
        wglMakeCurrent ( mDeviceContext, static_cast<HGLRC> ( mOpenGLRenderer.GetOpenGLContext() ) );
    }
    void OpenGLWinWindow::SwapBuffers()
    {
        ::SwapBuffers ( mDeviceContext );
    }

    void OpenGLWinWindow::Initialize()
    {
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
        wglMakeCurrent ( mDeviceContext, static_cast<HGLRC> ( mOpenGLRenderer.GetOpenGLContext() ) );
        RECT rect;
        GetClientRect ( static_cast<HWND> ( mWindowId ), &rect );
        glViewport ( rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLWinWindow::Finalize()
    {
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( !wglMakeCurrent ( reinterpret_cast<HDC> ( mOpenGLRenderer.GetDeviceContext() ), static_cast<HGLRC> ( mOpenGLRenderer.GetOpenGLContext() ) ) )
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
}
#endif