/*
Copyright (C) 2016-2019 Rodrigo Jose Hernandez Cordoba

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
#ifdef __unix__
#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "OpenGLFunctions.h"
#include "OpenGLRenderer.h"
#include "OpenGLMesh.h"
#include "OpenGLPipeline.h"
#include "OpenGLImage.h"
#include "OpenGLWindow.h"
#include "aeongames/LogLevel.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/Model.h"

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

namespace AeonGames
{
    int context_attribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 5,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    static const int visual_attribs[] =
    {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };

    void OpenGLRenderer::Initialize()
    {
        // Retrieve Display
        mWindowId = XOpenDisplay ( std::getenv ( "DISPLAY" ) );
        if ( !mWindowId )
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
        GLXFBConfig *frame_buffer_configs =
            glXChooseFBConfig ( static_cast<Display*> ( mWindowId ),
                                DefaultScreen ( static_cast<Display*> ( mWindowId )  ),
                                visual_attribs, &frame_buffer_config_count );
        if ( !frame_buffer_configs )
        {
            throw std::runtime_error ( "Failed to retrieve a framebuffer config" );
        }

        // Pick the FB config with the most samples per pixel
        int best_fbc = -1;
        int best_num_samp = -1;
        for ( int i = 0; i < frame_buffer_config_count; ++i )
        {
            int samp_buf, samples;
            glXGetFBConfigAttrib ( static_cast<Display*> ( mWindowId ), frame_buffer_configs[i], GLX_SAMPLE_BUFFERS, &samp_buf );
            glXGetFBConfigAttrib ( static_cast<Display*> ( mWindowId ), frame_buffer_configs[i], GLX_SAMPLES, &samples  );

            std::cout << LogLevel::Info << "Matching fbconfig " << i << " SAMPLE_BUFFERS = " << samp_buf <<
                      " SAMPLES = " << samples << std::endl;

            if ( best_fbc < 0 || ( samp_buf && samples > best_num_samp ) )
            {
                best_fbc = i, best_num_samp = samples;
            }
        }

        GLXFBConfig bestFbc = frame_buffer_configs[ best_fbc ];
        XFree ( frame_buffer_configs );
        if ( ! ( mOpenGLContext = glXCreateContextAttribsARB ( static_cast<Display*> ( mWindowId ), bestFbc, 0,
                                  True, context_attribs ) ) )
        {
            context_attribs[1] = 1;
            context_attribs[3] = 0;
            if ( ! ( mOpenGLContext = glXCreateContextAttribsARB ( static_cast<Display*> ( mWindowId ), bestFbc, 0,
                                      True, context_attribs ) ) )
            {
                throw std::runtime_error ( "glXCreateContextAttribsARB Failed." );
            }
        }

        // Verifying that context is a direct context
        if ( ! glXIsDirect (  static_cast<Display*> ( mWindowId ),  static_cast<GLXContext> ( mOpenGLContext ) ) )
        {
            std::cout << LogLevel ( LogLevel::Info ) <<
                      "Indirect GLX rendering context obtained" << std::endl;
        }
        else
        {
            std::cout << LogLevel ( LogLevel::Info ) <<
                      "Direct GLX rendering context obtained" << std::endl;
        }

        if ( !glXMakeCurrent ( static_cast<Display*> ( mWindowId ), 0, static_cast<GLXContext> ( mOpenGLContext ) ) )
        {

            std::cout << LogLevel ( LogLevel::Warning ) <<
                      "glxMakeCurrent Failed." << std::endl;
        }
    }

    void OpenGLRenderer::Finalize()
    {
        if ( mWindowId )
        {
            glXMakeCurrent ( static_cast<Display*> ( mWindowId ), 0, 0 );
            if ( mOpenGLContext != nullptr )
            {
                glXDestroyContext ( static_cast<Display*> ( mWindowId ), static_cast<GLXContext> ( mOpenGLContext ) );
                mOpenGLContext = nullptr;
            }
            XCloseDisplay ( static_cast<Display*> ( mWindowId ) );
            mWindowId = nullptr;
        }
    }

    void OpenGLWindow::OnResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight )
        {
            glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                             reinterpret_cast<::Window> ( mWindowId ),
                             static_cast<GLXContext> ( mDeviceContext ) );
            OPENGL_CHECK_ERROR_NO_THROW;
            glViewport ( aX, aY, aWidth, aHeight );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLWindow::BeginRender() const
    {
        glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ),
                         static_cast<GLXContext> ( mDeviceContext ) );
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

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
        glXSwapBuffers ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                         reinterpret_cast<::Window> ( mWindowId ) );
    }

    void OpenGLWindow::InitializePlatform()
    {
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
    }

    void OpenGLWindow::FinalizePlatform()
    {
        glXMakeCurrent (  static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ),
                          reinterpret_cast<::Window> ( mWindowId ),
                          static_cast<GLXContext> ( mDeviceContext ) );
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( !mOwnsWindowId )
        {
            glXMakeCurrent ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ), 0, 0 );
            if ( mDeviceContext != nullptr )
            {
                glXDestroyContext ( static_cast<Display*> ( mOpenGLRenderer.GetWindowId() ), static_cast<GLXContext> ( mDeviceContext ) );
                mDeviceContext = nullptr;
            }
        }
    }

}
#endif
