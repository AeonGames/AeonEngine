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

#ifndef AEONGAMES_FRUSTUM_H
#define AEONGAMES_FRUSTUM_H
/*! \file
    \brief Header for the frustum class.
    \author Rodrigo Hernandez.
    \copyright 2017
*/
#include <array>
#include "aeongames/Platform.hpp"
#include "aeongames/Plane.hpp"

namespace AeonGames
{
    class AABB;
    class Matrix4x4;
    class Vector3;
    /** @brief View frustum defined by six clipping planes, extracted from a projection (or view-projection) matrix. */
    class Frustum
    {
    public:
        ///@brief Default constructor.
        DLL Frustum();
        /** @brief Constructs a frustum by extracting the six clipping planes from a matrix.
            @param aMatrix Projection or view-projection matrix. */
        DLL Frustum ( const Matrix4x4& aMatrix );
        /// destructor.
        DLL ~Frustum();
        /** @brief Tests whether an axis-aligned bounding box intersects or is inside the frustum.
            @param aAABB The AABB to test.
            @return true if the AABB intersects or is inside the frustum. */
        DLL bool Intersects ( const AABB& aAABB ) const;
    private:
        /** @note Frustum planes' normals all point outward */
        std::array<Plane, 6> mPlanes;
    };
}
#endif
