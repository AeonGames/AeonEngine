/*
Copyright (C) 2016-2020 Rodrigo Jose Hernandez Cordoba

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
#include "math/3DMath.h"
#include "OpenGLFunctions.h"
#include "OpenGLX11Renderer.h"
#include "aeongames/LogLevel.h"

namespace AeonGames
{

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

    static int context_attribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 5,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    static GLXFBConfig GetGLXConfig ( Display* display )
    {
        int frame_buffer_config_count{};
        GLXFBConfig *frame_buffer_configs =
            glXChooseFBConfig ( display,
                                DefaultScreen ( display ),
                                visual_attribs, &frame_buffer_config_count );
        if ( !frame_buffer_configs )
        {
            throw std::runtime_error ( "Failed to retrieve a framebuffer config" );
        }

        std::sort ( frame_buffer_configs, frame_buffer_configs + frame_buffer_config_count,
                    [display] ( const GLXFBConfig & a, const GLXFBConfig & b )->bool
        {
            int a_sample_buffers{};
            int a_samples{};
            int b_sample_buffers{};
            int b_samples{};
            glXGetFBConfigAttrib ( display, a, GLX_SAMPLE_BUFFERS, &a_sample_buffers );
            glXGetFBConfigAttrib ( display, a, GLX_SAMPLES, &a_samples  );
            glXGetFBConfigAttrib ( display, b, GLX_SAMPLE_BUFFERS, &b_sample_buffers );
            glXGetFBConfigAttrib ( display, b, GLX_SAMPLES, &b_samples  );
            return a_sample_buffers >= b_sample_buffers && a_samples > b_samples;
        } );
        GLXFBConfig result = frame_buffer_configs[ 0 ];
        XFree ( frame_buffer_configs );
        return result;
    }

    void OpenGLX11Renderer::Initialize()
    {
        if ( !mDisplay )
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
        mGLXFBConfig = GetGLXConfig ( mDisplay );
        if ( nullptr == ( mGLXContext = glXCreateContextAttribsARB ( mDisplay, mGLXFBConfig, nullptr,
                                        True, context_attribs ) ) )
        {
            throw std::runtime_error ( "glXCreateContextAttribsARB Failed." );
        }

        // Verifying that context is a direct context
        if ( ! glXIsDirect (  mDisplay,  static_cast<GLXContext> ( mGLXContext ) ) )
        {
            std::cout << LogLevel ( LogLevel::Info ) <<
                      "Indirect GLX rendering context obtained" << std::endl;
        }
        else
        {
            std::cout << LogLevel ( LogLevel::Info ) <<
                      "Direct GLX rendering context obtained" << std::endl;
        }

        XVisualInfo* xvi = glXGetVisualFromFBConfig ( mDisplay, mGLXFBConfig );

        std::cout << *xvi << std::endl;

        Colormap cmap = XCreateColormap ( mDisplay, DefaultRootWindow ( mDisplay ), xvi->visual, AllocNone );
        XSetWindowAttributes swa
        {
            .colormap = cmap,
        };

        mWindow = XCreateWindow ( mDisplay, DefaultRootWindow ( mDisplay ),
                                  0, 0,
                                  32, 32,
                                  0,
                                  xvi->depth,
                                  InputOutput,
                                  xvi->visual,
                                  CWColormap, &swa );
        XFree ( xvi );
        if ( !MakeCurrent() )
        {
            throw std::runtime_error ( "glXMakeCurrent failed." );
        }
    }

    void OpenGLX11Renderer::Finalize()
    {
        if ( mDisplay )
        {
            glXMakeCurrent ( mDisplay, None, nullptr );
            if ( mGLXContext != nullptr )
            {
                glXDestroyContext ( mDisplay, mGLXContext );
                mGLXContext = nullptr;
            }
            if ( mWindow != None )
            {
                XDestroyWindow ( mDisplay, mWindow );
                mWindow = None;
            }
            XCloseDisplay ( mDisplay );
        }
    }

    OpenGLX11Renderer::OpenGLX11Renderer()
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

    OpenGLX11Renderer::~OpenGLX11Renderer()
    {
        FinalizeOverlay();
        Finalize();
    }

    void* OpenGLX11Renderer::GetContext() const
    {
        return mGLXContext;
    }

    bool OpenGLX11Renderer::MakeCurrent () const
    {
        return glXMakeCurrent ( mDisplay, mWindow, mGLXContext );
    }
    GLXFBConfig OpenGLX11Renderer::GetGLXFBConfig() const
    {
        return mGLXFBConfig;
    }
}
#endif
