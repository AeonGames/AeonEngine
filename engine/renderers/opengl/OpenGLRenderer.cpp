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
#ifdef __unix__
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

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = nullptr;
    const int context_attribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };
    const int visual_attribs[] =
    {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };
#endif
#ifdef _WIN32
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
#endif

    OpenGLRenderer::OpenGLRenderer()
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

    OpenGLRenderer::~OpenGLRenderer()
    {
        Finalize();
    }

    void OpenGLRenderer::BeginRender ( uintptr_t aWindowId ) const
    {
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [this, &aWindowId] ( const WindowData & window ) -> bool
        {
            if ( window.mWindowId == aWindowId )
            {
                return true;
            }
            return false;
        } );
        if ( i != mWindowRegistry.end() )
        {
#ifdef _WIN32
            if ( ( *i ).mDeviceContext != nullptr )
            {
                wglMakeCurrent ( ( *i ).mDeviceContext, reinterpret_cast<HGLRC> ( mOpenGLContext ) );
                glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            }
#else
            glXMakeCurrent ( ( *i ).mDisplay, reinterpret_cast<Window> ( ( *i ).mWindowId ),
                             static_cast<GLXContext> ( mOpenGLContext ) );
            glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#endif
        }
    }

    void OpenGLRenderer::Render ( uintptr_t aWindowId, const std::shared_ptr<Model> aModel ) const
    {
        auto& model = mModelMap.at ( aModel );
        model->Render ( mMatricesBuffer );
    }

    bool OpenGLRenderer::AllocateModelRenderData ( std::shared_ptr<Model> aModel )
    {
        if ( mModelMap.find ( aModel ) == mModelMap.end() )
        {
            /* We dont really need to cache OpenGL Models,
            since mModelMap IS our model cache.
            We DO need a deallocation function.*/
            mModelMap[aModel] = std::make_unique<OpenGLModel> ( aModel );
        }
        return true;
    }

    void OpenGLRenderer::EndRender ( uintptr_t aWindowId ) const
    {
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [this, &aWindowId] ( const WindowData & window ) -> bool
        {
            if ( window.mWindowId == aWindowId )
            {
                return true;
            }
            return false;
        } );
        if ( i != mWindowRegistry.end() )
        {
#if _WIN32
            if ( ( *i ).mDeviceContext != nullptr )
            {
                wglMakeCurrent ( ( *i ).mDeviceContext, reinterpret_cast<HGLRC> ( mOpenGLContext ) );
                SwapBuffers ( ( *i ).mDeviceContext );
            }
#else
            glXMakeCurrent ( ( *i ).mDisplay, reinterpret_cast<Window> ( ( *i ).mWindowId ),
                             static_cast<GLXContext> ( mOpenGLContext ) );
            glXSwapBuffers ( ( *i ).mDisplay, reinterpret_cast<Window> ( ( *i ).mWindowId ) );
#endif
        }
    }

    bool OpenGLRenderer::AddRenderingWindow ( uintptr_t aWindowId )
    {
        /**@todo Should a window wrapper be created? */
        /**@todo Should each window own a renderer instead of the renderer managing the windows? */
#ifdef WIN32
        mWindowRegistry.emplace_back();
        mWindowRegistry.back().mWindowId = aWindowId;
        mWindowRegistry.back().mDeviceContext = ( HDC ) GetDC ( reinterpret_cast<HWND> ( mWindowRegistry.back().mWindowId ) );
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int pf = ChoosePixelFormat ( mWindowRegistry.back().mDeviceContext, &pfd );
        SetPixelFormat ( mWindowRegistry.back().mDeviceContext, pf, &pfd );
        wglMakeCurrent ( mWindowRegistry.back().mDeviceContext, static_cast<HGLRC> ( mOpenGLContext ) );
        RECT rect;
        GetClientRect ( reinterpret_cast<HWND> ( mWindowRegistry.back().mWindowId ), &rect );
        glViewport ( 0, 0, rect.right, rect.bottom );
        OPENGL_CHECK_ERROR_NO_THROW;
        return true;
#if 0
        if ( ! ( mWindowRegistry.back().mOpenGLContext = wglCreateContextAttribs ( mWindowRegistry.back().mDeviceContext, static_cast<HGLRC> ( mOpenGLContext ), ContextAttribs ) ) )
        {
            return false;
        }
#endif
#else
        mWindowRegistry.emplace_back();
        mWindowRegistry.back().mDisplay = XOpenDisplay ( nullptr );
        if ( !mWindowRegistry.back().mDisplay )
        {
            return false;
        }
        mWindowRegistry.back().mWindowId = aWindowId;
#if 0
        if ( glXCreateContextAttribsARB )
        {
            // Get Window Attributes
            XWindowAttributes x_window_attributes{};
            XGetWindowAttributes ( mWindowRegistry.back().mDisplay, reinterpret_cast<Window> (  mWindowRegistry.back().mWindowId ), &x_window_attributes );

            int glx_fb_config_count;
            GLXFBConfig* glx_fb_config_list = glXGetFBConfigs (  mWindowRegistry.back().mDisplay, DefaultScreen (  mWindowRegistry.back().mDisplay ), &glx_fb_config_count );

            if ( !glx_fb_config_list )
            {
                return false;
            }
            // Pick the FB config/visual with the most samples per pixel
            int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
            for ( int i = 0; i < glx_fb_config_count; ++i )
            {
                XVisualInfo *vi = glXGetVisualFromFBConfig (  mWindowRegistry.back().mDisplay, glx_fb_config_list[i] );
                int value;
                glXGetFBConfigAttrib (  mWindowRegistry.back().mDisplay, glx_fb_config_list[i], GLX_DEPTH_SIZE, &value );
                std::cout << "Depth Buffer: " << value << std::endl;
                if ( ( vi ) && ( x_window_attributes.visual == vi->visual ) )
                {
                    std::cout << *vi << std::endl;
                    int samp_buf, samples;
                    glXGetFBConfigAttrib ( mWindowRegistry.back().mDisplay, glx_fb_config_list[i], GLX_SAMPLE_BUFFERS, &samp_buf );
                    glXGetFBConfigAttrib ( mWindowRegistry.back().mDisplay, glx_fb_config_list[i], GLX_SAMPLES       , &samples  );
                    if ( best_fbc < 0 || ( samp_buf && samples > best_num_samp ) )
                    {
                        best_fbc = i, best_num_samp = samples;
                    }
                    if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
                    {
                        worst_fbc = i, worst_num_samp = samples;
                    }
                }
                XFree ( vi );
            }

            if ( best_fbc < 0 )
            {
                XFree ( glx_fb_config_list );
                return false;
            }

            GLXFBConfig bestFbc = glx_fb_config_list[ best_fbc ];
            XFree ( glx_fb_config_list );

            XSync (  mWindowRegistry.back().mDisplay, False );
        }
        else
        {
            return false;
        }
#endif
        glGenBuffers ( 1, &mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferData ( GL_UNIFORM_BUFFER, sizeof ( mMatrices ),
                       mMatrices, GL_DYNAMIC_DRAW );
        OPENGL_CHECK_ERROR_NO_THROW;

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
        return true;
#endif
    }

    void OpenGLRenderer::RemoveRenderingWindow ( uintptr_t aWindowId )
    {
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [&aWindowId] ( const WindowData & aWindowData )
        {
            return aWindowData.mWindowId == aWindowId;
        } );
        if ( i != mWindowRegistry.end() )
        {
#ifdef WIN32
            ReleaseDC ( reinterpret_cast<HWND> ( i->mWindowId ), i->mDeviceContext );
#endif
#if 0
#ifdef WIN32
            if ( i->mDeviceContext != nullptr )
            {
                /*  AFTER this call all error checks will fail.
                    need to create shared context or somehow have a
                    single context for all windows. */
                wglMakeCurrent ( i->mDeviceContext, nullptr );
                OPENGL_CHECK_ERROR_NO_THROW;
                ReleaseDC ( reinterpret_cast<HWND> ( i->mWindowId ), i->mDeviceContext );
                i->mDeviceContext = nullptr;
            }
            if ( i->mOpenGLContext )
            {
                wglDeleteContext ( reinterpret_cast<HGLRC> ( i->mOpenGLContext ) );
                OPENGL_CHECK_ERROR_NO_THROW;
                i->mOpenGLContext = 0;
            }
#else
            if ( i->mWindowId && ( i->mWindowId == aWindowId ) )
            {
                if ( i->mOpenGLContext )
                {
                    glXMakeCurrent ( i->mDisplay, reinterpret_cast<Window> ( i->mWindowId ), nullptr );
                    glXDestroyContext ( i->mDisplay, i->mOpenGLContext );
                    i->mOpenGLContext = nullptr;
                }
                if ( i->mDisplay )
                {
                    XCloseDisplay ( i->mDisplay );
                    i->mDisplay = nullptr;
                }
            }
#endif
#endif
            mWindowRegistry.erase ( std::remove_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                    [&aWindowId] ( const WindowData & aWindowData )
            {
                return aWindowData.mWindowId == aWindowId;
            } )
            , mWindowRegistry.end() );
        }
    }

    void OpenGLRenderer::Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight )
    {
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [&aWindowId] ( const WindowData & aWindowData )
        {
            return aWindowId == aWindowData.mWindowId;
        } );
        if ( i != mWindowRegistry.end() )
        {
            if ( aWidth && aHeight )
            {
#ifdef WIN32
                wglMakeCurrent ( i->mDeviceContext, reinterpret_cast<HGLRC> ( mOpenGLContext ) );
                OPENGL_CHECK_ERROR_NO_THROW;
#else
                glXMakeCurrent ( i->mDisplay, reinterpret_cast<Window> ( i->mWindowId ), nullptr );
                OPENGL_CHECK_ERROR_NO_THROW;
#endif
                glViewport ( 0, 0, aWidth, aHeight );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
        }
    }

    void OpenGLRenderer::SetViewMatrix ( const float aMatrix[16] )
    {
        memcpy ( mViewMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }

    void OpenGLRenderer::SetProjectionMatrix ( const float aMatrix[16] )
    {
        memcpy ( mProjectionMatrix, aMatrix,  sizeof ( float ) * 16 );
        /* Flip Z axis to match Vulkan's right hand Normalized Device Coordinates (NDC).*/
        mProjectionMatrix[8]  *= -1;
        mProjectionMatrix[9]  *= -1;
        mProjectionMatrix[10] *= -1;
        mProjectionMatrix[11] *= -1;
        UpdateMatrices();
    }

    void OpenGLRenderer::SetModelMatrix ( const float aMatrix[16] )
    {
        memcpy ( mModelMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }

    void OpenGLRenderer::UpdateMatrices()
    {
        /** @todo Either publish this function or
            add arguments so just some matrices are
            updated based on which one changed.*/
        // Update mViewProjectionMatrix
        Multiply4x4Matrix ( mProjectionMatrix, mViewMatrix, mViewProjectionMatrix );
        // Update mModelViewMatrix
        Multiply4x4Matrix ( mViewMatrix, mModelMatrix, mModelViewMatrix );
        // Update mModelViewProjectionMatrix
        Multiply4x4Matrix ( mViewProjectionMatrix, mModelMatrix, mModelViewProjectionMatrix );
        /*  Calculate Normal Matrix
            Inverting a 3x3 matrix is cheaper than inverting a 4x4 matrix,
            so even if the shader alignment requires us to pad the 3x3 matrix into
            a 4x3 matrix we do these operations on a 3x3 basis.*/
        Extract3x3Matrix ( mModelViewMatrix, mNormalMatrix );
        Invert3x3Matrix ( mNormalMatrix, mNormalMatrix );
        Transpose3x3Matrix ( mNormalMatrix, mNormalMatrix );
        Convert3x3To4x3 ( mNormalMatrix, mNormalMatrix );

        glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBufferSubData ( GL_UNIFORM_BUFFER, 0, sizeof ( mMatrices ), mMatrices );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

#ifdef __unix__
    void OpenGLRenderer::Initialize()
    {
        // Retrieve Display
        Display* display = XOpenDisplay ( nullptr );
        if ( !display )
        {
            throw std::runtime_error ( "Failed retrieving X Display." );
        }
        // Retrieve Create Context function
        if ( !glXCreateContextAttribsARB )
        {
            if ( ! ( glXCreateContextAttribsARB =
                         ( PFNGLXCREATECONTEXTATTRIBSARBPROC )
                         glXGetProcAddressARB ( ( const GLubyte * ) "glXCreateContextAttribsARB" ) ) )
            {
                throw std::runtime_error ( "Failed retrieving glXCreateContextAttribsARB." );
            }
        }
        // Get best frame buffer configuration.
        int frame_buffer_config_count;
        GLXFBConfig *frame_buffer_configs = glXChooseFBConfig ( display, DefaultScreen ( display ),
                                            visual_attribs, &frame_buffer_config_count );
        if ( !frame_buffer_configs )
        {
            throw std::runtime_error ( "Failed to retrieve a framebuffer config" );
        }

        // Pick the FB config/visual with the most samples per pixel
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

        int i;
        for ( i = 0; i < frame_buffer_config_count; ++i )
        {
            XVisualInfo *vi = glXGetVisualFromFBConfig ( display, frame_buffer_configs[i] );
            if ( vi )
            {
                int samp_buf, samples;
                glXGetFBConfigAttrib ( display, frame_buffer_configs[i], GLX_SAMPLE_BUFFERS, &samp_buf );
                glXGetFBConfigAttrib ( display, frame_buffer_configs[i], GLX_SAMPLES       , &samples  );

                if ( best_fbc < 0 || ( samp_buf && samples > best_num_samp ) )
                {
                    best_fbc = i, best_num_samp = samples;
                }
                if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
                {
                    worst_fbc = i, worst_num_samp = samples;
                }
            }
            XFree ( vi );
        }

        GLXFBConfig bestFbc = frame_buffer_configs[ best_fbc ];
        XFree ( frame_buffer_configs );
        if ( ! ( mOpenGLContext = glXCreateContextAttribsARB ( display, bestFbc, 0,
                                  True, context_attribs ) ) )
        {
            throw std::runtime_error ( "glXCreateContextAttribsARB Failed." );
        }

        // Verifying that context is a direct context
        if ( ! glXIsDirect (  display,  static_cast<GLXContext> ( mOpenGLContext ) ) )
        {
            std::cout << LogLevel ( LogLevel::Level::Info ) <<
                      "Indirect GLX rendering context obtained" << std::endl;
        }
        else
        {
            std::cout << LogLevel ( LogLevel::Level::Info ) <<
                      "Direct GLX rendering context obtained" << std::endl;
        }
#if 0
        // do we need this call here?
        glXMakeCurrent (  mWindowRegistry.back().mDisplay, reinterpret_cast<Window> (  mWindowRegistry.back().mWindowId ),  mWindowRegistry.back().mOpenGLContext );
#endif
        if ( !LoadOpenGLAPI() )
        {
            throw std::runtime_error ( "Unable to Load OpenGL functions." );
        }
    }

    void OpenGLRenderer::Finalize()
    {
        if ( Display* display = XOpenDisplay ( nullptr ) )
        {
            glXMakeCurrent ( display, 0, 0 );
            if ( mOpenGLContext != nullptr )
            {
                glXDestroyContext ( display, static_cast<GLXContext> ( mOpenGLContext ) );
                mOpenGLContext = nullptr;
            }
        }
    }
#else
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
#endif
}
