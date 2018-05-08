/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include <utility>
#include <vector>
#include "aeongames/ResourceCache.h"
#include "aeongames/Pipeline.h"
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLTexture.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    OpenGLPipeline::OpenGLPipeline ( const Pipeline& aPipeline, const std::shared_ptr<const OpenGLRenderer>&  aOpenGLRenderer ) :
        mPipeline ( aPipeline ),
        mOpenGLRenderer ( aOpenGLRenderer ),
        mDefaultMaterial ( mPipeline.GetDefaultMaterial(), mOpenGLRenderer )
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

    void OpenGLPipeline::Use ( const OpenGLMaterial* aMaterial ) const
    {
        const OpenGLMaterial* material = ( aMaterial ) ? aMaterial : &mDefaultMaterial;
        glUseProgram ( mProgramId );
        OPENGL_CHECK_ERROR_NO_THROW;
        for ( GLenum i = 0; i < material->GetTextures().size(); ++i )
        {
            glActiveTexture ( GL_TEXTURE0 + i );
            OPENGL_CHECK_ERROR_NO_THROW;
            glBindTexture ( GL_TEXTURE_2D, material->GetTextures() [i]->GetTexture() );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        glBindBuffer ( GL_UNIFORM_BUFFER, mPropertiesBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBufferData ( GL_UNIFORM_BUFFER, material->GetUniformData().size(), material->GetUniformData().data(), GL_DYNAMIC_DRAW );
        OPENGL_CHECK_ERROR_THROW;
        glBindBufferBase ( GL_UNIFORM_BUFFER, 1, mPropertiesBuffer );
        OPENGL_CHECK_ERROR_THROW;
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

        const auto* vertex_shader_source_ptr = reinterpret_cast<const GLchar *> ( mPipeline.GetVertexShaderSource().c_str() );
        auto vertex_shader_len = static_cast<GLint> ( mPipeline.GetVertexShaderSource().length() );

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
                std::cout << mPipeline.GetVertexShaderSource() << std::endl;
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

        const auto* fragment_shader_source_ptr = reinterpret_cast<const GLchar *> ( mPipeline.GetFragmentShaderSource().c_str() );
        auto fragment_shader_len = static_cast<GLint> ( mPipeline.GetFragmentShaderSource().length() );

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
                std::cout << mPipeline.GetFragmentShaderSource() << std::endl;
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
                std::cout << mPipeline.GetVertexShaderSource() << std::endl;
                std::cout << mPipeline.GetFragmentShaderSource() << std::endl;
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

        // Properties
        if ( mPipeline.GetDefaultMaterial().GetUniformBlockSize() )
        {
#if 1
            for ( GLenum i = 0; i < mDefaultMaterial.GetTextures().size(); ++i )
            {
                glUniform1i ( i, i );
                OPENGL_CHECK_ERROR_THROW;
            }
#else
            // Keeping this code for reference
            GLuint uniform = 0;
            for ( auto& i : mPipeline.GetDefaultMaterial().GetUniformMetaData() )
            {
                if ( i.GetType() == Uniform::SAMPLER_2D )
                {
                    auto location = glGetUniformLocation ( mProgramId, i.GetName().c_str() );
                    OPENGL_CHECK_ERROR_THROW;
                    glUniform1i ( location, uniform++ );
                    OPENGL_CHECK_ERROR_THROW;
                }
            }
#endif
            /** @todo Keep a buffer per material? */
            glGenBuffers ( 1, &mPropertiesBuffer );
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
