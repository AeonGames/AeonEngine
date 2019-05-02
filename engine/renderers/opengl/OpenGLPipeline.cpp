/*
Copyright (C) 2016-2019 Rodrigo Jose Hernandez Cordoba

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
#include <array>
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufUtils.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "pipeline.pb.h"
#include "property.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "OpenGLPipeline.h"
#include "OpenGLMaterial.h"
#include "OpenGLImage.h"
#include "OpenGLUniformBuffer.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    static GLenum GetGLTopology ( PipelineBuffer_Topology aTopology )
    {
        switch ( aTopology )
        {
        case PipelineBuffer_Topology::PipelineBuffer_Topology_POINT_LIST:
            return GL_POINTS;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_LINE_STRIP:
            return GL_LINE_STRIP;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_LINE_LIST:
            return GL_LINES;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_STRIP:
            return GL_TRIANGLE_STRIP;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_FAN:
            return GL_TRIANGLE_FAN;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_LIST:
            return GL_TRIANGLES;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_LINE_LIST_WITH_ADJACENCY:
            return GL_LINES_ADJACENCY;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_LINE_STRIP_WITH_ADJACENCY:
            return GL_LINE_STRIP_ADJACENCY;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_LIST_WITH_ADJACENCY:
            return GL_TRIANGLES_ADJACENCY;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_STRIP_WITH_ADJACENCY:
            return GL_TRIANGLE_STRIP_ADJACENCY;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_PATCH_LIST:
            return GL_PATCHES;
        default:
            break;
        }
        return GL_POINTS;
    }

    OpenGLPipeline::OpenGLPipeline ( uint32_t aPath )
    {
        if ( aPath )
        {
            Pipeline::Load ( aPath );
        }
    }

    OpenGLPipeline::~OpenGLPipeline()
    {
        Unload();
    }

    const OpenGLMaterial& OpenGLPipeline::GetDefaultMaterial() const
    {
        return mDefaultMaterial;
    }

    static std::string GetVertexShaderCode ( const PipelineBuffer& aPipelineBuffer )
    {
        std::string vertex_shader{ "#version 450\n" };
        vertex_shader.append ( GetAttributesGLSL ( aPipelineBuffer ) );

        uint32_t uniform_block_binding{0};

        std::string transforms (
            "layout(binding = " + std::to_string ( uniform_block_binding++ ) + ", std140) uniform Matrices{\n"
            "mat4 ModelMatrix;\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );
        vertex_shader.append ( transforms );

        if ( aPipelineBuffer.default_material().property().size() )
        {
            std::string properties (
                "layout(binding = " + std::to_string ( uniform_block_binding++ ) +
                ",std140) uniform Properties{\n" +
                GetPropertiesGLSL ( aPipelineBuffer ) + "};\n" );

            if ( GetAttributes ( aPipelineBuffer ) & ( VertexWeightIdxBit | VertexWeightBit ) )
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
            for ( auto& i : aPipelineBuffer.default_material().sampler() )
            {
                samplers += "layout(binding = " + std::to_string ( sampler_binding ) + ") uniform sampler2D " + i.name() + ";\n";
                ++sampler_binding;
            }
            samplers.append ( "//----SAMPLERS-END----\n" );

            vertex_shader.append ( properties );
            vertex_shader.append ( samplers );
        }

        switch ( aPipelineBuffer.vertex_shader().source_case() )
        {
        case ShaderBuffer::SourceCase::kCode:
        {
            vertex_shader.append ( aPipelineBuffer.vertex_shader().code() );
        }
        break;
        default:
            throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
        }
        return vertex_shader;
    }

    static std::string GetFragmentShaderCode ( const PipelineBuffer& aPipelineBuffer )
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

        if ( aPipelineBuffer.default_material().property().size() )
        {
            std::string properties
            {
                "layout(binding = 1,std140) uniform Properties{\n" +
                GetPropertiesGLSL ( aPipelineBuffer ) +
                "};\n"};

            uint32_t sampler_binding = 0;
            std::string samplers ( "//----SAMPLERS-START----\n" );
            for ( auto& i : aPipelineBuffer.default_material().sampler() )
            {
                samplers += "layout(location = " + std::to_string ( sampler_binding ) + ") uniform sampler2D " + i.name() + ";\n";
                ++sampler_binding;
            }
            samplers.append ( "//----SAMPLERS-END----\n" );

            fragment_shader.append ( properties );
            fragment_shader.append ( samplers );
        }
        switch ( aPipelineBuffer.fragment_shader().source_case() )
        {
        case ShaderBuffer::SourceCase::kCode:
            fragment_shader.append ( aPipelineBuffer.fragment_shader().code() );
            break;
        default:
            throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
        }
        return fragment_shader;
    }

    void OpenGLPipeline::Load ( const PipelineBuffer& aPipelineBuffer )
    {
        if ( glIsProgram ( mProgramId ) )
        {
            throw std::runtime_error ( "OpenGL object already loaded." );
        }

        mTopology = GetGLTopology ( aPipelineBuffer.topology() );
        mDefaultMaterial.Load ( aPipelineBuffer.default_material() );
        std::string vertex_shader_code = GetVertexShaderCode ( aPipelineBuffer );
        std::string fragment_shader_code = GetFragmentShaderCode ( aPipelineBuffer );

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
        glAttachShader ( mProgramId, vertex_shader );
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
                std::cout << vertex_shader_code << std::endl;
                std::cout << fragment_shader_code << std::endl;
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

        // Samplers
#if 1
        for ( GLuint uniform = 0; uniform < GetDefaultMaterial().GetSamplers().size(); ++uniform )
        {
            glUniform1i ( uniform, uniform );
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
    }

    void OpenGLPipeline::Unload()
    {
        mTopology = 0;
        OPENGL_CHECK_ERROR_NO_THROW;
        if ( glIsProgram ( mProgramId ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteProgram ( mProgramId );
            OPENGL_CHECK_ERROR_NO_THROW;
            mProgramId = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
        mDefaultMaterial.Unload();
    }

    void OpenGLPipeline::Use ( const OpenGLMaterial& aMaterial, const OpenGLUniformBuffer* aSkeletonBuffer ) const
    {
        glUseProgram ( mProgramId );
        OPENGL_CHECK_ERROR_NO_THROW;

        for ( GLenum i = 0; i < aMaterial.GetSamplers().size(); ++i )
        {
            glActiveTexture ( GL_TEXTURE0 + i );
            OPENGL_CHECK_ERROR_NO_THROW;
            glBindTexture ( GL_TEXTURE_2D, reinterpret_cast<OpenGLImage*> ( std::get<1> ( aMaterial.GetSamplers() [i] ).Cast<Image>() )->GetTextureId() );
            OPENGL_CHECK_ERROR_NO_THROW;
        }

        // Binding 0 is the matrix buffer.
        GLuint index{1};
        if ( GLuint buffer = aMaterial.GetPropertiesBufferId() )
        {
            glBindBuffer ( GL_UNIFORM_BUFFER, buffer );
            OPENGL_CHECK_ERROR_THROW;
            glBindBufferBase ( GL_UNIFORM_BUFFER, index++, buffer );
            OPENGL_CHECK_ERROR_THROW;
        }

        if ( aSkeletonBuffer != nullptr && aSkeletonBuffer->GetBufferId() )
        {
            glBindBuffer ( GL_UNIFORM_BUFFER, aSkeletonBuffer->GetBufferId() );
            OPENGL_CHECK_ERROR_THROW;
            glBindBufferBase ( GL_UNIFORM_BUFFER, index++, aSkeletonBuffer->GetBufferId() );
            OPENGL_CHECK_ERROR_THROW;
        }
    }

    GLenum OpenGLPipeline::GetTopology() const
    {
        return mTopology;
    }
}
