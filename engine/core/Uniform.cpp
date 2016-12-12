/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Texture.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Uniform.h"

#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "property.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    static_assert ( sizeof ( std::shared_ptr<Texture> ) <= ( sizeof ( float ) * 4 ), "Size of shared pointer is bigger than a vec4" );
    Uniform::Uniform ( const std::string& aName, float aX ) :
        mName ( aName ),
        mType ( PropertyBuffer_Type_FLOAT )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = 0;
        ( reinterpret_cast<float*> ( mData ) ) [2] = 0;
        ( reinterpret_cast<float*> ( mData ) ) [3] = 0;
    }
    Uniform::Uniform ( const std::string& aName, float aX, float aY ) :
        mName ( aName ),
        mType ( PropertyBuffer_Type_FLOAT_VEC2 )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = 0;
        ( reinterpret_cast<float*> ( mData ) ) [3] = 0;
    }
    Uniform::Uniform ( const std::string& aName, float aX, float aY, float aZ ) :
        mName ( aName ),
        mType ( PropertyBuffer_Type_FLOAT_VEC3 )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
        ( reinterpret_cast<float*> ( mData ) ) [3] = 0;
    }
    Uniform::Uniform ( const std::string& aName, float aX, float aY, float aZ, float aW ) :
        mName ( aName ),
        mType ( PropertyBuffer_Type_FLOAT_VEC4 )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
        ( reinterpret_cast<float*> ( mData ) ) [3] = aW;
    }
    Uniform::Uniform ( const std::string & aName, const std::string & aFilename ) :
        mName ( aName ),
        mType ( PropertyBuffer_Type_SAMPLER_2D )
    {
        new ( mData ) std::shared_ptr<Texture> ( Get<Texture> ( aFilename ) );
    }
    Uniform::~Uniform()
    {
        switch ( mType )
        {
        case PropertyBuffer_Type_SAMPLER_2D:
            reinterpret_cast<std::shared_ptr<Texture>*> ( mData )->~shared_ptr();
            break;
        default:
            break;
        }
    }

    uint32_t Uniform::GetType() const
    {
        return mType;
    }

    const std::string Uniform::GetDeclaration() const
    {
        std::string declaration;
        switch ( mType )
        {
        case PropertyBuffer_Type_FLOAT:
            declaration = "float " + mName + ";\n";
            break;
        case PropertyBuffer_Type_FLOAT_VEC2:
            declaration = "vec2 " + mName + ";\n";
            break;
        case PropertyBuffer_Type_FLOAT_VEC3:
            declaration = "vec3 " + mName + ";\n";
            break;
        case PropertyBuffer_Type_FLOAT_VEC4:
            declaration = "vec4 " + mName + ";\n";
            break;
        case PropertyBuffer_Type_SAMPLER_2D:
            declaration = "sampler2D " + mName + ";\n";
            break;
        }
        return declaration;
    }
    const std::string & Uniform::GetName() const
    {
        return mName;
    }
    float Uniform::GetX() const
    {
        assert (
            mType == PropertyBuffer_Type_FLOAT ||
            mType == PropertyBuffer_Type_FLOAT_VEC2 ||
            mType == PropertyBuffer_Type_FLOAT_VEC3 ||
            mType == PropertyBuffer_Type_FLOAT_VEC4 );
        return mData[0];
    }
    float Uniform::GetY() const
    {
        assert ( mType == PropertyBuffer_Type_FLOAT_VEC2 ||
                 mType == PropertyBuffer_Type_FLOAT_VEC3 ||
                 mType == PropertyBuffer_Type_FLOAT_VEC4 );
        return mData[1];
    }
    float Uniform::GetZ() const
    {
        assert ( mType == PropertyBuffer_Type_FLOAT_VEC3 ||
                 mType == PropertyBuffer_Type_FLOAT_VEC4 );
        return mData[2];
    }
    float Uniform::GetW() const
    {
        assert ( mType == PropertyBuffer_Type_FLOAT_VEC4 );
        return mData[3];
    }
}
