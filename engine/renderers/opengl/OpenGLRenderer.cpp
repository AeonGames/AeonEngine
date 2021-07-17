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

#include "OpenGLRenderer.h"
#include "OpenGLBuffer.h"
#include "OpenGLX11Window.h"
#include "OpenGLWinAPIWindow.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Texture.h"
#include "aeongames/LogLevel.h"

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
        mMatrices.Initialize ( sizeof ( float ) * 16 * 3, GL_DYNAMIC_DRAW );

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
        mMatrices.Finalize();
    }

    OpenGLRenderer::OpenGLRenderer() = default;
    OpenGLRenderer::~OpenGLRenderer() = default;

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowProxy ( void * aWindowId ) const
    {
        return std::make_unique<OpenGLPlatformWindow> ( const_cast<OpenGLRenderer&>(*this), aWindowId );
    }

    std::unique_ptr<Window> OpenGLRenderer::CreateWindowInstance ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) const
    {
        return std::make_unique<OpenGLPlatformWindow> ( const_cast<OpenGLRenderer&>(*this), aX, aY, aWidth, aHeight, aFullScreen );
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

        glBindBuffer ( GL_UNIFORM_BUFFER, mMatrices.GetBufferId() );
        OPENGL_CHECK_ERROR_THROW;

        glBindBufferBase ( GL_UNIFORM_BUFFER, MATRICES, mMatrices.GetBufferId() );
        OPENGL_CHECK_ERROR_THROW;
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
        const OpenGLBuffer* buffer = reinterpret_cast<const OpenGLBuffer*> ( aSkeletonBuffer.GetBuffer() );
        if ( GLuint buffer_id = ( buffer != nullptr ) ? buffer->GetBufferId() : 0 )
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

    void OpenGLRenderer::SetModelMatrix ( const Matrix4x4& aMatrix )
    {
        mMatrices.WriteMemory ( 0, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );
    }
    void OpenGLRenderer::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mMatrices.WriteMemory ( sizeof ( float ) * 16, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );
    }
    void OpenGLRenderer::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mMatrices.WriteMemory ( sizeof ( float ) * 16 * 2, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );
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
    void OpenGLRenderer::AttachWindow ( void* aWindowId ){}
    void OpenGLRenderer::DetachWindow ( void* aWindowId ){}
}
