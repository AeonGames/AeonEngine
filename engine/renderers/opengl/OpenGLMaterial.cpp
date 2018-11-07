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
#include <regex>
#include <utility>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ResourceCache.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
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
        mVariables{},
        mUniformBuffer{}
    {
        if ( aPath )
        {
            Load ( aPath );
        }
    }

    OpenGLMaterial::OpenGLMaterial ( const OpenGLMaterial& aMaterial ) :
        mVariables{aMaterial.mVariables},
        mUniformBuffer{aMaterial.mUniformBuffer}
    {
    }

    OpenGLMaterial& OpenGLMaterial::operator= ( const OpenGLMaterial& aMaterial )
    {
        mVariables = aMaterial.mVariables;
        mUniformBuffer = aMaterial.mUniformBuffer;
        return *this;
    }

    std::unique_ptr<Material> OpenGLMaterial::Clone() const
    {
        return std::make_unique<OpenGLMaterial> ( *this );
    }

    void OpenGLMaterial::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }

    void OpenGLMaterial::Load ( const uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        try
        {
            Load ( buffer.data(), buffer.size() );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    void OpenGLMaterial::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static MaterialBuffer material_buffer;
        LoadProtoBufObject ( material_buffer, aBuffer, aBufferSize, "AEONMTL" );
        Load ( material_buffer );
        material_buffer.Clear();
    }

    void OpenGLMaterial::Load ( const MaterialBuffer& aMaterialBuffer )
    {
        size_t size = 0;
        mVariables.reserve ( aMaterialBuffer.property().size() );
        for ( auto& i : aMaterialBuffer.property() )
        {
            switch ( i.value_case() )
            {
            case PropertyBuffer::ValueCase::kScalarFloat:
                size += ( size % sizeof ( float ) ) ? sizeof ( float ) - ( size % sizeof ( float ) ) : 0; // Align to float
                mVariables.emplace_back ( i.uniform_name(), i.value_case(), size );
                size += sizeof ( float );
                break;
            case PropertyBuffer::ValueCase::kScalarUint:
                size += ( size % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( size % sizeof ( uint32_t ) ) : 0; // Align to uint
                mVariables.emplace_back ( i.uniform_name(), i.value_case(), size );
                size += sizeof ( uint32_t );
                break;
            case PropertyBuffer::ValueCase::kScalarInt:
                size += ( size % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( size % sizeof ( int32_t ) ) : 0; // Align to uint
                mVariables.emplace_back ( i.uniform_name(), i.value_case(), size );
                size += sizeof ( int32_t );
                break;
            case PropertyBuffer::ValueCase::kVector2:
                size += ( size % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( size % ( sizeof ( float ) * 2 ) ) : 0; // Align to 2 floats
                mVariables.emplace_back ( i.uniform_name(), i.value_case(), size );
                size += sizeof ( float ) * 2;
                break;
            case PropertyBuffer::ValueCase::kVector3:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                mVariables.emplace_back ( i.uniform_name(), i.value_case(), size );
                size += sizeof ( float ) * 3;
                break;
            case PropertyBuffer::ValueCase::kVector4:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                mVariables.emplace_back ( i.uniform_name(), i.value_case(), size );
                size += sizeof ( float ) * 4;
                break;
            default:
                break;
            }
        }

        size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // align the final value to 4 floats

        mUniformBuffer.Initialize ( static_cast<GLsizei> ( size ), GL_DYNAMIC_DRAW );
        uint8_t* pointer = reinterpret_cast<uint8_t*> ( mUniformBuffer.Map ( GL_WRITE_ONLY ) );
        for ( auto& i : mVariables )
        {
            auto j = std::find_if ( aMaterialBuffer.property().begin(), aMaterialBuffer.property().end(),
                                    [&i] ( const PropertyBuffer & property )
            {
                return property.uniform_name() == i.GetName();
            } );
            if ( j != aMaterialBuffer.property().end() )
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

    void OpenGLMaterial::Unload()
    {
        mUniformBuffer.Finalize();
    }

    void OpenGLMaterial::SetUint ( const std::string& aName, uint32_t aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kScalarUint ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( uint32_t ), &aValue );
        }
    }

    void OpenGLMaterial::SetSint ( const std::string& aName, int32_t aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kScalarInt ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( int32_t ), &aValue );
        }
    }

    void OpenGLMaterial::SetFloat ( const std::string& aName, float aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kScalarFloat ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ), &aValue );
        }
    }

    void OpenGLMaterial::SetFloatVec2 ( const std::string& aName, const Vector2& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kVector2 ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ) * 2, aValue.GetVector() );
        }
    }

    void OpenGLMaterial::SetFloatVec3 ( const std::string& aName, const Vector3& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kVector3 ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ) * 3, aValue.GetVector3() );
        }
    }

    void OpenGLMaterial::SetFloatVec4 ( const std::string& aName, const Vector4& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kVector4 ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ) * 4, aValue.GetVector4() );
        }
    }

    void OpenGLMaterial::SetSampler ( const std::string& aName, const ResourceId& aValue )
    {
        mSamplers[aName] = aValue;
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
        auto i = mSamplers.find ( aName );
        if ( i != mSamplers.end() )
        {
            return i->second;
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
}
