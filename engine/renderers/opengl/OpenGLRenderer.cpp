/*
Copyright (C) 2016-2022,2024,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include "OpenGLRenderer.hpp"
#include "OpenGLBuffer.hpp"
#include "OpenGLWindow.hpp"
#include "OpenGLFunctions.hpp"
#include "aeongames/Platform.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/GuiOverlay.hpp"
#include "aeongames/Scene.hpp"

namespace AeonGames
{
    std::atomic<size_t> OpenGLRenderer::mRendererCount{0};
#if defined(__unix__)
    Display* OpenGLRenderer::mDisplay {};
#endif
    /// @brief Overlay vertex shader GLSL source code.
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
    /// @brief Length of the overlay vertex shader source.
    const GLint vertex_shader_len { sizeof ( vertex_shader_code ) /*/ sizeof(vertex_shader_code[0])*/};
    /// @brief Pointer to the overlay vertex shader source.
    const GLchar* const vertex_shader_code_ptr = vertex_shader_code;

    /// @brief Overlay fragment shader GLSL source code.
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

    /// @brief Length of the overlay fragment shader source.
    const GLint fragment_shader_len { sizeof ( fragment_shader_code ) /*/ sizeof(fragment_shader_code[0])*/};
    /// @brief Pointer to the overlay fragment shader source.
    const GLchar* const fragment_shader_code_ptr = fragment_shader_code;

    /// @brief Overlay screen-quad vertex data (positions and texture coordinates).
    const float vertices[] =
        {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 0.0f,
            -1.0f, -1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 1.0f,
            1.0f,  1.0f,  1.0f, 0.0f
        };
    /// @brief Total byte size of the overlay vertex data.
    constexpr GLuint vertex_size{sizeof ( vertices ) };

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
        if ( gRendererWindowClass == 0 )
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
    static void DestroyRendererWindow ( HWND hWnd )
    {
        assert ( mRendererCount );
        DestroyWindow ( hWnd );
        if ( --mRendererCount == 0 )
        {
            UnregisterClass ( reinterpret_cast<LPCSTR> (
#if defined(_M_X64) || defined(__amd64__)
                                  0x0ULL +
#endif
                                  MAKELONG ( gRendererWindowClass, 0 ) ), nullptr );
            gRendererWindowClass = 0;
        }
    }
    OpenGLRenderer::OpenGLRenderer ( void* aWindow ) :
        mWindowId{CreateRendererWindow() },
        mDeviceContext{GetDC ( mWindowId ) }
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
                std::cout << LogLevel::Error << "Failed retrieving a pointer to wglGetExtensionsString" << std::endl;
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
                std::cout << LogLevel::Error << "Failed retrieving a pointer to wglCreateContextAttribsARB" << std::endl;
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
                std::cout << LogLevel::Error << "wglCreateContextAttribs Failed" << std::endl;
                throw std::runtime_error ( "wglCreateContextAttribs Failed" );
            }
        }
        else
        {
            wglDeleteContext ( static_cast<HGLRC> ( mOpenGLContext ) );
            mOpenGLContext = nullptr;
            ReleaseDC ( mWindowId, mDeviceContext );
            DestroyRendererWindow ( mWindowId );
            std::cout << LogLevel::Error << "WGL_ARB_create_context is not available" << std::endl;
            throw std::runtime_error ( "WGL_ARB_create_context is not available" );
        }
        // Make OpenGL Context current.
        MakeCurrent();
        if ( !LoadOpenGLAPI() )
        {
            std::cout << LogLevel::Error << "Unable to Load OpenGL functions." << std::endl;
            throw std::runtime_error ( "Unable to Load OpenGL functions." );
        }
        LogCapabilities();
        glGenVertexArrays ( 1, &mVertexArrayObject );
        glBindVertexArray ( mVertexArrayObject );
        InitializeOverlay();
        InitializeBindlessMaterials();
        AttachWindow ( static_cast<HWND> ( aWindow ) );
    }

    bool OpenGLRenderer::MakeCurrent ( HDC aDeviceContext )
    {
        if ( !wglMakeCurrent ( ( aDeviceContext == nullptr ) ? mDeviceContext : aDeviceContext, mOpenGLContext ) )
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
        FinalizeOverlay();
        mTextureStore.clear();
        mMeshStore.clear();
        FinalizeGeometry();
        mMaterialStore.clear();
        mMaterialStorageBuffer.Finalize();
        mPipelineStore.clear();
        wglMakeCurrent ( nullptr, nullptr );
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

    static GLXFBConfig GetGLXConfig ( Display* display, ::Window window )
    {
        XWindowAttributes xwa{};
        XGetWindowAttributes ( display, window, &xwa );
        VisualID xwvid = XVisualIDFromVisual ( xwa.visual );

        int frame_buffer_config_count{};
        GLXFBConfig *frame_buffer_configs =
            glXGetFBConfigs ( display,
                              DefaultScreen ( display ),
                              &frame_buffer_config_count );
        if ( !frame_buffer_configs )
        {
            std::cout << LogLevel::Error << "Failed to retrieve a framebuffer config" << std::endl;
            throw std::runtime_error ( "Failed to retrieve a framebuffer config" );
        }

        ( void ) std::remove_if ( frame_buffer_configs, frame_buffer_configs + frame_buffer_config_count,
                                  [display, xwvid] ( const GLXFBConfig & x ) -> bool
                                  {
                                      XVisualInfo *xvi = glXGetVisualFromFBConfig ( display, x );
                                      if ( xvi && xvi->visualid == xwvid )
    {
        XFree ( xvi );
            return false;
        }
        XFree ( xvi );
        return true;
                                  } );

        GLXFBConfig result = frame_buffer_configs[ 0 ];
        XFree ( frame_buffer_configs );
        return result;
    }

    OpenGLRenderer::OpenGLRenderer ( void* aWindow )
    {
        if ( mRendererCount == 0 )
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
            if ( mDisplay )
            {
                XCloseDisplay ( mDisplay );
            }
            mDisplay = XOpenDisplay ( nullptr );
        }

        // Retrieve Create Context function
        if ( !glXCreateContextAttribsARB )
        {
            if ( ! ( glXCreateContextAttribsARB =
                         ( PFNGLXCREATECONTEXTATTRIBSARBPROC )
                         glXGetProcAddressARB ( ( const GLubyte * ) "glXCreateContextAttribsARB" ) ) )
            {
                std::cout << LogLevel::Error << "Failed retrieving glXCreateContextAttribsARB." << std::endl;
                throw std::runtime_error ( "Failed retrieving glXCreateContextAttribsARB." );
            }
        }

        GLXFBConfig glxconfig = GetGLXConfig ( mDisplay, reinterpret_cast<::Window> ( aWindow ) );

        if ( nullptr == ( mOpenGLContext = glXCreateContextAttribsARB ( mDisplay, glxconfig, nullptr,
                                           True, context_attribs ) ) )
        {
            std::cout << LogLevel::Error << "glXCreateContextAttribsARB Failed." << std::endl;
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

        if ( !MakeCurrent ( reinterpret_cast<::Window> ( reinterpret_cast<::Window> ( aWindow ) ) ) )
        {
            std::cout << LogLevel::Error << "glXMakeCurrent failed." << std::endl;
            throw std::runtime_error ( "glXMakeCurrent failed." );
        }

        if ( !LoadOpenGLAPI() )
        {
            std::cout << LogLevel::Error << "Unable to Load OpenGL functions." << std::endl;
            throw std::runtime_error ( "Unable to Load OpenGL functions." );
        }

        LogCapabilities();
        glGenVertexArrays ( 1, &mVertexArrayObject );
        glBindVertexArray ( mVertexArrayObject );
        InitializeOverlay();
        InitializeBindlessMaterials();

        AttachWindow ( aWindow );
        ++mRendererCount;
    }


    bool OpenGLRenderer::MakeCurrent ( ::Window aWindowId )
    {
        return glXMakeCurrent ( mDisplay, ( aWindowId ) ? aWindowId : None, ( aWindowId ) ? mOpenGLContext : nullptr );
    }

    OpenGLRenderer::~OpenGLRenderer()
    {
        mWindowStore.clear();
        MakeCurrent();
        FinalizeOverlay();
        mTextureStore.clear();
        mMeshStore.clear();
        FinalizeGeometry();
        mMaterialStore.clear();
        mMaterialStorageBuffer.Finalize();
        mPipelineStore.clear();
        if ( mOpenGLContext != None )
        {
            glXDestroyContext ( mDisplay, mOpenGLContext );
            mOpenGLContext = None;
        }
        if ( --mRendererCount == 0 )
        {
            XCloseDisplay ( mDisplay );
            mDisplay = None;
        }
    }
#endif

    void OpenGLRenderer::LogCapabilities()
    {
        GLint major = 0;
        GLint minor = 0;
        glGetIntegerv ( GL_MAJOR_VERSION, &major );
        glGetIntegerv ( GL_MINOR_VERSION, &minor );
        const auto has_extension = [] ( const char* aName ) -> bool
                                   {
                                       GLint count = 0;
                                       glGetIntegerv ( GL_NUM_EXTENSIONS, &count );
                                       for ( GLint i = 0; i < count; ++i )
    {
        const GLubyte* extension = glGetStringi ( GL_EXTENSIONS, static_cast<GLuint> ( i ) );
            if ( extension != nullptr && std::strcmp ( reinterpret_cast<const char*> ( extension ), aName ) == 0 )
            {
                return true;
            }
        }
        return false;
                                   };
        mHasBindlessTexture = has_extension ( "GL_ARB_bindless_texture" );
        mHasIndirectParameters = has_extension ( "GL_ARB_indirect_parameters" );
        mHasComputeShader = ( major > 4 ) || ( major == 4 && minor >= 3 );
        std::cout << LogLevel::Info << "OpenGLRenderer version " << major << "." << minor
                  << " | bindless_texture: " << ( mHasBindlessTexture ? "yes" : "no" )
                  << " | indirect_parameters: " << ( mHasIndirectParameters ? "yes" : "no" )
                  << " | compute(4.3+): " << ( mHasComputeShader ? "yes" : "no" ) << std::endl;
        if ( !mHasBindlessTexture )
        {
            std::cout << LogLevel::Warning << "OpenGLRenderer: GL_ARB_bindless_texture unavailable; the bindless texture path will be disabled." << std::endl;
        }
        if ( !mHasIndirectParameters )
        {
            std::cout << LogLevel::Warning << "OpenGLRenderer: GL_ARB_indirect_parameters unavailable; the GPU-driven indirect-count path will be disabled." << std::endl;
        }
        if ( !mHasComputeShader )
        {
            std::cout << LogLevel::Warning << "OpenGLRenderer: compute shaders (GL 4.3+) unavailable; GPU culling will be disabled." << std::endl;
        }
    }

    void OpenGLRenderer::InitializeOverlay()
    {
        /* Initialize Overlay Program. Consider moving into a separate function. */
        GLint compile_status{};
        mOverlayProgram = glCreateProgram();
        OPENGL_CHECK_ERROR_THROW;
        GLuint vertex_shader = glCreateShader ( GL_VERTEX_SHADER );
        OPENGL_CHECK_ERROR_THROW;
        glShaderSource ( vertex_shader, 1, &vertex_shader_code_ptr, &vertex_shader_len );
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
        glUseProgram ( mOverlayProgram );
        OPENGL_CHECK_ERROR_THROW;
        glUniform1i ( 0, 0 );
        OPENGL_CHECK_ERROR_THROW;
        /* End of Overlay Program Initialization. */
        mOverlayQuad.Initialize ( vertex_size, GL_STATIC_DRAW, vertices );
    }

    void OpenGLRenderer::FinalizeOverlay()
    {
        OPENGL_CHECK_ERROR_NO_THROW;
        for ( auto& [window_id, overlay] : mOverlayTextureCache )
        {
            ( void ) window_id;
            if ( glIsTexture ( overlay.texture ) )
            {
                OPENGL_CHECK_ERROR_NO_THROW;
                glDeleteTextures ( 1, &overlay.texture );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
        }
        mOverlayTextureCache.clear();
        mOverlayQuad.Finalize();
        if ( glIsProgram ( mOverlayProgram ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glUseProgram ( 0 );
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteProgram ( mOverlayProgram );
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
        auto it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        if ( it != mMeshStore.end() )
        {
            std::cout << LogLevel::Warning << "Mesh with id " << aMesh.GetConsecutiveId() << " already loaded." << std::endl;
            return;
        }
        mMeshStore.emplace ( aMesh.GetConsecutiveId(), OpenGLMesh{*this, aMesh} );
    }

    void OpenGLRenderer::UnloadMesh ( const Mesh& aMesh )
    {
        auto it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        if ( it != mMeshStore.end() )
        {
            mMeshStore.erase ( it );
        }
    }

    void OpenGLRenderer::BindMesh ( const Mesh& aMesh, GLuint aSkinnedVertexBufferId, size_t aSkinnedVertexOffset, size_t aSkinnedVertexStride )
    {
        auto it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        if ( it == mMeshStore.end() )
        {
            LoadMesh ( aMesh );
            it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        }
        it->second.Bind ( aSkinnedVertexBufferId );
        if ( mCurrentPipeline != nullptr )
        {
            it->second.EnableAttributes ( mCurrentPipeline->GetVertexAttributes(), aSkinnedVertexOffset, aSkinnedVertexStride );
        }
    }

    void OpenGLRenderer::EnsureArenaCapacity ( GeometryArena& aArena, GLsizeiptr aRequired ) const
    {
        if ( aArena.mBuffer && aArena.mCapacity >= aRequired )
        {
            return;
        }
        // Grow geometrically (at least double, at least the required size) so a
        // scene's meshes amortise to a handful of reallocations. 16 MiB start.
        constexpr GLsizeiptr kInitialCapacity{ 16 * 1024 * 1024 };
        GLsizeiptr new_capacity{ aArena.mCapacity ? aArena.mCapacity * 2 : kInitialCapacity };
        while ( new_capacity < aRequired )
        {
            new_capacity *= 2;
        }
        auto new_buffer = std::make_unique<OpenGLBuffer>();
        new_buffer->Initialize ( static_cast<GLsizei> ( new_capacity ), GL_STATIC_DRAW, nullptr );
        if ( aArena.mBuffer && aArena.mUsed > 0 )
        {
            // Copy the live contents into the larger buffer. Runs only at mesh load.
            glCopyNamedBufferSubData ( aArena.mBuffer->GetBufferId(), new_buffer->GetBufferId(), 0, 0, aArena.mUsed );
            OPENGL_CHECK_ERROR_THROW;
        }
        // Retire the old buffer rather than deleting it: a frame that is still
        // recording may have bound it to the shared VAO, and deleting it now
        // could disturb that draw. Its data already lives in the new buffer.
        if ( aArena.mBuffer )
        {
            mRetiredGeometryBuffers.push_back ( std::move ( aArena.mBuffer ) );
        }
        aArena.mBuffer = std::move ( new_buffer );
        aArena.mCapacity = new_capacity;
    }

    OpenGLRenderer::GeometryAllocation OpenGLRenderer::RegisterMeshGeometry ( uint32_t aStride, const void* aVertexData,
            GLsizeiptr aVertexBytes, const uint32_t* aIndexData, uint32_t aIndexCount ) const
    {
        GeometryAllocation allocation{ 0, 0 };
        if ( aStride != 0 && aVertexBytes != 0 && aVertexData != nullptr )
        {
            // One arena per stride: mUsed stays a multiple of the stride (each
            // mesh contributes vertexCount*stride bytes), so base_vertex is exact.
            GeometryArena& arena = mGeometryVertexArenas[aStride];
            EnsureArenaCapacity ( arena, arena.mUsed + aVertexBytes );
            arena.mBuffer->WriteMemory ( arena.mUsed, aVertexBytes, aVertexData );
            allocation.mBaseVertex = static_cast<uint32_t> ( arena.mUsed / aStride );
            arena.mUsed += aVertexBytes;
        }
        if ( aIndexCount != 0 && aIndexData != nullptr )
        {
            const GLsizeiptr index_bytes = static_cast<GLsizeiptr> ( aIndexCount ) * static_cast<GLsizeiptr> ( sizeof ( uint32_t ) );
            EnsureArenaCapacity ( mGeometryIndexArena, mGeometryIndexArena.mUsed + index_bytes );
            mGeometryIndexArena.mBuffer->WriteMemory ( mGeometryIndexArena.mUsed, index_bytes, aIndexData );
            allocation.mFirstIndex = static_cast<uint32_t> ( mGeometryIndexArena.mUsed / sizeof ( uint32_t ) );
            mGeometryIndexArena.mUsed += index_bytes;
        }
        return allocation;
    }

    GLuint OpenGLRenderer::GetGeometryVertexBufferId ( uint32_t aStride ) const
    {
        auto it = mGeometryVertexArenas.find ( aStride );
        return ( it != mGeometryVertexArenas.end() && it->second.mBuffer ) ? it->second.mBuffer->GetBufferId() : 0;
    }

    GLuint OpenGLRenderer::GetGeometryIndexBufferId() const
    {
        return mGeometryIndexArena.mBuffer ? mGeometryIndexArena.mBuffer->GetBufferId() : 0;
    }

    void OpenGLRenderer::FinalizeGeometry()
    {
        mGeometryVertexArenas.clear();
        mGeometryIndexArena.mBuffer.reset();
        mGeometryIndexArena.mUsed = 0;
        mGeometryIndexArena.mCapacity = 0;
        mRetiredGeometryBuffers.clear();
    }

    void OpenGLRenderer::LoadPipeline ( const Pipeline& aPipeline )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it != mPipelineStore.end() )
        {
            std::cout << LogLevel::Error << "OpenGL object already loaded." << std::endl;
            throw std::runtime_error ( "OpenGL object already loaded." );
        }
        mPipelineStore.emplace ( aPipeline.GetConsecutiveId(), OpenGLPipeline{*this, aPipeline} );
    }

    void OpenGLRenderer::UnloadPipeline ( const Pipeline& aPipeline )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it == mPipelineStore.end() )
        {
            return;
        };
        mPipelineStore.erase ( it );
    }

    void OpenGLRenderer::BindPipeline ( const Pipeline& aPipeline )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it == mPipelineStore.end() )
        {
            LoadPipeline ( aPipeline );
            it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        };
        mCurrentPipeline = &it->second;
        glUseProgram ( mCurrentPipeline->GetProgramId() );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLRenderer::BindComputePipeline ( const Pipeline& aPipeline, uint32_t aComputeStageIndex )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it == mPipelineStore.end() )
        {
            LoadPipeline ( aPipeline );
            it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        };
        mCurrentPipeline = &it->second;
        glUseProgram ( mCurrentPipeline->GetComputeProgramId ( aComputeStageIndex ) );
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLRenderer::SetMaterial ( const Material& aMaterial )
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it == mMaterialStore.end() )
        {
            LoadMaterial ( aMaterial );
            it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        };
        it->second.Bind ( *mCurrentPipeline );
    }

    uint32_t OpenGLRenderer::GetMaterialBindlessIndex ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it == mMaterialStore.end() )
        {
            LoadMaterial ( aMaterial );
            it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        }
        return ( it != mMaterialStore.end() ) ? it->second.GetBindlessMaterialIndex() : UINT32_MAX;
    }

    void OpenGLRenderer::BindMaterialStorageBuffer() const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        // Bind the renderer-owned global bindless material SSBO to the current
        // pipeline's Bindless block. RenderMultiBatch needs this (the per-instance
        // material index comes from InstanceMaterials, so no per-material Bind is
        // called that would otherwise bind it).
        const OpenGLUniformBlock* material_block = mCurrentPipeline->GetStorageBlock ( Mesh::BINDLESS );
        if ( material_block == nullptr )
        {
            return;
        }
        glBindBufferBase ( GL_SHADER_STORAGE_BUFFER, material_block->binding, GetMaterialStorageBufferId() );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLRenderer::BindStorageBuffer ( uint32_t aBinding, const BufferAccessor& aBuffer ) const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        const OpenGLUniformBlock* storage_block{ mCurrentPipeline->GetStorageBlock ( aBinding ) };
        if ( storage_block == nullptr )
        {
            return;
        }
        const MemoryPoolBuffer* memory_pool_buffer = aBuffer.GetMemoryPoolBuffer();
        if ( GLuint buffer_id = ( memory_pool_buffer != nullptr ) ? reinterpret_cast<const OpenGLBuffer&> ( memory_pool_buffer->GetBuffer() ).GetBufferId() : 0 )
        {
            glBindBufferRange ( GL_SHADER_STORAGE_BUFFER, storage_block->binding, buffer_id, aBuffer.GetOffset(), aBuffer.GetSize() );
            OPENGL_CHECK_ERROR_THROW;
        }
    }

    void OpenGLRenderer::BindStorageBufferId ( uint32_t aBinding, GLuint aBufferId, size_t aOffset, size_t aSize ) const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        const OpenGLUniformBlock* storage_block{ mCurrentPipeline->GetStorageBlock ( aBinding ) };
        if ( storage_block == nullptr || aBufferId == 0 )
        {
            return;
        }
        glBindBufferRange ( GL_SHADER_STORAGE_BUFFER, storage_block->binding, aBufferId, aOffset, aSize );
        OPENGL_CHECK_ERROR_THROW;
    }

    const OpenGLMesh* OpenGLRenderer::GetOpenGLMesh ( const Mesh& aMesh )
    {
        auto it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        if ( it == mMeshStore.end() )
        {
            LoadMesh ( aMesh );
            it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        }
        if ( it != mMeshStore.end() )
        {
            return &it->second;
        }
        return nullptr;
    }

    void OpenGLRenderer::SetMatrices ( const OpenGLBuffer& aMatricesBuffer ) const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        const OpenGLUniformBlock* uniform_block{ mCurrentPipeline->GetUniformBlock ( Mesh::MATRICES ) };
        if ( uniform_block == nullptr )
        {
            return;
        }

        assert ( static_cast<const size_t> ( uniform_block->size ) <= aMatricesBuffer.GetSize() );
        glBindBufferRange ( GL_UNIFORM_BUFFER, uniform_block->binding, aMatricesBuffer.GetBufferId(), 0, aMatricesBuffer.GetSize() );
        OPENGL_CHECK_ERROR_THROW;

        //glBindBufferBase ( GL_UNIFORM_BUFFER, uniform_block->binding, aMatricesBuffer.GetBufferId() );
        //OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLRenderer::SetLights ( const OpenGLBuffer& aLightsBuffer ) const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        const OpenGLUniformBlock* storage_block{ mCurrentPipeline->GetStorageBlock ( Mesh::LIGHTS ) };
        if ( storage_block == nullptr )
        {
            // Pipeline doesn't sample lights (e.g. unlit shader); silently skip,
            // same convention as SetSkeleton.
            return;
        }
        glBindBufferRange ( GL_SHADER_STORAGE_BUFFER, storage_block->binding, aLightsBuffer.GetBufferId(), 0, aLightsBuffer.GetSize() );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLRenderer::SetClusterParams ( const OpenGLBuffer& aClusterParamsBuffer ) const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        const OpenGLUniformBlock* uniform_block{ mCurrentPipeline->GetUniformBlock ( Mesh::CLUSTER_PARAMS ) };
        if ( uniform_block == nullptr )
        {
            // Pipeline doesn't use clustered shading; silently skip,
            // same convention as SetLights/SetSkeleton.
            return;
        }
        assert ( static_cast<const size_t> ( uniform_block->size ) <= aClusterParamsBuffer.GetSize() );
        glBindBufferRange ( GL_UNIFORM_BUFFER, uniform_block->binding, aClusterParamsBuffer.GetBufferId(), 0, aClusterParamsBuffer.GetSize() );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLRenderer::SetGlobals ( const OpenGLBuffer& aGlobalsBuffer ) const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        // The Globals block is a std430 SSBO on the shading pipeline (works around
        // an NVIDIA std140-UBO fetch bug that green-tints the SH ambient) but stays
        // a std140 UBO elsewhere (e.g. tonemap, which reads only the first member).
        // Bind whichever the current pipeline reflected.
        if ( const OpenGLUniformBlock* storage_block = mCurrentPipeline->GetStorageBlock ( Mesh::GLOBALS ) )
        {
            glBindBufferRange ( GL_SHADER_STORAGE_BUFFER, storage_block->binding, aGlobalsBuffer.GetBufferId(), 0, aGlobalsBuffer.GetSize() );
            OPENGL_CHECK_ERROR_THROW;
            return;
        }
        const OpenGLUniformBlock* uniform_block{ mCurrentPipeline->GetUniformBlock ( Mesh::GLOBALS ) };
        if ( uniform_block == nullptr )
        {
            // Pipeline doesn't use scene globals; silently skip, same
            // convention as SetClusterParams/SetLights.
            return;
        }
        assert ( static_cast<const size_t> ( uniform_block->size ) <= aGlobalsBuffer.GetSize() );
        glBindBufferRange ( GL_UNIFORM_BUFFER, uniform_block->binding, aGlobalsBuffer.GetBufferId(), 0, aGlobalsBuffer.GetSize() );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLRenderer::SetShadowParams ( const OpenGLBuffer& aShadowParamsBuffer ) const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        const OpenGLUniformBlock* uniform_block{ mCurrentPipeline->GetUniformBlock ( Mesh::SHADOW_PARAMS ) };
        if ( uniform_block == nullptr )
        {
            // Pipeline doesn't sample shadows; silently skip, same convention
            // as SetClusterParams/SetLights.
            return;
        }
        assert ( static_cast<const size_t> ( uniform_block->size ) <= aShadowParamsBuffer.GetSize() );
        glBindBufferRange ( GL_UNIFORM_BUFFER, uniform_block->binding, aShadowParamsBuffer.GetBufferId(), 0, aShadowParamsBuffer.GetSize() );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLRenderer::SetSpotShadowParams ( const OpenGLBuffer& aSpotShadowParamsBuffer ) const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        const OpenGLUniformBlock* uniform_block{ mCurrentPipeline->GetUniformBlock ( Mesh::SPOT_SHADOW_PARAMS ) };
        if ( uniform_block == nullptr )
        {
            // Pipeline doesn't sample spot shadows; silently skip, same
            // convention as SetShadowParams/SetClusterParams.
            return;
        }
        assert ( static_cast<const size_t> ( uniform_block->size ) <= aSpotShadowParamsBuffer.GetSize() );
        glBindBufferRange ( GL_UNIFORM_BUFFER, uniform_block->binding, aSpotShadowParamsBuffer.GetBufferId(), 0, aSpotShadowParamsBuffer.GetSize() );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLRenderer::SetPointShadowParams ( const OpenGLBuffer& aPointShadowParamsBuffer ) const
    {
        if ( mCurrentPipeline == nullptr )
        {
            return;
        }
        const OpenGLUniformBlock* uniform_block{ mCurrentPipeline->GetUniformBlock ( Mesh::POINT_SHADOW_PARAMS ) };
        if ( uniform_block == nullptr )
        {
            return;
        }
        assert ( static_cast<const size_t> ( uniform_block->size ) <= aPointShadowParamsBuffer.GetSize() );
        glBindBufferRange ( GL_UNIFORM_BUFFER, uniform_block->binding, aPointShadowParamsBuffer.GetBufferId(), 0, aPointShadowParamsBuffer.GetSize() );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLRenderer::LoadMaterial ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it != mMaterialStore.end() )
        {
            return;
        }
        mMaterialStore.emplace (
            aMaterial.GetConsecutiveId(),
            OpenGLMaterial{*this, aMaterial} );
    }

    void OpenGLRenderer::UnloadMaterial ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it == mMaterialStore.end() )
        {
            return;
        }
        mMaterialStore.erase ( it );
    }

    void OpenGLRenderer::LoadTexture ( const Texture& aTexture )
    {
        auto it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        if ( it != mTextureStore.end() )
        {
            return;
        }
        mTextureStore.emplace ( aTexture.GetConsecutiveId(), OpenGLTexture{*this, aTexture} );
    }

    void OpenGLRenderer::UnloadTexture ( const Texture& aTexture )
    {
        auto it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        if ( it == mTextureStore.end() )
        {
            return;
        }
        mTextureStore.erase ( it );
    }

    GLuint OpenGLRenderer::GetTextureId ( const Texture& aTexture )
    {
        auto it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        if ( it == mTextureStore.end() )
        {
            LoadTexture ( aTexture );
            it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        }
        return it->second.GetTextureId();
    }

    GLuint64 OpenGLRenderer::GetTextureHandle ( const Texture& aTexture )
    {
        auto it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        if ( it == mTextureStore.end() )
        {
            LoadTexture ( aTexture );
            it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        }
        return it->second.GetHandle();
    }

    bool OpenGLRenderer::HasBindlessTexture() const
    {
        return mHasBindlessTexture;
    }

    void OpenGLRenderer::InitializeBindlessMaterials()
    {
        if ( !mHasBindlessTexture )
        {
            return;
        }
        // Capacity mirrors the Vulkan global material storage buffer (4096
        // records). Records are written on material load and read per draw,
        // selected by the MaterialIndex uniform.
        mBindlessMaterialCapacity = 4096;
        mMaterialStorageBuffer.Initialize ( static_cast<GLsizei> ( mBindlessMaterialCapacity * sizeof ( GpuMaterial ) ), GL_DYNAMIC_DRAW, nullptr );
    }

    uint32_t OpenGLRenderer::RegisterBindlessMaterial ( const GpuMaterial& aMaterial )
    {
        uint32_t index;
        if ( !mBindlessMaterialFreeSlots.empty() )
        {
            index = mBindlessMaterialFreeSlots.back();
            mBindlessMaterialFreeSlots.pop_back();
        }
        else
        {
            if ( mBindlessMaterialHighWater >= mBindlessMaterialCapacity )
            {
                throw std::runtime_error ( "OpenGLRenderer: bindless material capacity exceeded." );
            }
            index = mBindlessMaterialHighWater++;
        }
        mMaterialStorageBuffer.WriteMemory ( index * sizeof ( GpuMaterial ), sizeof ( GpuMaterial ), &aMaterial );
        return index;
    }

    void OpenGLRenderer::UnregisterBindlessMaterial ( uint32_t aIndex )
    {
        if ( aIndex == UINT32_MAX )
        {
            return;
        }
        mBindlessMaterialFreeSlots.push_back ( aIndex );
    }

    GLuint OpenGLRenderer::GetMaterialStorageBufferId() const
    {
        return mMaterialStorageBuffer.GetBufferId();
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
        mWindowStore.emplace ( aWindowId, OpenGLWindow {*this, mDisplay, reinterpret_cast<::Window> ( aWindowId ) } );
#elif defined(_WIN32)
        mWindowStore.emplace ( aWindowId, OpenGLWindow {*this, reinterpret_cast<::HWND> ( aWindowId ) } );
#endif
    }
    void OpenGLRenderer::DetachWindow ( void* aWindowId )
    {
        auto overlay_it = mOverlayTextureCache.find ( aWindowId );
        if ( overlay_it != mOverlayTextureCache.end() )
        {
            if ( glIsTexture ( overlay_it->second.texture ) )
            {
                glDeleteTextures ( 1, &overlay_it->second.texture );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            mOverlayTextureCache.erase ( overlay_it );
        }
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

    void OpenGLRenderer::SetLights ( void* aWindowId, std::span<const GpuLight> aLights )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetLights ( FilterLightsByType ( aLights ) );
    }

    void OpenGLRenderer::SetGlobals ( void* aWindowId, const GpuGlobals& aGlobals )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetGlobals ( aGlobals );
    }

    void OpenGLRenderer::SetEnvironmentMap ( void* aWindowId, const Texture* aEnvironmentMap )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetEnvironmentMap ( aEnvironmentMap );
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

    void OpenGLRenderer::BeginRender ( void* aWindowId, const Pipeline* aComputePipeline )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginRender ( aComputePipeline );
    }
    void OpenGLRenderer::BeginFrame ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginFrame();
    }
    void OpenGLRenderer::BeginRenderPass ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginRenderPass();
    }
    void OpenGLRenderer::EndDepthPrePass ( void* aWindowId, const Pipeline* aComputePipeline )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndDepthPrePass ( aComputePipeline );
    }
    void OpenGLRenderer::BeginShadowPass ( void* aWindowId, const Matrix4x4& aLightViewProjection )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginShadowPass ( aLightViewProjection );
    }
    void OpenGLRenderer::EndShadowPass ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndShadowPass();
    }
    void OpenGLRenderer::SetSpotShadowParams ( void* aWindowId, const GpuSpotShadowParams& aSpotShadowParams )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetSpotShadowParams ( aSpotShadowParams );
    }
    void OpenGLRenderer::BeginSpotShadowPass ( void* aWindowId, uint32_t aSlot, const Matrix4x4& aLightViewProjection )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginSpotShadowPass ( aSlot, aLightViewProjection );
    }
    void OpenGLRenderer::EndSpotShadowPass ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndSpotShadowPass();
    }
    void OpenGLRenderer::SetPointShadowParams ( void* aWindowId, const GpuPointShadowParams& aPointShadowParams )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetPointShadowParams ( aPointShadowParams );
    }
    void OpenGLRenderer::BeginPointShadowPass ( void* aWindowId, uint32_t aCaster )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginPointShadowPass ( aCaster );
    }
    void OpenGLRenderer::EndPointShadowPass ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndPointShadowPass();
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
    void OpenGLRenderer::Finish ( void* aWindowId )
    {
        // Block until the GPU has completed every command issued for this window
        // so the caller can safely read back GPU-written buffers or capture the
        // surface (the per-frame fences only gate ring reuse, not read-back).
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Finish();
    }
    void OpenGLRenderer::RecordGpuTimestamp ( void* aWindowId, uint32_t aSlot )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.RecordGpuTimestamp ( aSlot );
    }
    bool OpenGLRenderer::ReadGpuTimestamps ( void* aWindowId, std::array<uint64_t, kGpuTimestampMarks>& aTimestampsNs )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return false;
        }
        return it->second.ReadGpuTimestamps ( aTimestampsNs );
    }
    void OpenGLRenderer::SubmitRenderQueue ( void* aWindowId, const Scene& aScene, RenderPass aRenderPass )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        OpenGLWindow& aWindow = it->second;
        const std::vector<RenderItem>& queue = aScene.GetRenderQueue();
        const size_t count = queue.size();
        // A "poolable" item is a weightless, indexed static mesh living in the
        // shared geometry pool; poolable items sharing a pipeline draw together
        // with one indirect multi-draw.
        auto poolable = [this] ( const RenderItem & aItem ) -> bool
                        {
                            const OpenGLMesh* mesh = GetOpenGLMesh ( *aItem.mMesh );
                            return aItem.mSkinnedVertices == nullptr &&
                                   aItem.mMesh->GetIndexCount() != 0 &&
                                   mesh != nullptr && mesh->IsPooled();
                        };
        if ( aRenderPass == RenderPass::Shading )
        {
            // GPU-driven shading: frustum-cull each pooled pipeline group on the
            // GPU, then draw the compacted commands with glMultiDrawElements
            // IndirectCount. Skinned / private / non-indexed items draw
            // individually afterwards.
            size_t i = 0;
            while ( i < count )
            {
                if ( !poolable ( queue[i] ) )
                {
                    ++i;
                    continue;
                }
                const RenderItem& head = queue[i];
                size_t j = i + 1;
                while ( j < count && queue[j].mPipeline == head.mPipeline && poolable ( queue[j] ) )
                {
                    ++j;
                }
                mCullInstances.clear();
                mCullInstances.reserve ( j - i );
                for ( size_t k = i; k < j; ++k )
                {
                    const RenderItem& item = queue[k];
                    const OpenGLMesh* gl_mesh = GetOpenGLMesh ( *item.mMesh );
                    const AABB& aabb = item.mMesh->GetAABB();
                    const Vector3& center = aabb.GetCenter();
                    const Vector3& radii = aabb.GetRadii();
                    GpuCullInstance instance{};
                    instance.mModel = item.mTransform;
                    instance.mCenter[0] = center.GetX();
                    instance.mCenter[1] = center.GetY();
                    instance.mCenter[2] = center.GetZ();
                    instance.mRadii[0] = radii.GetX();
                    instance.mRadii[1] = radii.GetY();
                    instance.mRadii[2] = radii.GetZ();
                    instance.mDraw[0] = item.mMesh->GetIndexCount();
                    instance.mDraw[1] = gl_mesh->GetFirstIndex();
                    instance.mDraw[2] = gl_mesh->GetBaseVertex();
                    instance.mDraw[3] = item.mMaterial ? GetMaterialBindlessIndex ( *item.mMaterial ) : 0u;
                    mCullInstances.push_back ( instance );
                }
                aWindow.CullShadingBatch ( *head.mPipeline, *head.mMesh, mCullInstances );
                i = j;
            }
            aWindow.BarrierComputeToIndirect();
            aWindow.DrawCulledShadingBatches();
            for ( size_t k = 0; k < count; ++k )
            {
                const RenderItem& item = queue[k];
                if ( poolable ( item ) )
                {
                    continue;
                }
                aWindow.Render ( item.mTransform, *item.mMesh, *item.mPipeline, item.mMaterial,
                                 Topology::TRIANGLE_LIST, 0, 0xffffffff, 1, 0, item.mSkinnedVertices, aRenderPass );
            }
            return;
        }
        // The queue is sorted by (pipeline, material, mesh) so poolable runs --
        // and within them each mesh's instances -- are contiguous. Each pooled
        // pipeline group becomes one glMultiDrawElementsIndirect; skinned /
        // private / non-indexed items draw individually.
        size_t i = 0;
        while ( i < count )
        {
            const RenderItem& head = queue[i];
            if ( !poolable ( head ) )
            {
                aWindow.Render ( head.mTransform, *head.mMesh, *head.mPipeline, head.mMaterial,
                                 Topology::TRIANGLE_LIST, 0, 0xffffffff, 1, 0, head.mSkinnedVertices, aRenderPass );
                ++i;
                continue;
            }
            size_t j = i + 1;
            while ( j < count && queue[j].mPipeline == head.mPipeline && poolable ( queue[j] ) )
            {
                ++j;
            }
            mInstanceTransforms.clear();
            mSuperBatchMeshes.clear();
            mSuperBatchMaterials.clear();
            for ( size_t k = i; k < j; ++k )
            {
                mInstanceTransforms.push_back ( queue[k].mTransform );
                mSuperBatchMeshes.push_back ( queue[k].mMesh );
                mSuperBatchMaterials.push_back ( queue[k].mMaterial );
            }
            aWindow.RenderMultiBatch ( *head.mPipeline, mInstanceTransforms, mSuperBatchMeshes, mSuperBatchMaterials, aRenderPass );
            i = j;
        }
    }
    bool OpenGLRenderer::IsValidWindow ( void* aWindowId ) const
    {
        return mWindowStore.find ( aWindowId ) != mWindowStore.end();
    }
    void OpenGLRenderer::Render ( void* aWindowId,
                                  const Matrix4x4& aModelMatrix,
                                  const Mesh& aMesh,
                                  const Pipeline& aPipeline,
                                  const Material* aMaterial,
                                  Topology aTopology,
                                  uint32_t aVertexStart,
                                  uint32_t aVertexCount,
                                  uint32_t aInstanceCount,
                                  uint32_t aFirstInstance,
                                  const BufferAccessor* aSkinnedVertices,
                                  RenderPass aRenderPass ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Render ( aModelMatrix, aMesh, aPipeline, aMaterial, aTopology, aVertexStart, aVertexCount, aInstanceCount, aFirstInstance, aSkinnedVertices, aRenderPass );
    }

    void OpenGLRenderer::RenderInstanced ( void* aWindowId,
                                           std::span<const Matrix4x4> aModelMatrices,
                                           const Mesh& aMesh,
                                           const Pipeline& aPipeline,
                                           const Material* aMaterial,
                                           Topology aTopology,
                                           uint32_t aVertexStart,
                                           uint32_t aVertexCount,
                                           RenderPass aRenderPass )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.RenderInstanced ( aModelMatrices, aMesh, aPipeline, aMaterial, aTopology, aVertexStart, aVertexCount, aRenderPass );
    }

    void OpenGLRenderer::Dispatch ( void* aWindowId,
                                    const Pipeline& aPipeline,
                                    uint32_t aGroupCountX,
                                    uint32_t aGroupCountY,
                                    uint32_t aGroupCountZ,
                                    std::span<const StorageBufferBinding> aStorageBuffers,
                                    uint32_t aComputeStageIndex ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Dispatch ( aPipeline, aGroupCountX, aGroupCountY, aGroupCountZ, aStorageBuffers, aComputeStageIndex );
    }

    void OpenGLRenderer::Skin ( void* aWindowId,
                                const Pipeline& aSkinningPipeline,
                                const Mesh& aMesh,
                                const BufferAccessor& aSkinningMatrices,
                                const BufferAccessor& aSkinnedVertices ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Skin ( aSkinningPipeline, aMesh, aSkinningMatrices, aSkinnedVertices );
    }

    void OpenGLRenderer::Barrier ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Barrier();
    }

    const Frustum& OpenGLRenderer::GetFrustum ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            std::cout << LogLevel::Error << "Unknown Window Id." << std::endl;
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.GetFrustum();
    }
    const Matrix4x4& OpenGLRenderer::GetProjectionMatrix ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            std::cout << LogLevel::Error << "Unknown Window Id." << std::endl;
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.GetProjectionMatrix();
    }
    const BufferAccessor* OpenGLRenderer::GetFrameLightGrid ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return nullptr;
        }
        return &it->second.GetFrameLightGrid();
    }
    const BufferAccessor* OpenGLRenderer::GetFrameClusterActive ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return nullptr;
        }
        return &it->second.GetFrameClusterActive();
    }
    BufferAccessor OpenGLRenderer::AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            std::cout << LogLevel::Error << "Unknown Window Id." << std::endl;
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.AllocateSingleFrameUniformMemory ( aSize );
    }

    BufferAccessor OpenGLRenderer::AllocateSingleFrameStorageMemory ( void* aWindowId, size_t aSize )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            std::cout << LogLevel::Error << "Unknown Window Id." << std::endl;
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.AllocateSingleFrameStorageMemory ( aSize );
    }

    void* OpenGLRenderer::GetContext() const
    {
        return mOpenGLContext;
    }

    std::string_view OpenGLRenderer::GetName() const
    {
        return "OpenGL";
    }

    void OpenGLRenderer::RenderOverlay ( void* aWindowId, const GuiOverlay& aGuiOverlay )
    {
        const uint8_t* pixels = aGuiOverlay.GetPixels();
        const uint32_t width = aGuiOverlay.GetWidth();
        const uint32_t height = aGuiOverlay.GetHeight();
        if ( !pixels || !aGuiOverlay.GetWidth() || !aGuiOverlay.GetHeight() )
        {
            return;
        }

        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }

        auto& overlay = mOverlayTextureCache[aWindowId];
        if ( !glIsTexture ( overlay.texture ) )
        {
            glGenTextures ( 1, &overlay.texture );
            OPENGL_CHECK_ERROR_NO_THROW;
            overlay.width = 0;
            overlay.height = 0;
        }

        glBindTexture ( GL_TEXTURE_2D, overlay.texture );
        OPENGL_CHECK_ERROR_NO_THROW;

        if ( overlay.width != width || overlay.height != height )
        {
            glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA,
                           width, height,
                           0, GL_BGRA, GL_UNSIGNED_BYTE, pixels );
            OPENGL_CHECK_ERROR_NO_THROW;
            overlay.width = width;
            overlay.height = height;
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            OPENGL_CHECK_ERROR_NO_THROW;
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            OPENGL_CHECK_ERROR_NO_THROW;
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            OPENGL_CHECK_ERROR_NO_THROW;
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        else
        {
            glTexSubImage2D ( GL_TEXTURE_2D, 0,
                              0, 0,
                              width, height,
                              GL_BGRA, GL_UNSIGNED_BYTE, pixels );
            OPENGL_CHECK_ERROR_NO_THROW;
        }

        // Save state
        GLboolean depthTestEnabled = glIsEnabled ( GL_DEPTH_TEST );
        GLboolean blendEnabled = glIsEnabled ( GL_BLEND );
        GLint previousProgram{};
        GLint previousVertexArray{};
        glGetIntegerv ( GL_CURRENT_PROGRAM, &previousProgram );
        glGetIntegerv ( GL_VERTEX_ARRAY_BINDING, &previousVertexArray );

        // Set up for overlay rendering
        glDisable ( GL_DEPTH_TEST );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnable ( GL_BLEND );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        OPENGL_CHECK_ERROR_NO_THROW;

        glUseProgram ( mOverlayProgram );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindVertexArray ( mVertexArrayObject );
        OPENGL_CHECK_ERROR_NO_THROW;
        glActiveTexture ( GL_TEXTURE0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindTexture ( GL_TEXTURE_2D, overlay.texture );
        OPENGL_CHECK_ERROR_NO_THROW;

        glBindBuffer ( GL_ARRAY_BUFFER, mOverlayQuad.GetBufferId() );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnableVertexAttribArray ( 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glVertexAttribPointer ( 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof ( float ), 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glEnableVertexAttribArray ( 1 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glVertexAttribPointer ( 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof ( float ), reinterpret_cast<const void*> ( 2 * sizeof ( float ) ) );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDrawArrays ( GL_TRIANGLE_FAN, 0, 4 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDisableVertexAttribArray ( 1 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glDisableVertexAttribArray ( 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindBuffer ( GL_ARRAY_BUFFER, 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
        glBindVertexArray ( static_cast<GLuint> ( previousVertexArray ) );
        OPENGL_CHECK_ERROR_NO_THROW;

        // Restore state
        glUseProgram ( previousProgram );
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( depthTestEnabled )
        {
            glEnable ( GL_DEPTH_TEST );
        }
        if ( !blendEnabled )
        {
            glDisable ( GL_BLEND );
        }

        glBindTexture ( GL_TEXTURE_2D, 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
    }
}
