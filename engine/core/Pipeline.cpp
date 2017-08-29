/*
Copyright (C) 2017-2016 Rodrigo Jose Hernandez Cordoba

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
#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <regex>
#include <array>
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#include "aeongames/Utilities.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "pipeline.pb.h"
#include "property.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    static const std::array<const char*, 7> AttributeStrings
    {
        "VertexPosition",
        "VertexNormal",
        "VertexTangent",
        "VertexBitangent",
        "VertexUV",
        "VertexWeightIndices",
        "VertexWeights"
    };

    static const std::array<const char*, 7> AttributeTypes
    {
        "vec3",
        "vec3",
        "vec3",
        "vec3",
        "vec2",
        "vec4",
        "vec4"
    };

    Pipeline::Pipeline ( std::string  aFilename ) :
        mFilename ( std::move ( aFilename ) ), mAttributes ( 0 ), mVertexShader(), mFragmentShader()
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

    Pipeline::~Pipeline()
        = default;

    const std::string & Pipeline::GetVertexShaderSource() const
    {
        return mVertexShader;
    }

    const std::string & Pipeline::GetFragmentShaderSource() const
    {
        return mFragmentShader;
    }

    uint32_t Pipeline::GetAttributes() const
    {
        return mAttributes;
    }

    uint32_t Pipeline::GetLocation ( AttributeBits aAttributeBit ) const
    {
        return ffs ( aAttributeBit );
    }

    uint32_t Pipeline::GetStride() const
    {
        uint32_t stride = 0;
        for ( uint32_t i = 0; i < ffs ( ~VertexAllBits ); ++i )
        {
            if ( mAttributes & ( 1 << i ) )
            {
                stride += GetSize ( static_cast<AttributeBits> ( 1 << i ) );
            }
        }
        return stride;
    }

    Pipeline::AttributeFormat Pipeline::GetFormat ( AttributeBits aAttributeBit ) const
    {
        return ( aAttributeBit & VertexUVBit ) ? Vector2Float :
               ( aAttributeBit & ( VertexWeightIndicesBit | VertexWeightsBit ) ) ? Vector4Byte : Vector3Float;
    }

    uint32_t Pipeline::GetSize ( AttributeBits aAttributeBit ) const
    {
        switch ( GetFormat ( aAttributeBit ) )
        {
        case Vector2Float:
            return sizeof ( float ) * 2;
        case Vector3Float:
            return sizeof ( float ) * 3;
        case Vector4Byte:
            return sizeof ( uint8_t ) * 4;
        }
        return 0;
    }

    uint32_t Pipeline::GetOffset ( AttributeBits aAttributeBit ) const
    {
        uint32_t offset = 0;
        for ( uint32_t i = 1; i != aAttributeBit; i = i << 1 )
        {
#if 0
            if ( i & mAttributes )
            {
#endif
                offset += GetSize ( static_cast<AttributeBits> ( i ) );
#if 0
            }
#endif
        }
        return offset;
    }

    const std::shared_ptr<Material> Pipeline::GetDefaultMaterial() const
    {
        return mDefaultMaterial;
    }

    void Pipeline::Initialize()
    {
        static PipelineBuffer pipeline_buffer;
        LoadProtoBufObject<PipelineBuffer> ( pipeline_buffer, mFilename, "AEONPRG" );
        {
            mVertexShader.append ( "#version 430\n" );
            /* Find out which attributes are being used and add them to the shader source */
            std::smatch attribute_matches;
            /**@note static const regex: construct once, use for ever.*/
            static const std::regex attribute_regex (
                "\\bVertexPosition\\b|"
                "\\bVertexNormal\\b|"
                "\\bVertexTangent\\b|"
                "\\bVertexBitangent\\b|"
                "\\bVertexUV\\b|"
                "\\bVertexWeightIndices\\b|"
                "\\bVertexWeights\\b" );
            std::string code = pipeline_buffer.vertex_shader().code();
            while ( std::regex_search ( code, attribute_matches, attribute_regex ) )
            {
                std::cout << attribute_matches.str() << std::endl;
                for ( uint32_t i = 0; i < AttributeStrings.size(); ++i )
                {
                    if ( attribute_matches.str().substr ( 6 ) == AttributeStrings[i] + 6 )
                    {
                        if ( ! ( mAttributes & ( 1 << i ) ) )
                        {
                            mAttributes |= ( 1 << i );
                            mVertexShader.append ( "layout(location = " );
                            mVertexShader.append ( std::to_string ( i ) );
                            mVertexShader.append ( ") in " );
                            mVertexShader.append ( AttributeTypes[i] );
                            mVertexShader.append ( " " );
                            mVertexShader.append ( AttributeStrings[i] );
                            mVertexShader.append ( ";\n" );
                        }
                        break;
                    }
                }
                code = attribute_matches.suffix();
            }
            mVertexShader.append (
                "#ifdef VULKAN\n"
                "layout(set = 0, binding = 0, std140) uniform Matrices{\n"
                "#else\n"
                "layout(binding = 0, std140) uniform Matrices{\n"
                "#endif\n"
                "mat4 ViewMatrix;\n"
                "mat4 ProjectionMatrix;\n"
                "mat4 ModelMatrix;\n"
                "mat4 ViewProjectionMatrix;\n"
                "mat4 ModelViewMatrix;\n"
                "mat4 ModelViewProjectionMatrix;\n"
                "mat3 NormalMatrix;\n"
                "};\n"
            );
            mFragmentShader.append ( "#version 430\n" );
            mFragmentShader.append (
                "#ifdef VULKAN\n"
                "layout(set = 0, binding = 0, std140) uniform Matrices{\n"
                "#else\n"
                "layout(binding = 0, std140) uniform Matrices{\n"
                "#endif\n"
                "mat4 ViewMatrix;\n"
                "mat4 ProjectionMatrix;\n"
                "mat4 ModelMatrix;\n"
                "mat4 ViewProjectionMatrix;\n"
                "mat4 ModelViewMatrix;\n"
                "mat4 ModelViewProjectionMatrix;\n"
                "mat3 NormalMatrix;\n"
                "};\n" );
            if ( mAttributes & ( VertexWeightIndicesBit | VertexWeightsBit ) )
            {
                std::string skeleton (
                    "#ifdef VULKAN\n"
                    "layout(std140, set = 0, binding = 2) uniform Skeleton{\n"
                    "#else\n"
                    "layout(std140, binding = 2) uniform Skeleton{\n"
                    "#endif\n"
                    "mat4 skeleton[256];\n"
                    "};\n"
                );
                mVertexShader.append ( skeleton );
            }

            mDefaultMaterial = std::make_shared<Material> ( pipeline_buffer.default_material() );
            if ( mDefaultMaterial->GetUniformMetaData().size() )
            {
                uint32_t sampler_binding = 0;
                std::string properties (
                    "#ifdef VULKAN\n"
                    "layout(set = 0, binding = 1,std140) uniform Properties{\n"
                    "#else\n"
                    "layout(binding = 1,std140) uniform Properties{\n"
                    "#endif\n"
                );
                std::string samplers ( "//----SAMPLERS-START----\n" );
                for ( auto& i : mDefaultMaterial->GetUniformMetaData() )
                {
                    switch ( i.GetType() )
                    {
                    case Uniform::Type::SAMPLER_2D:
                        samplers.append ( "#ifdef VULKAN\n" );
                        samplers.append ( "layout(set = 1, binding = " +
                                          std::to_string ( sampler_binding ) +
                                          ", location =" + std::to_string ( sampler_binding ) + ") " );
                        samplers.append ( i.GetDeclaration() );
                        samplers.append ( "#else\n" );
                        samplers.append ( "layout(location = " + std::to_string ( sampler_binding ) + ") " );
                        samplers.append ( i.GetDeclaration() );
                        samplers.append ( "#endif\n" );
                        ++sampler_binding;
                        break;
                    default:
                        properties.append ( i.GetDeclaration() );
                        break;
                    }
                }
                properties.append ( "};\n" );
                samplers.append ( "//----SAMPLERS-END----\n" );
                mVertexShader.append ( properties );
                mVertexShader.append ( samplers );
                mFragmentShader.append ( properties );
                mFragmentShader.append ( samplers );
            }

            switch ( pipeline_buffer.vertex_shader().source_case() )
            {
            case ShaderBuffer::SourceCase::kCode:
            {
                mVertexShader.append ( pipeline_buffer.vertex_shader().code() );
            }
            break;
            default:
                throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
            }
            switch ( pipeline_buffer.fragment_shader().source_case() )
            {
            case ShaderBuffer::SourceCase::kCode:
                mFragmentShader.append ( pipeline_buffer.fragment_shader().code() );
                break;
            default:
                throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
            }
        }
        pipeline_buffer.Clear();
#if 0
        std::ofstream shader_vert ( "shader.vert" );
        std::ofstream shader_frag ( "shader.frag" );
        shader_vert << mVertexShader;
        shader_frag << mFragmentShader;
        shader_vert.close();
        shader_frag.close();
#endif
    }

    void Pipeline::Finalize()
    {
    }
}
