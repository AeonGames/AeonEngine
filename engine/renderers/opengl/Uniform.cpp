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
#include "Uniform.h"
namespace AeonGames
{
    static_assert ( sizeof ( std::shared_ptr<Texture> ) <= ( sizeof ( float ) * 4 ), "Size of shared pointer is bigger than a vec4" );
    Uniform::Uniform ( float aX ) : mType ( GL_FLOAT )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = 0;
        ( reinterpret_cast<float*> ( mData ) ) [2] = 0;
        ( reinterpret_cast<float*> ( mData ) ) [3] = 0;
    }
    Uniform::Uniform ( float aX, float aY ) : mType ( GL_FLOAT_VEC2 )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = 0;
        ( reinterpret_cast<float*> ( mData ) ) [3] = 0;
    }
    Uniform::Uniform ( float aX, float aY, float aZ ) : mType ( GL_FLOAT_VEC3 )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
        ( reinterpret_cast<float*> ( mData ) ) [3] = 0;
    }
    Uniform::Uniform ( float aX, float aY, float aZ, float aW ) : mType ( GL_FLOAT_VEC4 )
    {
        ( reinterpret_cast<float*> ( mData ) ) [0] = aX;
        ( reinterpret_cast<float*> ( mData ) ) [1] = aY;
        ( reinterpret_cast<float*> ( mData ) ) [2] = aZ;
        ( reinterpret_cast<float*> ( mData ) ) [3] = aW;
    }
    Uniform::~Uniform()
    {
    }
    int32_t Uniform::Location() const
    {
        return mLocation;
    }
    void Uniform::SetLocation ( int32_t aLocation )
    {
        mLocation = aLocation;
    }
}