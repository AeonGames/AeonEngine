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
#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <regex>
#include <array>
#include <mutex>
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
        {
            "VertexPosition",
            "VertexNormal",
            "VertexTangent",
            "VertexBitangent",
            "VertexUV",
            "VertexWeightIndices",
            "VertexWeights"
        }
    };

    static const std::array<const char*, 7> AttributeTypes
    {
        {
            "vec3",
            "vec3",
            "vec3",
            "vec3",
            "vec2",
            "uvec4",
            "vec4"
        }
    };

    Pipeline::Pipeline()
    {
    }

    DLL Pipeline::IRenderPipeline::~IRenderPipeline()
    {
    }

    Pipeline::Pipeline ( const std::string&  aFilename ) :
        mFilename ( aFilename ), mTopology ( Topology::POINT_LIST ), mAttributes ( 0 ), mVertexShader(), mFragmentShader()
    {
        try
        {
            Load ( aFilename );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Pipeline::Pipeline ( const void * aBuffer, size_t aBufferSize )
    {
        if ( !aBuffer && !aBufferSize )
        {
            throw std::runtime_error ( "Cannot initialize pipeline object with null data." );
        }
        try
        {
            Load ( aBuffer, aBufferSize );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Pipeline::~Pipeline()
        = default;

    void Pipeline::Load ( const std::string& aFilename )
    {
        static std::mutex m;
        static PipelineBuffer pipeline_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<PipelineBuffer> ( pipeline_buffer, aFilename, "AEONPRG" );
        Load ( pipeline_buffer );
        pipeline_buffer.Clear();
    }

    void Pipeline::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static std::mutex m;
        static PipelineBuffer pipeline_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<PipelineBuffer> ( pipeline_buffer, aBuffer, aBufferSize, "AEONPRG" );
        Load ( pipeline_buffer );
        pipeline_buffer.Clear();
    }

    void Pipeline::Load ( const PipelineBuffer& aPipelineBuffer )
    {
        switch ( aPipelineBuffer.topology() )
        {
        case PipelineBuffer_Topology_POINT_LIST:
            mTopology = Topology::POINT_LIST;
            break;
        case PipelineBuffer_Topology_LINE_STRIP:
            mTopology = Topology::LINE_STRIP;
            break;
        case PipelineBuffer_Topology_LINE_LIST:
            mTopology = Topology::LINE_LIST;
            break;
        case PipelineBuffer_Topology_TRIANGLE_STRIP:
            mTopology = Topology::TRIANGLE_STRIP;
            break;
        case PipelineBuffer_Topology_TRIANGLE_FAN:
            mTopology = Topology::TRIANGLE_FAN;
            break;
        case PipelineBuffer_Topology_TRIANGLE_LIST:
            mTopology = Topology::TRIANGLE_LIST;
            break;
        case PipelineBuffer_Topology_LINE_LIST_WITH_ADJACENCY:
            mTopology = Topology::LINE_LIST_WITH_ADJACENCY;
            break;
        case PipelineBuffer_Topology_LINE_STRIP_WITH_ADJACENCY:
            mTopology = Topology::LINE_STRIP_WITH_ADJACENCY;
            break;
        case PipelineBuffer_Topology_TRIANGLE_LIST_WITH_ADJACENCY:
            mTopology = Topology::TRIANGLE_LIST_WITH_ADJACENCY;
            break;
        case PipelineBuffer_Topology_TRIANGLE_STRIP_WITH_ADJACENCY:
            mTopology = Topology::TRIANGLE_STRIP_WITH_ADJACENCY;
            break;
        case PipelineBuffer_Topology_PATCH_LIST:
            mTopology = Topology::PATCH_LIST;
            break;
        default:
            break;
        }
        mVertexShader.append ( "#version 430\n" );
        mFragmentShader.append ( "#version 430\n" );
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
        std::string code = aPipelineBuffer.vertex_shader().code();
        while ( std::regex_search ( code, attribute_matches, attribute_regex ) )
        {
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
        std::string transforms (
            "#ifdef VULKAN\n"
            "layout(set = 0, binding = 0, std140) uniform Matrices{\n"
            "#else\n"
            "layout(binding = 0, std140) uniform Matrices{\n"
            "#endif\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );
        mVertexShader.append ( transforms );
        mFragmentShader.append ( transforms );

        mDefaultMaterial.Load ( aPipelineBuffer.default_material() );
        if ( mDefaultMaterial.GetUniforms().size() )
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
            for ( auto& i : mDefaultMaterial.GetUniforms() )
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
        if ( mAttributes & ( VertexWeightIndicesBit | VertexWeightsBit ) )
        {
            std::string skeleton (
                "#ifdef VULKAN\n"
                "layout(set = 0, binding = 2, std140) uniform Skeleton{\n"
                "#else\n"
                "layout(std140, binding = 2) uniform Skeleton{\n"
                "#endif\n"
                "mat4 skeleton[256];\n"
                "};\n"
            );
            mVertexShader.append ( skeleton );
            mFragmentShader.append ( skeleton );
        }
#if 0
        std::string mathlib (
            "vec4 MultQuatVec4(vec4 q, vec4 v)\n"
            "{\n"
            "	vec4 t = vec4(\n"
            "		(-q.y * v.x - q.z * v.y - q.w * v.z),\n"
            "		( q.x * v.x + q.z * v.z - q.w * v.y),\n"
            "		( q.x * v.y + q.w * v.x - q.y * v.z),\n"
            "		( q.x * v.z + q.y * v.y - q.z * v.x));\n"
            "	return vec4(\n"
            "		t.x * -q.y + t.y * q.x + t.z * -q.w - t.w * -q.z,\n"
            "		t.x * -q.z + t.z * q.x + t.w * -q.y - t.y * -q.w,\n"
            "		t.x * -q.w + t.w * q.x + t.y * -q.z - t.z * -q.y,\n"
            "		v.w);\n"
            "}\n"
            "vec4 MultQuats ( vec4 q1, vec4 q2 )\n"
            "{\n"
            "    return vec4(\n"
            "        q1.x * q2.x - q1.y * q2.y - q1.z * q2.z - q1.w * q2.w,\n"
            "        q1.x * q2.y + q1.y * q2.x + q1.z * q2.w - q1.w * q2.z,\n"
            "        q1.x * q2.z - q1.y * q2.w + q1.z * q2.x + q1.w * q2.y,\n"
            "        q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x);\n"
            "}\n"
        );
        mVertexShader.append ( mathlib );
        mFragmentShader.append ( mathlib );
#endif
        switch ( aPipelineBuffer.vertex_shader().source_case() )
        {
        case ShaderBuffer::SourceCase::kCode:
        {
            mVertexShader.append ( aPipelineBuffer.vertex_shader().code() );
        }
        break;
        default:
            throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
        }
        switch ( aPipelineBuffer.fragment_shader().source_case() )
        {
        case ShaderBuffer::SourceCase::kCode:
            mFragmentShader.append ( aPipelineBuffer.fragment_shader().code() );
            break;
        default:
            throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
        }
#if 1
        std::ofstream shader_vert ( "shader.vert" );
        std::ofstream shader_frag ( "shader.frag" );
        shader_vert << mVertexShader;
        shader_frag << mFragmentShader;
        shader_vert.close();
        shader_frag.close();
#endif
    }

    void Pipeline::Unload()
    {
        mFilename.clear();
        mAttributes = 0;
        mVertexShader.clear();
        mFragmentShader.clear();
        mDefaultMaterial.Unload();
    }

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
               ( aAttributeBit & VertexWeightIndicesBit ) ? Vector4Byte :
               ( aAttributeBit & VertexWeightsBit ) ? Vector4ByteNormalized : Vector3Float;
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
        case Vector4ByteNormalized:
            return sizeof ( uint8_t ) * 4;
        }
        return 0;
    }

    uint32_t Pipeline::GetOffset ( AttributeBits aAttributeBit ) const
    {
        uint32_t offset = 0;
        for ( uint32_t i = 1; i != aAttributeBit; i = i << 1 )
        {
            offset += GetSize ( static_cast<AttributeBits> ( i ) );
        }
        return offset;
    }

    const Material& Pipeline::GetDefaultMaterial() const
    {
        return mDefaultMaterial;
    }

    const Pipeline::Topology Pipeline::GetTopology() const
    {
        return mTopology;
    }

    void Pipeline::SetRenderPipeline ( std::unique_ptr<IRenderPipeline> aRenderPipeline ) const
    {
        mRenderPipeline = std::move ( aRenderPipeline );
    }

    const Pipeline::IRenderPipeline* const Pipeline::GetRenderPipeline() const
    {
        return mRenderPipeline.get();
    }
}
