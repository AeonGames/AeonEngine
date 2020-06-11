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
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
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

    static GLXFBConfig GetGLXConfig ( Display* display )
    {
        // Get best frame buffer configuration.
        int frame_buffer_config_count{};
        GLXFBConfig *frame_buffer_configs =
            glXChooseFBConfig ( display,
                                DefaultScreen ( display  ),
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
            glXGetFBConfigAttrib ( display, frame_buffer_configs[i], GLX_SAMPLE_BUFFERS, &samp_buf );
            glXGetFBConfigAttrib ( display, frame_buffer_configs[i], GLX_SAMPLES, &samples  );

            std::cout << LogLevel::Info << "Matching fbconfig " << i << " SAMPLE_BUFFERS = " << samp_buf <<
                      " SAMPLES = " << samples << std::endl;

            if ( best_fbc < 0 || ( samp_buf && samples > best_num_samp ) )
            {
                best_fbc = i, best_num_samp = samples;
            }
        }
        GLXFBConfig bestFbc = frame_buffer_configs[ best_fbc ];
        XFree ( frame_buffer_configs );
        return bestFbc;
    }

    void OpenGLX11Renderer::Initialize()
    {
        // Retrieve Display
        mDisplay = XOpenDisplay ( nullptr );
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

        if ( ! ( mOpenGLContext = glXCreateContextAttribsARB ( mDisplay, mGLXFBConfig, nullptr,
                                  True, context_attribs ) ) )
        {
            context_attribs[1] = 3;
            context_attribs[3] = 1;
            if ( ! ( mOpenGLContext = glXCreateContextAttribsARB ( mDisplay, mGLXFBConfig, nullptr,
                                      True, context_attribs ) ) )
            {
                throw std::runtime_error ( "glXCreateContextAttribsARB Failed." );
            }
        }

        // Verifying that context is a direct context
        if ( ! glXIsDirect (  mDisplay,  static_cast<GLXContext> ( mOpenGLContext ) ) )
        {
            std::cout << LogLevel ( LogLevel::Info ) <<
                      "Indirect GLX rendering context obtained" << std::endl;
        }
        else
        {
            std::cout << LogLevel ( LogLevel::Info ) <<
                      "Direct GLX rendering context obtained" << std::endl;
        }
        mPixmap = XCreatePixmap ( mDisplay, DefaultRootWindow ( mDisplay ), 16, 16, 32 );
        if ( !mPixmap )
        {
            throw std::runtime_error ( "Failed creating pixmap." );
        }
        mGLXPixmap = glXCreatePixmap ( mDisplay, mGLXFBConfig, mPixmap, nullptr );
        if ( !mGLXPixmap )
        {
            throw std::runtime_error ( "Failed creating glx pixmap." );
        }
        if ( !glXMakeCurrent ( mDisplay, mGLXPixmap, mOpenGLContext ) )
        {
            throw std::runtime_error ( "glXMakeCurrent failed." );
        }
    }

    void OpenGLX11Renderer::Finalize()
    {
        if ( mDisplay )
        {
            glXMakeCurrent ( mDisplay, None, nullptr );
            if ( mOpenGLContext != nullptr )
            {
                glXDestroyContext ( mDisplay, mOpenGLContext );
                mOpenGLContext = nullptr;
            }
            if ( mGLXPixmap != None )
            {
                glXDestroyGLXPixmap ( mDisplay, mGLXPixmap );
                mGLXPixmap = None;
            }
            if ( mPixmap != None )
            {
                XFreePixmap ( mDisplay, mPixmap );
                mPixmap = None;
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

    bool OpenGLX11Renderer::MakeCurrent ( void* aDrawable ) const
    {
        return glXMakeCurrent ( mDisplay, ( aDrawable != nullptr ) ? reinterpret_cast<GLXDrawable> ( aDrawable ) : mGLXPixmap, mOpenGLContext );
    }
}
#endif
