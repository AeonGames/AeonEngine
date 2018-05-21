/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Vector2.h"
#include "3DMath.h"

namespace AeonGames
{
    Vector2::Vector2()
    {
        memset ( mVector, 0, sizeof ( float ) * 2 );
    }

    Vector2::Vector2 ( const float* const aVector2 )
    {
        memcpy ( mVector, aVector2, sizeof ( float ) * 2 );
    }

    Vector2::Vector2 ( float aX, float aY )
    {
        mVector[0] = aX;
        mVector[1] = aY;
    }

    Vector2::~Vector2()
        = default;

    const float* const Vector2::GetVector() const
    {
        return mVector;
    }

    const float& Vector2::GetX() const
    {
        return mVector[0];
    }

    const float& Vector2::GetY() const
    {
        return mVector[1];
    }
    bool operator== ( const Vector2 & aLhs, const Vector2 & aRhs )
    {
        return memcmp ( aLhs.GetVector(), aRhs.GetVector(), sizeof ( float ) * 2 ) == 0;
    }
}
