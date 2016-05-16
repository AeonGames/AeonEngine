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
#include "OpenGLRenderer.h"
#include "aeongames/LogLevel.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
OpenGLRenderer::OpenGLRenderer() try :
        mGLXContext ( nullptr )
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

    bool OpenGLRenderer::InitializeRenderingWindow ( Display* aDisplay, Window aWindow )
    {
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
            XGetWindowAttributes ( aDisplay, aWindow, &x_window_attributes );

            int glx_fb_config_count;
            //GLXFBConfig *fbc = glXChooseFBConfig ( aDisplay, DefaultScreen ( aDisplay ),visual_attribs, &glx_fb_config_count );
            GLXFBConfig* glx_fb_config_list = glXGetFBConfigs ( aDisplay, DefaultScreen ( aDisplay ), &glx_fb_config_count );

            if ( !glx_fb_config_list )
            {
                return false;
            }
            // Pick the FB config/visual with the most samples per pixel
            int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
            for ( int i = 0; i < glx_fb_config_count; ++i )
            {
                XVisualInfo *vi = glXGetVisualFromFBConfig ( aDisplay, glx_fb_config_list[i] );
                if ( ( vi ) && ( x_window_attributes.visual == vi->visual ) )
                {
                    std::cout << *vi << std::endl;
                    int samp_buf, samples;
                    glXGetFBConfigAttrib ( aDisplay, glx_fb_config_list[i], GLX_SAMPLE_BUFFERS, &samp_buf );
                    glXGetFBConfigAttrib ( aDisplay, glx_fb_config_list[i], GLX_SAMPLES       , &samples  );
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
            mGLXContext = glXCreateContextAttribsARB ( aDisplay, bestFbc, 0, True, context_attribs );
            XSync ( aDisplay, False );
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
        if ( ! glXIsDirect ( aDisplay, mGLXContext ) )
        {
            std::cout << LogLevel ( LogLevel::Level::Info ) <<
                      "Indirect GLX rendering context obtained" << std::endl;
        }
        else
        {
            std::cout << LogLevel ( LogLevel::Level::Info ) <<
                      "Direct GLX rendering context obtained" << std::endl;
        }

        glXMakeCurrent ( aDisplay, aWindow, mGLXContext );
        return true;
    }

    void OpenGLRenderer::FinalizeRenderingWindow()
    {
    }

    void OpenGLRenderer::Initialize()
    {
        Display* display = XOpenDisplay ( 0 );
        int glx_fb_config_count = 0;
        //GLXFBConfig* glx_fb_config_list = glXChooseFBConfig(display,DefaultScreen(display),visual_attribs,&glx_fb_config_count);
        GLXFBConfig* glx_fb_config_list = glXGetFBConfigs ( display, DefaultScreen ( display ), &glx_fb_config_count );
        if ( !glx_fb_config_list )
        {
            throw std::runtime_error ( "No Frame Buffer configurations found." );
        }
        XVisualInfo *x_visual_info = glXGetVisualFromFBConfig ( display, glx_fb_config_list[0] );
        std::cout << *x_visual_info << std::endl;
        Window window = XCreateSimpleWindow ( display, RootWindow ( display, x_visual_info->screen ), 0, 0, 10, 10, 0,
                                              0, BlackPixel ( display, 0 ) );
        InitializeRenderingWindow ( display, window );
        if ( !LoadOpenGLAPI() )
        {
            throw std::runtime_error ( "Could not load OpenGL API." );
        }
    }

    void OpenGLRenderer::Finalize()
    {
    }
}
