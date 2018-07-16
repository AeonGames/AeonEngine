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
#include <string.h>
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
#include "aeongames/Vector2.h"
#include "aeongames/Vector3.h"
#include "aeongames/Vector4.h"
#include "aeongames/Image.h"

namespace AeonGames
{
    Material::Material() = default;

    Material::Material ( const std::string& aFilename )
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

    Material::Material ( const Material& aMaterial ) :
        mProperties ( aMaterial.mProperties ),
        mPropertyBlock ( aMaterial.mPropertyBlock ),
        mRenderMaterial{}
    {
        for ( auto&i : mProperties )
        {
            i.mMaterial = this;
        }
    }

    Material& Material::operator = ( const Material& aMaterial )
    {
        if ( this != &aMaterial )
        {
            mProperties = aMaterial.mProperties;
            for ( auto&i : mProperties )
            {
                i.mMaterial = this;
            }
            mPropertyBlock = aMaterial.mPropertyBlock;
            mRenderMaterial.reset();
        }
        return *this;
    }

    Material::~Material()
        = default;

    void Material::Load ( const std::string&  aFilename )
    {
        static MaterialBuffer material_buffer;
        LoadProtoBufObject ( material_buffer, aFilename, "AEONMTL" );
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

    static constexpr enum Material::PropertyType GetType ( PropertyBuffer::DefaultValueCase aPropertyBufferType )
    {
        switch ( aPropertyBufferType )
        {
        case PropertyBuffer::DefaultValueCase::kScalarFloat:
                    return Material::PropertyType::FLOAT;
            case PropertyBuffer::DefaultValueCase::kScalarUint:
                return Material::PropertyType::UINT;
            case PropertyBuffer::DefaultValueCase::kScalarInt:
                return Material::PropertyType::SINT;
            case PropertyBuffer::DefaultValueCase::kVector2:
                return Material::PropertyType::FLOAT_VEC2;
            case PropertyBuffer::DefaultValueCase::kVector3:
                return Material::PropertyType::FLOAT_VEC3;
            case PropertyBuffer::DefaultValueCase::kVector4:
                return Material::PropertyType::FLOAT_VEC4;
            default:
                break;
            }
            return Material::PropertyType::UNKNOWN;
        }

        static size_t GetPropertyBlockSize ( const MaterialBuffer& aMaterialBuffer )
    {
        size_t size = 0;
        for ( auto& i : aMaterialBuffer.property() )
        {
            switch ( i.default_value_case() )
            {
            case PropertyBuffer::DefaultValueCase::kScalarFloat:
                size += ( size % sizeof ( float ) ) ? sizeof ( float ) - ( size % sizeof ( float ) ) : 0; // Align to float
                size += sizeof ( float );
                break;
            case PropertyBuffer::DefaultValueCase::kScalarUint:
                size += ( size % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( size % sizeof ( uint32_t ) ) : 0; // Align to uint
                size += sizeof ( uint32_t );
                break;
            case PropertyBuffer::DefaultValueCase::kScalarInt:
                size += ( size % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( size % sizeof ( int32_t ) ) : 0; // Align to uint
                size += sizeof ( int32_t );
                break;
            case PropertyBuffer::DefaultValueCase::kVector2:
                size += ( size % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( size % ( sizeof ( float ) * 2 ) ) : 0; // Align to 2 floats
                size += sizeof ( float ) * 2;
                break;
            case PropertyBuffer::DefaultValueCase::kVector3:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                size += sizeof ( float ) * 3;
                break;
            case PropertyBuffer::DefaultValueCase::kVector4:
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

    void Material::Load ( const MaterialBuffer& aMaterialBuffer )
    {
        Unload();
        mProperties.reserve ( aMaterialBuffer.property().size() );
        mPropertyBlock.reserve ( AeonGames::GetPropertyBlockSize ( aMaterialBuffer ) );
        for ( auto& i : aMaterialBuffer.property() )
        {
            mProperties.emplace_back ( *this, i );
        }
        mPropertyBlock.resize ( AeonGames::GetPropertyBlockSize ( aMaterialBuffer ) );
    }

    void Material::Unload()
    {
        mProperties.clear();
        mPropertyBlock.clear();
    }

    const std::vector<Material::Property>& Material::GetProperties() const
    {
        return mProperties;
    }

    const std::vector<uint8_t>& Material::GetPropertyBlock() const
    {
        return mPropertyBlock;
    }

    void Material::SetRenderMaterial ( std::unique_ptr<IRenderMaterial> aRenderMaterial ) const
    {
        mRenderMaterial = std::move ( aRenderMaterial );
    }

    const Material::IRenderMaterial* const Material::GetRenderMaterial() const
    {
        return mRenderMaterial.get();
    }

    Material::Property::Property ( Material& aMaterial, const PropertyBuffer& aPropertyBuffer ) :
        mMaterial{&aMaterial},
        mName { aPropertyBuffer.uniform_name() },
        mType{AeonGames::GetType ( aPropertyBuffer.default_value_case() ) },
        mOffset{ mMaterial->mPropertyBlock.size() }
    {
        // Could use mType directly too.
        switch ( aPropertyBuffer.default_value_case() )
        {
        case PropertyBuffer::DefaultValueCase::kScalarFloat:
            mOffset += ( mOffset % sizeof ( float ) ) ? sizeof ( float ) - ( mOffset % sizeof ( float ) ) : 0;
            mMaterial->mPropertyBlock.resize ( mOffset + sizeof ( float ) );
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [0] = aPropertyBuffer.scalar_float();
            break;
        case PropertyBuffer::DefaultValueCase::kScalarUint:
            mOffset += ( mOffset % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( mOffset % sizeof ( uint32_t ) ) : 0;
            mMaterial->mPropertyBlock.resize ( mOffset + sizeof ( uint32_t ) );
            reinterpret_cast<uint32_t*> ( mMaterial->mPropertyBlock.data() + mOffset ) [0] = aPropertyBuffer.scalar_uint();
            break;
        case PropertyBuffer::DefaultValueCase::kScalarInt:
            mOffset += ( mOffset % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( mOffset % sizeof ( int32_t ) ) : 0;
            mMaterial->mPropertyBlock.resize ( mOffset + sizeof ( int32_t ) );
            reinterpret_cast<int32_t*> ( mMaterial->mPropertyBlock.data() + mOffset ) [0] = aPropertyBuffer.scalar_int();
            break;
        case PropertyBuffer::DefaultValueCase::kVector2:
            mOffset += ( mOffset % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( mOffset % ( sizeof ( float ) * 2 ) ) : 0;
            mMaterial->mPropertyBlock.resize ( mOffset + ( sizeof ( float ) * 2 ) );
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [0] = aPropertyBuffer.vector2().x();
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [1] = aPropertyBuffer.vector2().y();
            break;
        case PropertyBuffer::DefaultValueCase::kVector3:
            mOffset += ( mOffset % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( mOffset % ( sizeof ( float ) * 4 ) ) : 0;
            mMaterial->mPropertyBlock.resize ( mOffset + ( sizeof ( float ) * 3 ) );
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [0] = aPropertyBuffer.vector3().x();
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [1] = aPropertyBuffer.vector3().y();
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [2] = aPropertyBuffer.vector3().z();
            break;
        case PropertyBuffer::DefaultValueCase::kVector4:
            mOffset += ( mOffset % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( mOffset % ( sizeof ( float ) * 4 ) ) : 0;
            mMaterial->mPropertyBlock.resize ( mOffset + ( sizeof ( float ) * 4 ) );
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [0] = aPropertyBuffer.vector4().x();
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [1] = aPropertyBuffer.vector4().y();
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [2] = aPropertyBuffer.vector4().z();
            reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) [3] = aPropertyBuffer.vector4().w();
            break;
        case PropertyBuffer::DefaultValueCase::kTexture:
            mImage = std::make_unique<Image>();
            if ( !DecodeImage ( *mImage, aPropertyBuffer.texture() ) )
            {
                std::ostringstream stream;
                stream << "Unable to load image " << aPropertyBuffer.texture();
                throw std::runtime_error ( stream.str().c_str() );
            }
            break;
        default:
            throw std::runtime_error ( "Unknown Type." );
        }
    }

    Material::Property::Property ( const Property& aProperty ) :
        mMaterial{nullptr},
        mName { aProperty.mName },
        mType{ aProperty.mType },
        mOffset{ aProperty.mOffset }
    {
    }

    Material::Property& Material::Property::operator = ( const Property& aProperty )
    {
        if ( this != &aProperty )
        {
            mMaterial = nullptr;
            mName =  aProperty.mName;
            mType = aProperty.mType;
            mOffset = aProperty.mOffset;
        }
        return *this;
    }

    Material::Property::~Property()
    {
    }

    Material::PropertyType Material::Property::GetType() const
    {
        return mType;
    }

    const std::string Material::Property::GetDeclaration() const
    {
        std::string declaration;
        switch ( mType )
        {
        case Material::PropertyType::FLOAT:
            declaration = "float " + mName + ";\n";
            break;
        case Material::PropertyType::UINT:
            declaration = "uint " + mName + ";\n";
            break;
        case Material::PropertyType::SINT:
            declaration = "int " + mName + ";\n";
            break;
        case Material::PropertyType::FLOAT_VEC2:
            declaration = "vec2 " + mName + ";\n";
            break;
        case Material::PropertyType::FLOAT_VEC3:
            declaration = "vec3 " + mName + ";\n";
            break;
        case Material::PropertyType::FLOAT_VEC4:
            declaration = "vec4 " + mName + ";\n";
            break;
        case Material::PropertyType::SAMPLER_2D:
            declaration = "uniform sampler2D " + mName + ";\n";
            break;
        default:
            break;
        }
        return declaration;
    }

    const std::string & Material::Property::GetName() const
    {
        return mName;
    }

    void Material::Set ( const std::string& aName, uint32_t aValue )
    {
        auto i = std::find_if ( mProperties.begin(), mProperties.end(),
                                [&aName] ( const Property & aProperty )
        {
            return aName == aProperty.GetName();
        } );
        if ( i != mProperties.end() )
        {
            ( *i ).Set ( aValue );
        }
    }
    void Material::Property::Set ( uint32_t aValue )
    {
        if ( mType != Material::PropertyType::UINT )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        memcpy ( ( mMaterial->mPropertyBlock.data() + mOffset ), &aValue, sizeof ( uint32_t ) );
        if ( mMaterial->mRenderMaterial )
        {
            mMaterial->mRenderMaterial->Update ( ( mMaterial->mPropertyBlock.data() + mOffset ), mOffset, sizeof ( uint32_t ) );
        }
    }
    void Material::Set ( const std::string& aName, int32_t aValue )
    {
        auto i = std::find_if ( mProperties.begin(), mProperties.end(),
                                [&aName] ( const Property & aProperty )
        {
            return aName == aProperty.GetName();
        } );
        if ( i != mProperties.end() )
        {
            ( *i ).Set ( aValue );
        }
    }
    void Material::Property::Set ( int32_t aValue )
    {
        if ( mType != Material::PropertyType::SINT )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        memcpy ( ( mMaterial->mPropertyBlock.data() + mOffset ), &aValue, sizeof ( int32_t ) );
        if ( mMaterial->mRenderMaterial )
        {
            mMaterial->mRenderMaterial->Update ( ( mMaterial->mPropertyBlock.data() + mOffset ), mOffset, sizeof ( int32_t ) );
        }
    }
    void Material::Set ( const std::string& aName, float aValue )
    {
        auto i = std::find_if ( mProperties.begin(), mProperties.end(),
                                [&aName] ( const Property & aProperty )
        {
            return aName == aProperty.GetName();
        } );
        if ( i != mProperties.end() )
        {
            ( *i ).Set ( aValue );
        }
    }
    void Material::Property::Set ( float aValue )
    {
        if ( mType != Material::PropertyType::FLOAT )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        memcpy ( ( mMaterial->mPropertyBlock.data() + mOffset ), &aValue, sizeof ( float ) );
        if ( mMaterial->mRenderMaterial )
        {
            mMaterial->mRenderMaterial->Update ( ( mMaterial->mPropertyBlock.data() + mOffset ), mOffset, sizeof ( float ) );
        }
    }
    void Material::Set ( const std::string& aName, const Vector2& aValue )
    {
        auto i = std::find_if ( mProperties.begin(), mProperties.end(),
                                [&aName] ( const Property & aProperty )
        {
            return aName == aProperty.GetName();
        } );
        if ( i != mProperties.end() )
        {
            ( *i ).Set ( aValue );
        }
    }
    void Material::Property::Set ( const Vector2& aValue )
    {
        if ( mType != Material::PropertyType::FLOAT_VEC2 )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        memcpy ( ( mMaterial->mPropertyBlock.data() + mOffset ), aValue.GetVector(), sizeof ( float ) * 2 );
        if ( mMaterial->mRenderMaterial )
        {
            mMaterial->mRenderMaterial->Update ( ( mMaterial->mPropertyBlock.data() + mOffset ), mOffset, sizeof ( float ) * 2 );
        }
    }
    void Material::Set ( const std::string& aName, const Vector3& aValue )
    {
        auto i = std::find_if ( mProperties.begin(), mProperties.end(),
                                [&aName] ( const Property & aProperty )
        {
            return aName == aProperty.GetName();
        } );
        if ( i != mProperties.end() )
        {
            ( *i ).Set ( aValue );
        }
    }
    void Material::Property::Set ( const Vector3& aValue )
    {
        if ( mType != Material::PropertyType::FLOAT_VEC3 )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        memcpy ( ( mMaterial->mPropertyBlock.data() + mOffset ), aValue.GetVector3(), sizeof ( float ) * 3 );
        if ( mMaterial->mRenderMaterial )
        {
            mMaterial->mRenderMaterial->Update ( ( mMaterial->mPropertyBlock.data() + mOffset ), mOffset, sizeof ( float ) * 3 );
        }
    }
    void Material::Set ( const std::string& aName, const Vector4& aValue )
    {
        auto i = std::find_if ( mProperties.begin(), mProperties.end(),
                                [&aName] ( const Property & aProperty )
        {
            return aName == aProperty.GetName();
        } );
        if ( i != mProperties.end() )
        {
            ( *i ).Set ( aValue );
        }
    }
    void Material::Property::Set ( const Vector4& aValue )
    {
        if ( mType != Material::PropertyType::FLOAT_VEC4 )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        memcpy ( ( mMaterial->mPropertyBlock.data() + mOffset ), aValue.GetVector4(), sizeof ( float ) * 4 );
        if ( mMaterial->mRenderMaterial )
        {
            mMaterial->mRenderMaterial->Update ( ( mMaterial->mPropertyBlock.data() + mOffset ), mOffset, sizeof ( float ) * 4 );
        }
    }

    const Image* Material::Property::GetImage() const
    {
        if ( mType != Material::PropertyType::SAMPLER_2D )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        return mImage.get();
    }

    void Material::Property::Set ( const std::string& aFileName )
    {
        if ( mType != Material::PropertyType::SAMPLER_2D )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        mImage = std::make_unique<Image>();
        if ( !DecodeImage ( *mImage, aFileName ) )
        {
            std::ostringstream stream;
            stream << "Unable to load image " << aFileName;
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    uint32_t Material::Property::GetUint() const
    {
        if ( mType != Material::PropertyType::UINT )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        return *reinterpret_cast<uint32_t*> ( mMaterial->mPropertyBlock.data() + mOffset );
    }
    int32_t Material::Property::GetSint() const
    {
        if ( mType != Material::PropertyType::SINT )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        return *reinterpret_cast<int32_t*> ( mMaterial->mPropertyBlock.data() + mOffset );
    }
    float Material::Property::GetFloat() const
    {
        if ( mType != Material::PropertyType::FLOAT )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        return *reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset );
    }
    Vector2 Material::Property::GetVector2() const
    {
        if ( mType != Material::PropertyType::FLOAT_VEC2 )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        return Vector2{ reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) };
    }
    Vector3 Material::Property::GetVector3() const
    {
        if ( mType != Material::PropertyType::FLOAT_VEC3 )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        return Vector3{ reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) };
    }
    Vector4 Material::Property::GetVector4() const
    {
        if ( mType != Material::PropertyType::FLOAT_VEC4 )
        {
            throw std::runtime_error ( "Invalid Type." );
        }
        return Vector4{ reinterpret_cast<float*> ( mMaterial->mPropertyBlock.data() + mOffset ) };
    }
    size_t Material::GetSamplerCount() const
    {
        size_t sampler_count = 0;
        for ( auto&i : mProperties )
        {
            if ( i.GetType() == Material::PropertyType::SAMPLER_2D )
            {
                ++sampler_count;
            }
        }
        return sampler_count;
    }
}
