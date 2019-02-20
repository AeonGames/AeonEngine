/*
Copyright (C) 2016,2018,2019 Rodrigo Jose Hernandez Cordoba

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
#include <array>
#include <regex>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/CRC.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "reference.pb.h"
#include "pipeline.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    uint32_t GetReferenceBufferId ( const ReferenceBuffer& reference_buffer )
    {
        switch ( reference_buffer.reference_case() )
        {
        case ReferenceBuffer::kPath:
            return crc32i ( reference_buffer.path().c_str(), reference_buffer.path().size() );
        case ReferenceBuffer::kId:
            return reference_buffer.id();
        default:
            return 0;
        }
    }

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

    /**@note static const regex: construct once, use for ever.*/
    static const std::regex AttributeRegex
    {
        "\\bVertexPosition\\b|"
        "\\bVertexNormal\\b|"
        "\\bVertexTangent\\b|"
        "\\bVertexBitangent\\b|"
        "\\bVertexUV\\b|"
        "\\bVertexWeightIndices\\b|"
        "\\bVertexWeights\\b" };

    uint32_t GetAttributes ( const PipelineBuffer& aPipelineBuffer )
    {
        std::smatch attribute_matches;
        uint32_t attributes{};
        std::string code = aPipelineBuffer.vertex_shader().code();
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

    std::string GetAttributesGLSL ( const PipelineBuffer& aPipelineBuffer )
    {
        std::string attribute_code{};
        uint32_t attributes{GetAttributes ( aPipelineBuffer ) };
        for ( uint32_t i = 0; i < 7; ++i )
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

    std::string GetPropertiesGLSL ( const PipelineBuffer& aPipelineBuffer )
    {
        std::string properties{};
        for ( auto& i : aPipelineBuffer.default_material().property() )
        {
            switch ( i.value_case() )
            {
            case PropertyBuffer::ValueCase::kScalarFloat:
                properties += "float " + i.name() + ";\n";
                break;
            case PropertyBuffer::ValueCase::kScalarUint:
                properties += "uint " + i.name() + ";\n";
                break;
            case PropertyBuffer::ValueCase::kScalarInt:
                properties += "int " + i.name() + ";\n";
                break;
            case PropertyBuffer::ValueCase::kVector2:
                properties += "vec2 " + i.name() + ";\n";
                break;
            case PropertyBuffer::ValueCase::kVector3:
                properties += "vec3 " + i.name() + ";\n";
                break;
            case PropertyBuffer::ValueCase::kVector4:
                properties += "vec4 " + i.name() + ";\n";
                break;
            default:
                throw std::runtime_error ( "Unknown Type." );
            }
        }
        return properties;
    }
}
