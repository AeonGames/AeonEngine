/*
Copyright (C) 2016-2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLPipeline.h"
#include "OpenGLFunctions.h"
#include "aeongames/CRC.hpp"
#include <vector>
#include <algorithm>
#include <cassert>

namespace AeonGames
{

    OpenGLPipeline::OpenGLPipeline ( OpenGLPipeline&& aOpenGLPipeline ) :
        mOpenGLRenderer{aOpenGLPipeline.mOpenGLRenderer}
    {
        std::swap ( mPipeline, aOpenGLPipeline.mPipeline );
        std::swap ( mProgramId, aOpenGLPipeline.mProgramId );
        mAttributes.swap ( aOpenGLPipeline.mAttributes );
        mUniformBlocks.swap ( aOpenGLPipeline.mUniformBlocks );
        mUniforms.swap ( aOpenGLPipeline.mUniforms );
    }

    static const std::unordered_map<ShaderType, const GLenum> ShaderTypeToGLShaderType
    {
        { VERT, GL_VERTEX_SHADER },
        { FRAG, GL_FRAGMENT_SHADER },
        { COMP, GL_COMPUTE_SHADER },
        { TESC, GL_TESS_CONTROL_SHADER },
        { TESE, GL_TESS_EVALUATION_SHADER },
        { GEOM, GL_GEOMETRY_SHADER }
    };

    OpenGLPipeline::OpenGLPipeline ( const OpenGLRenderer& aOpenGLRenderer, const Pipeline& aPipeline ) :
        mOpenGLRenderer{aOpenGLRenderer}, mPipeline{&aPipeline}
    {
        mProgramId = glCreateProgram();
        OPENGL_CHECK_ERROR_THROW;
        GLint compile_status;
        GLint link_status;

        std::array<std::string_view, ShaderType::COUNT> shader_codes =
        {
            aPipeline.GetShaderCode ( VERT ),
            aPipeline.GetShaderCode ( FRAG ),
            aPipeline.GetShaderCode ( COMP ),
            aPipeline.GetShaderCode ( TESC ),
            aPipeline.GetShaderCode ( TESE ),
            aPipeline.GetShaderCode ( GEOM )
        };

        std::array<uint32_t, ShaderType::COUNT> shader_ids =
        {
            shader_codes.at ( VERT ).empty() ? 0 : glCreateShader ( ShaderTypeToGLShaderType.at ( VERT ) ),
            shader_codes.at ( FRAG ).empty() ? 0 : glCreateShader ( ShaderTypeToGLShaderType.at ( FRAG ) ),
            shader_codes.at ( COMP ).empty() ? 0 : glCreateShader ( ShaderTypeToGLShaderType.at ( COMP ) ),
            shader_codes.at ( TESC ).empty() ? 0 : glCreateShader ( ShaderTypeToGLShaderType.at ( TESC ) ),
            shader_codes.at ( TESE ).empty() ? 0 : glCreateShader ( ShaderTypeToGLShaderType.at ( TESE ) ),
            shader_codes.at ( GEOM ).empty() ? 0 : glCreateShader ( ShaderTypeToGLShaderType.at ( GEOM ) )
        };
        OPENGL_CHECK_ERROR_THROW;

        //-------------------------
        for ( size_t i = 0; i < shader_codes.size(); ++i )
        {
            if ( shader_ids.at ( i ) == 0 )
            {
                continue;
            }
            const auto* source_ptr = reinterpret_cast<const GLchar *> ( shader_codes.at ( i ).data() );
            auto source_len = static_cast<GLint> ( shader_codes.at ( i ).length() );

            glShaderSource (
                shader_ids.at ( i ),
                1,
                &source_ptr,
                &source_len );
            OPENGL_CHECK_ERROR_THROW;

            glCompileShader ( shader_ids.at ( i ) );
            OPENGL_CHECK_ERROR_THROW;
            glGetShaderiv ( shader_ids.at ( i ), GL_COMPILE_STATUS, &compile_status );
            OPENGL_CHECK_ERROR_THROW;
            if ( compile_status != GL_TRUE )
            {
                GLint info_log_len;
                glGetShaderiv ( shader_ids.at ( i ), GL_INFO_LOG_LENGTH, &info_log_len );
                OPENGL_CHECK_ERROR_THROW;
                std::string log_string;
                log_string.resize ( info_log_len );
                if ( info_log_len > 1 )
                {
                    glGetShaderInfoLog ( shader_ids.at ( i ), info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                    OPENGL_CHECK_ERROR_THROW;
                    std::cout << shader_codes.at ( i ) << std::endl;
                    std::cout << log_string << std::endl;
                    throw std::runtime_error ( log_string.c_str() );
                }
                log_string = ShaderTypeToString.at ( static_cast<ShaderType> ( i ) );
                throw std::runtime_error ( "Error Compiling Shaders." );
            }
            glAttachShader ( mProgramId, shader_ids.at ( i ) );
            OPENGL_CHECK_ERROR_THROW;
        }
        //-------------------------

        glLinkProgram ( mProgramId );
        OPENGL_CHECK_ERROR_THROW;
        glGetProgramiv ( mProgramId, GL_LINK_STATUS, &link_status );
        OPENGL_CHECK_ERROR_THROW;
        if ( link_status != GL_TRUE )
        {
            GLint info_log_len;
            glGetProgramiv ( mProgramId, GL_INFO_LOG_LENGTH, &info_log_len );
            OPENGL_CHECK_ERROR_THROW;
            std::string log_string;
            log_string.resize ( info_log_len );
            if ( info_log_len > 1 )
            {
                glGetProgramInfoLog ( mProgramId, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                for ( const auto& shader_code : shader_codes )
                {
                    if ( shader_code.empty() )
                    {
                        continue;
                    }
                    std::cout << shader_code << std::endl;
                }
                std::cout << log_string << std::endl;
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        for ( const auto& shader_id : shader_ids )
        {
            if ( shader_id == 0 )
            {
                continue;
            }
            glDetachShader ( mProgramId, shader_id );
            OPENGL_CHECK_ERROR_THROW;
            glDeleteShader ( shader_id );
            OPENGL_CHECK_ERROR_THROW;
        }
        ReflectAttributes();
        ReflectUniforms();
    }

    void OpenGLPipeline::ReflectAttributes()
    {
        mAttributes.clear();
        GLint active_attribute_count{};
        GLchar name[256] {};
        GLsizei length{};
        GLint size{};
        GLenum type{};
        GLint location{};
        glGetProgramiv ( mProgramId, GL_ACTIVE_ATTRIBUTES, &active_attribute_count );
        OPENGL_CHECK_ERROR_THROW;
        mAttributes.reserve ( active_attribute_count );
        for ( GLint i = 0; i < active_attribute_count; ++i )
        {
            glGetActiveAttrib ( mProgramId, i, sizeof ( name ), &length, &size, &type, name );
            OPENGL_CHECK_ERROR_THROW;
            location = glGetAttribLocation ( mProgramId, name );
            OPENGL_CHECK_ERROR_THROW;
            // Skip reserved attributes
            if ( location < 0 )
            {
                continue;
            }
            const uint32_t name_crc{crc32i ( name, length ) };
            auto it = std::lower_bound ( mAttributes.begin(), mAttributes.end(), name_crc,
                                         [] ( const OpenGLVariable & a, const uint32_t b )
            {
                return a.name < b;
            } );
            mAttributes.insert ( it, { name_crc, {location}, size, type } );
        }
        for ( const auto& attribute : mAttributes )
        {
            std::cout << LogLevel::Debug << "Attribute: " << attribute.name << " (crc: " << std::hex << attribute.name << std::dec << ", location: " << attribute.location << ", size: " << attribute.size << ", type: " << attribute.type << ")" << std::endl;
        }
    }

    void OpenGLPipeline::ReflectUniforms()
    {
        mUniforms.clear();
        mUniformBlocks.clear();
        GLint uniform_count;
        GLint non_block_uniform_count{};
        GLint block_uniform_count{};
        glGetProgramiv ( mProgramId, GL_ACTIVE_UNIFORMS, &uniform_count );
        OPENGL_CHECK_ERROR_THROW;
        non_block_uniform_count = uniform_count;

        GLint uniform_block_count = 0;
        glGetProgramiv ( mProgramId, GL_ACTIVE_UNIFORM_BLOCKS, &uniform_block_count );
        OPENGL_CHECK_ERROR_THROW;

        GLchar name[256];
        GLint indices[64];
        GLsizei length{};
        GLint size{};
        GLenum type{};
        GLint binding_offset_or_location{};

        mUniformBlocks.reserve ( uniform_block_count );
        for ( GLint i = 0; i < uniform_block_count; ++i )
        {
            glGetActiveUniformBlockName ( mProgramId, i, sizeof ( name ), &length, name );
            OPENGL_CHECK_ERROR_THROW;
            glGetActiveUniformBlockiv ( mProgramId, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size );
            OPENGL_CHECK_ERROR_THROW;
            glGetActiveUniformBlockiv ( mProgramId, i, GL_UNIFORM_BLOCK_BINDING, &binding_offset_or_location );
            OPENGL_CHECK_ERROR_THROW;
            glGetActiveUniformBlockiv ( mProgramId, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &block_uniform_count );
            OPENGL_CHECK_ERROR_THROW;
            assert ( block_uniform_count <= 64 );

            const uint32_t name_crc{crc32i ( name, length ) };
            auto it = std::lower_bound ( mUniformBlocks.begin(), mUniformBlocks.end(), name_crc,
                                         [] ( const OpenGLUniformBlock & a, const uint32_t b )
            {
                return a.name < b;
            } );
            mUniformBlocks.insert ( it, { name_crc, size, binding_offset_or_location } );

            it->uniforms.reserve ( block_uniform_count );
            non_block_uniform_count -= block_uniform_count;
            if ( block_uniform_count > 0 )
            {
                glGetActiveUniformBlockiv ( mProgramId, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices );
                OPENGL_CHECK_ERROR_THROW;
                for ( GLint j = 0; j < block_uniform_count; ++j )
                {
                    GLuint uniform_index = static_cast<GLuint> ( indices[ static_cast<size_t> ( j ) ] );
                    glGetActiveUniform ( mProgramId, uniform_index, sizeof ( name ), &length, &size, &type, name );
                    OPENGL_CHECK_ERROR_THROW;

                    glGetActiveUniformsiv ( mProgramId, 1, &uniform_index, GL_UNIFORM_OFFSET, &binding_offset_or_location );
                    OPENGL_CHECK_ERROR_THROW;

                    uint32_t name_crc{crc32i ( name, length ) };
                    auto il = std::lower_bound ( it->uniforms.begin(), it->uniforms.end(), name_crc,
                                                 [] ( const OpenGLVariable & a, const uint32_t b )
                    {
                        return a.name < b;
                    } );
                    it->uniforms.insert ( il, { name_crc, {binding_offset_or_location}, size, type } );
                }
            }
        }

        mUniforms.reserve ( non_block_uniform_count );
        for ( GLint i = 0; i < uniform_count; ++i )
        {
            glGetActiveUniform ( mProgramId, i, sizeof ( name ), &length, &size, &type, name );
            OPENGL_CHECK_ERROR_THROW;
            binding_offset_or_location =  glGetUniformLocation ( mProgramId, name );
            OPENGL_CHECK_ERROR_THROW;
            // If the uniform is part of a uniform block, its location will be -1
            if ( binding_offset_or_location < 0 )
            {
                continue;
            }
            uint32_t name_crc{crc32i ( name, length ) };
            auto il = std::lower_bound ( mUniforms.begin(), mUniforms.end(), name_crc,
                                         [] ( const OpenGLVariable & a, const uint32_t b )
            {
                return a.name < b;
            } );
            mUniforms.insert ( il, { name_crc, {binding_offset_or_location}, size, type } );
        }
        for ( const auto& uniform : mUniforms )
        {
            std::cout << LogLevel::Debug << "Uniform: " << uniform.name << " (crc: " << std::hex << uniform.name << std::dec << ", location: " << uniform.location << ", size: " << uniform.size << ", type: " << uniform.type << ")" << std::endl;
        }
        for ( const auto& block : mUniformBlocks )
        {
            std::cout << LogLevel::Debug << "Uniform Block: " << block.name << " (crc: " << std::hex << block.name << std::dec << ", size: " << block.size << ", binding: " << block.binding << ", active uniforms: " << block.uniforms.size() << ")" << std::endl;
            for ( const auto& uniform : block.uniforms )
            {
                std::cout << LogLevel::Debug << "\tUniform: " << uniform.name << " (crc: " << std::hex << uniform.name << std::dec << ", offset: " << uniform.offset << ", size: " << uniform.size << ", type: " << uniform.type << ")" << std::endl;
            }
        }
    }

    OpenGLPipeline::~OpenGLPipeline()
    {
        if ( glIsProgram ( mProgramId ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteProgram ( mProgramId );
            OPENGL_CHECK_ERROR_NO_THROW;
            mProgramId = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    GLint OpenGLPipeline::GetProgramId() const
    {
        return mProgramId;
    }

    const std::vector<OpenGLVariable>& OpenGLPipeline::GetVertexAttributes() const
    {
        return mAttributes;
    }

    const GLuint OpenGLPipeline::GetSamplerLocation ( uint32_t name_hash ) const
    {
        auto it = std::lower_bound ( mUniforms.begin(), mUniforms.end(), name_hash,
                                     [] ( const OpenGLVariable & a, const uint32_t b )
        {
            return a.name < b;
        } );
        if ( it == mUniforms.end() || it->location < 0 || it->type != GL_SAMPLER_2D )
        {
            return 0;
        }
        return it->location;
    }

    const OpenGLUniformBlock* OpenGLPipeline::GetUniformBlock ( uint32_t name ) const
    {
        auto it = std::lower_bound ( mUniformBlocks.begin(), mUniformBlocks.end(), name,
                                     [] ( const OpenGLUniformBlock & a, const uint32_t b )
        {
            return a.name < b;
        } );
        if ( it == mUniformBlocks.end() )
        {
            return nullptr;
        }
        return &*it;
    }
}
