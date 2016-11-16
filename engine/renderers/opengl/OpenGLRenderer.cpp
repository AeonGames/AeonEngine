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
#include <algorithm>
#include "math/3DMath.h"
#include "OpenGLFunctions.h"
#include "OpenGLRenderer.h"
#include "OpenGLMesh.h"
#include "OpenGLProgram.h"
#include "OpenGLTexture.h"
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
        for ( auto& i : mWindowRegistry )
        {
#ifdef _WIN32
            if ( i.mDeviceContext != nullptr )
            {
                wglMakeCurrent ( i.mDeviceContext, i.mOpenGLContext );
                glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            }
#else
            if ( i.mOpenGLContext != nullptr )
            {
                glXMakeCurrent ( i.mDisplay, reinterpret_cast<Window> ( i.mWindowId ), i.mOpenGLContext );
                glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            }
#endif
        }
    }

    void OpenGLRenderer::Render ( const std::shared_ptr<Mesh>& aMesh, const std::shared_ptr<Program>& aProgram ) const
    {
        OpenGLProgram* program = reinterpret_cast<OpenGLProgram*> ( aProgram.get() );
        program->Use();
        glBindBufferBase ( GL_UNIFORM_BUFFER, 0, mMatricesBuffer );
        OPENGL_CHECK_ERROR_NO_THROW;
        //aMesh->Render(); /// @note commenting pending re-write.
    }

    void OpenGLRenderer::EndRender() const
    {
        for ( auto& i : mWindowRegistry )
        {
#if _WIN32
            if ( i.mDeviceContext != nullptr )
            {
                wglMakeCurrent ( i.mDeviceContext, i.mOpenGLContext );
                SwapBuffers ( i.mDeviceContext );
            }
#else
            if ( i.mOpenGLContext != nullptr )
            {
                glXMakeCurrent ( i.mDisplay, reinterpret_cast<Window> ( i.mWindowId ), i.mOpenGLContext );
                glXSwapBuffers ( i.mDisplay, reinterpret_cast<Window> ( i.mWindowId ) );
            }
#endif
        }
    }

    bool OpenGLRenderer::AddRenderingWindow ( uintptr_t aWindowId )
    {
        /**@todo Should a window wrapper be created? */
        /**@todo Should each window own a renderer instead of the renderer managing the windows? */
#ifdef WIN32
        PIXELFORMATDESCRIPTOR pfd {};
        PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = nullptr;
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;
        mWindowRegistry.emplace_back();
        mWindowRegistry.back().mWindowId = aWindowId;
        mWindowRegistry.back().mDeviceContext = ( HDC ) GetDC ( reinterpret_cast<HWND> ( mWindowRegistry.back().mWindowId ) );
        int pf = ChoosePixelFormat ( mWindowRegistry.back().mDeviceContext, &pfd );
        SetPixelFormat ( mWindowRegistry.back().mDeviceContext, pf, &pfd );
        mWindowRegistry.back().mOpenGLContext = wglCreateContext ( mWindowRegistry.back().mDeviceContext );
        wglMakeCurrent ( mWindowRegistry.back().mDeviceContext, mWindowRegistry.back().mOpenGLContext );

        //---OpenGL 4.0 Context---//
        wglGetExtensionsStringARB = ( PFNWGLGETEXTENSIONSSTRINGARBPROC ) wglGetProcAddress ( "wglGetExtensionsStringARB" );
        if ( wglGetExtensionsStringARB != nullptr )
        {
            if ( strstr ( wglGetExtensionsStringARB ( mWindowRegistry.back().mDeviceContext ), "WGL_ARB_create_context" ) != nullptr )
            {
                const int ctxAttribs[] =
                {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
                    WGL_CONTEXT_PROFILE_MASK_ARB,
                    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    0
                };

                wglCreateContextAttribsARB = ( PFNWGLCREATECONTEXTATTRIBSARBPROC ) wglGetProcAddress ( "wglCreateContextAttribsARB" );
                wglMakeCurrent ( mWindowRegistry.back().mDeviceContext, nullptr );
                wglDeleteContext ( mWindowRegistry.back().mOpenGLContext );
                mWindowRegistry.back().mOpenGLContext = wglCreateContextAttribsARB ( mWindowRegistry.back().mDeviceContext,
                                                        ( mWindowRegistry.size() > 1 ) ? mWindowRegistry[0].mOpenGLContext : nullptr, ctxAttribs );
                if ( !wglMakeCurrent ( mWindowRegistry.back().mDeviceContext, mWindowRegistry.back().mOpenGLContext ) )
                {
                    std::cout << "wglMakeCurrent Failed. Error: " << GetLastError() << std::endl;
                    return false;
                }
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
                GetClientRect ( reinterpret_cast<HWND> ( mWindowRegistry.back().mWindowId ), &rect );
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
        mWindowRegistry.emplace_back();
        mWindowRegistry.back().mDisplay = XOpenDisplay ( nullptr );
        if ( !mWindowRegistry.back().mDisplay )
        {
            return false;
        }
        mWindowRegistry.back().mWindowId = aWindowId;
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
            mWindowRegistry.back().mOpenGLContext = glXCreateContextAttribsARB (  mWindowRegistry.back().mDisplay, bestFbc,
                                                    ( mWindowRegistry.size() > 1 ) ? mWindowRegistry[0].mOpenGLContext : nullptr,
                                                    True, context_attribs );
            XSync (  mWindowRegistry.back().mDisplay, False );
            if (  mWindowRegistry.back().mOpenGLContext != nullptr )
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
        if ( ! glXIsDirect (  mWindowRegistry.back().mDisplay,  mWindowRegistry.back().mOpenGLContext ) )
        {
            std::cout << LogLevel ( LogLevel::Level::Info ) <<
                      "Indirect GLX rendering context obtained" << std::endl;
        }
        else
        {
            std::cout << LogLevel ( LogLevel::Level::Info ) <<
                      "Direct GLX rendering context obtained" << std::endl;
        }
        glXMakeCurrent (  mWindowRegistry.back().mDisplay, reinterpret_cast<Window> (  mWindowRegistry.back().mWindowId ),  mWindowRegistry.back().mOpenGLContext );
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

    void OpenGLRenderer::RemoveRenderingWindow ( uintptr_t aWindowId )
    {
        if ( glIsBuffer ( mMatricesBuffer ) )
        {
            glDeleteBuffers ( 1, &mMatricesBuffer );
            mMatricesBuffer = 0;
        }
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [&aWindowId] ( const WindowData & aWindowData )
        {
            return aWindowData.mWindowId == aWindowId;
        } );
        if ( i != mWindowRegistry.end() )
        {
#ifdef WIN32
            if ( i->mDeviceContext != nullptr )
            {
                wglMakeCurrent ( i->mDeviceContext, nullptr );
                ReleaseDC ( reinterpret_cast<HWND> ( i->mWindowId ), i->mDeviceContext );
                i->mDeviceContext = nullptr;
            }
            if ( i->mOpenGLContext )
            {
                wglDeleteContext ( i->mOpenGLContext );
                i->mOpenGLContext = nullptr;
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
            mWindowRegistry.erase ( std::remove_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                    [&aWindowId] ( const WindowData & aWindowData )
            {
                return aWindowData.mWindowId == aWindowId;
            } )
            , mWindowRegistry.end() );
        }
    }

    void OpenGLRenderer::Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const
    {
        auto i = std::find_if ( mWindowRegistry.begin(), mWindowRegistry.end(),
                                [&aWindowId] ( const WindowData & aWindowData )
        {
            return aWindowId == aWindowData.mWindowId;
        } );
        if ( i != mWindowRegistry.end() )
        {
            if ( aWidth > 0 && aHeight > 0 )
            {
#ifdef WIN32
                wglMakeCurrent ( i->mDeviceContext, i->mOpenGLContext );
#else
                glXMakeCurrent ( i->mDisplay, reinterpret_cast<Window> ( i->mWindowId ), nullptr );
#endif
                glViewport ( 0, 0, aWidth, aHeight );
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
        //return Get<OpenGLMesh> ( aFilename ); /// @note commented, pending rewrite
        return nullptr;
    }

    std::shared_ptr<Program> OpenGLRenderer::GetProgram ( const std::string & aFilename ) const
    {
        //return Get<OpenGLProgram> ( aFilename ); /// @note commented, pending rewrite
        return nullptr;
    }

    std::shared_ptr<Texture> OpenGLRenderer::GetTexture ( const std::string & aFilename ) const
    {
        //return Get<OpenGLTexture> ( aFilename ); /// @note commented, pending rewrite
        return nullptr;
    }
}
