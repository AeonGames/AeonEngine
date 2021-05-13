/*
Copyright (C) 2016,2018,2019,2021 Rodrigo Jose Hernandez Cordoba

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
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "reference.pb.h"
#include "pipeline.pb.h"
#include "property.pb.h"
#include "vector3.pb.h"
#include "quaternion.pb.h"
#include "transform.pb.h"
#include "scene.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    uint32_t GetReferenceMsgId ( const ReferenceMsg& reference_buffer )
    {
        switch ( reference_buffer.reference_case() )
        {
        case ReferenceMsg::kPath:
            return crc32i ( reference_buffer.path().c_str(), reference_buffer.path().size() );
        case ReferenceMsg::kId:
            return reference_buffer.id();
        default:
            return 0;
        }
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

    /**@note static const regex: construct once, use for ever.*/
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

    uint32_t GetAttributes ( const PipelineMsg& aPipelineMsg )
    {
        std::smatch attribute_matches;
        uint32_t attributes{};
        std::string code = aPipelineMsg.vertex_shader().code();
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

    std::string GetAttributesGLSL ( const PipelineMsg& aPipelineMsg )
    {
        std::string attribute_code{};
        uint32_t attributes{GetAttributes ( aPipelineMsg ) };
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

    std::string GetPropertiesGLSL ( const PipelineMsg& aPipelineMsg )
    {
        std::string properties{};
        for ( auto& i : aPipelineMsg.uniform() )
        {
            switch ( i.type() )
            {
            case UniformDescriptorMsg::SCALAR_FLOAT:
                properties += "float " + i.name() + ";\n";
                break;
            case UniformDescriptorMsg::SCALAR_UINT:
                properties += "uint " + i.name() + ";\n";
                break;
            case UniformDescriptorMsg::SCALAR_INT:
                properties += "int " + i.name() + ";\n";
                break;
            case UniformDescriptorMsg::VECTOR_FLOAT_2:
                properties += "vec2 " + i.name() + ";\n";
                break;
            case UniformDescriptorMsg::VECTOR_FLOAT_3:
                properties += "vec3 " + i.name() + ";\n";
                break;
            case UniformDescriptorMsg::VECTOR_FLOAT_4:
                properties += "vec4 " + i.name() + ";\n";
                break;
            default:
                throw std::runtime_error ( "Unknown Type." );
            }
        }
        return properties;
    }

    Vector3 GetVector3 ( const Vector3Msg& aVector3 )
    {
        return {aVector3.x(), aVector3.y(), aVector3.z() };
    }
    Quaternion GetQuaternion ( const QuaternionMsg& aQuaternion )
    {
        return {aQuaternion.w(), aQuaternion.x(), aQuaternion.y(), aQuaternion.z() };
    }

    Transform GetTransform ( const TransformMsg& aTransform )
    {
        return
        {
            GetVector3 ( aTransform.scale() ),
            GetQuaternion ( aTransform.rotation() ),
            GetVector3 ( aTransform.translation() )
        };
    }

    Property GetProperty ( const ComponentPropertyMsg& aComponentPropertyMsg )
    {
        switch ( aComponentPropertyMsg.value_case() )
        {
        case ComponentPropertyMsg::kInt:
            return static_cast<int> ( aComponentPropertyMsg.int_() );
        case ComponentPropertyMsg::kLong:
            return static_cast<long> ( aComponentPropertyMsg.long_() );
        case ComponentPropertyMsg::kLongLong:
            return static_cast<long long> ( aComponentPropertyMsg.long_long() );
        case ComponentPropertyMsg::kUnsigned:
            return static_cast<unsigned> ( aComponentPropertyMsg.int_() );
        case ComponentPropertyMsg::kUnsignedLong:
            return static_cast<long> ( aComponentPropertyMsg.unsigned_long() );
        case ComponentPropertyMsg::kUnsignedLongLong:
            return static_cast<long long> ( aComponentPropertyMsg.unsigned_long_long() );
        case ComponentPropertyMsg::kFloat:
            return static_cast<float> ( aComponentPropertyMsg.float_() );
        case ComponentPropertyMsg::kDouble:
            return static_cast<double> ( aComponentPropertyMsg.double_() );
        case ComponentPropertyMsg::kString:
            return aComponentPropertyMsg.string();
        case ComponentPropertyMsg::kPath:
            return std::filesystem::path ( aComponentPropertyMsg.string() );
        case ComponentPropertyMsg::VALUE_NOT_SET:
            /// @todo Add component and property names to the exception message.
            throw std::runtime_error ( "Component property value not set." );
            break;
        }
        return Property{};
    }

    size_t GetUniformBufferSize ( const PipelineMsg& aPipelineMsg )
    {
        size_t size = 0;
        for ( auto& i : aPipelineMsg.uniform() )
        {
            switch ( i.type() )
            {
            case UniformDescriptorMsg::SCALAR_FLOAT:
                size += ( size % sizeof ( float ) ) ? sizeof ( float ) - ( size % sizeof ( float ) ) : 0; // Align to float
                size += sizeof ( float );
                break;
            case UniformDescriptorMsg::SCALAR_UINT:
                size += ( size % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( size % sizeof ( uint32_t ) ) : 0; // Align to uint
                size += sizeof ( uint32_t );
                break;
            case UniformDescriptorMsg::SCALAR_INT:
                size += ( size % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( size % sizeof ( int32_t ) ) : 0; // Align to int
                size += sizeof ( int32_t );
                break;
            case UniformDescriptorMsg::VECTOR_FLOAT_2:
                size += ( size % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( size % ( sizeof ( float ) * 2 ) ) : 0; // Align to 2 floats
                size += sizeof ( float ) * 2;
                break;
            case UniformDescriptorMsg::VECTOR_FLOAT_3:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                size += sizeof ( float ) * 3;
                break;
            case UniformDescriptorMsg::VECTOR_FLOAT_4:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                size += sizeof ( float ) * 4;
                break;
            default:
                break;
            }
        }
        return size + ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // align the final value to 4 float
    }

    Material::UniformKeyValue PropertyToKeyValue ( const PropertyMsg& aProperty )
    {
        switch ( aProperty.value_case() )
        {
        case PropertyMsg::ValueCase::kScalarFloat:
            return Material::UniformKeyValue{aProperty.name(), aProperty.scalar_float() };
        case PropertyMsg::ValueCase::kScalarUint:
            return Material::UniformKeyValue{aProperty.name(), aProperty.scalar_uint() };
        case PropertyMsg::ValueCase::kScalarInt:
            return Material::UniformKeyValue{aProperty.name(), aProperty.scalar_int() };
        case PropertyMsg::ValueCase::kVector2:
            return Material::UniformKeyValue{aProperty.name(), Vector2{aProperty.vector2().x(), aProperty.vector2().y() }};
        case PropertyMsg::ValueCase::kVector3:
            return Material::UniformKeyValue{aProperty.name(), Vector3{aProperty.vector3().x(), aProperty.vector3().y(), aProperty.vector3().z() }};
        case PropertyMsg::ValueCase::kVector4:
            return Material::UniformKeyValue{aProperty.name(), Vector4{aProperty.vector4().x(), aProperty.vector4().y(), aProperty.vector4().z(), aProperty.vector4().w() }};
        case PropertyMsg::ValueCase::kMatrix4X4:
            return Material::UniformKeyValue{aProperty.name(), Matrix4x4
                {
                    aProperty.matrix4x4().m0(),
                    aProperty.matrix4x4().m1(),
                    aProperty.matrix4x4().m2(),
                    aProperty.matrix4x4().m3(),
                    aProperty.matrix4x4().m4(),
                    aProperty.matrix4x4().m5(),
                    aProperty.matrix4x4().m6(),
                    aProperty.matrix4x4().m7(),
                    aProperty.matrix4x4().m8(),
                    aProperty.matrix4x4().m9(),
                    aProperty.matrix4x4().m10(),
                    aProperty.matrix4x4().m11(),
                    aProperty.matrix4x4().m12(),
                    aProperty.matrix4x4().m13(),
                    aProperty.matrix4x4().m14(),
                    aProperty.matrix4x4().m15()
                }};
        default:
            break;
        }
        throw std::runtime_error ( "Property contained no value." );
        return {};
    }
}
