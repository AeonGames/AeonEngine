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
#include "aeongames/Transform.h"

namespace AeonGames
{
    AABB::AABB()
    {
    }
    AABB::AABB ( const Vector3 & aCenter, const Vector3 & aRadii ) : mCenter ( aCenter ), mRadii ( aRadii )
    {
    }
    AABB::~AABB() = default;
    const Vector3 & AABB::GetCenter() const
    {
        return mCenter;
    }
    const Vector3 & AABB::GetRadii() const
    {
        return mRadii;
    }
    void AABB::SetCenter ( const Vector3 & aCenter )
    {
        mCenter = aCenter;
    }
    void AABB::SetRadii ( const Vector3 & aRadii )
    {
        mRadii = aRadii;
    }
    std::array<Vector3, 8> AABB::GetPoints ( const Vector3 & aOffset ) const
    {
        return std::array<Vector3, 8>
        {
            aOffset + mCenter + mRadii,
            aOffset + mCenter - mRadii,
            aOffset + mCenter + Vector3 ( -mRadii[0], mRadii[1], mRadii[2] ),
            aOffset + mCenter - Vector3 ( -mRadii[0], mRadii[1], mRadii[2] ),
            aOffset + mCenter + Vector3 ( mRadii[0], -mRadii[1], mRadii[2] ),
            aOffset + mCenter - Vector3 ( mRadii[0], -mRadii[1], mRadii[2] ),
            aOffset + mCenter + Vector3 ( mRadii[0], mRadii[1], -mRadii[2] ),
            aOffset + mCenter - Vector3 ( mRadii[0], mRadii[1], -mRadii[2] ),
        };
    }
}
