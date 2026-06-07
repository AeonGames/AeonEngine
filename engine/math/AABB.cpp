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
#include <algorithm>
#include <cmath>
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
        return {mRadii, Quaternion{}, mCenter};
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
    uint8_t AABB::OctantOf ( const Vector3 & aPoint ) const
    {
        uint8_t octant = 0;
        if ( aPoint[0] >= mCenter[0] )
        {
            octant |= 1u;
        }
        if ( aPoint[1] >= mCenter[1] )
        {
            octant |= 2u;
        }
        if ( aPoint[2] >= mCenter[2] )
        {
            octant |= 4u;
        }
        return octant;
    }
    AABB AABB::GetChildOctant ( uint8_t aOctant ) const
    {
        const Vector3 child_radii { mRadii[0] * 0.5f, mRadii[1] * 0.5f, mRadii[2] * 0.5f };
        const Vector3 child_center
        {
            mCenter[0] + ( ( aOctant & 1u ) ? child_radii[0] : -child_radii[0] ),
            mCenter[1] + ( ( aOctant & 2u ) ? child_radii[1] : -child_radii[1] ),
            mCenter[2] + ( ( aOctant & 4u ) ? child_radii[2] : -child_radii[2] )
        };
        return AABB { child_center, child_radii };
    }
    bool AABB::Contains ( const AABB & aInner ) const
    {
        const Vector3& inner_center = aInner.mCenter;
        const Vector3& inner_radii = aInner.mRadii;
        for ( size_t i = 0; i < 3; ++i )
        {
            if ( std::abs ( inner_center[i] - mCenter[i] ) + inner_radii[i] > mRadii[i] )
            {
                return false;
            }
        }
        return true;
    }
    bool AABB::Overlaps ( const AABB & aOther ) const
    {
        const Vector3& other_center = aOther.mCenter;
        const Vector3& other_radii = aOther.mRadii;
        for ( size_t i = 0; i < 3; ++i )
        {
            if ( std::abs ( mCenter[i] - other_center[i] ) > mRadii[i] + other_radii[i] )
            {
                return false;
            }
        }
        return true;
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
