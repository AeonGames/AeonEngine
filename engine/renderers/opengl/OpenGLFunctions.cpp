/*
Copyright (C) 2016-2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLFunctions.hpp"
#include "aeongames/LogLevel.hpp"
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

#ifdef WIN32
#include <GL/wglext.h>
#define GLGETPROCADDRESS(glFunctionType,glFunction) \
    if(glFunction==nullptr) { \
        glFunction = (glFunctionType)wglGetProcAddress(#glFunction); \
        if (glFunction == nullptr) \
        { \
            std::ostringstream stream; \
            stream << "OpenGL: Unable to load " #glFunction " function."; \
            throw std::runtime_error(stream.str().c_str()); \
        } \
    }
#elif defined(ANDROID)
#define GLGETPROCADDRESS(glFunctionType,glFunction) \
    if(glFunction==nullptr) { \
    glFunction = ( glFunctionType ) eglGetProcAddress ( (const char*) #glFunction "OES" ); \
        if (glFunction == nullptr) \
        { \
            std::ostringstream stream; \
            stream << "OpenGL: Unable to load " #glFunction " function."; \
            throw std::runtime_error(stream.str().c_str()); \
        } \
    }
#else
#include <GL/gl.h>
#include <GL/glx.h>
#define GLGETPROCADDRESS(glFunctionType,glFunction) \
    if(glFunction==nullptr) { \
    glFunction = ( glFunctionType ) glXGetProcAddress ( (const GLubyte*) #glFunction ); \
        if (glFunction == nullptr) \
        { \
            std::ostringstream stream; \
            stream << "OpenGL: Unable to load " #glFunction " function."; \
            throw std::runtime_error(stream.str().c_str()); \
        } \
    }
#endif

namespace AeonGames
{
#include "glDefinitions.h"
    bool LoadOpenGLAPI()
    {
#include "glAssignments.h"
        // Provide some information regarding OpenGL Drivers.
        std::cout << LogLevel::Info << "OpenGL Version " << glGetString ( GL_VERSION ) << std::endl;
        std::cout << LogLevel::Info << "GLSL Version " << glGetString ( GL_SHADING_LANGUAGE_VERSION ) << std::endl;
        std::cout << LogLevel::Info << "OpenGL Vendor " << glGetString ( GL_VENDOR ) << std::endl;
        GLint texSize = 0;
        glGetIntegerv ( GL_MAX_TEXTURE_SIZE, &texSize );
        std::cout << LogLevel::Info << "Maximum Texture Size: " << texSize << std::endl;
        GLint glInteger;
#ifdef ANDROID
        glGetIntegerv ( GL_MAX_VERTEX_UNIFORM_VECTORS, &glInteger );
        std::cout << LogLevel::Info << "GLSLES Max Vertex Uniform Vectors " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_FRAGMENT_UNIFORM_VECTORS, &glInteger );
        std::cout << LogLevel::Info << "GLSLES Max Fragment Uniform Vectors " << glInteger << std::endl;
        std::cout << glGetString ( GL_EXTENSIONS );
#else
        glGetIntegerv ( GL_MAX_VERTEX_UNIFORM_COMPONENTS, &glInteger );
        std::cout << LogLevel::Info << "GLSL Max Vertex Uniform Components " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &glInteger );
        std::cout << LogLevel::Info << "GLSL Max Fragment Uniform Components " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_VERTEX_ATTRIBS, &glInteger );
        std::cout << LogLevel::Info << "GLSL Max Vertex Attributes " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_UNIFORM_BLOCK_SIZE, &glInteger );
        std::cout << LogLevel::Info << "GLSL Max Uniform Block Size " << glInteger << std::endl;
        GLint extension_count;
        glGetIntegerv ( GL_NUM_EXTENSIONS, &extension_count );
        for ( GLint i = 0; i < extension_count; ++i )
        {
            std::cout << LogLevel::Info << glGetStringi ( GL_EXTENSIONS, i ) << std::endl;
        }
#endif
        return true;
    }
}
