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
#include "math/3DMath.h"
#include "OpenGLFunctions.h"
#include "OpenGLRenderer.h"
#include "OpenGLMesh.h"
#include "OpenGLProgram.h"
#include "aeongames/LogLevel.h"
#include "aeongames/ResourceCache.h"

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

    void OpenGLRenderer::BeginRender() const
    {
#ifdef _WIN32
        if ( mDeviceContext != nullptr )
        {
            wglMakeCurrent ( mDeviceContext, mOpenGLContext );
            glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        }
#else
        if ( mGLXContext != nullptr )
        {
            glXMakeCurrent ( mDisplay, mWindow, mGLXContext );
            glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        }
#endif
    }

    void OpenGLRenderer::Render ( const std::shared_ptr<Mesh>& aMesh, const std::shared_ptr<Program>& aProgram ) const
    {
        OpenGLProgram* program = reinterpret_cast<OpenGLProgram*> ( aProgram.get() );
        program->Use();
        glBindBufferBase ( GL_UNIFORM_BUFFER, 0, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        aMesh->Render();
    }

    void OpenGLRenderer::EndRender() const
    {
#if _WIN32
        if ( mDeviceContext != nullptr )
        {
            SwapBuffers ( mDeviceContext );
        }
#else
        if ( mGLXContext != nullptr )
        {
            glXSwapBuffers ( mDisplay, mWindow );
        }
#endif
    }

    bool OpenGLRenderer::RegisterRenderingWindow ( uintptr_t aWindowId )
    {
#ifdef WIN32
        mWindowId = aWindowId;
        PIXELFORMATDESCRIPTOR pfd{};
        PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = nullptr;
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
        mDeviceContext = GetDC ( reinterpret_cast<HWND> ( mWindowId ) );
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

                glGenBuffers ( 1, &mMatricesBuffer );
                OPENGL_CHECK_ERROR_NO_THROW;
                glBindBuffer ( GL_UNIFORM_BUFFER, mMatricesBuffer );
                OPENGL_CHECK_ERROR_NO_THROW;
                glBufferData ( GL_UNIFORM_BUFFER, sizeof ( mMatrices ),
                               mMatrices, GL_DYNAMIC_DRAW );
                OPENGL_CHECK_ERROR_NO_THROW;

                RECT rect;
                GetClientRect ( reinterpret_cast<HWND> ( mWindowId ), &rect );
                glViewport ( 0, 0, rect.right, rect.bottom );
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
        }
        else
        {
            return false;
        }
        return true;
#else
        mDisplay = XOpenDisplay ( nullptr );
        if ( !mDisplay )
        {
            return false;
        }
        mWindow = reinterpret_cast<Window> ( aWindowId );
        PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = ( PFNGLXCREATECONTEXTATTRIBSARBPROC )
                glXGetProcAddressARB ( ( const GLubyte * ) "glXCreateContextAttribsARB" );
        if ( glXCreateContextAttribsARB )
        {
            int context_attribs[] =
            {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                GLX_CONTEXT_MINOR_VERSION_ARB, 2,
                GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                None
            };

            // Get Window Attributes
            XWindowAttributes x_window_attributes{};
            XGetWindowAttributes ( mDisplay, mWindow, &x_window_attributes );

            int glx_fb_config_count;
            GLXFBConfig* glx_fb_config_list = glXGetFBConfigs ( mDisplay, DefaultScreen ( mDisplay ), &glx_fb_config_count );

            if ( !glx_fb_config_list )
            {
                return false;
            }
            // Pick the FB config/visual with the most samples per pixel
            int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
            for ( int i = 0; i < glx_fb_config_count; ++i )
            {
                XVisualInfo *vi = glXGetVisualFromFBConfig ( mDisplay, glx_fb_config_list[i] );
                int value;
                glXGetFBConfigAttrib ( mDisplay, glx_fb_config_list[i], GLX_DEPTH_SIZE, &value );
                std::cout << "Depth Buffer: " << value << std::endl;
                if ( ( vi ) && ( x_window_attributes.visual == vi->visual ) )
                {
                    std::cout << *vi << std::endl;
                    int samp_buf, samples;
                    glXGetFBConfigAttrib ( mDisplay, glx_fb_config_list[i], GLX_SAMPLE_BUFFERS, &samp_buf );
                    glXGetFBConfigAttrib ( mDisplay, glx_fb_config_list[i], GLX_SAMPLES       , &samples  );
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
            mGLXContext = glXCreateContextAttribsARB ( mDisplay, bestFbc, nullptr, True, context_attribs );
            XSync ( mDisplay, False );
            if ( mGLXContext != nullptr )
            {
                std::cout << LogLevel ( LogLevel::Level::Info ) <<
                          "Created GL " <<  context_attribs[1] <<
                          "." <<  context_attribs[3] << " context" << std::endl;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        // Verifying that context is a direct context
        if ( ! glXIsDirect ( mDisplay, mGLXContext ) )
        {
            std::cout << LogLevel ( LogLevel::Level::Info ) <<
                      "Indirect GLX rendering context obtained" << std::endl;
        }
        else
        {
            std::cout << LogLevel ( LogLevel::Level::Info ) <<
                      "Direct GLX rendering context obtained" << std::endl;
        }
        glXMakeCurrent ( mDisplay, mWindow, mGLXContext );
        if ( !LoadOpenGLAPI() )
        {
            std::cout << "Unable to Load OpenGL functions." << std::endl;
            return false;
        }

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

    void OpenGLRenderer::UnregisterRenderingWindow ( uintptr_t aWindowId )
    {
        if ( glIsBuffer ( mMatricesBuffer ) )
        {
            glDeleteBuffers ( 1, &mMatricesBuffer );
            mMatricesBuffer = 0;
        }
#ifdef WIN32
        if ( mWindowId && ( mWindowId == aWindowId  ) )
        {
            if ( mDeviceContext != nullptr )
            {
                wglMakeCurrent ( mDeviceContext, nullptr );
                ReleaseDC ( reinterpret_cast<HWND> ( mWindowId ), mDeviceContext );
                mDeviceContext = nullptr;
            }
            if ( mOpenGLContext )
            {
                wglDeleteContext ( mOpenGLContext );
                mOpenGLContext = nullptr;
            }
        }
#else
        if ( mWindow && ( mWindow == reinterpret_cast<Window> ( aWindowId ) ) )
        {
            if ( mGLXContext )
            {
                glXMakeCurrent ( mDisplay, mWindow, nullptr );
                glXDestroyContext ( mDisplay, mGLXContext );
                mGLXContext = nullptr;
            }
            if ( mDisplay )
            {
                XCloseDisplay ( mDisplay );
                mDisplay = nullptr;
            }
        }
#endif
    }

    void OpenGLRenderer::Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const
    {
        if ( aWidth > 0 && aHeight > 0 )
        {
            glViewport ( 0, 0, aWidth, aHeight );
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

    void OpenGLRenderer::Initialize()
    {
    }

    void OpenGLRenderer::Finalize()
    {
    }

    std::shared_ptr<Mesh> OpenGLRenderer::GetMesh ( const std::string & aFilename ) const
    {
        return Get<OpenGLMesh> ( aFilename );
    }

    std::shared_ptr<Program> OpenGLRenderer::GetProgram ( const std::string & aFilename ) const
    {
        return Get<OpenGLProgram> ( aFilename );
    }
}

AeonGames::Renderer * CreateRenderer()
{
    return new AeonGames::OpenGLRenderer;
}

void DestroyRenderer ( AeonGames::Renderer * aRenderer )
{
    delete reinterpret_cast<AeonGames::OpenGLRenderer *> ( aRenderer );
}
