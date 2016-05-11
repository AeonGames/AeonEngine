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
#endif
#endif

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
    // OpenGL Functions
#ifdef ANDROID
    // GL_OES_vertex_array_object
    static PFNGLGENVERTEXARRAYSOESPROC            glGenVertexArrays = nullptr;
    static PFNGLBINDVERTEXARRAYOESPROC            glBindVertexArray = nullptr;
    static PFNGLISVERTEXARRAYOESPROC              glIsVertexArray = nullptr;
    static PFNGLDELETEVERTEXARRAYSOESPROC         glDeleteVertexArrays = nullptr;
#else
    static PFNGLGETSTRINGIPROC                    glGetStringi = nullptr;
    static PFNGLATTACHSHADERPROC                  glAttachShader = nullptr;
    static PFNGLCOMPILESHADERPROC                 glCompileShader = nullptr;
    static PFNGLCREATEPROGRAMPROC                 glCreateProgram = nullptr;
    static PFNGLISPROGRAMPROC                     glIsProgram = nullptr;
    static PFNGLCREATESHADERPROC                  glCreateShader = nullptr;
    static PFNGLISSHADERPROC                      glIsShader = nullptr;
    static PFNGLDELETEPROGRAMPROC                 glDeleteProgram = nullptr;
    static PFNGLDELETESHADERPROC                  glDeleteShader = nullptr;
    static PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f = nullptr;
    static PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s = nullptr;
    static PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d = nullptr;
    static PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i = nullptr;
    static PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui = nullptr;
    static PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f = nullptr;
    static PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s = nullptr;
    static PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d = nullptr;
    static PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i = nullptr;
    static PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui = nullptr;
    static PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f = nullptr;
    static PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s = nullptr;
    static PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d = nullptr;
    static PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i = nullptr;
    static PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui = nullptr;
    static PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f = nullptr;
    static PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s = nullptr;
    static PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d = nullptr;
    static PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub = nullptr;
    static PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i = nullptr;
    static PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui = nullptr;
    static PFNGLGETATTRIBLOCATIONPROC             glGetAttribLocation = nullptr;
    static PFNGLBINDATTRIBLOCATIONPROC            glBindAttribLocation = nullptr;
    static PFNGLGETPROGRAMIVPROC                  glGetProgramiv = nullptr;
    static PFNGLGETPROGRAMINFOLOGPROC             glGetProgramInfoLog = nullptr;
    static PFNGLGETSHADERIVPROC                   glGetShaderiv = nullptr;
    static PFNGLGETSHADERINFOLOGPROC              glGetShaderInfoLog = nullptr;
    static PFNGLGETUNIFORMLOCATIONPROC            glGetUniformLocation = nullptr;
    static PFNGLGETACTIVEUNIFORMPROC              glGetActiveUniform = nullptr;
    static PFNGLGETACTIVEUNIFORMNAMEPROC          glGetActiveUniformName = nullptr;
    static PFNGLLINKPROGRAMPROC                   glLinkProgram = nullptr;
    static PFNGLSHADERSOURCEPROC                  glShaderSource = nullptr;
    static PFNGLUSEPROGRAMPROC                    glUseProgram = nullptr;
    static PFNGLUNIFORM1IPROC                     glUniform1i = nullptr;
    static PFNGLUNIFORMMATRIX2FVPROC              glUniformMatrix2fv = nullptr;
    static PFNGLUNIFORMMATRIX3FVPROC              glUniformMatrix3fv = nullptr;
    static PFNGLUNIFORMMATRIX4FVPROC              glUniformMatrix4fv = nullptr;
    static PFNGLGENBUFFERSPROC                    glGenBuffers = nullptr;
    static PFNGLBINDBUFFERPROC                    glBindBuffer = nullptr;
    static PFNGLBUFFERDATAPROC                    glBufferData = nullptr;
    static PFNGLGETBUFFERSUBDATAPROC              glGetBufferSubData = nullptr;
    static PFNGLDELETEBUFFERSPROC                 glDeleteBuffers = nullptr;
    static PFNGLGETACTIVEATTRIBPROC               glGetActiveAttrib = nullptr;
    // GL_ARB_vertex_array_object
    static PFNGLGENVERTEXARRAYSPROC               glGenVertexArrays = nullptr;
    static PFNGLBINDVERTEXARRAYPROC               glBindVertexArray = nullptr;
    static PFNGLDELETEVERTEXARRAYSPROC            glDeleteVertexArrays = nullptr;
    static PFNGLISVERTEXARRAYPROC                 glIsVertexArray = nullptr;
    static PFNGLUNIFORM1FVPROC                    glUniform1fv = nullptr;
    static PFNGLUNIFORM2FVPROC                    glUniform2fv = nullptr;
    static PFNGLUNIFORM3FVPROC                    glUniform3fv = nullptr;
    static PFNGLUNIFORM3FPROC                     glUniform3f = nullptr;
    static PFNGLUNIFORM4FVPROC                    glUniform4fv = nullptr;
    static PFNGLUNIFORM1IVPROC                    glUniform1iv = nullptr;
    static PFNGLUNIFORM2IVPROC                    glUniform2iv = nullptr;
    static PFNGLUNIFORM3IVPROC                    glUniform3iv = nullptr;
    static PFNGLUNIFORM4IVPROC                    glUniform4iv = nullptr;
    static PFNGLUNIFORM1UIVPROC                   glUniform1uiv = nullptr;
    static PFNGLUNIFORM2UIVPROC                   glUniform2uiv = nullptr;
    static PFNGLUNIFORM3UIVPROC                   glUniform3uiv = nullptr;
    static PFNGLUNIFORM4UIVPROC                   glUniform4uiv = nullptr;
    static PFNGLGETUNIFORMINDICESPROC             glGetUniformIndices = nullptr;
    static PFNGLCOMPRESSEDTEXIMAGE2DPROC          glCompressedTexImage2D = nullptr;
    static PFNGLDETACHSHADERPROC                  glDetachShader = nullptr;
    static PFNGLISBUFFERPROC                      glIsBuffer = nullptr;
    static PFNGLACTIVETEXTUREPROC                 glActiveTexture = nullptr;
    static PFNGLGENRENDERBUFFERSPROC              glGenRenderbuffers = nullptr;
    static PFNGLBINDRENDERBUFFERPROC              glBindRenderbuffer = nullptr;
    static PFNGLRENDERBUFFERSTORAGEPROC           glRenderbufferStorage = nullptr;
    static PFNGLFRAMEBUFFERRENDERBUFFERPROC       glFramebufferRenderbuffer = nullptr;
    static PFNGLDELETERENDERBUFFERSPROC           glDeleteRenderbuffers = nullptr;
    static PFNGLGENFRAMEBUFFERSPROC               glGenFramebuffers = nullptr;
    static PFNGLBINDFRAMEBUFFERPROC               glBindFramebuffer = nullptr;
    static PFNGLISRENDERBUFFERPROC                glIsRenderbuffer = nullptr;
    static PFNGLISFRAMEBUFFERPROC                 glIsFramebuffer = nullptr;
    static PFNGLDELETEFRAMEBUFFERSPROC            glDeleteFramebuffers = nullptr;
    static PFNGLCHECKFRAMEBUFFERSTATUSPROC        glCheckFramebufferStatus = nullptr;
    static PFNGLDRAWBUFFERSPROC                   glDrawBuffers = nullptr;
    static PFNGLBINDFRAGDATALOCATIONPROC          glBindFragDataLocation = nullptr;
    static PFNGLGETFRAGDATALOCATIONPROC           glGetFragDataLocation = nullptr;
    static PFNGLGETRENDERBUFFERPARAMETERIVPROC    glGetRenderbufferParameteriv = nullptr;
    // ARB_debug_output
    static PFNGLDEBUGMESSAGECONTROLPROC           glDebugMessageControl = nullptr;
    static PFNGLDEBUGMESSAGEINSERTPROC            glDebugMessageInsert = nullptr;
    static PFNGLDEBUGMESSAGECALLBACKPROC          glDebugMessageCallback = nullptr;
    static PFNGLGETDEBUGMESSAGELOGPROC            glGetDebugMessageLog = nullptr;
    static PFNGLGETPOINTERVPROC                   glGetPointerv = nullptr;
    static PFNGLMAPBUFFERPROC                     glMapBuffer = nullptr;
    static PFNGLUNMAPBUFFERPROC                   glUnmapBuffer = nullptr;
    static PFNGLGETBUFFERPARAMETERIVPROC          glGetBufferParameteriv = nullptr;
    static PFNGLENABLEVERTEXATTRIBARRAYPROC       glEnableVertexAttribArray = nullptr;
    static PFNGLDISABLEVERTEXATTRIBARRAYPROC      glDisableVertexAttribArray = nullptr;
    static PFNGLVERTEXATTRIBPOINTERPROC           glVertexAttribPointer = nullptr;
    static PFNGLUNIFORM1UIPROC                    glUniform1ui = nullptr;
    static PFNGLDRAWARRAYSINSTANCEDPROC           glDrawArraysInstanced = nullptr;
    static PFNGLFRAMEBUFFERPARAMETERIPROC         glFramebufferParameteri = nullptr;
#endif

OpenGLRenderer::OpenGLRenderer() try :
        mDeviceContext ( nullptr ),
                       mOpenGLContext ( nullptr )
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

    bool OpenGLRenderer::InitializeRenderingWindow ( HINSTANCE aInstance, HWND aHwnd )
    {
        PIXELFORMATDESCRIPTOR pfd;
        PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = nullptr;
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
        mDeviceContext = GetDC ( aHwnd );
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cRedBits = 0;
        pfd.cRedShift = 0;
        pfd.cGreenBits = 0;
        pfd.cGreenShift = 0;
        pfd.cBlueBits = 0;
        pfd.cBlueShift = 0;
        pfd.cAlphaBits = 0;
        pfd.cAlphaShift = 0;
        pfd.cAccumBits = 0;
        pfd.cAccumRedBits = 0;
        pfd.cAccumGreenBits = 0;
        pfd.cAccumBlueBits = 0;
        pfd.cAccumAlphaBits = 0;
        pfd.cDepthBits = 32;
        pfd.cStencilBits = 0;
        pfd.cAuxBuffers = 0;
        pfd.iLayerType = PFD_MAIN_PLANE;
        pfd.bReserved = 0;
        pfd.dwLayerMask = 0;
        pfd.dwVisibleMask = 0;
        pfd.dwDamageMask = 0;
        int pf = ChoosePixelFormat ( mDeviceContext, &pfd );
        SetPixelFormat ( mDeviceContext, pf, &pfd );
        mOpenGLContext = wglCreateContext ( mDeviceContext );
        wglMakeCurrent ( mDeviceContext, mOpenGLContext );

        //---OpenGL 3.2 Context---//
        wglGetExtensionsStringARB = ( PFNWGLGETEXTENSIONSSTRINGARBPROC ) wglGetProcAddress ( "wglGetExtensionsStringARB" );
        if ( wglGetExtensionsStringARB != nullptr )
        {
            if ( strstr ( wglGetExtensionsStringARB ( mDeviceContext ), "WGL_ARB_create_context" ) != nullptr )
            {
                const int ctxAttribs[] =
                {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                    WGL_CONTEXT_PROFILE_MASK_ARB,
                    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    0
                };

                wglCreateContextAttribsARB = ( PFNWGLCREATECONTEXTATTRIBSARBPROC ) wglGetProcAddress ( "wglCreateContextAttribsARB" );
                wglMakeCurrent ( mDeviceContext, nullptr );
                wglDeleteContext ( mOpenGLContext );
                mOpenGLContext = wglCreateContextAttribsARB ( mDeviceContext, nullptr, ctxAttribs );
                wglMakeCurrent ( mDeviceContext, mOpenGLContext );
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    void OpenGLRenderer::FinalizeRenderingWindow ( HINSTANCE aInstance, HWND aHwnd )
    {
        wglMakeCurrent ( mDeviceContext, nullptr );
        wglDeleteContext ( mOpenGLContext );
        ReleaseDC ( aHwnd, mDeviceContext );
    }

    void OpenGLRenderer::Initialize()
    {
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof ( WNDCLASSEX );
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = ( WNDPROC ) DefWindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = GetModuleHandle ( nullptr );
        wcex.hIcon = LoadIcon ( nullptr, IDI_WINLOGO );
        wcex.hCursor = LoadCursor ( nullptr, IDC_ARROW );
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = "AeonEngineTmpWnd";
        wcex.hIconSm = nullptr;
        ATOM atom = RegisterClassEx ( &wcex );
        HWND hWnd = CreateWindowEx ( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
                                     MAKEINTATOM ( atom ), "AeonEngine Temporary Window",
                                     WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                     0, 0, // Location
                                     10, 10, // dimensions
                                     nullptr,
                                     nullptr,
                                     GetModuleHandle ( nullptr ),
                                     nullptr );
        if ( !InitializeRenderingWindow ( GetModuleHandle ( nullptr ), hWnd ) )
        {
            DestroyWindow ( hWnd );
            UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                                  0x0ULL +
#endif
                                  MAKELONG ( atom, 0 ) ), nullptr );
            throw std::runtime_error ( "Unable to Initialize Rendering Window." );
        }
#ifdef ANDROID
        // GL_OES_vertex_array_object
        if ( strstr ( ( char const* ) glGetString ( GL_EXTENSIONS ), "GL_OES_vertex_array_object" ) != nullptr )
        {
            AEONGAMES_LOG_INFO ( "OpenGL ES: GL_OES_vertex_array_object supported" );
            GLGETPROCADDRESS ( PFNGLGENVERTEXARRAYSOESPROC, glGenVertexArrays );
            GLGETPROCADDRESS ( PFNGLBINDVERTEXARRAYOESPROC, glBindVertexArray );
            GLGETPROCADDRESS ( PFNGLDELETEVERTEXARRAYSOESPROC, glDeleteVertexArrays );
            GLGETPROCADDRESS ( PFNGLISVERTEXARRAYOESPROC, glIsVertexArray );
        }
        else
        {
            throw std::rutime_error ( "OpenGL ES: GL_OES_vertex_array_object NOT supported" );
        }
#else
        GLGETPROCADDRESS ( PFNGLGETSTRINGIPROC, glGetStringi );
        GLGETPROCADDRESS ( PFNGLATTACHSHADERPROC, glAttachShader );
        GLGETPROCADDRESS ( PFNGLCOMPILESHADERPROC, glCompileShader );
        GLGETPROCADDRESS ( PFNGLCREATEPROGRAMPROC, glCreateProgram );
        GLGETPROCADDRESS ( PFNGLISPROGRAMPROC, glIsProgram );
        GLGETPROCADDRESS ( PFNGLCREATESHADERPROC, glCreateShader );
        GLGETPROCADDRESS ( PFNGLISSHADERPROC, glIsShader );
        GLGETPROCADDRESS ( PFNGLDELETEPROGRAMPROC, glDeleteProgram );
        GLGETPROCADDRESS ( PFNGLDELETESHADERPROC, glDeleteShader );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB1FPROC, glVertexAttrib1f );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB1SPROC, glVertexAttrib1s );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB1DPROC, glVertexAttrib1d );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI1IPROC, glVertexAttribI1i );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI1UIPROC, glVertexAttribI1ui );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB2FPROC, glVertexAttrib2f );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB2SPROC, glVertexAttrib2s );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB2DPROC, glVertexAttrib2d );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI2IPROC, glVertexAttribI2i );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI2UIPROC, glVertexAttribI2ui );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB3FPROC, glVertexAttrib3f );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB3SPROC, glVertexAttrib3s );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB3DPROC, glVertexAttrib3d );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI3IPROC, glVertexAttribI3i );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI3UIPROC, glVertexAttribI3ui );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB4FPROC, glVertexAttrib4f );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB4SPROC, glVertexAttrib4s );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB4DPROC, glVertexAttrib4d );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIB4NUBPROC, glVertexAttrib4Nub );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI4IPROC, glVertexAttribI4i );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBI4UIPROC, glVertexAttribI4ui );
        GLGETPROCADDRESS ( PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation );
        GLGETPROCADDRESS ( PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation );
        GLGETPROCADDRESS ( PFNGLGETPROGRAMIVPROC, glGetProgramiv );
        GLGETPROCADDRESS ( PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog );
        GLGETPROCADDRESS ( PFNGLGETSHADERIVPROC, glGetShaderiv );
        GLGETPROCADDRESS ( PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog );
        GLGETPROCADDRESS ( PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation );
        GLGETPROCADDRESS ( PFNGLGETACTIVEUNIFORMPROC, glGetActiveUniform );
        GLGETPROCADDRESS ( PFNGLGETACTIVEUNIFORMNAMEPROC, glGetActiveUniformName );
        GLGETPROCADDRESS ( PFNGLLINKPROGRAMPROC, glLinkProgram );
        GLGETPROCADDRESS ( PFNGLSHADERSOURCEPROC, glShaderSource );
        GLGETPROCADDRESS ( PFNGLUSEPROGRAMPROC, glUseProgram );
        GLGETPROCADDRESS ( PFNGLUNIFORM1IPROC, glUniform1i );
        GLGETPROCADDRESS ( PFNGLUNIFORMMATRIX2FVPROC, glUniformMatrix2fv );
        GLGETPROCADDRESS ( PFNGLUNIFORMMATRIX3FVPROC, glUniformMatrix3fv );
        GLGETPROCADDRESS ( PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv );
        GLGETPROCADDRESS ( PFNGLGENBUFFERSPROC, glGenBuffers );
        GLGETPROCADDRESS ( PFNGLBINDBUFFERPROC, glBindBuffer );
        GLGETPROCADDRESS ( PFNGLBUFFERDATAPROC, glBufferData );
        GLGETPROCADDRESS ( PFNGLGETBUFFERSUBDATAPROC, glGetBufferSubData );
        GLGETPROCADDRESS ( PFNGLDELETEBUFFERSPROC, glDeleteBuffers );
        GLGETPROCADDRESS ( PFNGLGETACTIVEATTRIBPROC, glGetActiveAttrib );
        // GL_ARB_vertex_array_object
        GLGETPROCADDRESS ( PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays );
        GLGETPROCADDRESS ( PFNGLBINDVERTEXARRAYPROC, glBindVertexArray );
        GLGETPROCADDRESS ( PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays );
        GLGETPROCADDRESS ( PFNGLISVERTEXARRAYPROC, glIsVertexArray );
        GLGETPROCADDRESS ( PFNGLUNIFORM1FVPROC, glUniform1fv );
        GLGETPROCADDRESS ( PFNGLUNIFORM2FVPROC, glUniform2fv );
        GLGETPROCADDRESS ( PFNGLUNIFORM3FVPROC, glUniform3fv );
        GLGETPROCADDRESS ( PFNGLUNIFORM3FPROC, glUniform3f );
        GLGETPROCADDRESS ( PFNGLUNIFORM4FVPROC, glUniform4fv );
        GLGETPROCADDRESS ( PFNGLUNIFORM1IVPROC, glUniform1iv );
        GLGETPROCADDRESS ( PFNGLUNIFORM2IVPROC, glUniform2iv );
        GLGETPROCADDRESS ( PFNGLUNIFORM3IVPROC, glUniform3iv );
        GLGETPROCADDRESS ( PFNGLUNIFORM4IVPROC, glUniform4iv );
        GLGETPROCADDRESS ( PFNGLUNIFORM1UIVPROC, glUniform1uiv );
        GLGETPROCADDRESS ( PFNGLUNIFORM2UIVPROC, glUniform2uiv );
        GLGETPROCADDRESS ( PFNGLUNIFORM3UIVPROC, glUniform3uiv );
        GLGETPROCADDRESS ( PFNGLUNIFORM4UIVPROC, glUniform4uiv );
        GLGETPROCADDRESS ( PFNGLGETUNIFORMINDICESPROC, glGetUniformIndices );
        GLGETPROCADDRESS ( PFNGLCOMPRESSEDTEXIMAGE2DPROC, glCompressedTexImage2D );
        GLGETPROCADDRESS ( PFNGLDETACHSHADERPROC, glDetachShader );
        GLGETPROCADDRESS ( PFNGLISBUFFERPROC, glIsBuffer );
        GLGETPROCADDRESS ( PFNGLACTIVETEXTUREPROC, glActiveTexture );

        GLGETPROCADDRESS ( PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers );
        GLGETPROCADDRESS ( PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer );
        GLGETPROCADDRESS ( PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage );
        GLGETPROCADDRESS ( PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer );
        GLGETPROCADDRESS ( PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers );

        GLGETPROCADDRESS ( PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers );
        GLGETPROCADDRESS ( PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer );
        GLGETPROCADDRESS ( PFNGLISRENDERBUFFERPROC, glIsRenderbuffer );
        GLGETPROCADDRESS ( PFNGLISFRAMEBUFFERPROC, glIsFramebuffer );
        GLGETPROCADDRESS ( PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers );
        GLGETPROCADDRESS ( PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus );
        GLGETPROCADDRESS ( PFNGLDRAWBUFFERSPROC, glDrawBuffers );
        GLGETPROCADDRESS ( PFNGLBINDFRAGDATALOCATIONPROC, glBindFragDataLocation );
        GLGETPROCADDRESS ( PFNGLGETFRAGDATALOCATIONPROC, glGetFragDataLocation );
        GLGETPROCADDRESS ( PFNGLGETRENDERBUFFERPARAMETERIVPROC, glGetRenderbufferParameteriv );
        // ARB_debug_output
        GLGETPROCADDRESS ( PFNGLDEBUGMESSAGECONTROLPROC, glDebugMessageControl );
        GLGETPROCADDRESS ( PFNGLDEBUGMESSAGEINSERTPROC, glDebugMessageInsert );
        GLGETPROCADDRESS ( PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback );
        GLGETPROCADDRESS ( PFNGLGETDEBUGMESSAGELOGPROC, glGetDebugMessageLog );
        GLGETPROCADDRESS ( PFNGLGETPOINTERVPROC, glGetPointerv );
        GLGETPROCADDRESS ( PFNGLMAPBUFFERPROC, glMapBuffer );
        GLGETPROCADDRESS ( PFNGLUNMAPBUFFERPROC, glUnmapBuffer );
        GLGETPROCADDRESS ( PFNGLGETBUFFERPARAMETERIVPROC, glGetBufferParameteriv );
        GLGETPROCADDRESS ( PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray );
        GLGETPROCADDRESS ( PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray );
        GLGETPROCADDRESS ( PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer );
        GLGETPROCADDRESS ( PFNGLUNIFORM1UIPROC, glUniform1ui );
        GLGETPROCADDRESS ( PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced );
        GLGETPROCADDRESS ( PFNGLFRAMEBUFFERPARAMETERIPROC, glFramebufferParameteri );

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
        FinalizeRenderingWindow ( GetModuleHandle ( nullptr ), hWnd );
        DestroyWindow ( hWnd );
        UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                              0x0ULL +
#endif
                              MAKELONG ( atom, 0 ) ), nullptr );
#endif
    }

    void OpenGLRenderer::Finalize()
    {
    }
}
