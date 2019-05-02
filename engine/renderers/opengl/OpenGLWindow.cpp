/*
Copyright (C) 2017-2019 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Frustum.h"
#include "aeongames/AABB.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/LogLevel.h"
#include "OpenGLWindow.h"
#include "OpenGLRenderer.h"
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLMesh.h"
#include "OpenGLFunctions.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <utility>

namespace AeonGames
{
#ifdef WIN32
    LRESULT CALLBACK AeonEngineWindowProc (
        _In_ HWND   hwnd,
        _In_ UINT   uMsg,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam
    )
    {
        //OpenGLWindow* window = reinterpret_cast<OpenGLWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        switch ( uMsg )
        {
        case WM_CLOSE:
        //DestroyWindow(hwnd);
        //break;
        case WM_DESTROY:
            PostQuitMessage ( 0 );
            break;
        default:
            return DefWindowProc ( hwnd, uMsg, wParam, lParam );
        }
        return 0;
    }
#elif defined(__unix__)
    std::ostream &operator<< ( std::ostream &out, const XVisualInfo& aXVisualInfo )
    {
        out << "Visual: " << aXVisualInfo.visual << std::endl;
        out << "VisualID: " << aXVisualInfo.visualid << std::endl;
        out << "Screen: " << aXVisualInfo.screen << std::endl;
        out << "Depth: " << aXVisualInfo.depth << std::endl;
        out << "Class: " << aXVisualInfo.c_class << std::endl;
        out << "Red Mask: " << aXVisualInfo.red_mask << std::endl;
        out << "Green Mask: " << aXVisualInfo.green_mask << std::endl;
        out << "Blue Mask: " << aXVisualInfo.blue_mask << std::endl;
        out << "Colormap Size: " << aXVisualInfo.colormap_size << std::endl;
        out << "Bits Per RGB: " << aXVisualInfo.bits_per_rgb << std::endl;
        return out;
    }
#endif

    OpenGLWindow::OpenGLWindow ( const OpenGLRenderer& aOpenGLRenderer, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) :
        mOpenGLRenderer ( aOpenGLRenderer ), mOwnsWindowId{ true }, mFullScreen{aFullScreen}
    {
#ifdef WIN32
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
        mWindowId = CreateWindowEx ( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
                                     MAKEINTATOM ( atom ), "OpenGL AeonEngine Window",
                                     WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                     rect.left, rect.top, // Location
                                     rect.right - rect.left, rect.bottom - rect.top, // Dimensions
                                     nullptr,
                                     nullptr,
                                     GetModuleHandle ( nullptr ),
                                     nullptr );
        SetWindowLongPtr ( reinterpret_cast<HWND> ( mWindowId ), GWLP_USERDATA, reinterpret_cast<LONG_PTR> ( this ) );
#endif
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }
    OpenGLWindow::OpenGLWindow ( void* aWindowId, const OpenGLRenderer&  aOpenGLRenderer ) :
        mOpenGLRenderer ( aOpenGLRenderer ), mWindowId ( aWindowId )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    OpenGLWindow::~OpenGLWindow()
    {
        Finalize();
    }

    void* OpenGLWindow::GetWindowId() const
    {
        return mWindowId;
    }

    void OpenGLWindow::OnResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight )
        {
#ifdef WIN32
            HDC hdc = GetDC ( reinterpret_cast<HWND> ( mWindowId ) );
            wglMakeCurrent ( hdc, reinterpret_cast<HGLRC> ( mOpenGLRenderer.GetOpenGLContext() ) );
            OPENGL_CHECK_ERROR_NO_THROW;
            ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), hdc );
#else
            glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                             reinterpret_cast<::Window> ( mWindowId ),
                             static_cast<GLXContext> ( mDeviceContext ) );
            OPENGL_CHECK_ERROR_NO_THROW;
#endif
            glViewport ( aX, aY, aWidth, aHeight );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLWindow::BeginRender() const
    {
#ifdef _WIN32
        if ( mDeviceContext )
        {
            EndRender();
            std::cout << LogLevel::Error << "BeginRender call without a previous EndRender call." << std::endl;
            return;
        }
        mDeviceContext = reinterpret_cast<void*> ( GetDC ( reinterpret_cast<HWND> ( mWindowId ) ) );
        wglMakeCurrent ( reinterpret_cast<HDC> ( mDeviceContext ), static_cast<HGLRC> ( mOpenGLRenderer.GetOpenGLContext() ) );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#else
        glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ),
                         static_cast<GLXContext> ( mDeviceContext ) );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#endif

        Matrix4x4 projection_matrix =
            mProjectionMatrix * Matrix4x4
        {
            // Flip Matrix
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, ( sizeof ( float ) * 16 ) * 1, sizeof ( float ) * 16, ( projection_matrix ).GetMatrix4x4() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, ( sizeof ( float ) * 16 ) * 2, sizeof ( float ) * 16, mViewMatrix.GetMatrix4x4() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBufferBase ( GL_UNIFORM_BUFFER, 0, mMatricesBuffer );
    }

    void OpenGLWindow::EndRender() const
    {
#if _WIN32
        if ( !mDeviceContext )
        {
            std::cout << LogLevel::Error << "EndRender call without a previous BeginRender call." << std::endl;
            return;
        }
        SwapBuffers ( reinterpret_cast<HDC> ( mDeviceContext ) );
        ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), reinterpret_cast<HDC> ( mDeviceContext ) );
        mDeviceContext = nullptr;
#else
        glXSwapBuffers ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ) );
#endif
    }

    void OpenGLWindow::Render ( const Matrix4x4& aModelMatrix,
                                const Mesh& aMesh,
                                const Pipeline& aPipeline,
                                const Material* aMaterial,
                                const UniformBuffer* aSkeleton,
                                uint32_t aVertexStart,
                                uint32_t aVertexCount,
                                uint32_t aInstanceCount,
                                uint32_t aFirstInstance ) const
    {
        const OpenGLMesh& opengl_mesh{reinterpret_cast<const OpenGLMesh&> ( aMesh ) };
        const OpenGLPipeline& opengl_pipeline{reinterpret_cast<const OpenGLPipeline&> ( aPipeline ) };
        opengl_pipeline.Use (
            reinterpret_cast<const OpenGLMaterial&> ( ( aMaterial ) ?
                    *aMaterial : opengl_pipeline.GetDefaultMaterial() ),
            reinterpret_cast<const OpenGLUniformBuffer*> ( aSkeleton ) );
        OPENGL_CHECK_ERROR_NO_THROW;
        glNamedBufferSubData ( mMatricesBuffer, ( sizeof ( float ) * 16 ) * 0, sizeof ( float ) * 16, aModelMatrix.GetMatrix4x4() );
        OPENGL_CHECK_ERROR_NO_THROW;

        /// @todo Add some sort of way to make use of the aFirstInstance parameter
        opengl_mesh.BindVertexArray();
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( aMesh.GetIndexCount() )
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, opengl_mesh.GetIndexBufferId() );
            OPENGL_CHECK_ERROR_NO_THROW;
            glDrawElementsInstanced ( opengl_pipeline.GetTopology(), ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                                      opengl_mesh.GetIndexType(), reinterpret_cast<const uint8_t*> ( 0 ) + aMesh.GetIndexSize() *aVertexStart, aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glDrawArraysInstanced ( opengl_pipeline.GetTopology(), aVertexStart, ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(), aInstanceCount );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    const GLuint OpenGLWindow::GetMatricesBuffer() const
    {
        return mMatricesBuffer;
    }

    void OpenGLWindow::Initialize()
    {
#if _WIN32
        HDC hdc = GetDC ( static_cast<HWND> ( mWindowId ) );
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
        wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLRenderer.GetOpenGLContext() ) );
        ReleaseDC ( static_cast<HWND> ( mWindowId ), hdc );
        RECT rect;
        GetClientRect ( static_cast<HWND> ( mWindowId ), &rect );
        glViewport ( rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top );
        OPENGL_CHECK_ERROR_THROW;
#else
        if ( !mOwnsWindowId )
        {
            /* We need to create a separate OpenGL Context for external Windows. */
            XWindowAttributes x_window_attributes {};
            XGetWindowAttributes ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                                   reinterpret_cast<::Window> ( mWindowId ), &x_window_attributes );
            int frame_buffer_config_count;
            GLXFBConfig *frame_buffer_configs =
                glXChooseFBConfig ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                                    DefaultScreen ( mOpenGLRenderer.GetWindowId()  ),
                                    nullptr, &frame_buffer_config_count );
            if ( !frame_buffer_configs )
            {
                throw std::runtime_error ( "Failed to retrieve a framebuffer configs" );
            }
            for ( int i = 0; i < frame_buffer_config_count; ++i )
            {
                XVisualInfo *visual_info = glXGetVisualFromFBConfig ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ), frame_buffer_configs[i] );
                if ( visual_info && visual_info->visualid == x_window_attributes.visual->visualid )
                {
                    int context_attribs[] =
                    {
                        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                        GLX_CONTEXT_MINOR_VERSION_ARB, 5,
                        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                        None
                    };
                    std::cout << LogLevel::Info << *visual_info << std::endl;
                    if ( ! ( mDeviceContext =
                                 glXCreateContextAttribsARB (
                                     static_cast<Display*> (
                                         mOpenGLRenderer.GetWindowId() ),
                                     frame_buffer_configs[i],
                                     static_cast<GLXContext> (
                                         mOpenGLRenderer.GetOpenGLContext() ),
                                     True,
                                     context_attribs ) ) )
                    {
                        std::cout << LogLevel::Error << "Failed to create OpenGL context for external window." << std::endl;
                    }
                    XFree ( visual_info );
                    break;
                }
                XFree ( visual_info );
            }
            XFree ( frame_buffer_configs );
        }
        else
        {
            /** @todo This needs to be revisited once code for native Window is in place*/
            mDeviceContext = mOpenGLRenderer.GetOpenGLContext();
        }
        if ( !mDeviceContext )
        {
            throw std::runtime_error ( "Unable to find a usable OpenGL context." );
        }
        XSetErrorHandler ( [] ( Display * display, XErrorEvent * error_event ) -> int
        {
            char error_string[1024];
            XGetErrorText ( display, error_event->error_code, error_string, 1024 );
            std::cout << LogLevel::Error << "Error Code " << static_cast<int> ( error_event->error_code ) << " " << error_string << std::endl;
            return 0;
        } );
        if ( !glXMakeCurrent (  static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                                reinterpret_cast<::Window> ( mWindowId ),
                                static_cast<GLXContext> ( mDeviceContext ) ) )
        {
            throw std::runtime_error ( "glXMakeCurrent call Failed." );
        }
        XSetErrorHandler ( nullptr );
#ifdef SINGLE_VAO
        glGenVertexArrays ( 1, &mVAO );
        OPENGL_CHECK_ERROR_THROW;
        glBindVertexArray ( mVAO );
        OPENGL_CHECK_ERROR_THROW;
#endif
#endif
        glClearColor ( 0.5f, 0.5f, 0.5f, 1.0f );
        OPENGL_CHECK_ERROR_NO_THROW;
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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

        // Initialize Matrix Buffer
        glGenBuffers ( 1, &mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        float matrices[48] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glBufferData ( GL_UNIFORM_BUFFER, sizeof ( float ) * 48,
                       matrices, GL_DYNAMIC_DRAW );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLWindow::Finalize()
    {
#ifdef __unix__
        glXMakeCurrent (  static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                          reinterpret_cast<::Window> ( mWindowId ),
                          static_cast<GLXContext> ( mDeviceContext ) );
#endif
        if ( glIsBuffer ( mMatricesBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteBuffers ( 1, &mMatricesBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mMatricesBuffer = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
#ifdef SINGLE_VAO
        if ( glIsVertexArray ( mVAO ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteVertexArrays ( 1, &mVAO );
            OPENGL_CHECK_ERROR_NO_THROW;
            mVAO = 0;
        }
#endif
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( mOwnsWindowId )
        {
#ifdef _WIN32
            HDC hdc = GetDC ( static_cast<HWND> ( mWindowId ) );
            wglMakeCurrent ( hdc, static_cast<HGLRC> ( mOpenGLRenderer.GetOpenGLContext() ) );
            OPENGL_CHECK_ERROR_NO_THROW;
            ATOM atom = GetClassWord ( static_cast<HWND> ( mWindowId ), GCW_ATOM );
            wglMakeCurrent ( hdc, nullptr );
            ReleaseDC ( static_cast<HWND> ( mWindowId ), hdc );
            DestroyWindow ( static_cast<HWND> ( mWindowId ) );
            UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                                  0x0ULL +
#endif
                                  MAKELONG ( atom, 0 ) ), nullptr );
#endif
        }
        else
        {
#ifdef __unix__
            glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ), 0, 0 );
            if ( mDeviceContext != nullptr )
            {
                glXDestroyContext ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ), static_cast<GLXContext> ( mDeviceContext ) );
                mDeviceContext = nullptr;
            }
#endif
        }
    }


    void OpenGLWindow::Run ( const Scene& aScene )
    {
#if _WIN32
        MSG msg;
        bool done = false;
        ShowWindow ( static_cast<HWND> ( mWindowId ), SW_SHOW );
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
                BeginRender();
                Window::Render ( aScene );
                EndRender();
            }
        }
#else
#endif
    }
}
