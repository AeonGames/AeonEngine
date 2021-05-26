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
#include "OpenGLTexture.h"
#include "OpenGLBuffer.h"
#include "OpenGLX11Window.h"
#include "OpenGLWinAPIWindow.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include <unordered_map>

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

    void OpenGLRenderer::LoadMesh ( const Mesh& aMesh )
    {
        auto it = mBufferStore.find(aMesh.GetConsecutiveId());
        if(it==mBufferStore.end())
        {
            if(aMesh.GetIndexCount()>0)
            {
                mBufferStore.emplace(aMesh.GetConsecutiveId(),
                std::vector<OpenGLBuffer>{
                    {static_cast<GLsizei>(aMesh.GetVertexBuffer().size()), GL_STATIC_DRAW, aMesh.GetVertexBuffer().data()},
                    {static_cast<GLsizei>(aMesh.GetIndexBuffer().size()), GL_STATIC_DRAW, aMesh.GetIndexBuffer().data()}
                });
            }
            else
            {
                mBufferStore.emplace(aMesh.GetConsecutiveId(),
                std::vector<OpenGLBuffer>{
                    {static_cast<GLsizei>(aMesh.GetVertexBuffer().size()), GL_STATIC_DRAW, aMesh.GetVertexBuffer().data()}
                });
            }
        }
    }
    void OpenGLRenderer::UnloadMesh ( const Mesh& aMesh )
    {
        auto it = mBufferStore.find(aMesh.GetConsecutiveId());
        if(it!=mBufferStore.end())
        {
            mBufferStore.erase(it);
        }
    }

    void OpenGLRenderer::BindMesh(const Mesh& aMesh) const
    {
        auto it = mBufferStore.find(aMesh.GetConsecutiveId());
        if(it==mBufferStore.end())
        {
            return;
        }
        glBindBuffer ( GL_ARRAY_BUFFER, it->second[0].GetBufferId() );
        OPENGL_CHECK_ERROR_THROW;

        size_t offset{0};
        if ( aMesh.GetVertexFlags() & Mesh::POSITION_BIT )
        {
            glEnableVertexAttribArray ( 0 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, aMesh.GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 0 );
        }

        if ( aMesh.GetVertexFlags() & Mesh::NORMAL_BIT )
        {
            glEnableVertexAttribArray ( 1 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, aMesh.GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 1 );
        }

        if ( aMesh.GetVertexFlags() & Mesh::TANGENT_BIT )
        {
            glEnableVertexAttribArray ( 2 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 2, 3, GL_FLOAT, GL_FALSE, aMesh.GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 2 );
        }

        if ( aMesh.GetVertexFlags() & Mesh::BITANGENT_BIT )
        {
            glEnableVertexAttribArray ( 3 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 3, 3, GL_FLOAT, GL_FALSE, aMesh.GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 3 );
        }

        if ( aMesh.GetVertexFlags() & Mesh::UV_BIT )
        {
            glEnableVertexAttribArray ( 4 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 4, 2, GL_FLOAT, GL_FALSE, aMesh.GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 2;
        }
        else
        {
            glDisableVertexAttribArray ( 4 );
        }

        if ( aMesh.GetVertexFlags() & Mesh::WEIGHT_IDX_BIT )
        {
            glEnableVertexAttribArray ( 5 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribIPointer ( 5, 4, GL_UNSIGNED_BYTE, aMesh.GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
        }
        else
        {
            glDisableVertexAttribArray ( 5 );
        }

        if ( aMesh.GetVertexFlags() & Mesh::WEIGHT_BIT )
        {
            glEnableVertexAttribArray ( 6 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 6, 4, GL_UNSIGNED_BYTE, GL_TRUE, aMesh.GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( uint8_t ) * 4;
        }
        else
        {
            glDisableVertexAttribArray ( 6 );
        }

        if ( aMesh.GetVertexFlags() & Mesh::COLOR_BIT )
        {
            glEnableVertexAttribArray ( 7 );
            OPENGL_CHECK_ERROR_THROW;
            glVertexAttribPointer ( 7, 3, GL_FLOAT, GL_FALSE, aMesh.GetStride(), reinterpret_cast<const void*> ( offset ) );
            OPENGL_CHECK_ERROR_THROW;
            offset += sizeof ( float ) * 3;
        }
        else
        {
            glDisableVertexAttribArray ( 7 );
        }

        //---Index Buffer---
        if (it->second.size()==2)
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, it->second[1].GetBufferId() );
        }
        else
        {
            glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }
        OPENGL_CHECK_ERROR_THROW;
    }

    static std::string GetVertexShaderCode ( const Pipeline& aPipeline )
    {
        std::string vertex_shader{ "#version 450\n" };
        vertex_shader.append ( aPipeline.GetAttributes() );

        uint32_t uniform_block_binding{0};

        std::string transforms (
            "layout(binding = " + std::to_string ( uniform_block_binding++ ) + ", std140) uniform Matrices{\n"
            "mat4 ModelMatrix;\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );
        vertex_shader.append ( transforms );

        if ( aPipeline.GetUniformDescriptors().size() )
        {
            std::string properties (
                "layout(binding = " + std::to_string ( uniform_block_binding++ ) +
                ",std140) uniform Properties{\n" +
                aPipeline.GetProperties() + "};\n" );

            if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
            {
                std::string skeleton (
                    "layout(std140, binding = " + std::to_string ( uniform_block_binding++ ) + ") uniform Skeleton{\n"
                    "mat4 skeleton[256];\n"
                    "};\n"
                );
                vertex_shader.append ( skeleton );
            }

            uint32_t sampler_binding = 0;
            std::string samplers ( "//----SAMPLERS-START----\n" );
            for ( auto& i : aPipeline.GetSamplerDescriptors() )
            {
                samplers += "layout(binding = " + std::to_string ( sampler_binding ) + ") uniform sampler2D " + i + ";\n";
                ++sampler_binding;
            }
            samplers.append ( "//----SAMPLERS-END----\n" );

            vertex_shader.append ( properties );
            vertex_shader.append ( samplers );
        }

        vertex_shader.append ( aPipeline.GetVertexShaderCode() );
        return vertex_shader;
    }

    static std::string GetFragmentShaderCode ( const Pipeline& aPipeline )
    {
        std::string fragment_shader{"#version 450\n"};
        std::string transforms (
            "layout(binding = 0, std140) uniform Matrices{\n"
            "mat4 ModelMatrix;\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );

        fragment_shader.append ( transforms );

        if ( aPipeline.GetUniformDescriptors().size() )
        {
            std::string properties
            {
                "layout(binding = 1,std140) uniform Properties{\n" +
                aPipeline.GetProperties() +
                "};\n"};

            uint32_t sampler_binding = 0;
            std::string samplers ( "//----SAMPLERS-START----\n" );
            for ( auto& i : aPipeline.GetSamplerDescriptors() )
            {
                samplers += "layout(location = " + std::to_string ( sampler_binding ) + ") uniform sampler2D " + i + ";\n";
                ++sampler_binding;
            }
            samplers.append ( "//----SAMPLERS-END----\n" );
            fragment_shader.append ( properties );
            fragment_shader.append ( samplers );
        }
        fragment_shader.append ( aPipeline.GetFragmentShaderCode() );
        return fragment_shader;
    }

    void OpenGLRenderer::LoadPipeline(const Pipeline& aPipeline)
    {
        auto it = mProgramStore.find(aPipeline.GetConsecutiveId());
        if(it!=mProgramStore.end())
        {
            throw std::runtime_error ( "OpenGL object already loaded." );
        }

        std::string vertex_shader_code = GetVertexShaderCode ( aPipeline );
        std::string fragment_shader_code = GetFragmentShaderCode ( aPipeline );

        //--------------------------------------------------
        // Begin OpenGL Specific code
        //--------------------------------------------------
        GLuint program = glCreateProgram();
        OPENGL_CHECK_ERROR_THROW;
        GLint compile_status;
        GLint link_status;
        //-------------------------
        uint32_t vertex_shader = glCreateShader ( GL_VERTEX_SHADER );
        OPENGL_CHECK_ERROR_THROW;

        const auto* vertex_shader_source_ptr = reinterpret_cast<const GLchar *> ( vertex_shader_code.c_str() );
        auto vertex_shader_len = static_cast<GLint> ( vertex_shader_code.length() );

        glShaderSource (
            vertex_shader,
            1,
            &vertex_shader_source_ptr,
            &vertex_shader_len );
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
                throw std::runtime_error ( log_string.c_str() );
            }
            throw std::runtime_error ( "Error Compiling Shaders." );
        }
        glAttachShader ( program, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        //-------------------------

        uint32_t fragment_shader = glCreateShader ( GL_FRAGMENT_SHADER );
        OPENGL_CHECK_ERROR_THROW;

        const auto* fragment_shader_source_ptr = reinterpret_cast<const GLchar *> ( fragment_shader_code.c_str() );
        auto fragment_shader_len = static_cast<GLint> ( fragment_shader_code.length() );

        glShaderSource ( fragment_shader, 1, &fragment_shader_source_ptr, &fragment_shader_len );
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
                std::cout << fragment_shader_code << std::endl;
                std::cout << log_string << std::endl;
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        glAttachShader ( program, fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        //-------------------------
        glLinkProgram ( program );
        OPENGL_CHECK_ERROR_THROW;
        glGetProgramiv ( program, GL_LINK_STATUS, &link_status );
        OPENGL_CHECK_ERROR_THROW;
        if ( link_status != GL_TRUE )

        {
            GLint info_log_len;
            glGetProgramiv ( program, GL_INFO_LOG_LENGTH, &info_log_len );
            OPENGL_CHECK_ERROR_THROW;
            std::string log_string;
            log_string.resize ( info_log_len );
            if ( info_log_len > 1 )
            {
                glGetProgramInfoLog ( program, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                std::cout << vertex_shader_code << std::endl;
                std::cout << fragment_shader_code << std::endl;
                std::cout << log_string << std::endl;
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        glDetachShader ( program, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDetachShader ( program, fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDeleteShader ( vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDeleteShader ( fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        /* We need to bind the program to set any samplers. */
        glUseProgram ( program );
        OPENGL_CHECK_ERROR_THROW;

        // Samplers
#if 1
        for ( GLint i = 0; i < static_cast<GLint>(aPipeline.GetSamplerDescriptors().size()); ++i )
        {
            glUniform1i ( i, i );
            OPENGL_CHECK_ERROR_THROW;
        }
#else
        // Keeping this code for reference
        GLuint uniform = 0;
        for ( auto& i : GetDefaultMaterial().GetPropertyMetaData() )
        {
            if ( i.GetType() == Property::SAMPLER_2D )
            {
                auto location = glGetPropertyLocation ( mProgramId, i.GetName().c_str() );
                OPENGL_CHECK_ERROR_THROW;
                glProperty1i ( location, uniform++ );
                OPENGL_CHECK_ERROR_THROW;
            }
        }
#endif
        mProgramStore.emplace(aPipeline.GetConsecutiveId(),program);
    }

    void OpenGLRenderer::UnloadPipeline(const Pipeline& aPipeline)
    {
        auto it = mProgramStore.find(aPipeline.GetConsecutiveId());
        if(it==mProgramStore.end()){return;};

        if ( glIsProgram ( it->second ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteProgram ( it->second );
            OPENGL_CHECK_ERROR_NO_THROW;
            mProgramStore.erase(it);
        }
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLRenderer::UsePipeline ( const Pipeline& aPipeline, const Material* aMaterial, const BufferAccessor* aSkeletonBuffer) const
    {
        auto it = mProgramStore.find(aPipeline.GetConsecutiveId());
        if(it==mProgramStore.end()){return;};

        glUseProgram ( it->second );
        OPENGL_CHECK_ERROR_NO_THROW;

        // Binding 0 is the matrix buffer.
        GLuint index{1};

        if ( aMaterial )
        {
            for ( GLenum i = 0; i < aMaterial->GetSamplers().size(); ++i )
            {
                glActiveTexture ( GL_TEXTURE0 + i );
                OPENGL_CHECK_ERROR_NO_THROW;
                glBindTexture ( GL_TEXTURE_2D, reinterpret_cast<OpenGLTexture*> ( std::get<1> ( aMaterial->GetSamplers() [i] ).Cast<Texture>() )->GetTextureId() );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
            if ( GLuint buffer = mBufferStore.at( aMaterial->GetConsecutiveId() )[0].GetBufferId() )
            {
                glBindBuffer ( GL_UNIFORM_BUFFER, buffer );
                OPENGL_CHECK_ERROR_THROW;
                glBindBufferBase ( GL_UNIFORM_BUFFER, index++, buffer );
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        auto* buffer = ( aSkeletonBuffer != nullptr ) ? reinterpret_cast<const OpenGLBuffer*> ( aSkeletonBuffer->GetBuffer() ) : nullptr;
        if ( GLuint buffer_id = ( buffer != nullptr ) ? buffer->GetBufferId() : 0 )
        {
            glBindBufferRange ( GL_UNIFORM_BUFFER, index++, buffer_id, aSkeletonBuffer->GetOffset(), aSkeletonBuffer->GetSize() );
            OPENGL_CHECK_ERROR_THROW;
        };
    }

    void OpenGLRenderer::LoadMaterial(const Material& aMaterial)
    {
        auto it = mBufferStore.find(aMaterial.GetConsecutiveId());
        if(it!=mBufferStore.end()){return;}

        mBufferStore.emplace(
            aMaterial.GetConsecutiveId(),
            std::vector<OpenGLBuffer>
            {
                {static_cast<GLsizei>(aMaterial.GetUniformBuffer().size()), GL_STATIC_DRAW, aMaterial.GetUniformBuffer().data()}
            });
    }

    void OpenGLRenderer::UnloadMaterial(const Material& aMaterial)
    {
        auto it = mBufferStore.find(aMaterial.GetConsecutiveId());
        if(it==mBufferStore.end()){return;}
        mBufferStore.erase(it);
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
