/*
Copyright (C) 2016-2017 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include <vector>
#include "aeongames/ResourceCache.h"
#include "aeongames/Pipeline.h"
#include "OpenGLPipeline.h"
#include "OpenGLTexture.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    OpenGLPipeline::OpenGLPipeline ( const std::shared_ptr<Pipeline> aProgram ) :
        mPipeline ( aProgram ),
        mProgramId ( 0 )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    OpenGLPipeline::~OpenGLPipeline()
    {
        Finalize();
    }

    void OpenGLPipeline::Use() const
    {
        glUseProgram ( mProgramId );
        OPENGL_CHECK_ERROR_NO_THROW;
        for ( auto& i : mTextures )
        {
            glActiveTexture ( GL_TEXTURE0 + i.second );
            OPENGL_CHECK_ERROR_NO_THROW;
            glBindTexture ( GL_TEXTURE_2D, i.first->GetTexture() );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }

    void OpenGLPipeline::Initialize()
    {
        //--------------------------------------------------
        // Begin OpenGL Specific code
        //--------------------------------------------------
        mProgramId = glCreateProgram();
        OPENGL_CHECK_ERROR_THROW;
        GLint compile_status;
        GLint link_status;
        //-------------------------
        uint32_t vertex_shader = glCreateShader ( GL_VERTEX_SHADER );
        OPENGL_CHECK_ERROR_THROW;

        const GLchar* vertex_shader_source_ptr = reinterpret_cast<const GLchar *> ( mPipeline->GetVertexShaderSource().c_str() );
        GLint vertex_shader_len = static_cast<GLint> ( mPipeline->GetVertexShaderSource().length() );

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
                std::cout << mPipeline->GetVertexShaderSource() << std::endl;
                std::cout << log_string << std::endl;
                throw std::runtime_error ( log_string.c_str() );
            }
            throw std::runtime_error ( "Error Compiling Shaders." );
        }
        glAttachShader ( mProgramId, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        //-------------------------

        uint32_t fragment_shader = glCreateShader ( GL_FRAGMENT_SHADER );
        OPENGL_CHECK_ERROR_THROW;

        const GLchar* fragment_shader_source_ptr = reinterpret_cast<const GLchar *> ( mPipeline->GetFragmentShaderSource().c_str() );
        GLint fragment_shader_len = static_cast<GLint> ( mPipeline->GetFragmentShaderSource().length() );

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
                std::cout << mPipeline->GetFragmentShaderSource() << std::endl;
                std::cout << log_string << std::endl;
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        glAttachShader ( mProgramId, fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
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
                std::cout << mPipeline->GetVertexShaderSource() << std::endl;
                std::cout << mPipeline->GetFragmentShaderSource() << std::endl;
                std::cout << log_string << std::endl;
                OPENGL_CHECK_ERROR_THROW;
            }
        }
        glDetachShader ( mProgramId, vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDetachShader ( mProgramId, fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDeleteShader ( vertex_shader );
        OPENGL_CHECK_ERROR_THROW;
        glDeleteShader ( fragment_shader );
        OPENGL_CHECK_ERROR_THROW;
        /* We need to bind the program to set any samplers. */
        glUseProgram ( mProgramId );
        OPENGL_CHECK_ERROR_THROW;

        GLint block_size;

        // Matrices
        mMatricesBlockIndex = glGetUniformBlockIndex ( mProgramId, "Matrices" );
        OPENGL_CHECK_ERROR_THROW;

        glGetActiveUniformBlockiv ( mProgramId, mMatricesBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &block_size );
        OPENGL_CHECK_ERROR_THROW;
        assert ( static_cast<uint32_t> ( block_size ) >= ( sizeof ( float ) * 16 * 6 ) + ( sizeof ( float ) * 12 * 1 ) );
        glUniformBlockBinding ( mProgramId, mMatricesBlockIndex, 0 );
        OPENGL_CHECK_ERROR_THROW;

        // Properties
        if ( mPipeline->GetDefaultMaterial().GetUniformMetaData().size() )
        {
            {
                // Get offsets (initialize mUniformMetaData)
                std::vector<const GLchar *> uniform_names ( mPipeline->GetDefaultMaterial().GetUniformMetaData().size() );
                for ( std::size_t i = 0; i < mPipeline->GetDefaultMaterial().GetUniformMetaData().size(); ++i )
                {
                    uniform_names[i] = mPipeline->GetDefaultMaterial().GetUniformMetaData() [i].GetName().c_str();
                }
                std::vector<GLuint> uniform_indices ( mPipeline->GetDefaultMaterial().GetUniformMetaData().size() );
                glGetUniformIndices ( mProgramId, static_cast<GLsizei> ( mPipeline->GetDefaultMaterial().GetUniformMetaData().size() ),
                                      uniform_names.data(), uniform_indices.data() );
                OPENGL_CHECK_ERROR_THROW;

                std::vector<GLint> uniform_offset ( mPipeline->GetDefaultMaterial().GetUniformMetaData().size() );
                glGetActiveUniformsiv ( mProgramId, static_cast<GLsizei> ( mPipeline->GetDefaultMaterial().GetUniformMetaData().size() ),
                                        uniform_indices.data(),
                                        GL_UNIFORM_OFFSET, uniform_offset.data() );
                OPENGL_CHECK_ERROR_THROW;

                // Get and initialize data block (initialize mUniformData)
                mPropertiesBlockIndex = glGetUniformBlockIndex ( mProgramId, "Properties" );
                OPENGL_CHECK_ERROR_THROW;
                glGetActiveUniformBlockiv ( mProgramId, mPropertiesBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &block_size );
                OPENGL_CHECK_ERROR_THROW;
                glUniformBlockBinding ( mProgramId, mPropertiesBlockIndex, 1 );
                OPENGL_CHECK_ERROR_THROW;
                mUniformData.resize ( block_size );
                GLint image_unit = 0;
                // Set default uniform values
                for ( std::size_t i = 0; i < mPipeline->GetDefaultMaterial().GetUniformMetaData().size(); ++i )
                {
                    switch ( mPipeline->GetDefaultMaterial().GetUniformMetaData() [i].GetType() )
                    {
                    case Uniform::FLOAT_VEC4:
                        * ( reinterpret_cast<float*> ( mUniformData.data() + uniform_offset[i] ) + 3 ) = mPipeline->GetDefaultMaterial().GetUniformMetaData() [i].GetW();
                    /* Intentional Pass-Thru */
                    case Uniform::FLOAT_VEC3:
                        * ( reinterpret_cast<float*> ( mUniformData.data() + uniform_offset[i] ) + 2 ) = mPipeline->GetDefaultMaterial().GetUniformMetaData() [i].GetZ();
                    /* Intentional Pass-Thru */
                    case Uniform::FLOAT_VEC2:
                        * ( reinterpret_cast<float*> ( mUniformData.data() + uniform_offset[i] ) + 1 ) = mPipeline->GetDefaultMaterial().GetUniformMetaData() [i].GetY();
                    /* Intentional Pass-Thru */
                    case Uniform::FLOAT:
                        * ( reinterpret_cast<float*> ( mUniformData.data() + uniform_offset[i] ) + 0 ) = mPipeline->GetDefaultMaterial().GetUniformMetaData() [i].GetX();
                        break;
                    case Uniform::SAMPLER_2D:
                    {
                        if ( image_unit >= ( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS - GL_TEXTURE0 ) )
                        {
                            throw std::runtime_error ( "OpenGL texture image unit values exausted (Too many samplers in shader)." );
                        }
                        mTextures.emplace_back ( Get<OpenGLTexture> ( mPipeline->GetDefaultMaterial().GetUniformMetaData() [i].GetImage() ), image_unit );
                        auto location = glGetUniformLocation ( mProgramId, mPipeline->GetDefaultMaterial().GetUniformMetaData() [i].GetName().c_str() );
                        OPENGL_CHECK_ERROR_THROW;
                        glUniform1i ( location, image_unit++ );
                        OPENGL_CHECK_ERROR_THROW;
                    }
                    break;
                    default:
                        break;
                    }
                }
            }
            glGenBuffers ( 1, &mPropertiesBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBindBuffer ( GL_UNIFORM_BUFFER, mPropertiesBuffer );
            OPENGL_CHECK_ERROR_THROW;
            glBufferData ( GL_UNIFORM_BUFFER, mUniformData.size(), mUniformData.data(), GL_DYNAMIC_DRAW );
            OPENGL_CHECK_ERROR_THROW;
            glBindBufferBase ( GL_UNIFORM_BUFFER, 1, mPropertiesBuffer );
            OPENGL_CHECK_ERROR_THROW;
        }
    }

    void OpenGLPipeline::Finalize()
    {
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( glIsProgram ( mProgramId ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteProgram ( mProgramId );
            OPENGL_CHECK_ERROR_NO_THROW;
            mProgramId = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( glIsBuffer ( mPropertiesBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteBuffers ( 1, &mPropertiesBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mPropertiesBuffer = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
    }
}
