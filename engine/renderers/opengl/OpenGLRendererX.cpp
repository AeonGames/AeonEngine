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
#include "OpenGLImage.h"
#include "aeongames/LogLevel.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Mesh.h"
#include "aeongames/Model.h"

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

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = nullptr;
    const int context_attribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 5,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };
    const int visual_attribs[] =
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
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };

    void OpenGLRenderer::Initialize()
    {
        // Retrieve Display
        mWindowId = XOpenDisplay ( nullptr );
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

        // Pick the FB config/visual with the most samples per pixel
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

        int i;
        for ( i = 0; i < frame_buffer_config_count; ++i )
        {
            XVisualInfo *vi = glXGetVisualFromFBConfig ( static_cast<Display*> ( mWindowId ), frame_buffer_configs[i] );
            if ( vi )
            {
                std::cout << "Visual: " << *vi << std::endl;
                int samp_buf, samples;
                glXGetFBConfigAttrib ( static_cast<Display*> ( mWindowId ), frame_buffer_configs[i], GLX_SAMPLE_BUFFERS, &samp_buf );
                glXGetFBConfigAttrib ( static_cast<Display*> ( mWindowId ), frame_buffer_configs[i], GLX_SAMPLES, &samples  );

                if ( best_fbc < 0 || ( samp_buf && samples > best_num_samp ) )
                {
                    best_fbc = i, best_num_samp = samples;
                    std::cout << "Best Visual " << *vi << std::endl;
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
        if ( ! ( mOpenGLContext = glXCreateContextAttribsARB ( static_cast<Display*> ( mWindowId ), bestFbc, 0,
                                  True, context_attribs ) ) )
        {
            throw std::runtime_error ( "glXCreateContextAttribsARB Failed." );
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

        if ( !LoadOpenGLAPI() )
        {
            throw std::runtime_error ( "Unable to Load OpenGL functions." );
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
}

