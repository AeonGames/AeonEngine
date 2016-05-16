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
#include "OpenGLFunctions.h"
#include "aeongames/LogLevel.h"
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

#ifdef WIN32
#include "wglext.h"
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
#include "glxext.h"
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
    extern "C"
    {
        extern bool CreateOpenGLContext();
        extern void DestroyOpenGLContext();
#ifdef ANDROID
        // GL_OES_vertex_array_object
        PFNGLGENVERTEXARRAYSOESPROC            glGenVertexArrays = NULL;
        PFNGLBINDVERTEXARRAYOESPROC            glBindVertexArray = NULL;
        PFNGLISVERTEXARRAYOESPROC              glIsVertexArray = NULL;
        PFNGLDELETEVERTEXARRAYSOESPROC         glDeleteVertexArrays = NULL;
#else
        PFNGLGETSTRINGIPROC                    glGetStringi = NULL;
        PFNGLATTACHSHADERPROC                  glAttachShader = NULL;
        PFNGLCOMPILESHADERPROC                 glCompileShader = NULL;
        PFNGLCREATEPROGRAMPROC                 glCreateProgram = NULL;
        PFNGLISPROGRAMPROC                     glIsProgram = NULL;
        PFNGLCREATESHADERPROC                  glCreateShader = NULL;
        PFNGLISSHADERPROC                      glIsShader = NULL;
        PFNGLDELETEPROGRAMPROC                 glDeleteProgram = NULL;
        PFNGLDELETESHADERPROC                  glDeleteShader = NULL;

        PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f = NULL;
        PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s = NULL;
        PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d = NULL;
        PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i = NULL;
        PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui = NULL;
        PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f = NULL;
        PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s = NULL;
        PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d = NULL;
        PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i = NULL;
        PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui = NULL;
        PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f = NULL;
        PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s = NULL;
        PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d = NULL;
        PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i = NULL;
        PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui = NULL;
        PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f = NULL;
        PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s = NULL;
        PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d = NULL;
        PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub = NULL;
        PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i = NULL;
        PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui = NULL;
#if 0
        PFNGLVERTEXATTRIBL1DPROC glVertexAttribL1d = NULL;
        PFNGLVERTEXATTRIBL2DPROC glVertexAttribL2d = NULL;
        PFNGLVERTEXATTRIBL3DPROC glVertexAttribL3d = NULL;
        PFNGLVERTEXATTRIBL4DPROC glVertexAttribL4d = NULL;
#endif

        PFNGLGETATTRIBLOCATIONPROC             glGetAttribLocation = NULL;
        PFNGLBINDATTRIBLOCATIONPROC            glBindAttribLocation = NULL;
        PFNGLGETPROGRAMIVPROC                  glGetProgramiv = NULL;
        PFNGLGETPROGRAMINFOLOGPROC             glGetProgramInfoLog = NULL;
        PFNGLGETSHADERIVPROC                   glGetShaderiv = NULL;
        PFNGLGETSHADERINFOLOGPROC              glGetShaderInfoLog = NULL;
        PFNGLGETUNIFORMLOCATIONPROC            glGetUniformLocation = NULL;
        PFNGLGETACTIVEUNIFORMPROC              glGetActiveUniform = NULL;
        PFNGLGETACTIVEUNIFORMNAMEPROC          glGetActiveUniformName = NULL;
        PFNGLLINKPROGRAMPROC                   glLinkProgram = NULL;
        PFNGLSHADERSOURCEPROC                  glShaderSource = NULL;
        PFNGLUSEPROGRAMPROC                    glUseProgram = NULL;
        PFNGLUNIFORM1IPROC                     glUniform1i = NULL;
        PFNGLUNIFORMMATRIX2FVPROC              glUniformMatrix2fv = NULL;
        PFNGLUNIFORMMATRIX3FVPROC              glUniformMatrix3fv = NULL;
        PFNGLUNIFORMMATRIX4FVPROC              glUniformMatrix4fv = NULL;
        PFNGLGENBUFFERSPROC                    glGenBuffers = NULL;
        PFNGLBINDBUFFERPROC                    glBindBuffer = NULL;
        PFNGLBUFFERDATAPROC                    glBufferData = NULL;
        PFNGLGETBUFFERSUBDATAPROC              glGetBufferSubData = NULL;
        PFNGLDELETEBUFFERSPROC                 glDeleteBuffers = NULL;
        PFNGLGETACTIVEATTRIBPROC               glGetActiveAttrib = NULL;
#ifndef DISABLE_VAO
        // GL_ARB_vertex_array_object
        PFNGLGENVERTEXARRAYSPROC               glGenVertexArrays = NULL;
        PFNGLBINDVERTEXARRAYPROC               glBindVertexArray = NULL;
        PFNGLDELETEVERTEXARRAYSPROC            glDeleteVertexArrays = NULL;
        PFNGLISVERTEXARRAYPROC                 glIsVertexArray = NULL;
#endif
        PFNGLUNIFORM1FVPROC                    glUniform1fv = NULL;
        PFNGLUNIFORM2FVPROC                    glUniform2fv = NULL;
        PFNGLUNIFORM3FVPROC                    glUniform3fv = NULL;
        PFNGLUNIFORM3FPROC                     glUniform3f = NULL;
        PFNGLUNIFORM4FVPROC                    glUniform4fv = NULL;
        PFNGLUNIFORM1IVPROC                    glUniform1iv = NULL;
        PFNGLUNIFORM2IVPROC                    glUniform2iv = NULL;
        PFNGLUNIFORM3IVPROC                    glUniform3iv = NULL;
        PFNGLUNIFORM4IVPROC                    glUniform4iv = NULL;
        PFNGLUNIFORM1UIVPROC                   glUniform1uiv = NULL;
        PFNGLUNIFORM2UIVPROC                   glUniform2uiv = NULL;
        PFNGLUNIFORM3UIVPROC                   glUniform3uiv = NULL;
        PFNGLUNIFORM4UIVPROC                   glUniform4uiv = NULL;
        PFNGLGETUNIFORMINDICESPROC             glGetUniformIndices = NULL;
        PFNGLCOMPRESSEDTEXIMAGE2DPROC          glCompressedTexImage2D = NULL;
        PFNGLDETACHSHADERPROC                  glDetachShader = NULL;
        PFNGLISBUFFERPROC                      glIsBuffer = NULL;
        PFNGLACTIVETEXTUREPROC                 glActiveTexture = NULL;
        PFNGLGENRENDERBUFFERSPROC              glGenRenderbuffers = NULL;
        PFNGLBINDRENDERBUFFERPROC              glBindRenderbuffer = NULL;
        PFNGLRENDERBUFFERSTORAGEPROC           glRenderbufferStorage = NULL;
        PFNGLFRAMEBUFFERRENDERBUFFERPROC       glFramebufferRenderbuffer = NULL;
        PFNGLDELETERENDERBUFFERSPROC           glDeleteRenderbuffers = NULL;
        PFNGLGENFRAMEBUFFERSPROC               glGenFramebuffers = NULL;
        PFNGLBINDFRAMEBUFFERPROC               glBindFramebuffer = NULL;
        PFNGLISRENDERBUFFERPROC                glIsRenderbuffer = NULL;
        PFNGLISFRAMEBUFFERPROC                 glIsFramebuffer = NULL;
        PFNGLDELETEFRAMEBUFFERSPROC            glDeleteFramebuffers = NULL;
        PFNGLCHECKFRAMEBUFFERSTATUSPROC        glCheckFramebufferStatus = NULL;
        PFNGLDRAWBUFFERSPROC                   glDrawBuffers = NULL;
        PFNGLBINDFRAGDATALOCATIONPROC          glBindFragDataLocation = NULL;
        PFNGLGETFRAGDATALOCATIONPROC           glGetFragDataLocation = NULL;
        PFNGLGETRENDERBUFFERPARAMETERIVPROC    glGetRenderbufferParameteriv = NULL;
        // ARB_debug_output
        PFNGLDEBUGMESSAGECONTROLPROC           glDebugMessageControl = NULL;
        PFNGLDEBUGMESSAGEINSERTPROC            glDebugMessageInsert = NULL;
        PFNGLDEBUGMESSAGECALLBACKPROC          glDebugMessageCallback = NULL;
        PFNGLGETDEBUGMESSAGELOGPROC            glGetDebugMessageLog = NULL;
        PFNGLGETPOINTERVPROC                   glGetPointerv = NULL;
        PFNGLMAPBUFFERPROC                     glMapBuffer = NULL;
        PFNGLUNMAPBUFFERPROC                   glUnmapBuffer = NULL;
        PFNGLGETBUFFERPARAMETERIVPROC          glGetBufferParameteriv = NULL;
#ifndef __CYGWIN__
        PFNGLENABLEVERTEXATTRIBARRAYPROC       glEnableVertexAttribArray = NULL;
        PFNGLDISABLEVERTEXATTRIBARRAYPROC      glDisableVertexAttribArray = NULL;
        PFNGLVERTEXATTRIBPOINTERPROC           glVertexAttribPointer = NULL;
        PFNGLUNIFORM1UIPROC                    glUniform1ui = NULL;
        PFNGLDRAWARRAYSINSTANCEDPROC           glDrawArraysInstanced = NULL;
        PFNGLFRAMEBUFFERPARAMETERIPROC         glFramebufferParameteri = NULL;
#endif
#endif
    }
    bool LoadOpenGLAPI()
    {
#ifdef WIN32
        const bool contextAvailable = ( wglGetCurrentContext() != NULL );
#else
        const bool contextAvailable = ( glXGetCurrentContext() != NULL );
#endif
#ifdef ANDROID
        // GL_OES_vertex_array_object
        if ( strstr ( ( char const* ) glGetString ( GL_EXTENSIONS ), "GL_OES_vertex_array_object" ) != NULL )
        {
            AEONGAMES_LOG_INFO ( "OpenGL ES: GL_OES_vertex_array_object supported" );
            GLGETPROCADDRESS ( PFNGLGENVERTEXARRAYSOESPROC,       glGenVertexArrays               );
            GLGETPROCADDRESS ( PFNGLBINDVERTEXARRAYOESPROC,       glBindVertexArray               );
            GLGETPROCADDRESS ( PFNGLDELETEVERTEXARRAYSOESPROC,    glDeleteVertexArrays            );
            GLGETPROCADDRESS ( PFNGLISVERTEXARRAYOESPROC,         glIsVertexArray                 );
        }
        else
        {
            AEONGAMES_LOG_INFO ( "OpenGL ES: GL_OES_vertex_array_object NOT supported" );
        }
#else
        GLGETPROCADDRESS ( PFNGLGETSTRINGIPROC,                    glGetStringi                    );
        GLGETPROCADDRESS ( PFNGLATTACHSHADERPROC,                  glAttachShader                  );
        GLGETPROCADDRESS ( PFNGLCOMPILESHADERPROC,                 glCompileShader                 );
        GLGETPROCADDRESS ( PFNGLCREATEPROGRAMPROC,                 glCreateProgram                 );
        GLGETPROCADDRESS ( PFNGLISPROGRAMPROC,                     glIsProgram                     );
        GLGETPROCADDRESS ( PFNGLCREATESHADERPROC,                  glCreateShader                  );
        GLGETPROCADDRESS ( PFNGLISSHADERPROC,                      glIsShader                      );
        GLGETPROCADDRESS ( PFNGLDELETEPROGRAMPROC,                 glDeleteProgram                 );
        GLGETPROCADDRESS ( PFNGLDELETESHADERPROC,                  glDeleteShader                  );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB1FPROC,                glVertexAttrib1f                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB1SPROC,                glVertexAttrib1s                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB1DPROC,                glVertexAttrib1d                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI1IPROC,               glVertexAttribI1i               );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI1UIPROC,              glVertexAttribI1ui              );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB2FPROC,                glVertexAttrib2f                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB2SPROC,                glVertexAttrib2s                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB2DPROC,                glVertexAttrib2d                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI2IPROC,               glVertexAttribI2i               );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI2UIPROC,              glVertexAttribI2ui              );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB3FPROC,                glVertexAttrib3f                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB3SPROC,                glVertexAttrib3s                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB3DPROC,                glVertexAttrib3d                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI3IPROC,               glVertexAttribI3i               );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI3UIPROC,              glVertexAttribI3ui              );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB4FPROC,                glVertexAttrib4f                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB4SPROC,                glVertexAttrib4s                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB4DPROC,                glVertexAttrib4d                );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB4NUBPROC,              glVertexAttrib4Nub              );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI4IPROC,               glVertexAttribI4i               );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI4UIPROC,              glVertexAttribI4ui              );
#if 0
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBL1DPROC,               glVertexAttribL1d               );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBL2DPROC,               glVertexAttribL2d               );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBL3DPROC,               glVertexAttribL3d               );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBL4DPROC,               glVertexAttribL4d               );
#endif
        GLGETPROCADDRESS ( PFNGLGETATTRIBLOCATIONPROC,             glGetAttribLocation             );
        GLGETPROCADDRESS ( PFNGLBINDATTRIBLOCATIONPROC,            glBindAttribLocation            );
        GLGETPROCADDRESS ( PFNGLGETPROGRAMIVPROC,                  glGetProgramiv                  );
        GLGETPROCADDRESS ( PFNGLGETPROGRAMINFOLOGPROC,             glGetProgramInfoLog             );
        GLGETPROCADDRESS ( PFNGLGETSHADERIVPROC,                   glGetShaderiv                   );
        GLGETPROCADDRESS ( PFNGLGETSHADERINFOLOGPROC,              glGetShaderInfoLog              );
        GLGETPROCADDRESS ( PFNGLGETUNIFORMLOCATIONPROC,            glGetUniformLocation            );
        GLGETPROCADDRESS ( PFNGLGETACTIVEUNIFORMPROC,              glGetActiveUniform              );
        GLGETPROCADDRESS ( PFNGLGETACTIVEUNIFORMNAMEPROC,          glGetActiveUniformName          );
        GLGETPROCADDRESS ( PFNGLLINKPROGRAMPROC,                   glLinkProgram                   );
        GLGETPROCADDRESS ( PFNGLSHADERSOURCEPROC,                  glShaderSource                  );
        GLGETPROCADDRESS ( PFNGLUSEPROGRAMPROC,                    glUseProgram                    );
        GLGETPROCADDRESS ( PFNGLUNIFORM1IPROC,                     glUniform1i                     );
        GLGETPROCADDRESS ( PFNGLUNIFORMMATRIX2FVPROC,              glUniformMatrix2fv              );
        GLGETPROCADDRESS ( PFNGLUNIFORMMATRIX3FVPROC,              glUniformMatrix3fv              );
        GLGETPROCADDRESS ( PFNGLUNIFORMMATRIX4FVPROC,              glUniformMatrix4fv              );
        GLGETPROCADDRESS ( PFNGLGENBUFFERSPROC,                    glGenBuffers                    );
        GLGETPROCADDRESS ( PFNGLBINDBUFFERPROC,                    glBindBuffer                    );
        GLGETPROCADDRESS ( PFNGLBUFFERDATAPROC,                    glBufferData                    );
        GLGETPROCADDRESS ( PFNGLGETBUFFERSUBDATAPROC,              glGetBufferSubData              );
        GLGETPROCADDRESS ( PFNGLDELETEBUFFERSPROC,                 glDeleteBuffers                 );
        GLGETPROCADDRESS ( PFNGLGETACTIVEATTRIBPROC,               glGetActiveAttrib               );
#ifndef DISABLE_VAO
        // GL_ARB_vertex_array_object
        GLGETPROCADDRESS ( PFNGLGENVERTEXARRAYSPROC,               glGenVertexArrays               );
        GLGETPROCADDRESS ( PFNGLBINDVERTEXARRAYPROC,               glBindVertexArray               );
        GLGETPROCADDRESS ( PFNGLDELETEVERTEXARRAYSPROC,            glDeleteVertexArrays            );
        GLGETPROCADDRESS ( PFNGLISVERTEXARRAYPROC,                 glIsVertexArray                 );
#endif
        GLGETPROCADDRESS ( PFNGLUNIFORM1FVPROC,                    glUniform1fv                    );
        GLGETPROCADDRESS ( PFNGLUNIFORM2FVPROC,                    glUniform2fv                    );
        GLGETPROCADDRESS ( PFNGLUNIFORM3FVPROC,                    glUniform3fv                    );
        GLGETPROCADDRESS ( PFNGLUNIFORM3FPROC,                     glUniform3f                     );
        GLGETPROCADDRESS ( PFNGLUNIFORM4FVPROC,                    glUniform4fv                    );
        GLGETPROCADDRESS ( PFNGLUNIFORM1IVPROC,                    glUniform1iv                    );
        GLGETPROCADDRESS ( PFNGLUNIFORM2IVPROC,                    glUniform2iv                    );
        GLGETPROCADDRESS ( PFNGLUNIFORM3IVPROC,                    glUniform3iv                    );
        GLGETPROCADDRESS ( PFNGLUNIFORM4IVPROC,                    glUniform4iv                    );
        GLGETPROCADDRESS ( PFNGLUNIFORM1UIVPROC,                   glUniform1uiv                   );
        GLGETPROCADDRESS ( PFNGLUNIFORM2UIVPROC,                   glUniform2uiv                   );
        GLGETPROCADDRESS ( PFNGLUNIFORM3UIVPROC,                   glUniform3uiv                   );
        GLGETPROCADDRESS ( PFNGLUNIFORM4UIVPROC,                     glUniform4uiv );
        GLGETPROCADDRESS ( PFNGLGETUNIFORMINDICESPROC,               glGetUniformIndices );
        GLGETPROCADDRESS ( PFNGLCOMPRESSEDTEXIMAGE2DPROC,          glCompressedTexImage2D          );
        GLGETPROCADDRESS ( PFNGLDETACHSHADERPROC,                  glDetachShader                  );
        GLGETPROCADDRESS ( PFNGLISBUFFERPROC,                      glIsBuffer                      );
        GLGETPROCADDRESS ( PFNGLACTIVETEXTUREPROC,                 glActiveTexture                 );

        GLGETPROCADDRESS ( PFNGLGENRENDERBUFFERSPROC,              glGenRenderbuffers              );
        GLGETPROCADDRESS ( PFNGLBINDRENDERBUFFERPROC,              glBindRenderbuffer              );
        GLGETPROCADDRESS ( PFNGLRENDERBUFFERSTORAGEPROC,           glRenderbufferStorage           );
        GLGETPROCADDRESS ( PFNGLFRAMEBUFFERRENDERBUFFERPROC,       glFramebufferRenderbuffer       );
        GLGETPROCADDRESS ( PFNGLDELETERENDERBUFFERSPROC,           glDeleteRenderbuffers           );

        GLGETPROCADDRESS ( PFNGLGENFRAMEBUFFERSPROC,               glGenFramebuffers               );
        GLGETPROCADDRESS ( PFNGLBINDFRAMEBUFFERPROC,               glBindFramebuffer               );
        GLGETPROCADDRESS ( PFNGLISRENDERBUFFERPROC,                glIsRenderbuffer                );
        GLGETPROCADDRESS ( PFNGLISFRAMEBUFFERPROC,                 glIsFramebuffer                 );
        GLGETPROCADDRESS ( PFNGLDELETEFRAMEBUFFERSPROC,            glDeleteFramebuffers            );
        GLGETPROCADDRESS ( PFNGLCHECKFRAMEBUFFERSTATUSPROC,        glCheckFramebufferStatus        );
        GLGETPROCADDRESS ( PFNGLDRAWBUFFERSPROC,                   glDrawBuffers                   );
        GLGETPROCADDRESS ( PFNGLBINDFRAGDATALOCATIONPROC,          glBindFragDataLocation          );
        GLGETPROCADDRESS ( PFNGLGETFRAGDATALOCATIONPROC,           glGetFragDataLocation           );
        GLGETPROCADDRESS ( PFNGLGETRENDERBUFFERPARAMETERIVPROC,    glGetRenderbufferParameteriv    );
        // ARB_debug_output
        GLGETPROCADDRESS ( PFNGLDEBUGMESSAGECONTROLPROC,           glDebugMessageControl           );
        GLGETPROCADDRESS ( PFNGLDEBUGMESSAGEINSERTPROC,            glDebugMessageInsert            );
        GLGETPROCADDRESS ( PFNGLDEBUGMESSAGECALLBACKPROC,          glDebugMessageCallback          );
        GLGETPROCADDRESS ( PFNGLGETDEBUGMESSAGELOGPROC,            glGetDebugMessageLog            );
        GLGETPROCADDRESS ( PFNGLGETPOINTERVPROC,                   glGetPointerv                   );
        GLGETPROCADDRESS ( PFNGLMAPBUFFERPROC,                     glMapBuffer                     );
        GLGETPROCADDRESS ( PFNGLUNMAPBUFFERPROC,                   glUnmapBuffer                   );
        GLGETPROCADDRESS ( PFNGLGETBUFFERPARAMETERIVPROC,          glGetBufferParameteriv          );
#ifndef __CYGWIN__
        GLGETPROCADDRESS ( PFNGLENABLEVERTEXATTRIBARRAYPROC,       glEnableVertexAttribArray       );
        GLGETPROCADDRESS ( PFNGLDISABLEVERTEXATTRIBARRAYPROC,      glDisableVertexAttribArray      );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBPOINTERPROC,           glVertexAttribPointer           );
        GLGETPROCADDRESS ( PFNGLUNIFORM1UIPROC,                    glUniform1ui                    );
        GLGETPROCADDRESS ( PFNGLDRAWARRAYSINSTANCEDPROC,           glDrawArraysInstanced           );
        GLGETPROCADDRESS ( PFNGLFRAMEBUFFERPARAMETERIPROC,         glFramebufferParameteri         );
#endif
#endif

        // Provide some information regarding OpenGL Drivers.
        std::cout << LogLevel ( LogLevel::Level::Info ) << "OpenGL Version " << glGetString ( GL_VERSION ) << std::endl;
        std::cout << LogLevel ( LogLevel::Level::Info ) << "GLSL Version " << glGetString ( GL_SHADING_LANGUAGE_VERSION ) << std::endl;
        std::cout << LogLevel ( LogLevel::Level::Info ) << "OpenGL Vendor " << glGetString ( GL_VENDOR ) << std::endl;
        GLint texSize = 0;
        glGetIntegerv ( GL_MAX_TEXTURE_SIZE, &texSize );
        std::cout << LogLevel ( LogLevel::Level::Info ) << "Maximum Texture Size: " << texSize << std::endl;
        GLint glInteger;
#ifdef ANDROID
        glGetIntegerv ( GL_MAX_VERTEX_UNIFORM_VECTORS, &glInteger );
        std::cout << "GLSLES Max Vertex Uniform Vectors " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_FRAGMENT_UNIFORM_VECTORS, &glInteger );
        std::cout << "GLSLES Max Fragment Uniform Vectors " << glInteger << std::endl;
        std::cout << glGetString ( GL_EXTENSIONS );
#else
        glGetIntegerv ( GL_MAX_VERTEX_UNIFORM_COMPONENTS, &glInteger );
        std::cout << LogLevel ( LogLevel::Level::Info ) << "GLSL Max Vertex Uniform Components " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &glInteger );
        std::cout << LogLevel ( LogLevel::Level::Info ) << "GLSL Max Fragment Uniform Components " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_VERTEX_ATTRIBS, &glInteger );
        std::cout << LogLevel ( LogLevel::Level::Info ) << "GLSL Max Vertex Attributes " << glInteger << std::endl;
        GLint extension_count;
        glGetIntegerv ( GL_NUM_EXTENSIONS, &extension_count );
        for ( GLint i = 0; i < extension_count; ++i )
        {
            std::cout << LogLevel ( LogLevel::Level::Info ) << glGetStringi ( GL_EXTENSIONS, i ) << std::endl;
        }
#endif
        return true;
    }
}
