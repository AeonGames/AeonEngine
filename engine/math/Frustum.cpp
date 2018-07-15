/*
Copyright (C) 2015-2018 Rodrigo Jose Hernandez Cordoba

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
#include "3DMath.h"
#include "aeongames/Plane.h"
#include "aeongames/Frustum.h"
#include "aeongames/AABB.h"
#include "aeongames/Matrix4x4.h"
#include <iostream>

namespace AeonGames
{
    Frustum::Frustum ( const Matrix4x4 & aMatrix ) :
        mPlanes
    {
        {
            // Left clipping plane
            Plane
            {
                - ( aMatrix[3] + aMatrix[0] ),
                    - ( aMatrix[7] + aMatrix[4] ),
                    - ( aMatrix[11] + aMatrix[8] ),
                    ( aMatrix[15] + aMatrix[12] )
                },
            // Right clipping plane
            Plane
            {
                - ( aMatrix[3] - aMatrix[0] ),
                    - ( aMatrix[7] - aMatrix[4] ),
                    - ( aMatrix[11] - aMatrix[8] ),
                    ( aMatrix[15] - aMatrix[12] )
                },
            // Top clipping plane
            Plane
            {
                - ( aMatrix[3] + aMatrix[1] ),
                    - ( aMatrix[7] + aMatrix[5] ),
                    - ( aMatrix[11] + aMatrix[9] ),
                    ( aMatrix[15] + aMatrix[13] )
                },
            // Bottom clipping plane
            Plane
            {
                - ( aMatrix[3] - aMatrix[1] ),
                    - ( aMatrix[7] - aMatrix[5] ),
                    - ( aMatrix[11] - aMatrix[9] ),
                    ( aMatrix[15] - aMatrix[13] )
                },
            // Near clipping plane
            Plane
            {
                - ( aMatrix[3] + aMatrix[2] ),
                    - ( aMatrix[7] + aMatrix[6] ),
                    - ( aMatrix[11] + aMatrix[10] ),
                    ( aMatrix[15] + aMatrix[14] )
                },
            // Far clipping plane
            Plane
            {
                - ( aMatrix[3] - aMatrix[2] ),
                    - ( aMatrix[7] - aMatrix[6] ),
                    - ( aMatrix[11] - aMatrix[10] ),
                    ( aMatrix[15] - aMatrix[14] )
                }
        }
    }
    {
    }
    Frustum::~Frustum()
        = default;
    bool Frustum::Intersects ( const AABB & aAABB ) const
    {
        for ( auto& plane : mPlanes )
        {
            if ( aAABB.GetDistanceToPlane ( plane ) > 0.0f )
            {
                return false;
            }
        }
        return true;
    }
}
