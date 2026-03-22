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

#ifndef AEONGAMES_PLANE_H
#define AEONGAMES_PLANE_H
/*! \file
    \brief Header for the plane class.
    \author Rodrigo Hernandez.
    \copyright 2017,2018
*/

#include "aeongames/Platform.hpp"
#include "aeongames/Vector3.hpp"

namespace AeonGames
{
    class AABB;
    /*! \brief Plane class. */
    class Plane
    {
    public:
        ///@brief Default constructor.
        DLL Plane ();
        /** Construct a plane from a normal vector and distance.
         * @param aNormalX X component of the plane normal.
         * @param aNormalY Y component of the plane normal.
         * @param aNormalZ Z component of the plane normal.
         * @param aDistance Distance from the origin along the normal.
         */
        DLL Plane ( float aNormalX, float aNormalY, float aNormalZ, float aDistance );
        /// destructor.
        DLL ~Plane();
        /** Get the plane normal vector.
         * @return Const reference to the normal vector.
         */
        DLL const Vector3& GetNormal() const;
        /** Get the distance from the origin to the plane.
         * @return Const reference to the distance value.
         */
        DLL const float& GetDistance() const;
    private:
        Vector3 mNormal;
        float mDistance{};
    };
}
#endif
