/*
Copyright (C) 2017-2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Plane.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Transform.hpp"

namespace AeonGames
{
    AABB::AABB()
        = default;
    AABB::AABB ( const Vector3 & aCenter, const Vector3 & aRadii ) : mCenter ( aCenter ), mRadii ( aRadii )
    {
    }
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
            {
                aOffset + mCenter + mRadii,
                aOffset + mCenter - mRadii,
                aOffset + mCenter + Vector3 ( -mRadii[0], mRadii[1], mRadii[2] ),
                aOffset + mCenter - Vector3 ( -mRadii[0], mRadii[1], mRadii[2] ),
                aOffset + mCenter + Vector3 ( mRadii[0], -mRadii[1], mRadii[2] ),
                aOffset + mCenter - Vector3 ( mRadii[0], -mRadii[1], mRadii[2] ),
                aOffset + mCenter + Vector3 ( mRadii[0], mRadii[1], -mRadii[2] ),
                aOffset + mCenter - Vector3 ( mRadii[0], mRadii[1], -mRadii[2] )
            }
        };
    }

    Transform AABB::GetTransform() const
    {
        return {mRadii.IsZero() ? Vector3{1, 1, 1} : mRadii, Quaternion{}, mCenter};
    }

    float AABB::GetDistanceToPlane ( const Plane & aPlane ) const
    {
        return Dot ( aPlane.GetNormal(),
                     mCenter +
                     Vector3
        {
            ( aPlane.GetNormal() [0] < 0 ) ? mRadii[0] : -mRadii[0],
            ( aPlane.GetNormal() [1] < 0 ) ? mRadii[1] : -mRadii[1],
            ( aPlane.GetNormal() [2] < 0 ) ? mRadii[2] : -mRadii[2]
        } ) - aPlane.GetDistance();
    }
    AABB& AABB::operator+= ( const AABB& lhs )
    {
        Vector3 min =
        {
            std::min ( mCenter[0] - mRadii[0], lhs.mCenter[0] - lhs.mRadii[0] ),
            std::min ( mCenter[1] - mRadii[1], lhs.mCenter[1] - lhs.mRadii[1] ),
            std::min ( mCenter[2] - mRadii[2], lhs.mCenter[2] - lhs.mRadii[2] )
        };
        Vector3 max =
        {
            std::max ( mCenter[0] + mRadii[0], lhs.mCenter[0] + lhs.mRadii[0] ),
            std::max ( mCenter[1] + mRadii[1], lhs.mCenter[1] + lhs.mRadii[1] ),
            std::max ( mCenter[2] + mRadii[2], lhs.mCenter[2] + lhs.mRadii[2] )
        };
        mCenter = ( min + max ) / 2.0f;
        mRadii = max - mCenter;
        return *this;
    }
}
