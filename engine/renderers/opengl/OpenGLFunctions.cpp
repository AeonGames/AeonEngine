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
        PFNGLGENVERTEXARRAYSOESPROC            glGenVertexArrays = nullptr;
        PFNGLBINDVERTEXARRAYOESPROC            glBindVertexArray = nullptr;
        PFNGLISVERTEXARRAYOESPROC              glIsVertexArray = nullptr;
        PFNGLDELETEVERTEXARRAYSOESPROC         glDeleteVertexArrays = nullptr;
#else
        PFNGLGETSTRINGIPROC                    glGetStringi = nullptr;
        PFNGLATTACHSHADERPROC                  glAttachShader = nullptr;
        PFNGLCOMPILESHADERPROC                 glCompileShader = nullptr;
        PFNGLCREATEPROGRAMPROC                 glCreateProgram = nullptr;
        PFNGLISPROGRAMPROC                     glIsProgram = nullptr;
        PFNGLCREATESHADERPROC                  glCreateShader = nullptr;
        PFNGLISSHADERPROC                      glIsShader = nullptr;
        PFNGLDELETEPROGRAMPROC                 glDeleteProgram = nullptr;
        PFNGLDELETESHADERPROC                  glDeleteShader = nullptr;

        PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f = nullptr;
        PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s = nullptr;
        PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d = nullptr;
        PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i = nullptr;
        PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui = nullptr;
        PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f = nullptr;
        PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s = nullptr;
        PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d = nullptr;
        PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i = nullptr;
        PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui = nullptr;
        PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f = nullptr;
        PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s = nullptr;
        PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d = nullptr;
        PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i = nullptr;
        PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui = nullptr;
        PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f = nullptr;
        PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s = nullptr;
        PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d = nullptr;
        PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub = nullptr;
        PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i = nullptr;
        PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui = nullptr;
#if 0
        PFNGLVERTEXATTRIBL1DPROC glVertexAttribL1d = nullptr;
        PFNGLVERTEXATTRIBL2DPROC glVertexAttribL2d = nullptr;
        PFNGLVERTEXATTRIBL3DPROC glVertexAttribL3d = nullptr;
        PFNGLVERTEXATTRIBL4DPROC glVertexAttribL4d = nullptr;
#endif

        PFNGLGETATTRIBLOCATIONPROC             glGetAttribLocation = nullptr;
        PFNGLBINDATTRIBLOCATIONPROC            glBindAttribLocation = nullptr;
        PFNGLGETPROGRAMIVPROC                  glGetProgramiv = nullptr;
        PFNGLGETPROGRAMINFOLOGPROC             glGetProgramInfoLog = nullptr;
        PFNGLGETSHADERIVPROC                   glGetShaderiv = nullptr;
        PFNGLGETSHADERINFOLOGPROC              glGetShaderInfoLog = nullptr;
        PFNGLGETUNIFORMLOCATIONPROC            glGetUniformLocation = nullptr;
        PFNGLGETACTIVEUNIFORMPROC              glGetActiveUniform = nullptr;
        PFNGLGETACTIVEUNIFORMNAMEPROC          glGetActiveUniformName = nullptr;
        PFNGLLINKPROGRAMPROC                   glLinkProgram = nullptr;
        PFNGLSHADERSOURCEPROC                  glShaderSource = nullptr;
        PFNGLUSEPROGRAMPROC                    glUseProgram = nullptr;
        PFNGLUNIFORM1IPROC                     glUniform1i = nullptr;
        PFNGLUNIFORMMATRIX2FVPROC              glUniformMatrix2fv = nullptr;
        PFNGLUNIFORMMATRIX3FVPROC              glUniformMatrix3fv = nullptr;
        PFNGLUNIFORMMATRIX4FVPROC              glUniformMatrix4fv = nullptr;
        PFNGLGENBUFFERSPROC                    glGenBuffers = nullptr;
        PFNGLBINDBUFFERPROC                    glBindBuffer = nullptr;
        PFNGLBUFFERDATAPROC                    glBufferData = nullptr;
        PFNGLGETBUFFERSUBDATAPROC              glGetBufferSubData = nullptr;
        PFNGLDELETEBUFFERSPROC                 glDeleteBuffers = nullptr;
        PFNGLGETACTIVEATTRIBPROC               glGetActiveAttrib = nullptr;
#ifndef DISABLE_VAO
        // GL_ARB_vertex_array_object
        PFNGLGENVERTEXARRAYSPROC               glGenVertexArrays = nullptr;
        PFNGLBINDVERTEXARRAYPROC               glBindVertexArray = nullptr;
        PFNGLDELETEVERTEXARRAYSPROC            glDeleteVertexArrays = nullptr;
        PFNGLISVERTEXARRAYPROC                 glIsVertexArray = nullptr;
#endif
        PFNGLUNIFORM1FVPROC                    glUniform1fv = nullptr;
        PFNGLUNIFORM2FVPROC                    glUniform2fv = nullptr;
        PFNGLUNIFORM3FVPROC                    glUniform3fv = nullptr;
        PFNGLUNIFORM3FPROC                     glUniform3f = nullptr;
        PFNGLUNIFORM4FVPROC                    glUniform4fv = nullptr;
        PFNGLUNIFORM1IVPROC                    glUniform1iv = nullptr;
        PFNGLUNIFORM2IVPROC                    glUniform2iv = nullptr;
        PFNGLUNIFORM3IVPROC                    glUniform3iv = nullptr;
        PFNGLUNIFORM4IVPROC                    glUniform4iv = nullptr;
        PFNGLUNIFORM1UIVPROC                   glUniform1uiv = nullptr;
        PFNGLUNIFORM2UIVPROC                   glUniform2uiv = nullptr;
        PFNGLUNIFORM3UIVPROC                   glUniform3uiv = nullptr;
        PFNGLUNIFORM4UIVPROC                   glUniform4uiv = nullptr;
        PFNGLGETUNIFORMINDICESPROC             glGetUniformIndices = nullptr;
        PFNGLCOMPRESSEDTEXIMAGE2DPROC          glCompressedTexImage2D = nullptr;
        PFNGLDETACHSHADERPROC                  glDetachShader = nullptr;
        PFNGLISBUFFERPROC                      glIsBuffer = nullptr;
        PFNGLACTIVETEXTUREPROC                 glActiveTexture = nullptr;
        PFNGLGENRENDERBUFFERSPROC              glGenRenderbuffers = nullptr;
        PFNGLBINDRENDERBUFFERPROC              glBindRenderbuffer = nullptr;
        PFNGLRENDERBUFFERSTORAGEPROC           glRenderbufferStorage = nullptr;
        PFNGLFRAMEBUFFERRENDERBUFFERPROC       glFramebufferRenderbuffer = nullptr;
        PFNGLDELETERENDERBUFFERSPROC           glDeleteRenderbuffers = nullptr;
        PFNGLGENFRAMEBUFFERSPROC               glGenFramebuffers = nullptr;
        PFNGLBINDFRAMEBUFFERPROC               glBindFramebuffer = nullptr;
        PFNGLISRENDERBUFFERPROC                glIsRenderbuffer = nullptr;
        PFNGLISFRAMEBUFFERPROC                 glIsFramebuffer = nullptr;
        PFNGLDELETEFRAMEBUFFERSPROC            glDeleteFramebuffers = nullptr;
        PFNGLCHECKFRAMEBUFFERSTATUSPROC        glCheckFramebufferStatus = nullptr;
        PFNGLDRAWBUFFERSPROC                   glDrawBuffers = nullptr;
        PFNGLBINDFRAGDATALOCATIONPROC          glBindFragDataLocation = nullptr;
        PFNGLGETFRAGDATALOCATIONPROC           glGetFragDataLocation = nullptr;
        PFNGLGETRENDERBUFFERPARAMETERIVPROC    glGetRenderbufferParameteriv = nullptr;
        // ARB_debug_output
        PFNGLDEBUGMESSAGECONTROLPROC           glDebugMessageControl = nullptr;
        PFNGLDEBUGMESSAGEINSERTPROC            glDebugMessageInsert = nullptr;
        PFNGLDEBUGMESSAGECALLBACKPROC          glDebugMessageCallback = nullptr;
        PFNGLGETDEBUGMESSAGELOGPROC            glGetDebugMessageLog = nullptr;
#if _WIN32
        PFNGLGETPOINTERVPROC                   glGetPointerv = nullptr;
#endif
        PFNGLMAPBUFFERPROC                     glMapBuffer = nullptr;
        PFNGLUNMAPBUFFERPROC                   glUnmapBuffer = nullptr;
        PFNGLGETBUFFERPARAMETERIVPROC          glGetBufferParameteriv = nullptr;
#ifndef __CYGWIN__
        PFNGLENABLEVERTEXATTRIBARRAYPROC       glEnableVertexAttribArray = nullptr;
        PFNGLDISABLEVERTEXATTRIBARRAYPROC      glDisableVertexAttribArray = nullptr;
        PFNGLVERTEXATTRIBPOINTERPROC           glVertexAttribPointer = nullptr;
        PFNGLUNIFORM1UIPROC                    glUniform1ui = nullptr;
        PFNGLDRAWARRAYSINSTANCEDPROC           glDrawArraysInstanced = nullptr;
        PFNGLFRAMEBUFFERPARAMETERIPROC         glFramebufferParameteri = nullptr;
#endif
#endif
    }
    bool LoadOpenGLAPI()
    {
#ifdef ANDROID
        // GL_OES_vertex_array_object
        if ( strstr ( ( char const* ) glGetString ( GL_EXTENSIONS ), "GL_OES_vertex_array_object" ) != nullptr )
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
#if _WIN32
        GLGETPROCADDRESS ( PFNGLGETPOINTERVPROC,                   glGetPointerv                   );
#endif
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
        std::cout << "OpenGL Version " << glGetString ( GL_VERSION ) << std::endl;
        std::cout << "GLSL Version " << glGetString ( GL_SHADING_LANGUAGE_VERSION ) << std::endl;
        std::cout << "OpenGL Vendor " << glGetString ( GL_VENDOR ) << std::endl;
        GLint texSize = 0;
        glGetIntegerv ( GL_MAX_TEXTURE_SIZE, &texSize );
        std::cout << "Maximum Texture Size: " << texSize << std::endl;
        GLint glInteger;
#ifdef ANDROID
        glGetIntegerv ( GL_MAX_VERTEX_UNIFORM_VECTORS, &glInteger );
        std::cout << "GLSLES Max Vertex Uniform Vectors " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_FRAGMENT_UNIFORM_VECTORS, &glInteger );
        std::cout << "GLSLES Max Fragment Uniform Vectors " << glInteger << std::endl;
        std::cout << glGetString ( GL_EXTENSIONS );
#else
        glGetIntegerv ( GL_MAX_VERTEX_UNIFORM_COMPONENTS, &glInteger );
        std::cout << "GLSL Max Vertex Uniform Components " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &glInteger );
        std::cout << "GLSL Max Fragment Uniform Components " << glInteger << std::endl;
        glGetIntegerv ( GL_MAX_VERTEX_ATTRIBS, &glInteger );
        std::cout << "GLSL Max Vertex Attributes " << glInteger << std::endl;
        GLint extension_count;
        glGetIntegerv ( GL_NUM_EXTENSIONS, &extension_count );
        for ( GLint i = 0; i < extension_count; ++i )
        {
            std::cout << glGetStringi ( GL_EXTENSIONS, i ) << std::endl;
        }
#endif
        return true;
    }
}
