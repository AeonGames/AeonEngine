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

#ifndef AEONGAMES_AABB_H
#define AEONGAMES_AABB_H
/*! \file
    \brief Header for the axis aligned bounding box class.
    \author Rodrigo Hernandez.
    \copyright 2017,2018
*/

#include <array>
#include <cstdint>
#include "aeongames/Platform.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Transform.hpp"

namespace AeonGames
{
    /*! \brief Axis Aligned Bounding Box class. */
    class AABB
    {
    public:
        ///@brief Default constructor.
        DLL AABB();
        /** @brief Construct an AABB from a center point and radii.
         *  @param aCenter Center position of the bounding box.
         *  @param aRadii Half-extents along each axis.
         */
        DLL AABB ( const Vector3& aCenter, const Vector3& aRadii );
        /** @brief Get the center position of the AABB.
         *  @return Const reference to the center vector.
         */
        DLL const Vector3& GetCenter() const;
        /** @brief Get the radii (half-extents) of the AABB.
         *  @return Const reference to the radii vector.
         */
        DLL const Vector3& GetRadii() const;
        /** @brief Set the center position of the AABB.
         *  @param aCenter New center position.
         */
        DLL void SetCenter ( const Vector3& aCenter );
        /** @brief Set the radii (half-extents) of the AABB.
         *  @param aRadii New radii values.
         */
        DLL void SetRadii ( const Vector3& aRadii );
        /** @brief Get the eight corner points of the AABB.
         *  @param aOffset Optional offset applied to all points.
         *  @return Array of eight corner vertices.
         */
        DLL std::array<Vector3, 8> GetPoints ( const Vector3& aOffset = { 0.0f, 0.0f, 0.0f, } ) const;
        /** Get the AABB as a transform, the AABB center becomes translation and the radii becomes scale. */
        DLL Transform GetTransform() const;
        /** Returns the shortest distance from any point in the plane's surface
        to the support point of the AABB in the plain normal inverted direction.
        In other words returns the required displacement of the AABB along the normal direction
        to leave the AABB just touching the plane from the positive side.*/
        DLL float GetDistanceToPlane ( const Plane& aPlane ) const;
        /** @brief Index of the octant of this AABB that contains @p aPoint.
         *
         *  Octant bits: bit0=+X, bit1=+Y, bit2=+Z. A point lying exactly on the
         *  center plane of an axis is assigned to the positive side of that axis.
         *  @param aPoint Point to classify against this box's center.
         *  @return Octant index in the range [0,7]. */
        DLL uint8_t OctantOf ( const Vector3& aPoint ) const;
        /** @brief Bounds of one of this AABB's eight child octants.
         *  @param aOctant Octant index: bit0=+X, bit1=+Y, bit2=+Z.
         *  @return The half-sized child box centered in the requested octant. */
        DLL AABB GetChildOctant ( uint8_t aOctant ) const;
        /** @brief Test whether another AABB is fully contained within this one.
         *  @param aInner The box to test for containment.
         *  @return True if @p aInner lies entirely inside this box. */
        DLL bool Contains ( const AABB& aInner ) const;
        /** @brief Test whether this AABB overlaps another (touching counts).
         *  @param aOther The box to test against.
         *  @return True if the two boxes intersect or touch. */
        DLL bool Overlaps ( const AABB& aOther ) const;
        /*! \name Operators */
        //@{
        /** @brief Expand this AABB to enclose another AABB.
         *  @param lhs The AABB to merge into this one.
         *  @return Reference to this AABB after expansion.
         */
        DLL AABB& operator+= ( const AABB& lhs );
        //@}
    private:
        Vector3 mCenter{};
        Vector3 mRadii{};
    };
}
#endif
