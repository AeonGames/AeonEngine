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
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufUtils.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Material.h"

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

    size_t Material::LoadVariables ( const MaterialBuffer& aMaterialBuffer )
    {
        size_t size = 0;
        mVariables.reserve ( aMaterialBuffer.property().size() );
        for ( auto& i : aMaterialBuffer.property() )
        {
            switch ( i.value_case() )
            {
            case PropertyBuffer::ValueCase::kScalarFloat:
                size += ( size % sizeof ( float ) ) ? sizeof ( float ) - ( size % sizeof ( float ) ) : 0; // Align to float
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( float );
                break;
            case PropertyBuffer::ValueCase::kScalarUint:
                size += ( size % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( size % sizeof ( uint32_t ) ) : 0; // Align to uint
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( uint32_t );
                break;
            case PropertyBuffer::ValueCase::kScalarInt:
                size += ( size % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( size % sizeof ( int32_t ) ) : 0; // Align to int
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( int32_t );
                break;
            case PropertyBuffer::ValueCase::kVector2:
                size += ( size % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( size % ( sizeof ( float ) * 2 ) ) : 0; // Align to 2 floats
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( float ) * 2;
                break;
            case PropertyBuffer::ValueCase::kVector3:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( float ) * 3;
                break;
            case PropertyBuffer::ValueCase::kVector4:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( float ) * 4;
                break;
            default:
                break;
            }
        }
        size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // align the final value to 4 floats
        return size;
    }

    void Material::LoadSamplers ( const MaterialBuffer& aMaterialBuffer )
    {
        mSamplers.reserve ( aMaterialBuffer.sampler().size() );
        for ( auto& i : aMaterialBuffer.sampler() )
        {
            std::get<1> ( mSamplers.emplace_back ( i.name(), ResourceId{"Image"_crc32, GetReferenceBufferId ( i.image() ) } ) ).Store();
        }
    }

    void Material::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }

    void Material::Load ( const uint32_t aId )
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

    void Material::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static MaterialBuffer material_buffer;
        LoadProtoBufObject ( material_buffer, aBuffer, aBufferSize, "AEONMTL" );
        Load ( material_buffer );
        material_buffer.Clear();
    }
}
