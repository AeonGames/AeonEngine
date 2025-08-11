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
#include "aeongames/Pipeline.h"
#include "OpenGLPipeline.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{

    OpenGLPipeline::OpenGLPipeline ( OpenGLPipeline&& aOpenGLPipeline ) :
        mOpenGLRenderer{aOpenGLPipeline.mOpenGLRenderer}
    {
        std::swap ( mPipeline, aOpenGLPipeline.mPipeline );
        std::swap ( mProgramId, aOpenGLPipeline.mProgramId );
    }
#if 0
    static std::string GetVertexShaderCode ( const Pipeline& aPipeline )
    {
        std::string vertex_shader{ "#version 450\n" };
        vertex_shader.append ( aPipeline.GetAttributes() );

        std::string transforms (
            "layout(binding = " + std::to_string ( MATRICES ) + ", std140) uniform Matrices{\n"
            "mat4 ModelMatrix;\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );
        vertex_shader.append ( transforms );

        if ( aPipeline.GetUniformDescriptors().size() )
        {
            std::string properties (
                "layout(binding = " + std::to_string ( MATERIAL ) +
                ",std140) uniform Properties{\n" +
                aPipeline.GetProperties() + "};\n" );

            if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
            {
                std::string skeleton (
                    "layout(std140, binding = " + std::to_string ( SKELETON ) + ") uniform Skeleton{\n"
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
            "layout(binding = " + std::to_string ( MATRICES ) + ", std140) uniform Matrices{\n"
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
                "layout(binding = " + std::to_string ( MATERIAL ) + ",std140) uniform Properties{\n" +
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
#endif

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

    uint32_t OpenGLPipeline::GetProgramId() const
    {
        return mProgramId;
    }
}
