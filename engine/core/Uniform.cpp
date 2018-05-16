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
#include <cstring>
#include <cassert>
#include "aeongames/Image.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Uniform.h"

namespace AeonGames
{
    Uniform::Uniform ( const std::string& aName, float aX, uint8_t* aData ) :
        mName ( aName ),
        mType ( Uniform::Type::FLOAT ),
        mData{ aData }
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
    }
    Uniform::Uniform ( const std::string & aName, uint32_t aX, uint8_t* aData ) :
        mName{aName},
        mType ( Uniform::Type::UINT ),
        mData{ aData }
    {
        ( reinterpret_cast<uint32_t*> ( mData ) ) [0] = aX;
    }
    Uniform::Uniform ( const std::string & aName, int32_t aX, uint8_t* aData ) :
        mName{ aName },
        mType ( Uniform::Type::SINT ),
        mData{ aData }
    {
        ( reinterpret_cast<int32_t*> ( mData ) ) [0] = aX;
    }
    Uniform::Uniform ( const std::string&  aName, float aX, float aY, uint8_t* aData ) :
        mName ( aName ),
        mType ( Uniform::Type::FLOAT_VEC2 ),
        mData{ aData }
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
    }
    Uniform::Uniform ( const std::string&  aName, float aX, float aY, float aZ, uint8_t* aData ) :
        mName ( aName ),
        mType ( Uniform::Type::FLOAT_VEC3 ),
        mData{ aData }
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
    }
    Uniform::Uniform ( const std::string&  aName, float aX, float aY, float aZ, float aW, uint8_t* aData ) :
        mName ( aName ),
        mType ( Uniform::Type::FLOAT_VEC4 ),
        mData{ aData }
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
        ( reinterpret_cast<float*> ( mData ) ) [3] = aW;
    }
    Uniform::Uniform ( const std::string&  aName, const std::string & aFilename ) :
        mName ( aName ),
        mType ( Uniform::Type::SAMPLER_2D ),
        mData{ nullptr }
    {
#if 0
        /// To be refactored
        /**@todo Temporarily hardcoding ".png" identifier,
        the AeonGames::GetImage has to change to deduce image type based on extension.*/
        new ( mData ) std::shared_ptr<Image> ( AeonGames::GetImage ( ".png", aFilename ) );
#endif
    }
    Uniform::~Uniform()
    {
        switch ( mType )
        {
        case Uniform::Type::SAMPLER_2D:
            reinterpret_cast<std::shared_ptr<Image>*> ( mData )->~shared_ptr();
            break;
        default:
            break;
        }
    }

    Uniform::Type Uniform::GetType() const
    {
        return mType;
    }

    const std::string Uniform::GetDeclaration() const
    {
        std::string declaration;
        switch ( mType )
        {
        case Uniform::Type::FLOAT:
            declaration = "float " + mName + ";\n";
            break;
        case Uniform::Type::UINT:
            declaration = "uint " + mName + ";\n";
            break;
        case Uniform::Type::SINT:
            declaration = "int " + mName + ";\n";
            break;
        case Uniform::Type::FLOAT_VEC2:
            declaration = "vec2 " + mName + ";\n";
            break;
        case Uniform::Type::FLOAT_VEC3:
            declaration = "vec3 " + mName + ";\n";
            break;
        case Uniform::Type::FLOAT_VEC4:
            declaration = "vec4 " + mName + ";\n";
            break;
        case Uniform::Type::SAMPLER_2D:
            declaration = "uniform sampler2D " + mName + ";\n";
            break;
        default:
            break;
        }
        return declaration;
    }

    const std::string & Uniform::GetName() const
    {
        return mName;
    }

    uint32_t Uniform::GetUInt() const
    {
        assert ( mType == Uniform::Type::UINT );
        return ( reinterpret_cast<const uint32_t*> ( mData ) ) [0];
    }

    DLL int32_t Uniform::GetSInt() const
    {
        assert ( mType == Uniform::Type::SINT );
        return ( reinterpret_cast<const int32_t*> ( mData ) ) [0];
    }

    float Uniform::GetX() const
    {
        assert (
            mType == Uniform::Type::FLOAT ||
            mType == Uniform::Type::FLOAT_VEC2 ||
            mType == Uniform::Type::FLOAT_VEC3 ||
            mType == Uniform::Type::FLOAT_VEC4 );
        return ( reinterpret_cast<const float*> ( mData ) ) [0];
    }
    float Uniform::GetY() const
    {
        assert ( mType == Uniform::Type::FLOAT_VEC2 ||
                 mType == Uniform::Type::FLOAT_VEC3 ||
                 mType == Uniform::Type::FLOAT_VEC4 );
        return ( reinterpret_cast<const float*> ( mData ) ) [1];
    }
    float Uniform::GetZ() const
    {
        assert ( mType == Uniform::Type::FLOAT_VEC3 ||
                 mType == Uniform::Type::FLOAT_VEC4 );
        return ( reinterpret_cast<const float*> ( mData ) ) [2];
    }
    float Uniform::GetW() const
    {
        assert ( mType == Uniform::Type::FLOAT_VEC4 );
        return ( reinterpret_cast<const float*> ( mData ) ) [3];
    }
    const std::shared_ptr<Image> Uniform::GetImage() const
    {
        assert ( mType == Uniform::Type::SAMPLER_2D );
        return * ( reinterpret_cast<const std::shared_ptr<Image>*> ( mData ) );
    }

    void Uniform::SetUInt ( uint32_t aValue )
    {
        assert ( mType == Uniform::Type::UINT );
        ( reinterpret_cast<uint32_t*> ( mData ) ) [0] = aValue;
    }
    void Uniform::SetSInt ( int32_t aValue )
    {
        assert ( mType == Uniform::Type::SINT );
        ( reinterpret_cast<int32_t*> ( mData ) ) [0] = aValue;
    }
    void Uniform::SetX ( float aValue )
    {
        assert (
            mType == Uniform::Type::FLOAT ||
            mType == Uniform::Type::FLOAT_VEC2 ||
            mType == Uniform::Type::FLOAT_VEC3 ||
            mType == Uniform::Type::FLOAT_VEC4 );
        ( reinterpret_cast<float*> ( mData ) ) [0] = aValue;
    }
    void Uniform::SetY ( float aValue )
    {
        assert (
            mType == Uniform::Type::FLOAT_VEC2 ||
            mType == Uniform::Type::FLOAT_VEC3 ||
            mType == Uniform::Type::FLOAT_VEC4 );
        ( reinterpret_cast<float*> ( mData ) ) [1] = aValue;
    }
    void Uniform::SetZ ( float aValue )
    {
        assert (
            mType == Uniform::Type::FLOAT_VEC3 ||
            mType == Uniform::Type::FLOAT_VEC4 );
        ( reinterpret_cast<float*> ( mData ) ) [2] = aValue;
    }
    void Uniform::SetW ( float aValue )
    {
        assert ( mType == Uniform::Type::FLOAT_VEC4 );
        ( reinterpret_cast<float*> ( mData ) ) [3] = aValue;
    }
    void Uniform::Set ( void* aValue )
    {
        switch ( mType )
        {
        case UINT:
            memcpy ( mData, aValue, sizeof ( uint32_t ) );
            break;
        case FLOAT:
            memcpy ( mData, aValue, sizeof ( float ) );
            break;
        case SINT:
            memcpy ( mData, aValue, sizeof ( int32_t ) );
            break;
        case FLOAT_VEC2:
            memcpy ( mData, aValue, sizeof ( float ) * 2 );
            break;
        case FLOAT_VEC3:
            memcpy ( mData, aValue, sizeof ( float ) * 3 );
            break;
        case FLOAT_VEC4:
            memcpy ( mData, aValue, sizeof ( float ) * 4 );
            break;
        default:
            // Do nothing for now.
            break;
        }
    }
}
