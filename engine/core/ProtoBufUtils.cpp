/*
Copyright (C) 2016,2018,2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.hpp"
#include "aeongames/ProtoBufUtils.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Mesh.hpp"
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
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    static_assert ( static_cast<uint32_t> ( Mesh::BYTE ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_BYTE ), "Mesh AttributeType and MeshMSG AttributeType do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::UNSIGNED_BYTE ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_UNSIGNED_BYTE ), "Mesh AttributeType and MeshMSG AttributeType do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::HALF_FLOAT ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_HALF_FLOAT ), "Mesh AttributeType and MeshMSG AttributeType do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::INT ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_INT ), "Mesh AttributeType and MeshMSG AttributeType do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::UNSIGNED_INT ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_UNSIGNED_INT ), "Mesh AttributeType and MeshMSG AttributeType do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::FLOAT ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_FLOAT ), "Mesh AttributeType and MeshMSG AttributeType do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::FIXED ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_FIXED ), "Mesh AttributeType and MeshMSG AttributeType do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::DOUBLE ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_DOUBLE ), "Mesh AttributeType and MeshMSG AttributeType do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::SHORT ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_SHORT ), "Mesh AttributeType and MeshMSG AttributeType do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::UNSIGNED_SHORT ) == static_cast<uint32_t> ( AttributeMsg_AttributeType_UNSIGNED_SHORT ), "Mesh AttributeType and MeshMSG AttributeType do not match" );

    static_assert ( static_cast<uint32_t> ( Mesh::POSITION ) == static_cast<uint32_t> ( AttributeMsg_AttributeSemantic_POSITION ), "Mesh AttributeSemantic and MeshMSG AttributeSemantic do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::NORMAL ) == static_cast<uint32_t> ( AttributeMsg_AttributeSemantic_NORMAL ), "Mesh AttributeSemantic and MeshMSG AttributeSemantic do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::TANGENT ) == static_cast<uint32_t> ( AttributeMsg_AttributeSemantic_TANGENT ), "Mesh AttributeSemantic and MeshMSG AttributeSemantic do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::BITANGENT ) == static_cast<uint32_t> ( AttributeMsg_AttributeSemantic_BITANGENT ), "Mesh AttributeSemantic and MeshMSG AttributeSemantic do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::TEXCOORD ) == static_cast<uint32_t> ( AttributeMsg_AttributeSemantic_TEXCOORD ), "Mesh AttributeSemantic and MeshMSG AttributeSemantic do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::WEIGHT_INDEX ) == static_cast<uint32_t> ( AttributeMsg_AttributeSemantic_WEIGHT_INDEX ), "Mesh AttributeSemantic and MeshMSG AttributeSemantic do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::WEIGHT_VALUE ) == static_cast<uint32_t> ( AttributeMsg_AttributeSemantic_WEIGHT_VALUE ), "Mesh AttributeSemantic and MeshMSG AttributeSemantic do not match" );
    static_assert ( static_cast<uint32_t> ( Mesh::COLOR ) == static_cast<uint32_t> ( AttributeMsg_AttributeSemantic_COLOR ), "Mesh AttributeSemantic and MeshMSG AttributeSemantic do not match" );

    uint32_t GetReferenceMsgId ( const ReferenceMsg& reference_buffer )
    {
        switch ( reference_buffer.reference_case() )
        {
        case ReferenceMsg::kPath:
        {
            // Register the path so that a bare-basename reference can be
            // recovered and resolved to an on-disk file by trying candidate
            // extensions (see ResolveResourceCrc in AeonEngine.cpp).
            const uint32_t crc = crc32i ( reference_buffer.path().c_str(), reference_buffer.path().size() );
            RegisterResourceString ( crc, reference_buffer.path() );
            return crc;
        }
        case ReferenceMsg::kId:
            return reference_buffer.id();
        default:
            return 0;
        }
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

    Property GetProperty ( const PropertyMsg& aPropertyMsg )
    {
        switch ( aPropertyMsg.value_case() )
        {
        case PropertyMsg::kInt:
            return static_cast<int> ( aPropertyMsg.int_() );
        case PropertyMsg::kLong:
            return static_cast<long> ( aPropertyMsg.long_() );
        case PropertyMsg::kLongLong:
            return static_cast<long long> ( aPropertyMsg.long_long() );
        case PropertyMsg::kUnsigned:
            return static_cast<unsigned> ( aPropertyMsg.int_() );
        case PropertyMsg::kUnsignedLong:
            return static_cast<unsigned long> ( aPropertyMsg.unsigned_long() );
        case PropertyMsg::kUnsignedLongLong:
            return static_cast<unsigned long long> ( aPropertyMsg.unsigned_long_long() );
        case PropertyMsg::kFloat:
            return static_cast<float> ( aPropertyMsg.float_() );
        case PropertyMsg::kDouble:
            return static_cast<double> ( aPropertyMsg.double_() );
        case PropertyMsg::kString:
            return aPropertyMsg.string();
        case PropertyMsg::kPath:
            return std::filesystem::path ( aPropertyMsg.path() );
        case PropertyMsg::VALUE_NOT_SET:
            /// @todo Add component and property names to the exception message.
            throw std::runtime_error ( "Component property value not set." );
            break;
        }
        return Property{};
    }
#if 0
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
#endif
    Material::UniformKeyValue UniformVariableToKeyValue ( const UniformVariableMsg& aUniformVariable )
    {
        switch ( aUniformVariable.value_case() )
        {
        case UniformVariableMsg::ValueCase::kScalarFloat:
            return Material::UniformKeyValue{aUniformVariable.name(), aUniformVariable.scalar_float() };
        case UniformVariableMsg::ValueCase::kScalarUint:
            return Material::UniformKeyValue{aUniformVariable.name(), aUniformVariable.scalar_uint() };
        case UniformVariableMsg::ValueCase::kScalarInt:
            return Material::UniformKeyValue{aUniformVariable.name(), aUniformVariable.scalar_int() };
        case UniformVariableMsg::ValueCase::kVector2:
            return Material::UniformKeyValue{aUniformVariable.name(), Vector2{aUniformVariable.vector2().x(), aUniformVariable.vector2().y() }};
        case UniformVariableMsg::ValueCase::kVector3:
            return Material::UniformKeyValue{aUniformVariable.name(), Vector3{aUniformVariable.vector3().x(), aUniformVariable.vector3().y(), aUniformVariable.vector3().z() }};
        case UniformVariableMsg::ValueCase::kVector4:
            return Material::UniformKeyValue{aUniformVariable.name(), Vector4{aUniformVariable.vector4().x(), aUniformVariable.vector4().y(), aUniformVariable.vector4().z(), aUniformVariable.vector4().w() }};
        case UniformVariableMsg::ValueCase::kMatrix4X4:
            return Material::UniformKeyValue{aUniformVariable.name(), Matrix4x4
                {
                    aUniformVariable.matrix4x4().m0(),
                                    aUniformVariable.matrix4x4().m1(),
                                    aUniformVariable.matrix4x4().m2(),
                                    aUniformVariable.matrix4x4().m3(),
                                    aUniformVariable.matrix4x4().m4(),
                                    aUniformVariable.matrix4x4().m5(),
                                    aUniformVariable.matrix4x4().m6(),
                                    aUniformVariable.matrix4x4().m7(),
                                    aUniformVariable.matrix4x4().m8(),
                                    aUniformVariable.matrix4x4().m9(),
                                    aUniformVariable.matrix4x4().m10(),
                                    aUniformVariable.matrix4x4().m11(),
                                    aUniformVariable.matrix4x4().m12(),
                                    aUniformVariable.matrix4x4().m13(),
                                    aUniformVariable.matrix4x4().m14(),
                                    aUniformVariable.matrix4x4().m15()
                }};
        default:
            break;
        }
        throw std::runtime_error ( "Property contained no value." );
        return {};
    }
}
