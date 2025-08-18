/*
Copyright (C) 2016-2022,2024,2025 Rodrigo Jose Hernandez Cordoba

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

#include <atomic>
#include <cassert>
#include "OpenGLRenderer.h"
#include "OpenGLBuffer.h"
#include "OpenGLWindow.h"
#include "OpenGLFunctions.h"
#include "aeongames/Platform.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/LogLevel.hpp"

namespace AeonGames
{
    std::atomic<size_t> OpenGLRenderer::mRendererCount{0};
#if defined(__unix__)
    Display* OpenGLRenderer::mDisplay {};
#endif
    const GLchar vertex_shader_code[] =
        R"(#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 Pos;
out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    Pos = aPos;
    TexCoords = aTexCoords;
}
)";
    const GLint vertex_shader_len { sizeof(vertex_shader_code) /*/ sizeof(vertex_shader_code[0])*/};
    const GLchar* const vertex_shader_code_ptr = vertex_shader_code;

    const GLchar fragment_shader_code[] =
        R"(#version 450 core
out vec4 FragColor;
  
in vec2 Pos;
in vec2 TexCoords;

layout (location = 0) uniform sampler2D OverlayTexture;

void main()
{ 
    FragColor = texture(OverlayTexture, TexCoords);
}
)";

    const GLint fragment_shader_len { sizeof(fragment_shader_code) /*/ sizeof(fragment_shader_code[0])*/};
    const GLchar* const fragment_shader_code_ptr = fragment_shader_code;

    const float vertices[] = {
                                 // positions   // texCoords
                                 -1.0f,  1.0f,  0.0f, 1.0f,
                                 -1.0f, -1.0f,  0.0f, 0.0f,
                                 1.0f, -1.0f,  1.0f, 0.0f,
                                 1.0f,  1.0f,  1.0f, 1.0f
                             };
    constexpr GLuint vertex_size{sizeof(vertices)};

#ifdef _WIN32
    static PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsString = nullptr;
    static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs = nullptr;
    const int ContextAttribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 5,
            WGL_CONTEXT_PROFILE_MASK_ARB,
            WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

    static ATOM gRendererWindowClass{0};
    static std::atomic<size_t> mRendererCount{0};

    static HWND CreateRendererWindow()
    {
        RECT rect = { 0, 0, 10, 10 };
        if(gRendererWindowClass == 0)
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
            wcex.lpszClassName = "AeonEngineOpenGLInternalWindow";
            wcex.hIconSm = nullptr;
            gRendererWindowClass = RegisterClassEx ( &wcex );
        }
        ++mRendererCount;
        return CreateWindowEx ( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
                                MAKEINTATOM ( gRendererWindowClass ), "AeonEngine OpenGL Internal Window",
                                WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                0, 0, // Location
                                rect.right - rect.left, rect.bottom - rect.top, // dimensions
                                nullptr,
                                nullptr,
                                GetModuleHandle ( nullptr ),
                                nullptr );
    }
    static void DestroyRendererWindow(HWND hWnd)
    {
        assert(mRendererCount);
        DestroyWindow ( hWnd );
        if(--mRendererCount == 0)
        {
            UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                                  0x0ULL +
#endif
                                  MAKELONG ( gRendererWindowClass, 0 ) ), nullptr );
            gRendererWindowClass = 0;
        }
    }
    OpenGLRenderer::OpenGLRenderer(void* aWindow) :
        mWindowId{CreateRendererWindow()},
        mDeviceContext{GetDC(mWindowId)}
    {
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof ( PIXELFORMATDESCRIPTOR );
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int pf = ChoosePixelFormat ( mDeviceContext, &pfd );
        SetPixelFormat ( mDeviceContext, pf, &pfd );

        // Create OpenGL Context
        mOpenGLContext = wglCreateContext ( mDeviceContext );
        MakeCurrent();

        // Get newer functions if needed
        if ( !wglGetExtensionsString )
        {
            if ( ! ( wglGetExtensionsString = ( PFNWGLGETEXTENSIONSSTRINGARBPROC ) wglGetProcAddress ( "wglGetExtensionsStringARB" ) ) )
            {
                wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
                mOpenGLContext = nullptr;
                ReleaseDC ( mWindowId, mDeviceContext );
                DestroyRendererWindow ( mWindowId );
                throw std::runtime_error ( "Failed retrieving a pointer to wglGetExtensionsString" );
            }
        }
        if ( !wglCreateContextAttribs )
        {
            if ( ! ( wglCreateContextAttribs = ( PFNWGLCREATECONTEXTATTRIBSARBPROC ) wglGetProcAddress ( "wglCreateContextAttribsARB" ) ) )
            {
                wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
                mOpenGLContext = nullptr;
                ReleaseDC ( mWindowId, mDeviceContext );
                DestroyRendererWindow ( mWindowId );
                throw std::runtime_error ( "Failed retrieving a pointer to wglCreateContextAttribsARB" );
            }
        }
        if ( strstr ( wglGetExtensionsString ( mDeviceContext ), "WGL_ARB_create_context" ) != nullptr )
        {
            wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
            if ( ! ( mOpenGLContext = wglCreateContextAttribs ( mDeviceContext, nullptr, ContextAttribs ) ) )
            {
                ReleaseDC ( mWindowId, mDeviceContext );
                DestroyRendererWindow ( mWindowId );
                throw std::runtime_error ( "wglCreateContextAttribs Failed" );
            }
        }
        else
        {
            wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
            mOpenGLContext = nullptr;
            ReleaseDC ( mWindowId, mDeviceContext );
            DestroyRendererWindow ( mWindowId );
            throw std::runtime_error ( "WGL_ARB_create_context is not available" );
        }
        // Make OpenGL Context current.
        MakeCurrent();
        if ( !LoadOpenGLAPI() )
        {
            throw std::runtime_error ( "Unable to Load OpenGL functions." );
        }
        glGenVertexArrays ( 1, &mVertexArrayObject );
        glBindVertexArray ( mVertexArrayObject );
        AttachWindow ( static_cast<HWND> ( aWindow ) );
    }

    bool OpenGLRenderer::MakeCurrent(HDC aDeviceContext)
    {
        if ( !wglMakeCurrent ( (aDeviceContext==nullptr) ? mDeviceContext : aDeviceContext, mOpenGLContext ) )
        {
            LPSTR pBuffer = NULL;
            FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                            nullptr, GetLastError(), MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPSTR ) &pBuffer, 0, nullptr );
            if ( pBuffer != nullptr )
            {
                std::cout << pBuffer << std::endl;
                LocalFree ( pBuffer );
            }
            return false;
        }
        return true;
    }

    OpenGLRenderer::~OpenGLRenderer()
    {
        mWindowStore.clear();
        MakeCurrent();
        mTextureStore.clear();
        mMeshStore.clear();
        mMaterialStore.clear();
        mPipelineStore.clear();
        wglMakeCurrent (nullptr, nullptr);
        if ( wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) ) != TRUE )
        {
            std::cout << LogLevel::Error << "wglDeleteContext failed." << std::endl;
        }
        mOpenGLContext = nullptr;
        ReleaseDC ( mWindowId, mDeviceContext );
        DestroyRendererWindow ( mWindowId );
        mDeviceContext = nullptr;
        mWindowId = nullptr;
    }
#elif defined(__unix__)
    static int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 5,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };

    static GLXFBConfig GetGLXConfig ( Display* display, ::Window window)
    {
        XWindowAttributes xwa{};
        XGetWindowAttributes(display, window, &xwa);
        VisualID xwvid = XVisualIDFromVisual(xwa.visual);

        int frame_buffer_config_count{};
        GLXFBConfig *frame_buffer_configs =
            glXGetFBConfigs ( display,
                              DefaultScreen ( display ),
                              &frame_buffer_config_count );
        if ( !frame_buffer_configs )
        {
            throw std::runtime_error ( "Failed to retrieve a framebuffer config" );
        }

        (void) std::remove_if(frame_buffer_configs, frame_buffer_configs + frame_buffer_config_count,
                              [display,xwvid] ( const GLXFBConfig & x ) -> bool
                              {
                                  XVisualInfo *xvi = glXGetVisualFromFBConfig(display, x);
                                  if(xvi && xvi->visualid == xwvid)
    {
        XFree(xvi);
            return false;
        }
        XFree(xvi);
        return true;
                              });

        GLXFBConfig result = frame_buffer_configs[ 0 ];
        XFree ( frame_buffer_configs );
        return result;
    }

    OpenGLRenderer::OpenGLRenderer(void* aWindow)
    {
        if(mRendererCount == 0)
        {
            XSetErrorHandler ( [] ( Display * mDisplay, XErrorEvent * error_event ) -> int
                               {
                                   char error_string[1024];
                                   XGetErrorText ( mDisplay, error_event->error_code, error_string, 1024 );
                                   std::cout << AeonGames::LogLevel::Error << error_string << std::endl;
                                   std::cout << AeonGames::LogLevel::Error << "Error Code " << static_cast<int> ( error_event->error_code ) << std::endl;
                                   std::cout << AeonGames::LogLevel::Error << "Request Code " << static_cast<int> ( error_event->request_code ) << std::endl;
                                   std::cout << AeonGames::LogLevel::Error << "Minor Code " << static_cast<int> ( error_event->minor_code ) << std::endl;
                                   std::cout << AeonGames::LogLevel::Error << "Display " << error_event->display << std::endl;
                                   std::cout << AeonGames::LogLevel::Error << "Resource Id " << error_event->resourceid << std::endl;
                                   std::cout << AeonGames::LogLevel::Error << "Serial " << error_event->serial << std::endl;
                                   std::cout << AeonGames::LogLevel::Error << "Type " << error_event->type << std::endl;
                                   return 0;
                               } );
            if(mDisplay){XCloseDisplay(mDisplay);}
            mDisplay = XOpenDisplay ( nullptr );
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

        GLXFBConfig glxconfig = GetGLXConfig(mDisplay,reinterpret_cast<::Window>(aWindow));

        if ( nullptr == ( mOpenGLContext = glXCreateContextAttribsARB ( mDisplay, glxconfig, nullptr,
                                           True, context_attribs ) ) )
        {
            throw std::runtime_error ( "glXCreateContextAttribsARB Failed." );
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

        if ( !MakeCurrent(reinterpret_cast<::Window>(reinterpret_cast<::Window>(aWindow))) )
        {
            throw std::runtime_error ( "glXMakeCurrent failed." );
        }

        if ( !LoadOpenGLAPI() )
        {
            throw std::runtime_error ( "Unable to Load OpenGL functions." );
        }

        glGenVertexArrays ( 1, &mVertexArrayObject );
        glBindVertexArray ( mVertexArrayObject );

        AttachWindow(aWindow);
        ++mRendererCount;
    }


    bool OpenGLRenderer::MakeCurrent(::Window aWindowId)
    {
        return glXMakeCurrent (mDisplay, (aWindowId) ? aWindowId : None , (aWindowId) ? mOpenGLContext : nullptr );
    }

    OpenGLRenderer::~OpenGLRenderer()
    {
        mWindowStore.clear();
        MakeCurrent();
        mTextureStore.clear();
        mMeshStore.clear();
        mMaterialStore.clear();
        mPipelineStore.clear();
        if ( mOpenGLContext != None )
        {
            glXDestroyContext ( mDisplay, mOpenGLContext );
            mOpenGLContext = None;
        }
        if(--mRendererCount == 0)
        {
            XCloseDisplay(mDisplay);
            mDisplay = None;
        }
    }
#endif

    void OpenGLRenderer::InitializeOverlay()
    {
        /* Initialize Overlay Program. Consider moving into a separate function. */
        GLint compile_status{};
        mOverlayProgram = glCreateProgram();
        OPENGL_CHECK_ERROR_THROW;
        GLuint vertex_shader = glCreateShader ( GL_VERTEX_SHADER );
        OPENGL_CHECK_ERROR_THROW;
        glShaderSource (vertex_shader,1,&vertex_shader_code_ptr,&vertex_shader_len );
        OPENGL_CHECK_ERROR_THROW;
        glCompileShader ( vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glGetShaderiv ( vertex_shader, GL_COMPILE_STATUS, &compile_status );
        OPENGL_CHECK_ERROR_THROW;
        if ( compile_status != GL_TRUE )
        {
            GLint info_log_len;
            glGetShaderiv ( vertex_shader, GL_INFO_LOG_LENGTH, &info_log_len );
            OPENGL_CHECK_ERROR_THROW;
            std::string log_string;
            log_string.resize ( info_log_len );
            if ( info_log_len > 1 )
            {
                glGetShaderInfoLog ( vertex_shader, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                OPENGL_CHECK_ERROR_THROW;
                std::cout << vertex_shader_code << std::endl;
                std::cout << log_string << std::endl;
            }
        }
        glAttachShader ( mOverlayProgram, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        //-------------------------
        uint32_t fragment_shader = glCreateShader ( GL_FRAGMENT_SHADER );
        OPENGL_CHECK_ERROR_THROW;
        glShaderSource ( fragment_shader, 1, &fragment_shader_code_ptr, &fragment_shader_len );
        OPENGL_CHECK_ERROR_THROW;
        glCompileShader ( fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        glGetShaderiv ( fragment_shader, GL_COMPILE_STATUS, &compile_status );
        OPENGL_CHECK_ERROR_THROW;
        if ( compile_status != GL_TRUE )
        {
            GLint info_log_len;
            glGetShaderiv ( fragment_shader, GL_INFO_LOG_LENGTH, &info_log_len );
            OPENGL_CHECK_ERROR_THROW;
            std::string log_string;
            log_string.resize ( info_log_len );
            if ( info_log_len > 1 )
            {
                glGetShaderInfoLog ( fragment_shader, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                OPENGL_CHECK_ERROR_THROW;
                std::cout << fragment_shader_code << std::endl;
                std::cout << log_string << std::endl;
            }
        }
        glAttachShader ( mOverlayProgram, fragment_shader );
        OPENGL_CHECK_ERROR_THROW;

        glLinkProgram ( mOverlayProgram );
        OPENGL_CHECK_ERROR_THROW;
        glDetachShader ( mOverlayProgram, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDetachShader ( mOverlayProgram, fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDeleteShader ( vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDeleteShader ( fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        glUseProgram(mOverlayProgram);
        OPENGL_CHECK_ERROR_THROW;
        glUniform1i ( 0, 0 );
        OPENGL_CHECK_ERROR_THROW;
        /* End of Overlay Program Initialization. */
        mOverlayQuad.Initialize(vertex_size, GL_STATIC_DRAW, vertices);
    }

    void OpenGLRenderer::FinalizeOverlay()
    {
        OPENGL_CHECK_ERROR_NO_THROW;
        mOverlayQuad.Finalize();
        if(glIsProgram(mOverlayProgram))
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glUseProgram(0);
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteProgram(mOverlayProgram);
            OPENGL_CHECK_ERROR_NO_THROW;
            mOverlayProgram = 0;
        }
        if ( glIsVertexArray ( mVertexArrayObject ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteVertexArrays ( 1, &mVertexArrayObject );
            OPENGL_CHECK_ERROR_NO_THROW;
            mVertexArrayObject = 0;
        }
    }

    void OpenGLRenderer::LoadMesh ( const Mesh& aMesh )
    {
        auto it = mMeshStore.find(aMesh.GetConsecutiveId());
        if(it!=mMeshStore.end())
        {
            std::cout << LogLevel::Warning << "Mesh with id " << aMesh.GetConsecutiveId() << " already loaded." << std::endl;
            return;
        }
        mMeshStore.emplace(aMesh.GetConsecutiveId(),OpenGLMesh{*this,aMesh});
    }

    void OpenGLRenderer::UnloadMesh ( const Mesh& aMesh )
    {
        auto it = mMeshStore.find(aMesh.GetConsecutiveId());
        if(it!=mMeshStore.end())
        {
            mMeshStore.erase(it);
        }
    }

    void OpenGLRenderer::BindMesh(const Mesh& aMesh)
    {
        auto it = mMeshStore.find(aMesh.GetConsecutiveId());
        if(it==mMeshStore.end())
        {
            LoadMesh(aMesh);
            it = mMeshStore.find(aMesh.GetConsecutiveId());
        }
        it->second.Bind();
        GLint currentProgram{0};
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        auto pipeline = std::find_if(mPipelineStore.begin(), mPipelineStore.end(),
        [currentProgram](const auto& pair) {
            return pair.second.GetProgramId() == currentProgram;
        });
        if (pipeline != mPipelineStore.end())
        {
            it->second.EnableAttributes(pipeline->second.GetVertexAttributes());
        }
    }

    void OpenGLRenderer::LoadPipeline(const Pipeline& aPipeline)
    {
        auto it = mPipelineStore.find(aPipeline.GetConsecutiveId());
        if(it!=mPipelineStore.end())
        {
            throw std::runtime_error ( "OpenGL object already loaded." );
        }
        mPipelineStore.emplace(aPipeline.GetConsecutiveId(),OpenGLPipeline{*this,aPipeline});
    }

    void OpenGLRenderer::UnloadPipeline(const Pipeline& aPipeline)
    {
        auto it = mPipelineStore.find(aPipeline.GetConsecutiveId());
        if(it==mPipelineStore.end()){return;};
        mPipelineStore.erase(it);
    }

    void OpenGLRenderer::BindPipeline ( const Pipeline& aPipeline)
    {
        auto it = mPipelineStore.find(aPipeline.GetConsecutiveId());
        if(it==mPipelineStore.end())
        {
            LoadPipeline(aPipeline);
            it = mPipelineStore.find(aPipeline.GetConsecutiveId());
        };

        glUseProgram ( it->second.GetProgramId() );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLRenderer::SetMaterial ( const Material& aMaterial)
    {
        auto it = mMaterialStore.find(aMaterial.GetConsecutiveId());
        if(it==mMaterialStore.end())
        {
            LoadMaterial(aMaterial);
            it = mMaterialStore.find(aMaterial.GetConsecutiveId());
        };
        it->second.Bind();
    }

    void OpenGLRenderer::SetSkeleton ( const BufferAccessor& aSkeletonBuffer) const
    {
        const OpenGLMemoryPoolBuffer* memory_pool_buffer = reinterpret_cast<const OpenGLMemoryPoolBuffer*> ( aSkeletonBuffer.GetMemoryPoolBuffer() );
        if ( GLuint buffer_id = ( memory_pool_buffer != nullptr ) ? reinterpret_cast<const OpenGLBuffer&>(memory_pool_buffer->GetBuffer()).GetBufferId() : 0 )
        {
            glBindBufferRange ( GL_UNIFORM_BUFFER, SKELETON, buffer_id, aSkeletonBuffer.GetOffset(), aSkeletonBuffer.GetSize() );
            OPENGL_CHECK_ERROR_THROW;
        };
    }

    void OpenGLRenderer::LoadMaterial(const Material& aMaterial)
    {
        auto it = mMaterialStore.find(aMaterial.GetConsecutiveId());
        if(it!=mMaterialStore.end()){return;}
        mMaterialStore.emplace(
            aMaterial.GetConsecutiveId(),
            OpenGLMaterial{*this,aMaterial});
    }

    void OpenGLRenderer::UnloadMaterial(const Material& aMaterial)
    {
        auto it = mMaterialStore.find(aMaterial.GetConsecutiveId());
        if(it==mMaterialStore.end()){return;}
        mMaterialStore.erase(it);
    }

    void OpenGLRenderer::LoadTexture(const Texture& aTexture)
    {
        auto it = mTextureStore.find(aTexture.GetConsecutiveId());
        if(it!=mTextureStore.end())
        {
            return;
        }
        mTextureStore.emplace(aTexture.GetConsecutiveId(),OpenGLTexture{*this,aTexture});
    }

    void OpenGLRenderer::UnloadTexture(const Texture& aTexture)
    {
        auto it = mTextureStore.find(aTexture.GetConsecutiveId());
        if(it==mTextureStore.end()){return;}
        mTextureStore.erase(it);
    }

    GLuint OpenGLRenderer::GetTextureId ( const Texture& aTexture )
    {
        auto it = mTextureStore.find(aTexture.GetConsecutiveId());
        if(it==mTextureStore.end())
        {
            LoadTexture(aTexture);
            it = mTextureStore.find(aTexture.GetConsecutiveId());
        }
        return it->second.GetTextureId();
    }

    GLuint OpenGLRenderer::GetVertexArrayObject() const
    {
        return mVertexArrayObject;
    }

    GLuint OpenGLRenderer::GetOverlayProgram() const
    {
        return mOverlayProgram;
    }

    GLuint OpenGLRenderer::GetOverlayQuad() const
    {
        return mOverlayQuad.GetBufferId();
    }

    void OpenGLRenderer::AttachWindow ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it != mWindowStore.end() )
        {
            std::cout << LogLevel::Warning << " Window " << aWindowId << " Already Loaded at: " << __FUNCTION__ << std::endl;
            return;
        }
#if defined(__unix__)
        mWindowStore.emplace ( aWindowId, OpenGLWindow{*this, mDisplay, reinterpret_cast<::Window>(aWindowId)} );
#elif defined(_WIN32)
        mWindowStore.emplace ( aWindowId, OpenGLWindow{*this, reinterpret_cast<::HWND>(aWindowId)} );
#endif
    }
    void OpenGLRenderer::DetachWindow ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        mWindowStore.erase ( it );
    }

    void OpenGLRenderer::SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetProjectionMatrix ( aMatrix );
    }

    void OpenGLRenderer::SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetViewMatrix ( aMatrix );
    }

    void OpenGLRenderer::SetClearColor ( void* aWindowId, float R, float G, float B, float A )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetClearColor ( R, G, B, A );
    }
    void OpenGLRenderer::ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.ResizeViewport ( aX, aY, aWidth, aHeight );
    }

    void OpenGLRenderer::BeginRender ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginRender();
    }
    void OpenGLRenderer::EndRender ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndRender();
    }
    void OpenGLRenderer::Render ( void* aWindowId,
                                  const Matrix4x4& aModelMatrix,
                                  const Mesh& aMesh,
                                  const Pipeline& aPipeline,
                                  const Material* aMaterial,
                                  const BufferAccessor* aSkeleton,
                                  Topology aTopology,
                                  uint32_t aVertexStart,
                                  uint32_t aVertexCount,
                                  uint32_t aInstanceCount,
                                  uint32_t aFirstInstance ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Render ( aModelMatrix, aMesh, aPipeline, aMaterial, aSkeleton, aTopology, aVertexStart, aVertexCount, aInstanceCount, aFirstInstance );
    }

    const Frustum& OpenGLRenderer::GetFrustum ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.GetFrustum();
    }
    BufferAccessor OpenGLRenderer::AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.AllocateSingleFrameUniformMemory ( aSize );
    }

    void* OpenGLRenderer::GetContext() const
    {
        return mOpenGLContext;
    }
}
