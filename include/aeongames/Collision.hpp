/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_COLLISION_H
#define AEONGAMES_COLLISION_H
#include <cstdint>
#include <vector>
#include "aeongames/AABB.hpp"
#include "aeongames/Plane.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Resource.hpp"

namespace AeonGames
{
    class CollisionMsg;
    /** @brief Static collision geometry resource.
     *
     *  Loads a baked @c .cln collision file (a Kd-tree of convex brushes, each
     *  brush expressed as a 6-DOP slab plus optional bevel planes) and answers
     *  swept and overlap queries against it. The Kd-tree provides broad-phase
     *  acceleration so queries do not have to test every brush in the file. */
    class Collision final : public Resource
    {
    public:
        /** @brief Default constructor. */
        DLL Collision();
        /** @brief Destructor. */
        DLL ~Collision() final;
        /** @brief Load collision data from a raw @c .cln memory buffer.
         *  @param aBuffer Pointer to the buffer.
         *  @param aBufferSize Size of the buffer in bytes. */
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        /** @brief Load collision data from a protobuf message.
         *  @param aCollisionMsg The protobuf message to load from. */
        DLL void LoadFromPBMsg ( const CollisionMsg& aCollisionMsg );
        /** @brief Release all collision data. */
        DLL void Unload() final;
        /** @brief Get the axis-aligned bounding box enclosing all geometry.
         *  @return Const reference to the AABB. */
        DLL const AABB& GetAABB() const;
        /** @brief Sweep an axis-aligned box through the collision geometry.
         *
         *  Moves @p aBox along @p aDisplacement and returns the fraction of the
         *  displacement that can be travelled before the box first contacts a
         *  brush.
         *  @param aBox Axis-aligned box (center and half-extents) to sweep; use
         *         zero half-extents for a ray/point.
         *  @param aDisplacement Displacement vector to sweep along.
         *  @param[out] aContactPlane Optional. When a contact occurs, receives
         *         the contact plane. Left untouched when nothing is hit.
         *  @return Fraction in [0,1]; 1 means no contact along the displacement. */
        DLL float Sweep ( const AABB& aBox, const Vector3& aDisplacement, Plane* aContactPlane = nullptr ) const;
        /** @brief Cast a ray (point sweep) through the collision geometry.
         *  @param aOrigin Ray origin.
         *  @param aDirection Ray direction and length; the returned fraction is
         *         relative to this vector's length.
         *  @param[out] aContactPlane Optional contact plane on hit.
         *  @return Fraction in [0,1]; 1 means no contact along the ray. */
        DLL float RayCast ( const Vector3& aOrigin, const Vector3& aDirection, Plane* aContactPlane = nullptr ) const;
        /** @brief Test whether an axis-aligned box overlaps any brush.
         *  @param aBox Axis-aligned box (center and half-extents) to test.
         *  @return true if the box intersects the solid volume of any brush. */
        DLL bool Overlap ( const AABB& aBox ) const;
    private:
        /** @brief Convex brush: a 6-DOP slab plus a range of bevel planes. */
        struct Brush
        {
            float Positive[3]; ///< +X,+Y,+Z slab distances.
            float Negative[3]; ///< -X,-Y,-Z slab distances (stored already negated).
            uint32_t PlaneStart; ///< First index into the plane-index array.
            uint32_t PlaneCount; ///< Number of bevel planes for this brush.
        };
        /** @brief Internal Kd-tree split node. */
        struct KdNode
        {
            uint32_t Axis;     ///< Split axis (0=X,1=Y,2=Z).
            float Distance;    ///< Split position along the axis.
            int32_t NearIndex; ///< Child with coordinate < Distance (>=0 node, <0 leaf).
            int32_t FarIndex;  ///< Child with coordinate > Distance (>=0 node, <0 leaf).
        };
        /** @brief Internal Kd-tree leaf referencing a range of brush indices. */
        struct KdLeaf
        {
            uint32_t BrushStart; ///< First index into the brush-index array.
            uint32_t BrushCount; ///< Number of brushes in this leaf.
        };
        /** @brief Build the world-space plane for slab index @p aIndex of a brush.
         *
         *  Indices -6..-1 select the implicit 6-DOP slab planes (+X,+Y,+Z then
         *  -X,-Y,-Z); indices >=0 select bevel planes via the plane-index array,
         *  flipping the normal for negative plane references. */
        Plane GetBrushPlane ( const Brush& aBrush, int32_t aIndex ) const;
        /** @brief Slab-clip a swept box against a single brush.
         *  @return Entry fraction in [0,1]; 1 on miss. */
        float TraceBrush ( const Brush& aBrush, const Vector3& aOrigin, const Vector3& aDisplacement, const Vector3& aRadii, Plane& aContactPlane ) const;
        /** @brief Test a static box against a single brush. */
        bool OverlapBrush ( const Brush& aBrush, const Vector3& aOrigin, const Vector3& aRadii ) const;
        AABB mAABB{};
        std::vector<Plane> mPlanes{};
        std::vector<int32_t> mPlaneIndices{};
        std::vector<int32_t> mBrushIndices{};
        std::vector<Brush> mBrushes{};
        std::vector<KdNode> mKdNodes{};
        std::vector<KdLeaf> mKdLeaves{};
    };
}
#endif
