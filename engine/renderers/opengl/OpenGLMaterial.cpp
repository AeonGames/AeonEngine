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
#include <fstream>
#include <sstream>
#include <ostream>
#include <regex>
#include <utility>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/ResourceCache.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/AeonEngine.h"
#include "aeongames/Material.h"
#include "aeongames/CRC.h"
#include "OpenGLFunctions.h"
#include "OpenGLMaterial.h"
#include "OpenGLImage.h"

namespace AeonGames
{
    OpenGLMaterial::OpenGLMaterial ( uint32_t aPath ) :
        Material{},
        mUniformBuffer{}
    {
        if ( aPath )
        {
            Material::Load ( aPath );
        }
    }

    OpenGLMaterial::OpenGLMaterial ( std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers ) :
        Material{},
        mUniformBuffer{}
    {
        OpenGLMaterial::Load ( aUniforms, aSamplers );
    }

    void OpenGLMaterial::Load ( std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers )
    {

    }

    OpenGLMaterial::OpenGLMaterial ( const OpenGLMaterial& aMaterial ) :
        Material{aMaterial},
        mUniformBuffer{aMaterial.mUniformBuffer}
    {
    }

    OpenGLMaterial& OpenGLMaterial::operator= ( const OpenGLMaterial& aMaterial )
    {
        this->Material::operator= ( aMaterial );
        mUniformBuffer = aMaterial.mUniformBuffer;
        return *this;
    }

    std::unique_ptr<Material> OpenGLMaterial::Clone() const
    {
        return std::make_unique<OpenGLMaterial> ( *this );
    }

    void OpenGLMaterial::Load ( const MaterialBuffer& aMaterialBuffer )
    {
        size_t size = LoadVariables ( aMaterialBuffer );
        if ( size )
        {
            mUniformBuffer.Initialize ( static_cast<GLsizei> ( size ), GL_DYNAMIC_DRAW );
            auto* pointer = reinterpret_cast<uint8_t*> ( mUniformBuffer.Map ( GL_WRITE_ONLY ) );
            for ( auto& i : mVariables )
            {
                auto& material_properties = aMaterialBuffer.property();
                auto j = std::find_if ( material_properties.begin(), material_properties.end(),
                                        [&i] ( const PropertyBuffer & property )
                {
                    return property.name() == i.GetName();
                } );
                if ( j != material_properties.end() )
                {
                    switch ( i.GetType() )
                    {
                    case PropertyBuffer::ValueCase::kScalarFloat:
                        ( *reinterpret_cast<float*> ( pointer + i.GetOffset() ) ) = j->scalar_float();
                        break;
                    case PropertyBuffer::ValueCase::kScalarUint:
                        ( *reinterpret_cast<uint32_t*> ( pointer + i.GetOffset() ) ) = j->scalar_uint();
                        break;
                    case PropertyBuffer::ValueCase::kScalarInt:
                        ( *reinterpret_cast<int32_t*> ( pointer + i.GetOffset() ) ) = j->scalar_int();
                        break;
                    case PropertyBuffer::ValueCase::kVector2:
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [0] = j->vector2().x();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [1] = j->vector2().y();
                        break;
                    case PropertyBuffer::ValueCase::kVector3:
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [0] = j->vector3().x();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [1] = j->vector3().y();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [2] = j->vector3().z();
                        break;
                    case PropertyBuffer::ValueCase::kVector4:
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [0] = j->vector4().x();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [1] = j->vector4().y();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [2] = j->vector4().z();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [3] = j->vector4().w();
                        break;
                    default:
                        break;
                    }
                }
            }
            mUniformBuffer.Unmap();
        }
        LoadSamplers ( aMaterialBuffer );
    }

    void OpenGLMaterial::Unload()
    {
        mUniformBuffer.Finalize();
    }

    void OpenGLMaterial::Set ( const std::string& aName, const UniformValue& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( i != mVariables.end() )
        {
            switch ( i->GetType() )
            {
            // Let the exception from std::get be thrown if called with wrong values.
            case PropertyBuffer::ValueCase::kScalarUint:
                mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( uint32_t ), &std::get<uint32_t> ( aValue ) );
                break;
            case PropertyBuffer::ValueCase::kScalarInt:
                mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( int32_t ), &std::get<int32_t> ( aValue ) );
                break;
            case PropertyBuffer::ValueCase::kScalarFloat:
                mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ), &std::get<float> ( aValue ) );
                break;
            case PropertyBuffer::ValueCase::kVector2:
                mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ) * 2, std::get<Vector2> ( aValue ).GetVector() );
                break;
            case PropertyBuffer::ValueCase::kVector3:
                mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ) * 3, std::get<Vector3> ( aValue ).GetVector3() );
                break;
            case PropertyBuffer::ValueCase::kVector4:
                mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ) * 4, std::get<Vector4> ( aValue ).GetVector4() );
                break;
            }
        }
    }

    void OpenGLMaterial::SetSampler ( const std::string& aName, const ResourceId& aValue )
    {
        auto i = std::find_if ( mSamplers.begin(), mSamplers.end(),
                                [&aName] ( const std::tuple<std::string, ResourceId>& aTuple )
        {
            return std::get<0> ( aTuple ) == aName;
        } );
        if ( i != mSamplers.end() )
        {
            std::get<1> ( *i ) = aValue;
        }
    }

    uint32_t OpenGLMaterial::GetUint ( const std::string& aName )
    {
        return 0;
    }
    int32_t OpenGLMaterial::GetSint ( const std::string& aName )
    {
        return 0;
    }
    float OpenGLMaterial::GetFloat ( const std::string& aName )
    {
        return 0.0f;
    }
    Vector2 OpenGLMaterial::GetFloatVec2 ( const std::string& aName )
    {
        return Vector2{};
    }
    Vector3 OpenGLMaterial::GetFloatVec3 ( const std::string& aName )
    {
        return Vector3{};
    }
    Vector4 OpenGLMaterial::GetFloatVec4 ( const std::string& aName )
    {
        return Vector4{};
    }
    ResourceId OpenGLMaterial::GetSampler ( const std::string& aName )
    {
        auto i = std::find_if ( mSamplers.begin(), mSamplers.end(),
                                [&aName] ( const std::tuple<std::string, ResourceId>& aTuple )
        {
            return std::get<0> ( aTuple ) == aName;
        } );
        if ( i != mSamplers.end() )
        {
            return std::get<1> ( *i );
        }
        return ResourceId{"Image"_crc32, 0};
    }

    OpenGLMaterial::~OpenGLMaterial()
    {
        Unload();
    }

    GLuint OpenGLMaterial::GetPropertiesBufferId() const
    {
        return mUniformBuffer.GetBufferId();
    }

    const std::vector<std::tuple<std::string, ResourceId>>& OpenGLMaterial::GetSamplers() const
    {
        return mSamplers;
    }
}
