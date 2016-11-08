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
#include "OpenGLFunctions.h"
#include "OpenGLTexture.h"
#include "aeongames/ResourceCache.h"
#include "Uniform.h"
#include <cstring>

namespace AeonGames
{
    static_assert ( sizeof ( std::shared_ptr<Texture> ) <= ( sizeof ( float ) * 4 ), "Size of shared pointer is bigger than a vec4" );
    Uniform::Uniform ( const std::string& aName, float aX ) :
        mName ( aName ),
        mType ( GL_FLOAT )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = 0;
        ( reinterpret_cast<float*> ( mData ) ) [2] = 0;
        ( reinterpret_cast<float*> ( mData ) ) [3] = 0;
    }
    Uniform::Uniform ( const std::string& aName, float aX, float aY ) :
        mName ( aName ),
        mType ( GL_FLOAT_VEC2 )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = 0;
        ( reinterpret_cast<float*> ( mData ) ) [3] = 0;
    }
    Uniform::Uniform ( const std::string& aName, float aX, float aY, float aZ ) :
        mName ( aName ),
        mType ( GL_FLOAT_VEC3 )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
        ( reinterpret_cast<float*> ( mData ) ) [3] = 0;
    }
    Uniform::Uniform ( const std::string& aName, float aX, float aY, float aZ, float aW ) :
        mName ( aName ),
        mType ( GL_FLOAT_VEC4 )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
        ( reinterpret_cast<float*> ( mData ) ) [3] = aW;
    }
    Uniform::Uniform ( const std::string & aName, const std::string & aFilename ) :
        mName ( aName ),
        mType ( GL_SAMPLER_2D )
    {
        new ( mData ) std::shared_ptr<Texture> ( Get<OpenGLTexture> ( aFilename ) );
    }
    Uniform::~Uniform()
    {
        switch ( mType )
        {
        case GL_SAMPLER_2D:
            reinterpret_cast<std::shared_ptr<Texture>*> ( mData )->~shared_ptr();
            break;
        default:
            break;
        }
    }
    void Uniform::SetOffset ( const uint32_t aOffset )
    {
        mOffset = aOffset;
    }
    uint32_t Uniform::Offset() const
    {
        return mOffset;
    }
    const std::string Uniform::GetDeclaration() const
    {
        std::string declaration;
        switch ( mType )
        {
        case GL_FLOAT:
            declaration = "float " + mName + ";\n";
            break;
        case GL_FLOAT_VEC2:
            declaration = "vec2 " + mName + ";\n";
            break;
        case GL_FLOAT_VEC3:
            declaration = "vec3 " + mName + ";\n";
            break;
        case GL_FLOAT_VEC4:
            declaration = "vec4 " + mName + ";\n";
            break;
        case GL_SAMPLER_2D:
            declaration = "sampler2D " + mName + ";\n";
            break;
        }
        return declaration;
    }
    const std::string & Uniform::GetName() const
    {
        return mName;
    }
    void Uniform::CopyTo ( uint8_t * aMemory ) const
    {
        switch ( mType )
        {
        case GL_FLOAT:
            memcpy ( ( aMemory + mOffset ), mData, sizeof ( float ) );
            break;
        case GL_FLOAT_VEC2:
            memcpy ( ( aMemory + mOffset ), mData, sizeof ( float ) * 2 );
            break;
        case GL_FLOAT_VEC3:
            memcpy ( ( aMemory + mOffset ), mData, sizeof ( float ) * 3 );
            break;
        case GL_FLOAT_VEC4:
            memcpy ( ( aMemory + mOffset ), mData, sizeof ( float ) * 4 );
            break;
        case GL_SAMPLER_2D:
            ( * ( reinterpret_cast<uint64_t*> ( aMemory + mOffset ) ) ) =
                reinterpret_cast<const std::shared_ptr<Texture>*> ( mData )->get()->GetHandle();
            break;
        }
    }
}
