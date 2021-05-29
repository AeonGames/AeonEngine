/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Material.h"
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    Material::Material()
        = default;
    Material::~Material()
        = default;

    Material::Material ( const Material& aMaterial ) : mVariables{aMaterial.mVariables}, mSamplers{aMaterial.mSamplers} {}
    Material& Material::operator= ( const Material& aMaterial )
    {
        mVariables = aMaterial.mVariables;
        mSamplers = aMaterial.mSamplers;
        return *this;
    }

    const std::vector<uint8_t>& Material::GetUniformBuffer() const
    {
        return mUniformBuffer;
    }

    size_t Material::LoadVariables ( const MaterialMsg& aMaterialMsg )
    {
        size_t size = 0;
        mVariables.reserve ( aMaterialMsg.property().size() );
        for ( auto& i : aMaterialMsg.property() )
        {
            switch ( i.value_case() )
            {
            case PropertyMsg::ValueCase::kScalarFloat:
                size += ( size % sizeof ( float ) ) ? sizeof ( float ) - ( size % sizeof ( float ) ) : 0; // Align to float
                mVariables.emplace_back ( i.name(),  size );
                size += sizeof ( float );
                break;
            case PropertyMsg::ValueCase::kScalarUint:
                size += ( size % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( size % sizeof ( uint32_t ) ) : 0; // Align to uint
                mVariables.emplace_back ( i.name(),  size );
                size += sizeof ( uint32_t );
                break;
            case PropertyMsg::ValueCase::kScalarInt:
                size += ( size % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( size % sizeof ( int32_t ) ) : 0; // Align to int
                mVariables.emplace_back ( i.name(),  size );
                size += sizeof ( int32_t );
                break;
            case PropertyMsg::ValueCase::kVector2:
                size += ( size % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( size % ( sizeof ( float ) * 2 ) ) : 0; // Align to 2 floats
                mVariables.emplace_back ( i.name(),  size );
                size += sizeof ( float ) * 2;
                break;
            case PropertyMsg::ValueCase::kVector3:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                mVariables.emplace_back ( i.name(),  size );
                size += sizeof ( float ) * 3;
                break;
            case PropertyMsg::ValueCase::kVector4:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                mVariables.emplace_back ( i.name(),  size );
                size += sizeof ( float ) * 4;
                break;
            case PropertyMsg::ValueCase::kMatrix4X4:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                mVariables.emplace_back ( i.name(),  size );
                size += sizeof ( float ) * 16;
                break;
            default:
                break;
            }
        }
        size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // align the final value to 4 floats
        return size;
    }

    void Material::LoadSamplers ( const MaterialMsg& aMaterialMsg )
    {
        mSamplers.reserve ( aMaterialMsg.sampler().size() );
        for ( auto& i : aMaterialMsg.sampler() )
        {
            std::get<1> ( mSamplers.emplace_back ( i.name(), ResourceId{"Texture"_crc32, GetReferenceMsgId ( i.image() ) } ) ).Store();
        }
    }

    static size_t GetUniformValueOffset ( const Material::UniformValue& aValue, size_t& aLastOffset )
    {
        return std::visit ( [&aLastOffset] ( auto&& arg )
        {
            size_t value_size = sizeof ( std::decay_t<decltype ( arg ) > );
            size_t size = aLastOffset;
            aLastOffset += value_size;
            if ( value_size <= 4 )
            {
                size += ( size % 4 ) ? 4 - ( size % 4 ) : 0;
            }
            else if ( value_size <= 8 )
            {
                size += ( size % 8 ) ? 8 - ( size % 8 ) : 0;
            }
            else
            {
                size += ( size % 16 ) ? 16 - ( size % 16 ) : 0;
            }
            return size;
        }, aValue );
    }

    size_t Material::LoadVariables ( std::initializer_list<UniformKeyValue> aUniforms )
    {
        size_t size = 0;
        mVariables.reserve ( aUniforms.size() );
        for ( auto& i : aUniforms )
        {
            mVariables.emplace_back ( std::get<0> ( i ),  GetUniformValueOffset ( std::get<1> ( i ), size ) );
        }
        size += ( size % ( 16 ) ) ? ( 16 ) - ( size % ( 16 ) ) : 0; // align the final value to 4 floats
        return size;
    }

    void Material::LoadSamplers ( std::initializer_list<SamplerKeyValue> aSamplers )
    {
        mSamplers.reserve ( aSamplers.size() );
        for ( auto& i : aSamplers )
        {
            std::get<1> ( mSamplers.emplace_back ( std::get<0> ( i ), std::get<1> ( i ) ) ).Store();
        }
    }

    size_t GetUniformValueSize ( const Material::UniformValue& aValue )
    {
        return std::visit ( [] ( auto&& arg )
        {
            return sizeof ( std::decay_t<decltype ( arg ) > );
        }, aValue );
    }

    const void* GetUniformValuePointer ( const Material::UniformValue& aValue )
    {
        if ( std::holds_alternative<Vector2> ( aValue ) )
        {
            return std::get<Vector2> ( aValue ).GetVector();
        }
        else if ( std::holds_alternative<Vector3> ( aValue ) )
        {
            return std::get<Vector3> ( aValue ).GetVector3();
        }
        if ( std::holds_alternative<Vector4> ( aValue ) )
        {
            return std::get<Vector4> ( aValue ).GetVector4();
        }
        if ( std::holds_alternative<Matrix4x4> ( aValue ) )
        {
            return std::get<Matrix4x4> ( aValue ).GetMatrix4x4();
        }
        return std::visit ( [] ( const auto & value )
        {
            return reinterpret_cast<const void*> ( &value );
        }, aValue );
    }

    void Material::LoadFromMemory ( const void* aBuffer, size_t aBufferSize )
    {
        LoadFromProtoBufObject<Material, MaterialMsg, "AEONMTL"_mgk> ( *this, aBuffer, aBufferSize );
    }

    void Material::LoadFromPBMsg ( const MaterialMsg& aMaterialMsg )
    {
        size_t size = LoadVariables ( aMaterialMsg );
        if ( size )
        {
            mUniformBuffer.resize ( size );
            for ( auto& i : aMaterialMsg.property() )
            {
                Set ( PropertyToKeyValue ( i ) );
            }
        }
        LoadSamplers ( aMaterialMsg );
    }

    void Material::Set ( size_t aIndex, const UniformValue& aValue )
    {
        memcpy ( mUniformBuffer.data() + mVariables.at ( aIndex ).GetOffset(), GetUniformValuePointer ( aValue ), GetUniformValueSize ( aValue ) );
    }

    void Material::Set ( const UniformKeyValue& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aValue] ( const UniformVariable & variable )
        {
            return variable.GetName() == std::get<std::string> ( aValue );
        } );
        if ( i != mVariables.end() )
        {
            size_t value_size = GetUniformValueSize ( std::get<UniformValue> ( aValue ) );
            // Do some bounds checking
            auto j = i + 1;
            if ( ( ( j != mVariables.end() ) ? ( j->GetOffset() - i->GetOffset() ) : ( mUniformBuffer.size() - i->GetOffset() ) ) < value_size )
            {
                throw std::runtime_error ( "Value type size exceeds original type size." );
            }
            memcpy ( mUniformBuffer.data() + i->GetOffset(), GetUniformValuePointer ( std::get<UniformValue> ( aValue ) ), value_size );
        }
    }

    void Material::SetSampler ( const std::string& aName, const ResourceId& aValue )
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

    const std::vector<std::tuple<std::string, ResourceId>>& Material::GetSamplers() const
    {
        return mSamplers;
    }

    ResourceId Material::GetSampler ( const std::string& aName )
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
        return ResourceId{"Texture"_crc32, 0};
    }

    void Material::Unload()
    {
        mVariables.clear();
        mSamplers.clear();
        mUniformBuffer.clear();
    }
}
