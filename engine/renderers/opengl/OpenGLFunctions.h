/*
Copyright (C) 2016,2018,2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLFUNCTIONS_H
#define AEONGAMES_OPENGLFUNCTIONS_H

#ifdef ANDROID
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#elif defined (_WIN32)
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>
#include <GL/wglext.h>
#else
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glcorearb.h>
/// @todo Decouple Protobuf from Renderers and remove Status undefinitions
#ifdef Status
#undef Status
#endif
#endif // ANDROID
#include <sstream>
#include <iostream>
namespace AeonGames
{
#include "glDeclarations.h"
    bool LoadOpenGLAPI();
#define OPENGL_CHECK_ERROR_THROW \
 { \
     if (int glError = glGetError()) \
     { \
         const char* error_string = (glError == GL_INVALID_ENUM) ? "GL_INVALID_ENUM" : \
             (glError == GL_INVALID_VALUE) ? "GL_INVALID_VALUE" : \
             (glError == GL_INVALID_OPERATION) ? "GL_INVALID_OPERATION" : \
             (glError == GL_STACK_OVERFLOW) ? "GL_STACK_OVERFLOW" : \
             (glError == GL_STACK_UNDERFLOW) ? "GL_STACK_UNDERFLOW" : \
             (glError == GL_OUT_OF_MEMORY) ? "GL_OUT_OF_MEMORY" : "Unknown Error Code"; \
         std::ostringstream stream; \
         stream << "OpenGL Error " << error_string << " (Code " << glError << " ) " << __FILE__ << ":" << __LINE__; \
         std::cout << stream.str() << std::endl; \
         throw std::runtime_error(stream.str().c_str()); \
     } \
 }
#define OPENGL_CHECK_ERROR_NO_THROW \
 { \
     if (int glError = glGetError()) \
     { \
         const char* error_string = (glError == GL_INVALID_ENUM) ? "GL_INVALID_ENUM" : \
             (glError == GL_INVALID_VALUE) ? "GL_INVALID_VALUE" : \
             (glError == GL_INVALID_OPERATION) ? "GL_INVALID_OPERATION" : \
             (glError == GL_STACK_OVERFLOW) ? "GL_STACK_OVERFLOW" : \
             (glError == GL_STACK_UNDERFLOW) ? "GL_STACK_UNDERFLOW" : \
             (glError == GL_OUT_OF_MEMORY) ? "GL_OUT_OF_MEMORY" : "Unknown Error Code"; \
         std::ostringstream stream; \
         stream << "OpenGL Error " << error_string << " (Code " << glError << " ) " << __FILE__ << ":" << __LINE__; \
         std::cout << stream.str() << std::endl; \
     } \
 }

}
#endif
