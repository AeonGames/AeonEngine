/*
Copyright (C) 2016-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLPipeline.hpp"
#include "OpenGLFunctions.hpp"
#include "OpenGLRenderer.hpp"
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
        mComputeProgramIds.swap ( aOpenGLPipeline.mComputeProgramIds );
        mAttributes.swap ( aOpenGLPipeline.mAttributes );
        mUniformBlocks.swap ( aOpenGLPipeline.mUniformBlocks );
        mStorageBlocks.swap ( aOpenGLPipeline.mStorageBlocks );
        mSamplerLocations.swap ( aOpenGLPipeline.mSamplerLocations );
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
        // Compiles a single shader stage source and attaches it to the given
        // program, throwing on any compilation error.
        auto compile_and_attach = [] ( GLuint program, GLenum gl_type, std::string_view source, ShaderType log_type )
        {
            GLuint shader_id = glCreateShader ( gl_type );
            OPENGL_CHECK_ERROR_THROW;
            const auto* source_ptr = reinterpret_cast<const GLchar*> ( source.data() );
            auto source_len = static_cast<GLint> ( source.length() );
            glShaderSource ( shader_id, 1, &source_ptr, &source_len );
            OPENGL_CHECK_ERROR_THROW;
            glCompileShader ( shader_id );
            OPENGL_CHECK_ERROR_THROW;
            GLint compile_status{};
            glGetShaderiv ( shader_id, GL_COMPILE_STATUS, &compile_status );
            OPENGL_CHECK_ERROR_THROW;
            if ( compile_status != GL_TRUE )
            {
                GLint info_log_len{};
                glGetShaderiv ( shader_id, GL_INFO_LOG_LENGTH, &info_log_len );
                OPENGL_CHECK_ERROR_THROW;
                if ( info_log_len > 1 )
                {
                    std::string log_string;
                    log_string.resize ( info_log_len );
                    glGetShaderInfoLog ( shader_id, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                    OPENGL_CHECK_ERROR_THROW;
                    std::cout << source << std::endl;
                    std::cout << log_string << std::endl;
                    std::cout << LogLevel::Error << log_string << std::endl;
                    throw std::runtime_error ( log_string.c_str() );
                }
                std::cout << LogLevel::Error << "Error Compiling " << ShaderTypeToString.at ( log_type ) << " Shader." << std::endl;
                throw std::runtime_error ( "Error Compiling Shaders." );
            }
            glAttachShader ( program, shader_id );
            OPENGL_CHECK_ERROR_THROW;
            return shader_id;
        };

        // Links a program and logs any link error.
        auto link_program = [] ( GLuint program )
        {
            glLinkProgram ( program );
            OPENGL_CHECK_ERROR_THROW;
            GLint link_status{};
            glGetProgramiv ( program, GL_LINK_STATUS, &link_status );
            OPENGL_CHECK_ERROR_THROW;
            if ( link_status != GL_TRUE )
            {
                GLint info_log_len{};
                glGetProgramiv ( program, GL_INFO_LOG_LENGTH, &info_log_len );
                OPENGL_CHECK_ERROR_THROW;
                if ( info_log_len > 1 )
                {
                    std::string log_string;
                    log_string.resize ( info_log_len );
                    glGetProgramInfoLog ( program, info_log_len, nullptr, const_cast<GLchar*> ( log_string.data() ) );
                    std::cout << log_string << std::endl;
                    OPENGL_CHECK_ERROR_THROW;
                }
            }
        };

        // Graphics stages are linked together into a single graphics program.
        // Compute stages cannot share a GL program with graphics stages, so
        // each compute stage is linked into its own dedicated program.
        static constexpr std::array<ShaderType, 5> graphics_stages{ { VERT, FRAG, TESC, TESE, GEOM } };
        const std::string_view renderer { mOpenGLRenderer.GetName() };
        std::vector<GLuint> graphics_shader_ids;
        bool has_graphics_stage{false};
        for ( ShaderType stage : graphics_stages )
        {
            if ( !aPipeline.GetShaderCode ( stage, renderer ).empty() )
            {
                has_graphics_stage = true;
                break;
            }
        }

        if ( has_graphics_stage )
        {
            mProgramId = glCreateProgram();
            OPENGL_CHECK_ERROR_THROW;
            for ( ShaderType stage : graphics_stages )
            {
                const std::string_view code{ aPipeline.GetShaderCode ( stage, renderer ) };
                if ( code.empty() )
                {
                    continue;
                }
                graphics_shader_ids.push_back ( compile_and_attach ( mProgramId, ShaderTypeToGLShaderType.at ( stage ), code, stage ) );
            }
            link_program ( mProgramId );
            for ( GLuint shader_id : graphics_shader_ids )
            {
                glDetachShader ( mProgramId, shader_id );
                OPENGL_CHECK_ERROR_THROW;
                glDeleteShader ( shader_id );
                OPENGL_CHECK_ERROR_THROW;
            }
        }

        const uint32_t compute_stage_count = aPipeline.GetComputeStageCount ( renderer );
        mComputeProgramIds.reserve ( compute_stage_count );
        for ( uint32_t c = 0; c < compute_stage_count; ++c )
        {
            GLuint program = glCreateProgram();
            OPENGL_CHECK_ERROR_THROW;
            GLuint shader_id = compile_and_attach ( program, GL_COMPUTE_SHADER, aPipeline.GetComputeShaderCode ( c, renderer ), COMP );
            link_program ( program );
            glDetachShader ( program, shader_id );
            OPENGL_CHECK_ERROR_THROW;
            glDeleteShader ( shader_id );
            OPENGL_CHECK_ERROR_THROW;
            mComputeProgramIds.push_back ( program );
        }

        // Reflection. Attributes and samplers only come from the graphics
        // program; uniform and storage blocks are merged across the graphics
        // program and every compute program so the renderer can resolve a
        // block by name regardless of which stage uses it.
        mUniformBlocks.clear();
        mStorageBlocks.clear();
        mSamplerLocations.clear();
        if ( mProgramId != 0 )
        {
            ReflectAttributes();
            ReflectUniforms ( mProgramId, true );
            ReflectStorageBlocks ( mProgramId );
        }
        for ( GLuint program : mComputeProgramIds )
        {
            ReflectUniforms ( program, false );
            ReflectStorageBlocks ( program );
        }
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

    void OpenGLPipeline::ReflectUniforms ( GLuint aProgramId, bool aReflectSamplers )
    {
        GLint block_uniform_count{};
        GLint uniform_block_count = 0;
        glGetProgramiv ( aProgramId, GL_ACTIVE_UNIFORM_BLOCKS, &uniform_block_count );
        OPENGL_CHECK_ERROR_THROW;

        GLchar name[256];
        std::vector<GLint> indices;
        GLsizei length{};
        GLint size{};
        GLenum type{};
        GLint binding_offset_or_location{};

        for ( GLint i = 0; i < uniform_block_count; ++i )
        {
            glGetActiveUniformBlockName ( aProgramId, i, sizeof ( name ), &length, name );
            OPENGL_CHECK_ERROR_THROW;
            glGetActiveUniformBlockiv ( aProgramId, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size );
            OPENGL_CHECK_ERROR_THROW;
            glGetActiveUniformBlockiv ( aProgramId, i, GL_UNIFORM_BLOCK_BINDING, &binding_offset_or_location );
            OPENGL_CHECK_ERROR_THROW;
            glGetActiveUniformBlockiv ( aProgramId, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &block_uniform_count );
            OPENGL_CHECK_ERROR_THROW;

            const uint32_t name_crc{crc32i ( name, length ) };
            auto it = std::lower_bound ( mUniformBlocks.begin(), mUniformBlocks.end(), name_crc,
                                         [] ( const OpenGLUniformBlock & a, const uint32_t b )
            {
                return a.name < b;
            } );
            // A block of the same name reflected from another program (graphics
            // or a different compute stage) is already recorded; skip it.
            if ( it != mUniformBlocks.end() && it->name == name_crc )
            {
                continue;
            }
            it = mUniformBlocks.insert ( it, { name_crc, size, binding_offset_or_location } );

            it->uniforms.reserve ( block_uniform_count );
            if ( block_uniform_count > 0 )
            {
                indices.resize ( static_cast<size_t> ( block_uniform_count ) );
                glGetActiveUniformBlockiv ( aProgramId, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data() );
                OPENGL_CHECK_ERROR_THROW;
                for ( GLint j = 0; j < block_uniform_count; ++j )
                {
                    GLuint uniform_index = static_cast<GLuint> ( indices[ static_cast<size_t> ( j ) ] );
                    glGetActiveUniform ( aProgramId, uniform_index, sizeof ( name ), &length, &size, &type, name );
                    OPENGL_CHECK_ERROR_THROW;

                    glGetActiveUniformsiv ( aProgramId, 1, &uniform_index, GL_UNIFORM_OFFSET, &binding_offset_or_location );
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

        if ( !aReflectSamplers )
        {
            return;
        }

        GLint uniform_count;
        GLint sampler_count{0};
        glGetProgramiv ( aProgramId, GL_ACTIVE_UNIFORMS, &uniform_count );
        OPENGL_CHECK_ERROR_THROW;
        for ( GLint i = 0; i < uniform_count; ++i )
        {
            glGetActiveUniform ( aProgramId, i, sizeof ( name ), &length, &size, &type, name );
            OPENGL_CHECK_ERROR_THROW;
            sampler_count += ( type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE || type == GL_SAMPLER_3D ||
                               type == GL_SAMPLER_2D_ARRAY || type == GL_SAMPLER_2D_SHADOW || type == GL_SAMPLER_CUBE_SHADOW ) ? 1 : 0;
        }
        mSamplerLocations.reserve ( sampler_count );
        for ( GLint i = 0; i < uniform_count; ++i )
        {
            glGetActiveUniform ( aProgramId, i, sizeof ( name ), &length, &size, &type, name );
            OPENGL_CHECK_ERROR_THROW;
            if ( type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE || type == GL_SAMPLER_3D ||
                 type == GL_SAMPLER_2D_ARRAY || type == GL_SAMPLER_2D_SHADOW || type == GL_SAMPLER_CUBE_SHADOW )
            {
                // Store the sampler's texture unit (its layout(binding = N)
                // value), NOT its uniform location. The material binds its
                // texture with glBindTextureUnit, which takes a unit; the
                // uniform location is only an API handle and generally differs
                // from the binding, so using it would bind the texture to the
                // wrong unit and the sampler would read an empty one (black).
                const GLint location = glGetUniformLocation ( aProgramId, name );
                OPENGL_CHECK_ERROR_THROW;
                binding_offset_or_location = 0;
                if ( location >= 0 )
                {
                    glGetUniformiv ( aProgramId, location, &binding_offset_or_location );
                    OPENGL_CHECK_ERROR_THROW;
                }
                uint32_t name_crc{crc32i ( name, length ) };
                auto il = std::lower_bound ( mSamplerLocations.begin(), mSamplerLocations.end(), name_crc,
                                             [] ( const OpenGLSamplerLocation & a, const uint32_t b )
                {
                    return a.name < b;
                } );
                mSamplerLocations.insert ( il, { name_crc, binding_offset_or_location } );
            }
        }

        for ( const auto& sampler : mSamplerLocations )
        {
            std::cout << LogLevel::Debug << "Sampler: " << sampler.name << " (crc: " << std::hex << sampler.name << std::dec << ", unit: " << sampler.location << ")" << std::endl;
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
        for ( GLuint& compute_program : mComputeProgramIds )
        {
            if ( glIsProgram ( compute_program ) )
            {
                OPENGL_CHECK_ERROR_NO_THROW;
                glDeleteProgram ( compute_program );
                OPENGL_CHECK_ERROR_NO_THROW;
                compute_program = 0;
            }
        }
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    GLint OpenGLPipeline::GetProgramId() const
    {
        return mProgramId;
    }

    uint32_t OpenGLPipeline::GetComputeStageCount() const
    {
        return static_cast<uint32_t> ( mComputeProgramIds.size() );
    }

    GLuint OpenGLPipeline::GetComputeProgramId ( uint32_t aIndex ) const
    {
        return mComputeProgramIds.at ( aIndex );
    }

    const std::vector<OpenGLVariable>& OpenGLPipeline::GetVertexAttributes() const
    {
        return mAttributes;
    }

    const GLuint OpenGLPipeline::GetSamplerLocation ( uint32_t name_hash ) const
    {
        auto it = std::lower_bound ( mSamplerLocations.begin(), mSamplerLocations.end(), name_hash,
                                     [] ( const OpenGLSamplerLocation & a, const uint32_t b )
        {
            return a.name < b;
        } );
        if ( it == mSamplerLocations.end() || it->location < 0 )
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

    void OpenGLPipeline::ReflectStorageBlocks ( GLuint aProgramId )
    {
        GLint storage_block_count{0};
        glGetProgramInterfaceiv ( aProgramId, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &storage_block_count );
        OPENGL_CHECK_ERROR_THROW;
        if ( storage_block_count <= 0 )
        {
            return;
        }
        mStorageBlocks.reserve ( mStorageBlocks.size() + static_cast<size_t> ( storage_block_count ) );

        const GLenum props[] = { GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE };
        GLint values[2] {};
        GLchar name[256] {};
        GLsizei length{};
        for ( GLint i = 0; i < storage_block_count; ++i )
        {
            glGetProgramResourceName ( aProgramId, GL_SHADER_STORAGE_BLOCK,
                                       static_cast<GLuint> ( i ),
                                       sizeof ( name ), &length, name );
            OPENGL_CHECK_ERROR_THROW;
            glGetProgramResourceiv ( aProgramId, GL_SHADER_STORAGE_BLOCK,
                                     static_cast<GLuint> ( i ),
                                     2, props, 2, nullptr, values );
            OPENGL_CHECK_ERROR_THROW;
            const GLint binding = values[0];
            const GLint size    = values[1];
            const uint32_t name_crc{crc32i ( name, length ) };
            auto it = std::lower_bound ( mStorageBlocks.begin(), mStorageBlocks.end(), name_crc,
                                         [] ( const OpenGLUniformBlock & a, const uint32_t b )
            {
                return a.name < b;
            } );
            // A block of the same name reflected from another program is
            // already recorded; skip it.
            if ( it != mStorageBlocks.end() && it->name == name_crc )
            {
                continue;
            }
            mStorageBlocks.insert ( it, { name_crc, size, binding } );
        }
        for ( const auto& block : mStorageBlocks )
        {
            std::cout << LogLevel::Debug << "Storage Block: " << block.name << " (crc: " << std::hex << block.name << std::dec << ", size: " << block.size << ", binding: " << block.binding << ")" << std::endl;
        }
    }

    const OpenGLUniformBlock* OpenGLPipeline::GetStorageBlock ( uint32_t name ) const
    {
        auto it = std::lower_bound ( mStorageBlocks.begin(), mStorageBlocks.end(), name,
                                     [] ( const OpenGLUniformBlock & a, const uint32_t b )
        {
            return a.name < b;
        } );
        if ( it == mStorageBlocks.end() || it->name != name )
        {
            return nullptr;
        }
        return &*it;
    }
}
