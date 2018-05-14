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
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/Material.h"

namespace AeonGames
{
    Material::Material() = default;

    Material::Material ( const std::string&  aFilename ) :
        mFilename ( aFilename )
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

    Material::Material ( const void* aBuffer, size_t aBufferSize )
    {
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

    Material::Material ( const MaterialBuffer & aMaterialBuffer )
    {
        try
        {
            Load ( aMaterialBuffer );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Material::~Material()
        = default;
    DLL Material::IRenderMaterial::~IRenderMaterial()
        = default;

    void Material::Load ( const std::string&  aFilename )
    {
        static MaterialBuffer material_buffer;
        LoadProtoBufObject ( material_buffer, mFilename, "AEONMTL" );
        Load ( material_buffer );
        material_buffer.Clear();
    }


    void Material::Load ( const void* aBuffer, size_t aBufferSize )
    {
        Unload();
        static MaterialBuffer material_buffer;
        LoadProtoBufObject ( material_buffer, aBuffer, aBufferSize, "AEONMTL" );
        Load ( material_buffer );
        material_buffer.Clear();
    }

    void Material::Load ( const MaterialBuffer& aMaterialBuffer )
    {
        Unload();
        mUniformMetaData.reserve ( aMaterialBuffer.property().size() );
        for ( auto& i : aMaterialBuffer.property() )
        {
            switch ( i.default_value_case() )
            {
            case PropertyBuffer::DefaultValueCase::kScalarFloat:
                mUniformMetaData.emplace_back ( i.uniform_name(), i.scalar_float() );
                break;
            case PropertyBuffer::DefaultValueCase::kScalarUint:
                mUniformMetaData.emplace_back ( i.uniform_name(), i.scalar_uint() );
                break;
            case PropertyBuffer::DefaultValueCase::kScalarInt:
                mUniformMetaData.emplace_back ( i.uniform_name(), i.scalar_int() );
                break;
            case PropertyBuffer::DefaultValueCase::kVector2:
                mUniformMetaData.emplace_back ( i.uniform_name(), i.vector2().x(), i.vector2().y() );
                break;
            case PropertyBuffer::DefaultValueCase::kVector3:
                mUniformMetaData.emplace_back ( i.uniform_name(), i.vector3().x(), i.vector3().y(), i.vector3().z() );
                break;
            case PropertyBuffer::DefaultValueCase::kVector4:
                mUniformMetaData.emplace_back ( i.uniform_name(), i.vector4().x(), i.vector4().y(), i.vector4().z(), i.vector4().w() );
                break;
            case PropertyBuffer::DefaultValueCase::kTexture:
                mUniformMetaData.emplace_back ( i.uniform_name(), i.texture() );
                break;
#if 0
            case PropertyBuffer_Type_SAMPLER_CUBE:
                //type_name = "samplerCube ";
                /* To be continued ... */
                break;
#endif
            default:
                throw std::runtime_error ( "Unknown Type." );
            }
        }
    }

    void Material::Unload()
    {
        mFilename.clear();
        mUniformMetaData.clear();
    }

    const std::vector<Uniform>& Material::GetUniformMetaData() const
    {
        return mUniformMetaData;
    }

    uint32_t Material::GetUniformBlockSize() const
    {
        uint32_t size = 0;
        for ( auto& i : mUniformMetaData )
        {
            switch ( i.GetType() )
            {
            case Uniform::FLOAT:
                size += ( size % sizeof ( float ) ) ? sizeof ( float ) - ( size % sizeof ( float ) ) : 0; // Align to float
                size += sizeof ( float );
                break;
            case Uniform::UINT:
                size += ( size % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( size % sizeof ( uint32_t ) ) : 0; // Align to uint
                size += sizeof ( uint32_t );
                break;
            case Uniform::SINT:
                size += ( size % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( size % sizeof ( int32_t ) ) : 0; // Align to uint
                size += sizeof ( int32_t );
                break;
            case Uniform::FLOAT_VEC2:
                size += ( size % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( size % ( sizeof ( float ) * 2 ) ) : 0; // Align to 2 floats
                size += sizeof ( float ) * 2;
                break;
            case Uniform::FLOAT_VEC3:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                size += sizeof ( float ) * 3;
                break;
            case Uniform::FLOAT_VEC4:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                size += sizeof ( float ) * 4;
                break;
            default:
                break;
            }
        }
        size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // align the final value to 4 floats
        return size;
    }

    void Material::SetRenderMaterial ( std::unique_ptr<IRenderMaterial> aRenderMaterial ) const
    {
        mRenderMaterial = std::move ( aRenderMaterial );
    }

    const Material::IRenderMaterial* const Material::GetRenderMaterial() const
    {
        return mRenderMaterial.get();
    }
}
