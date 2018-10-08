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
#include "aeongames/Material.h"
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLTexture.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    OpenGLPipeline::OpenGLPipeline ( const OpenGLRenderer&  aOpenGLRenderer ) :
        Pipeline(),
        mOpenGLRenderer ( aOpenGLRenderer )
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

    void OpenGLPipeline::Use ( const OpenGLMaterial& aMaterial ) const
    {
        glUseProgram ( mProgramId );
        OPENGL_CHECK_ERROR_NO_THROW;
        GLenum index{0};
        for ( auto& i : aMaterial.GetMaterial().GetProperties() )
        {
            if ( i.GetType() == Material::Material::PropertyType::SAMPLER_2D )
            {
                glActiveTexture ( GL_TEXTURE0 + index++ );
                OPENGL_CHECK_ERROR_NO_THROW;
                glBindTexture ( GL_TEXTURE_2D, reinterpret_cast<const OpenGLTexture*> ( i.GetImage() )->GetTexture() );
                OPENGL_CHECK_ERROR_NO_THROW;
            }
        }

        glBindBuffer ( GL_UNIFORM_BUFFER, aMaterial.GetPropertiesBufferId() );
        OPENGL_CHECK_ERROR_THROW;
        glBindBufferBase ( GL_UNIFORM_BUFFER, 1, aMaterial.GetPropertiesBufferId() );
        OPENGL_CHECK_ERROR_THROW;
    }

    GLenum OpenGLPipeline::GetGLTopology() const
    {
        switch ( GetTopology() )
        {
        case Pipeline::Topology::POINT_LIST:
            return GL_POINTS;
        case Pipeline::Topology::LINE_STRIP:
            return GL_LINE_STRIP;
        case Pipeline::Topology::LINE_LIST:
            return GL_LINES;
        case Pipeline::Topology::TRIANGLE_STRIP:
            return GL_TRIANGLE_STRIP;
        case Pipeline::Topology::TRIANGLE_FAN:
            return GL_TRIANGLE_FAN;
        case Pipeline::Topology::TRIANGLE_LIST:
            return GL_TRIANGLES;
        case Pipeline::Topology::LINE_LIST_WITH_ADJACENCY:
            return GL_LINES_ADJACENCY;
        case Pipeline::Topology::LINE_STRIP_WITH_ADJACENCY:
            return GL_LINE_STRIP_ADJACENCY;
        case Pipeline::Topology::TRIANGLE_LIST_WITH_ADJACENCY:
            return GL_TRIANGLES_ADJACENCY;
        case Pipeline::Topology::TRIANGLE_STRIP_WITH_ADJACENCY:
            return GL_TRIANGLE_STRIP_ADJACENCY;
        case Pipeline::Topology::PATCH_LIST:
            return GL_PATCHES;
        default:
            break;
        }
        return GL_POINTS;
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

        const auto* vertex_shader_source_ptr = reinterpret_cast<const GLchar *> ( GetVertexShaderSource().c_str() );
        auto vertex_shader_len = static_cast<GLint> ( GetVertexShaderSource().length() );

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
                std::cout << GetVertexShaderSource() << std::endl;
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

        const auto* fragment_shader_source_ptr = reinterpret_cast<const GLchar *> ( GetFragmentShaderSource().c_str() );
        auto fragment_shader_len = static_cast<GLint> ( GetFragmentShaderSource().length() );

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
                std::cout << GetFragmentShaderSource() << std::endl;
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
                std::cout << GetVertexShaderSource() << std::endl;
                std::cout << GetFragmentShaderSource() << std::endl;
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
        if ( GetDefaultMaterial().GetPropertyBlock().size() )
        {
#if 1
            GLuint uniform = 0;
            for ( auto& i : GetDefaultMaterial().GetProperties() )
            {
                if ( i.GetType() == Material::SAMPLER_2D )
                {
                    glUniform1i ( uniform, uniform );
                    OPENGL_CHECK_ERROR_THROW;
                    uniform++;
                }
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
    }
}
