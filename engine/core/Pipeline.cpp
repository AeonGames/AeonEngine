/*
Copyright (C) 2016-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Pipeline.h"

namespace AeonGames
{
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

    Pipeline::~Pipeline()
        = default;

    Topology Pipeline::GetTopology() const
    {
        return mTopology;
    }
    const std::string& Pipeline::GetVertexShaderCode() const
    {
        return mVertexShaderCode;
    }
    const std::string& Pipeline::GetFragmentShaderCode() const
    {
        return mFragmentShaderCode;
    }

    const std::vector<std::tuple<UniformType, std::string>>& Pipeline::GetUniformDescriptors() const
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
            case UniformDescriptorMsg::SCALAR_FLOAT:
                properties += "float " + std::get<1> ( i ) + ";\n";
                break;
            case UniformDescriptorMsg::SCALAR_UINT:
                properties += "uint " + std::get<1> ( i ) + ";\n";
                break;
            case UniformDescriptorMsg::SCALAR_INT:
                properties += "int " + std::get<1> ( i ) + ";\n";
                break;
            case UniformDescriptorMsg::VECTOR_FLOAT_2:
                properties += "vec2 " + std::get<1> ( i ) + ";\n";
                break;
            case UniformDescriptorMsg::VECTOR_FLOAT_3:
                properties += "vec3 " + std::get<1> ( i ) + ";\n";
                break;
            case UniformDescriptorMsg::VECTOR_FLOAT_4:
                properties += "vec4 " + std::get<1> ( i ) + ";\n";
                break;
            default:
                throw std::runtime_error ( "Unknown Type." );
            }
        }
        return properties;
    }

    static const std::array<const char*, 8> AttributeStrings
    {
        {
            "VertexPosition",
            "VertexNormal",
            "VertexTangent",
            "VertexBitangent",
            "VertexUV",
            "VertexWeightIndices",
            "VertexWeights",
            "VertexColor"
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

    void Pipeline::Load ( const PipelineMsg& aPipelineMsg )
    {
        mTopology = TopologyMap.at ( aPipelineMsg.topology() );
        mVertexShaderCode = aPipelineMsg.vertex_shader().code();
        mFragmentShaderCode = aPipelineMsg.fragment_shader().code();
        mUniformDescriptors.reserve ( aPipelineMsg.uniform().size() );
        mSamplerDescriptors.reserve ( aPipelineMsg.sampler().size() );
        for ( auto& i : aPipelineMsg.uniform() )
        {
            mUniformDescriptors.emplace_back ( UniformTypeMap.at ( i.type() ), i.name() );
        }
        for ( auto& i : aPipelineMsg.sampler() )
        {
            mSamplerDescriptors.emplace_back ( i.name() );
        }
    }

    void Pipeline::Unload()
    {
        mTopology = {};
        mVertexShaderCode = {};
        mFragmentShaderCode = {};
        mUniformDescriptors.clear();
        mSamplerDescriptors.clear();
    }
}
