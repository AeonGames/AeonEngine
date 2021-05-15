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
#include "aeongames/ProtoBufClasses.h"
#include "OpenGLRenderer.h"
#include "OpenGLTexture.h"
#include "OpenGLMesh.h"
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLBuffer.h"
#include "OpenGLX11Window.h"
#include "OpenGLWinAPIWindow.h"

namespace AeonGames
{
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
    const GLuint vertex_size{sizeof(vertices)};

    void OpenGLRenderer::InitializeOverlay()
    {
        glGenVertexArrays ( 1, &mVertexArrayObject );
        OPENGL_CHECK_ERROR_THROW;
        glBindVertexArray ( mVertexArrayObject );
        OPENGL_CHECK_ERROR_THROW;

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

    OpenGLRenderer::OpenGLRenderer() = default;
    OpenGLRenderer::~OpenGLRenderer() = default;

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowProxy ( void * aWindowId ) const
    {
        return std::make_unique<OpenGLPlatformWindow> ( *this, aWindowId );
    }

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowInstance ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) const
    {
        return std::make_unique<OpenGLPlatformWindow> ( *this, aX, aY, aWidth, aHeight, aFullScreen );
    }

    std::unique_ptr<Mesh> OpenGLRenderer::CreateMesh ( uint32_t aPath ) const
    {
        return std::make_unique<OpenGLMesh> ( aPath );
    }

    std::unique_ptr<Pipeline> OpenGLRenderer::CreatePipeline ( uint32_t aPath ) const
    {
        return std::make_unique<OpenGLPipeline> ( aPath );
    }

    std::unique_ptr<Material> OpenGLRenderer::CreateMaterial ( uint32_t aPath ) const
    {
        return std::make_unique<OpenGLMaterial> ( aPath );
    }

    std::unique_ptr<Texture> OpenGLRenderer::CreateTexture ( uint32_t aPath ) const
    {
        return std::make_unique<OpenGLTexture> ( aPath );
    }

    std::unique_ptr<Buffer> OpenGLRenderer::CreateBuffer ( size_t aSize, const void* aData ) const
    {
        return std::make_unique<OpenGLBuffer> ( static_cast<GLsizei> ( aSize ), GL_DYNAMIC_DRAW, aData );
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
}
