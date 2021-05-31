/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLX11Window.h"
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

    static int context_attribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 5,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    Display* OpenGLX11Renderer::GetDisplay() const
    {
        return mDisplay;
    }

    void OpenGLX11Renderer::Initialize()
    {
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

        mGLXFBConfig = OpenGLX11Window::GetGLXConfig ( mDisplay );

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

        mColorMap = XCreateColormap ( mDisplay, DefaultRootWindow ( mDisplay ), xvi->visual, AllocNone );
        XSetWindowAttributes swa
        {
            .colormap = mColorMap,
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
            if ( mColorMap != 0 )
            {
                XFreeColormap ( mDisplay, mColorMap );
                mColorMap = 0;
            }
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
            mDisplay = nullptr;
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

    Colormap OpenGLX11Renderer::GetColorMap() const
    {
        return mColorMap;
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
