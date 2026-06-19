/*
Copyright (C) 2016-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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

#include <unordered_map>
#include "aeongames/Pipeline.hpp"
#include "aeongames/ProtoBufClasses.hpp"
#include "aeongames/ProtoBufHelpers.hpp"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "pipeline.pb.h"
#include "property.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
#if 0
    static const std::unordered_map<PipelineMsg_Topology, Topology> TopologyMap
    {
        {PipelineMsg_Topology::PipelineMsg_Topology_POINT_LIST, POINT_LIST},
        {PipelineMsg_Topology::PipelineMsg_Topology_LINE_STRIP, LINE_STRIP},
        {PipelineMsg_Topology::PipelineMsg_Topology_LINE_LIST, LINE_LIST},
        {PipelineMsg_Topology::PipelineMsg_Topology_TRIANGLE_STRIP, TRIANGLE_STRIP},
        {PipelineMsg_Topology::PipelineMsg_Topology_TRIANGLE_FAN, TRIANGLE_FAN},
        {PipelineMsg_Topology::PipelineMsg_Topology_TRIANGLE_LIST, TRIANGLE_LIST},
        {PipelineMsg_Topology::PipelineMsg_Topology_LINE_LIST_WITH_ADJACENCY, LINE_LIST_WITH_ADJACENCY},
        {PipelineMsg_Topology::PipelineMsg_Topology_LINE_STRIP_WITH_ADJACENCY, LINE_STRIP_WITH_ADJACENCY},
        {PipelineMsg_Topology::PipelineMsg_Topology_TRIANGLE_LIST_WITH_ADJACENCY, TRIANGLE_LIST_WITH_ADJACENCY},
        {PipelineMsg_Topology::PipelineMsg_Topology_TRIANGLE_STRIP_WITH_ADJACENCY, TRIANGLE_STRIP_WITH_ADJACENCY},
        {PipelineMsg_Topology::PipelineMsg_Topology_PATCH_LIST, PATCH_LIST},
    };

    static const std::unordered_map<UniformDescriptorMsg_Type, UniformType> UniformTypeMap
    {
        {UniformDescriptorMsg_Type::UniformDescriptorMsg_Type_SCALAR_FLOAT, SCALAR_FLOAT},
        {UniformDescriptorMsg_Type::UniformDescriptorMsg_Type_SCALAR_UINT, SCALAR_UINT},
        {UniformDescriptorMsg_Type::UniformDescriptorMsg_Type_SCALAR_INT, SCALAR_INT},
        {UniformDescriptorMsg_Type::UniformDescriptorMsg_Type_VECTOR_FLOAT_2, VECTOR_FLOAT_2},
        {UniformDescriptorMsg_Type::UniformDescriptorMsg_Type_VECTOR_FLOAT_3, VECTOR_FLOAT_3},
        {UniformDescriptorMsg_Type::UniformDescriptorMsg_Type_VECTOR_FLOAT_4, VECTOR_FLOAT_4},
    };
#endif
    Pipeline::Pipeline() = default;

    Pipeline::~Pipeline()
        = default;
    uint32_t Pipeline::GetTopologyClass() const
    {
        return mTopologyClass;
    }

#if 0
    const std::string& Pipeline::GetVertexShaderCode() const
    {
        return mVertexShaderCode;
    }
    const std::string& Pipeline::GetFragmentShaderCode() const
    {
        return mFragmentShaderCode;
    }

    const std::vector<std::tuple<UniformType, std::string >> & Pipeline::GetUniformDescriptors() const
    {
        return mUniformDescriptors;
    }

    const std::vector<std::string>& Pipeline::GetSamplerDescriptors() const
    {
        return mSamplerDescriptors;
    }

    std::string Pipeline::GetProperties () const
    {
        std::string properties{};
        for ( auto& i : mUniformDescriptors )
        {
            switch ( std::get<0> ( i ) )
            {
            case SCALAR_FLOAT:
                properties += "float " + std::get<1> ( i ) + ";\n";
                break;
            case SCALAR_UINT:
                properties += "uint " + std::get<1> ( i ) + ";\n";
                break;
            case SCALAR_INT:
                properties += "int " + std::get<1> ( i ) + ";\n";
                break;
            case VECTOR_FLOAT_2:
                properties += "vec2 " + std::get<1> ( i ) + ";\n";
                break;
            case VECTOR_FLOAT_3:
                properties += "vec3 " + std::get<1> ( i ) + ";\n";
                break;
            case VECTOR_FLOAT_4:
                properties += "vec4 " + std::get<1> ( i ) + ";\n";
                break;
            default:
                std::cout << LogLevel::Error << "Unknown Type." << std::endl;
                throw std::runtime_error ( "Unknown Type." );
            }
        }
        return properties;
    }

    static const std::array<const char*, 8> AttributeStrings
    {
        {
            "VertexPosition", // 0
            "VertexNormal",   // 1
            "VertexTangent",  // 2
            "VertexBitangent",// 3
            "VertexUV",      // 4
            "VertexWeightIndices", // 5
            "VertexWeights", // 6
            "VertexColor"    // 7
        }
    };

    static const std::array<const char*, 8> AttributeTypes
    {
        {
            "vec3",
            "vec3",
            "vec3",
            "vec3",
            "vec2",
            "uvec4",
            "vec4",
            "vec3"
        }
    };

    uint32_t Pipeline::GetAttributeBitmap() const
    {
        static const std::regex AttributeRegex
        {
            "\\bVertexPosition\\b|"
            "\\bVertexNormal\\b|"
            "\\bVertexTangent\\b|"
            "\\bVertexBitangent\\b|"
            "\\bVertexUV\\b|"
            "\\bVertexWeightIndices\\b|"
            "\\bVertexWeights\\b|"
            "\\bVertexColor\\b" };

        std::smatch attribute_matches;
        uint32_t attributes{};
        std::string code{mVertexShaderCode};
        while ( std::regex_search ( code, attribute_matches, AttributeRegex ) )
        {
            for ( uint32_t i = 0; i < AttributeStrings.size(); ++i )
            {
                if ( attribute_matches.str().substr ( 6 ) == AttributeStrings[i] + 6 )
                {
                    if ( ! ( attributes & ( 1 << i ) ) )
                    {
                        attributes |= ( 1 << i );
                    }
                    break;
                }
            }
            code = attribute_matches.suffix();
        }
        return attributes;
    }

    std::string Pipeline::GetAttributes() const
    {
        std::string attribute_code{};
        uint32_t attributes{ GetAttributeBitmap() };
        for ( uint32_t i = 0; i < AttributeStrings.size(); ++i )
        {
            if ( attributes & ( 1 << i ) )
            {
                attribute_code.append ( "layout(location = " );
                attribute_code.append ( std::to_string ( i ) );
                attribute_code.append ( ") in " );
                attribute_code.append ( AttributeTypes[i] );
                attribute_code.append ( " " );
                attribute_code.append ( AttributeStrings[i] );
                attribute_code.append ( ";\n" );
            }
        }
        return attribute_code;
    }
#endif

    namespace
    {
        /** True when @p aRenderer is one of the comma-separated names in the
         *  selector key @p aKey (e.g. "Vulkan,Metal"). */
        bool RendererInSet ( const std::string& aKey, std::string_view aRenderer )
        {
            const std::string_view key{ aKey };
            size_t start = 0;
            while ( true )
            {
                const size_t comma = key.find ( ',', start );
                const size_t end = ( comma == std::string_view::npos ) ? key.size() : comma;
                if ( key.substr ( start, end - start ) == aRenderer )
                {
                    return true;
                }
                if ( comma == std::string_view::npos )
                {
                    return false;
                }
                start = comma + 1;
            }
        }

        /** Resolve a stage's per-renderer variant: a non-empty selector whose set
         *  contains @p aRenderer wins (even when its value is empty/disabling, so
         *  it overrides the default rather than inheriting it); otherwise the
         *  default "" selector is used. Returns nullptr when neither exists. The
         *  tool guarantees non-empty selectors are disjoint, so at most one
         *  matches. */
        template<typename Map>
        const typename Map::mapped_type* ResolveVariant ( const Map& aVariants, std::string_view aRenderer )
        {
            for ( const auto& entry : aVariants )
            {
                if ( !entry.first.empty() && RendererInSet ( entry.first, aRenderer ) )
                {
                    return &entry.second;
                }
            }
            const auto it = aVariants.find ( std::string{} );
            return ( it != aVariants.end() ) ? &it->second : nullptr;
        }
    }

    const std::string_view Pipeline::GetShaderCode ( ShaderType aType, std::string_view aRenderer ) const
    {
        const std::string* code = ResolveVariant ( mShaderVariants[aType], aRenderer );
        return code ? std::string_view{ *code } :
               std::string_view{};
    }

    uint32_t Pipeline::GetComputeStageCount ( std::string_view aRenderer ) const
    {
        const std::vector<std::string>* stages = ResolveVariant ( mComputeVariants, aRenderer );
        return stages ? static_cast<uint32_t> ( stages->size() ) : 0u;
    }

    const std::string_view Pipeline::GetComputeShaderCode ( uint32_t aIndex, std::string_view aRenderer ) const
    {
        const std::vector<std::string>* stages = ResolveVariant ( mComputeVariants, aRenderer );
        return ( stages && aIndex < stages->size() ) ? std::string_view{ ( *stages ) [aIndex] } :
               std::string_view{};
    }

    void Pipeline::LoadFromMemory ( const void* aBuffer, size_t aBufferSize )
    {
        LoadFromProtoBufObject<Pipeline, PipelineMsg, "AEONPLN"_mgk> ( *this, aBuffer, aBufferSize );
    }

    void Pipeline::LoadFromPBMsg ( const PipelineMsg& aPipelineMsg )
    {
        for ( auto& variants : mShaderVariants )
        {
            variants.clear();
        }
        mComputeVariants.clear();
        auto copy_stage = [] ( std::unordered_map<std::string, std::string>& aDst, const auto & aSrc )
        {
            for ( const auto& entry : aSrc )
            {
                aDst[entry.first] = entry.second;
            }
        };
        copy_stage ( mShaderVariants[VERT], aPipelineMsg.vert() );
        copy_stage ( mShaderVariants[FRAG], aPipelineMsg.frag() );
        copy_stage ( mShaderVariants[TESC], aPipelineMsg.tesc() );
        copy_stage ( mShaderVariants[TESE], aPipelineMsg.tese() );
        copy_stage ( mShaderVariants[GEOM], aPipelineMsg.geom() );
        for ( const auto& entry : aPipelineMsg.comp() )
        {
            std::vector<std::string>& stages = mComputeVariants[entry.first];
            stages.assign ( entry.second.stage().begin(), entry.second.stage().end() );
        }
        if ( aPipelineMsg.has_topology_class() )
        {
            mTopologyClass = static_cast<uint32_t> ( aPipelineMsg.topology_class() );
            mTopologyClass = mTopologyClass == 0 ? TOPOLOGY_CLASS_TRIANGLE : mTopologyClass;
        }
        else
        {
            mTopologyClass = TOPOLOGY_CLASS_TRIANGLE;
        }
    }

    void Pipeline::Unload()
    {
        for ( auto& variants : mShaderVariants )
        {
            variants.clear();
        }
        mComputeVariants.clear();
    }
}
