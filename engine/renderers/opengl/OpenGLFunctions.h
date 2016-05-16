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
#ifndef AEONGAMES_OPENGLFUNCTIONS_H
#define AEONGAMES_OPENGLFUNCTIONS_H

#ifdef ANDROID
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#else
#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include "glcorearb.h"
#include <GL/gl.h>
#else
#include "glcorearb.h"
#include <GL/gl.h>
#endif
#endif // ANDROID

namespace AeonGames
{
    extern "C"
    {
#ifdef ANDROID
#ifndef DISABLE_VAO
        // GL_OES_vertex_array_object
        extern PFNGLGENVERTEXARRAYSOESPROC         glGenVertexArrays;
        extern PFNGLBINDVERTEXARRAYOESPROC         glBindVertexArray;
        extern PFNGLISVERTEXARRAYOESPROC           glIsVertexArray;
        extern PFNGLDELETEVERTEXARRAYSOESPROC      glDeleteVertexArrays;
#endif
#else
        extern PFNGLGETSTRINGIPROC                  glGetStringi;
        extern PFNGLATTACHSHADERPROC                glAttachShader;
        extern PFNGLCOMPILESHADERPROC               glCompileShader;
        extern PFNGLCREATEPROGRAMPROC               glCreateProgram;
        extern PFNGLISPROGRAMPROC                   glIsProgram;
        extern PFNGLCREATESHADERPROC                glCreateShader;
        extern PFNGLISSHADERPROC                    glIsShader;

        extern PFNGLDELETEPROGRAMPROC               glDeleteProgram;
        extern PFNGLDELETESHADERPROC                glDeleteShader;
        extern PFNGLENABLEVERTEXATTRIBARRAYPROC     glEnableVertexAttribArray;
        extern PFNGLDISABLEVERTEXATTRIBARRAYPROC    glDisableVertexAttribArray;

        extern PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f;
        extern PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s;
        extern PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d;
        extern PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i;
        extern PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui;
        extern PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f;
        extern PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s;
        extern PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d;
        extern PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i;
        extern PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui;
        extern PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f;
        extern PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s;
        extern PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d;
        extern PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i;
        extern PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui;
        extern PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;
        extern PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s;
        extern PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d;
        extern PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub;
        extern PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i;
        extern PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui;
#if 0
        extern PFNGLVERTEXATTRIBL1DPROC glVertexAttribL1d;
        extern PFNGLVERTEXATTRIBL2DPROC glVertexAttribL2d;
        extern PFNGLVERTEXATTRIBL3DPROC glVertexAttribL3d;
        extern PFNGLVERTEXATTRIBL4DPROC glVertexAttribL4d;
#endif


        extern PFNGLGETATTRIBLOCATIONPROC           glGetAttribLocation;
        extern PFNGLBINDATTRIBLOCATIONPROC          glBindAttribLocation;
        extern PFNGLGETPROGRAMIVPROC                glGetProgramiv;
        extern PFNGLGETPROGRAMINFOLOGPROC           glGetProgramInfoLog;
        extern PFNGLGETSHADERIVPROC                 glGetShaderiv;
        extern PFNGLGETSHADERINFOLOGPROC            glGetShaderInfoLog;
        extern PFNGLGETUNIFORMLOCATIONPROC          glGetUniformLocation;
        extern PFNGLGETACTIVEUNIFORMPROC            glGetActiveUniform;
        extern PFNGLGETACTIVEUNIFORMNAMEPROC        glGetActiveUniformName;
        extern PFNGLLINKPROGRAMPROC                 glLinkProgram;
        extern PFNGLSHADERSOURCEPROC                glShaderSource;
        extern PFNGLUSEPROGRAMPROC                  glUseProgram;
        extern PFNGLUNIFORM1IPROC                   glUniform1i;
        extern PFNGLUNIFORM1UIPROC                  glUniform1ui;
        extern PFNGLUNIFORMMATRIX2FVPROC            glUniformMatrix2fv;
        extern PFNGLUNIFORMMATRIX3FVPROC            glUniformMatrix3fv;
        extern PFNGLUNIFORMMATRIX4FVPROC            glUniformMatrix4fv;
        extern PFNGLVERTEXATTRIBPOINTERPROC         glVertexAttribPointer;
        extern PFNGLGENBUFFERSPROC                  glGenBuffers;
        extern PFNGLBINDBUFFERPROC                  glBindBuffer;
        extern PFNGLBUFFERDATAPROC                  glBufferData;
        extern PFNGLGETBUFFERSUBDATAPROC            glGetBufferSubData;
        extern PFNGLDELETEBUFFERSPROC               glDeleteBuffers;
        extern PFNGLGETACTIVEATTRIBPROC             glGetActiveAttrib;
#ifndef DISABLE_VAO
        // GL_ARB_vertex_array_object
        extern PFNGLGENVERTEXARRAYSPROC             glGenVertexArrays;
        extern PFNGLBINDVERTEXARRAYPROC             glBindVertexArray;
        extern PFNGLDELETEVERTEXARRAYSPROC          glDeleteVertexArrays;
        extern PFNGLISVERTEXARRAYPROC               glIsVertexArray;
#endif
        extern PFNGLUNIFORM1FVPROC                  glUniform1fv;
        extern PFNGLUNIFORM2FVPROC                  glUniform2fv;
        extern PFNGLUNIFORM3FVPROC                  glUniform3fv;
        extern PFNGLUNIFORM3FPROC                   glUniform3f;
        extern PFNGLUNIFORM4FVPROC                  glUniform4fv;
        extern PFNGLUNIFORM1IVPROC                  glUniform1iv;
        extern PFNGLUNIFORM2IVPROC                  glUniform2iv;
        extern PFNGLUNIFORM3IVPROC                  glUniform3iv;
        extern PFNGLUNIFORM4IVPROC                  glUniform4iv;
        extern PFNGLUNIFORM1UIVPROC                 glUniform1uiv;
        extern PFNGLUNIFORM2UIVPROC                 glUniform2uiv;
        extern PFNGLUNIFORM3UIVPROC                 glUniform3uiv;
        extern PFNGLUNIFORM4UIVPROC                 glUniform4uiv;
        extern PFNGLGETUNIFORMINDICESPROC           glGetUniformIndices;
        extern PFNGLCOMPRESSEDTEXIMAGE2DPROC        glCompressedTexImage2D;
        extern PFNGLDETACHSHADERPROC                glDetachShader;
        extern PFNGLISBUFFERPROC                    glIsBuffer;
        extern PFNGLACTIVETEXTUREPROC               glActiveTexture;
        extern PFNGLDRAWARRAYSINSTANCEDPROC         glDrawArraysInstanced;
        extern PFNGLGENRENDERBUFFERSPROC            glGenRenderbuffers;
        extern PFNGLBINDRENDERBUFFERPROC            glBindRenderbuffer;
        extern PFNGLRENDERBUFFERSTORAGEPROC         glRenderbufferStorage;
        extern PFNGLFRAMEBUFFERRENDERBUFFERPROC     glFramebufferRenderbuffer;
        extern PFNGLDELETERENDERBUFFERSPROC         glDeleteRenderbuffers;
        extern PFNGLGENFRAMEBUFFERSPROC             glGenFramebuffers;
        extern PFNGLBINDFRAMEBUFFERPROC             glBindFramebuffer;
        extern PFNGLISRENDERBUFFERPROC              glIsRenderbuffer;
        extern PFNGLISFRAMEBUFFERPROC               glIsFramebuffer;
        extern PFNGLFRAMEBUFFERPARAMETERIPROC       glFramebufferParameteri;
        extern PFNGLDELETEFRAMEBUFFERSPROC          glDeleteFramebuffers;
        extern PFNGLCHECKFRAMEBUFFERSTATUSPROC      glCheckFramebufferStatus;
        extern PFNGLDRAWBUFFERSPROC                 glDrawBuffers;
        extern PFNGLBINDFRAGDATALOCATIONPROC        glBindFragDataLocation;
        extern PFNGLGETRENDERBUFFERPARAMETERIVPROC  glGetRenderbufferParameteriv;
        extern PFNGLGETFRAGDATALOCATIONPROC         glGetFragDataLocation;
        // ARB_debug_output
        extern PFNGLDEBUGMESSAGECONTROLPROC         glDebugMessageControl;
        extern PFNGLDEBUGMESSAGEINSERTPROC          glDebugMessageInsert;
        extern PFNGLDEBUGMESSAGECALLBACKPROC        glDebugMessageCallback;
        extern PFNGLGETDEBUGMESSAGELOGPROC          glGetDebugMessageLog;
#if _WIN32
        extern PFNGLGETPOINTERVPROC                 glGetPointerv;
#endif
        extern PFNGLMAPBUFFERPROC                   glMapBuffer;
        extern PFNGLUNMAPBUFFERPROC                 glUnmapBuffer;
        extern PFNGLGETBUFFERPARAMETERIVPROC        glGetBufferParameteriv;
#endif
    }
    bool LoadOpenGLAPI();
}
#endif
