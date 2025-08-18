/*
Copyright (C) 2015-2018,2025 Rodrigo Jose Hernandez Cordoba

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
#include <iostream>
#include "aeongames/Vector4.hpp"
#include "3DMath.h"

namespace AeonGames
{
    Vector4::Vector4()
    {
        memset ( mVector, 0, sizeof ( float ) * 4 );
    }

    Vector4::Vector4 ( const float* const aVector4 )
    {
        memcpy ( mVector, aVector4, sizeof ( float ) * 4 );
    }

    Vector4::Vector4 ( float aX, float aY, float aZ, float aW )
    {
        mVector[0] = aX;
        mVector[1] = aY;
        mVector[2] = aZ;
        mVector[3] = aW;
    }

    Vector4::~Vector4()
        = default;

    const float* const Vector4::GetVector4() const
    {
        return mVector;
    }

    const float& Vector4::GetX() const
    {
        return mVector[0];
    }

    const float& Vector4::GetY() const
    {
        return mVector[1];
    }

    const float& Vector4::GetZ() const
    {
        return mVector[2];
    }

    const float& Vector4::GetW() const
    {
        return mVector[3];
    }
    bool operator== ( const Vector4 & aLhs, const Vector4 & aRhs )
    {
        return memcmp ( aLhs.GetVector4(), aRhs.GetVector4(), sizeof ( float ) * 4 ) == 0;
    }
}
