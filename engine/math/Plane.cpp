/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#include <stdexcept>
#include "3DMath.h"
#include "aeongames/Plane.h"
#include "aeongames/AABB.h"

namespace AeonGames
{
    Plane::Plane ( float aNormalX, float aNormalY, float aNormalZ, float aDistance ) : mNormal ( aNormalX, aNormalY, aNormalZ ), mDistance ( aDistance )
    {
        float length = sqrtf ( Dot ( mNormal, mNormal ) );
        if ( !length )
        {
            throw std::runtime_error ( "Zero lenght normal passed to plane constructor." );
        }
        mNormal /= length;
        mDistance /= length;
    }

    Plane::~Plane()
        = default;

    float Plane::GetDistanceTo ( const Vector3 & aLocation, const AABB & aAABB ) const
    {
        Vector3 offsets
        {
            ( mNormal[0] < 0 ) ? aAABB.GetRadii() [0] : -aAABB.GetRadii() [0],
            ( mNormal[1] < 0 ) ? aAABB.GetRadii() [1] : -aAABB.GetRadii() [1],
            ( mNormal[2] < 0 ) ? aAABB.GetRadii() [2] : -aAABB.GetRadii() [2]
        };
        float dist = mDistance - Dot ( offsets, mNormal );
        return Dot ( mNormal, ( aLocation + aAABB.GetCenter() ) ) - dist;
    }
}
