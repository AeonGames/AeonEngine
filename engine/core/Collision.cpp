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
#include "aeongames/ProtoBufClasses.hpp"
#include "aeongames/ProtoBufHelpers.hpp"
// The Windows headers define "near"/"far" as empty macros for legacy
// compatibility, which would clobber the generated KdTreeNodeMsg accessors.
// They must be undefined BEFORE collision.pb.h is parsed.
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "collision.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cassert>
#include "aeongames/Collision.hpp"

namespace AeonGames
{
    namespace
    {
        /** @brief Maximum Kd-tree traversal stack depth.
         *  A median-split tree over N points has depth ~log2(N); 128 comfortably
         *  covers any practical mesh while keeping the stack on the C++ stack. */
        constexpr int kTraversalStackSize = 128;

        /** @brief Decode a protobuf bytes field into a vector of int32 values. */
        void DecodeInt32Array ( const std::string& aBytes, std::vector<int32_t>& aOut )
        {
            const size_t count = aBytes.size() / sizeof ( int32_t );
            aOut.resize ( count );
            if ( count != 0 )
            {
                std::memcpy ( aOut.data(), aBytes.data(), count * sizeof ( int32_t ) );
            }
        }
    }

    Collision::Collision() = default;
    Collision::~Collision()
    {
        Unload();
    }

    void Collision::LoadFromMemory ( const void* aBuffer, size_t aBufferSize )
    {
        LoadFromProtoBufObject<Collision, CollisionMsg, "AEONCLN"_mgk> ( *this, aBuffer, aBufferSize );
    }

    void Collision::LoadFromPBMsg ( const CollisionMsg& aCollisionMsg )
    {
        Unload();

        mAABB = AABB
        {
            {
                aCollisionMsg.center().x(),
                             aCollisionMsg.center().y(),
                             aCollisionMsg.center().z()
            },
            {
                aCollisionMsg.radii().x(),
                             aCollisionMsg.radii().y(),
                             aCollisionMsg.radii().z()
            }
        };

        mPlanes.reserve ( aCollisionMsg.plane_size() );
        for ( const auto& plane : aCollisionMsg.plane() )
        {
            mPlanes.emplace_back ( plane.x(), plane.y(), plane.z(), plane.d() );
        }

        DecodeInt32Array ( aCollisionMsg.planeindices(), mPlaneIndices );
        DecodeInt32Array ( aCollisionMsg.brushindices(), mBrushIndices );

        mBrushes.reserve ( aCollisionMsg.brush_size() );
        for ( const auto& brush : aCollisionMsg.brush() )
        {
            mBrushes.push_back (
                Brush
            {
                {
                    brush.sixdop().positive().x(),
                         brush.sixdop().positive().y(),
                         brush.sixdop().positive().z()
                },
                {
                    brush.sixdop().negative().x(),
                         brush.sixdop().negative().y(),
                         brush.sixdop().negative().z()
                },
                brush.planestart(),
                     brush.planecount()
            } );
        }

        mKdNodes.reserve ( aCollisionMsg.kdnode_size() );
        for ( const auto& node : aCollisionMsg.kdnode() )
        {
            mKdNodes.push_back ( KdNode{ node.axis(), node.distance(), node.near(), node.far() } );
        }

        mKdLeaves.reserve ( aCollisionMsg.kdleaf_size() );
        for ( const auto& leaf : aCollisionMsg.kdleaf() )
        {
            mKdLeaves.push_back ( KdLeaf{ leaf.brushstart(), leaf.brushcount() } );
        }
    }

    void Collision::Unload()
    {
        mAABB = AABB{};
        mPlanes.clear();
        mPlaneIndices.clear();
        mBrushIndices.clear();
        mBrushes.clear();
        mKdNodes.clear();
        mKdLeaves.clear();
    }

    const AABB& Collision::GetAABB() const
    {
        return mAABB;
    }

    Plane Collision::GetBrushPlane ( const Brush& aBrush, int32_t aIndex ) const
    {
        // Indices -6..-1 are the implicit 6-DOP slab planes.
        switch ( aIndex )
        {
        case -6: // +X
            return Plane{ 1.0f, 0.0f, 0.0f, aBrush.Positive[0] };
        case -5: // +Y
            return Plane{ 0.0f, 1.0f, 0.0f, aBrush.Positive[1] };
        case -4: // +Z
            return Plane{ 0.0f, 0.0f, 1.0f, aBrush.Positive[2] };
        case -3: // -X
            return Plane{ -1.0f, 0.0f, 0.0f, aBrush.Negative[0] };
        case -2: // -Y
            return Plane{ 0.0f, -1.0f, 0.0f, aBrush.Negative[1] };
        case -1: // -Z
            return Plane{ 0.0f, 0.0f, -1.0f, aBrush.Negative[2] };
        default:
            break;
        }
        const int32_t plane_index = mPlaneIndices[aBrush.PlaneStart + aIndex];
        const Plane& plane = ( plane_index < 0 ) ? mPlanes[ -1 - plane_index ] : mPlanes[plane_index];
        if ( plane_index < 0 )
        {
            // A negative index references a plane whose normal must be flipped.
            const Vector3& normal = plane.GetNormal();
            return Plane{ -normal.GetX(), -normal.GetY(), -normal.GetZ(), -plane.GetDistance() };
        }
        return plane;
    }

    float Collision::TraceBrush ( const Brush& aBrush, const Vector3& aOrigin, const Vector3& aDisplacement, const Vector3& aRadii, Plane& aContactPlane ) const
    {
        float tfirst = 0.0f;
        float tlast = 1.0f;
        Plane hit{};
        // Indices -6..-1 are the implicit 6-DOP slab planes, processed in the
        // same loop as the explicit bevel planes to avoid code duplication.
        for ( int32_t i = -6; i < static_cast<int32_t> ( aBrush.PlaneCount ); ++i )
        {
            const Plane plane = GetBrushPlane ( aBrush, i );
            const Vector3& normal = plane.GetNormal();
            // Push the plane outward by the swept box's support along the normal
            // (Minkowski expansion) so the trace accounts for the box extents.
            const float support = Dot ( Abs ( normal ), aRadii );
            const float distance = aOrigin.GetDistanceToPlane ( plane ) - support;
            const float denominator = Dot ( normal, aDisplacement );
            if ( denominator == 0.0f )
            {
                if ( distance > 0.0f )
                {
                    // Parallel to the plane and already outside it: brush missed.
                    return 1.0f;
                }
                continue;
            }
            const float t = -distance / denominator;
            if ( denominator < 0.0f )
            {
                // Entering this half-space: keep the latest entry time.
                if ( t > tfirst )
                {
                    tfirst = t;
                    hit = plane;
                }
            }
            else if ( t < tlast )
            {
                // Exiting this half-space: keep the earliest exit time.
                tlast = t;
            }
            if ( tfirst > tlast )
            {
                // Entry past exit: the swept box never overlaps this brush.
                return 1.0f;
            }
        }
        aContactPlane = hit;
        return tfirst;
    }

    bool Collision::OverlapBrush ( const Brush& aBrush, const Vector3& aOrigin, const Vector3& aRadii ) const
    {
        for ( int32_t i = -6; i < static_cast<int32_t> ( aBrush.PlaneCount ); ++i )
        {
            const Plane plane = GetBrushPlane ( aBrush, i );
            const float support = Dot ( Abs ( plane.GetNormal() ), aRadii );
            const float distance = aOrigin.GetDistanceToPlane ( plane ) - support;
            if ( distance > 0.0f )
            {
                // Outside one half-space is enough to be outside the convex brush.
                return false;
            }
        }
        return true;
    }

    float Collision::Sweep ( const Vector3& aOrigin, const Vector3& aDisplacement, const Vector3& aRadii, Plane* aContactPlane ) const
    {
        float best_fraction = 1.0f;
        Plane best_plane{};

        auto trace_brush_index = [&] ( int32_t aBrushIndex )
        {
            Plane plane;
            const float fraction = TraceBrush ( mBrushes[aBrushIndex], aOrigin, aDisplacement, aRadii, plane );
            if ( fraction < best_fraction )
            {
                best_fraction = fraction;
                best_plane = plane;
            }
        };

        auto trace_leaf = [&] ( int32_t aLeafIndex )
        {
            const KdLeaf& leaf = mKdLeaves[aLeafIndex];
            for ( uint32_t k = 0; k < leaf.BrushCount; ++k )
            {
                trace_brush_index ( mBrushIndices[leaf.BrushStart + k] );
            }
        };

        if ( !mKdNodes.empty() )
        {
            int32_t stack[kTraversalStackSize];
            int top = 0;
            stack[top++] = 0;
            while ( top > 0 )
            {
                const KdNode& node = mKdNodes[stack[--top]];
                const float radius = aRadii[node.Axis];
                const float a0 = aOrigin[node.Axis];
                const float a1 = a0 + aDisplacement[node.Axis];
                const float lo = std::min ( a0, a1 ) - radius;
                const float hi = std::max ( a0, a1 ) + radius;
                // Descend into every child the swept segment can overlap.
                if ( lo < node.Distance )
                {
                    if ( node.NearIndex >= 0 )
                    {
                        assert ( top < kTraversalStackSize && "Kd-tree traversal stack overflow." );
                        stack[top++] = node.NearIndex;
                    }
                    else
                    {
                        trace_leaf ( -1 - node.NearIndex );
                    }
                }
                if ( hi > node.Distance )
                {
                    if ( node.FarIndex >= 0 )
                    {
                        assert ( top < kTraversalStackSize && "Kd-tree traversal stack overflow." );
                        stack[top++] = node.FarIndex;
                    }
                    else
                    {
                        trace_leaf ( -1 - node.FarIndex );
                    }
                }
            }
        }
        else if ( !mKdLeaves.empty() )
        {
            trace_leaf ( 0 );
        }
        else
        {
            for ( size_t b = 0; b < mBrushes.size(); ++b )
            {
                trace_brush_index ( static_cast<int32_t> ( b ) );
            }
        }

        if ( aContactPlane != nullptr && best_fraction < 1.0f )
        {
            *aContactPlane = best_plane;
        }
        return best_fraction;
    }

    float Collision::RayCast ( const Vector3& aOrigin, const Vector3& aDirection, Plane* aContactPlane ) const
    {
        return Sweep ( aOrigin, aDirection, Vector3{ 0.0f, 0.0f, 0.0f }, aContactPlane );
    }

    bool Collision::Overlap ( const Vector3& aOrigin, const Vector3& aRadii ) const
    {
        bool overlaps = false;

        auto overlap_leaf = [&] ( int32_t aLeafIndex ) -> bool
        {
            const KdLeaf& leaf = mKdLeaves[aLeafIndex];
            for ( uint32_t k = 0; k < leaf.BrushCount; ++k )
            {
                if ( OverlapBrush ( mBrushes[mBrushIndices[leaf.BrushStart + k]], aOrigin, aRadii ) )
                {
                    return true;
                }
            }
            return false;
        };

        if ( !mKdNodes.empty() )
        {
            int32_t stack[kTraversalStackSize];
            int top = 0;
            stack[top++] = 0;
            while ( top > 0 && !overlaps )
            {
                const KdNode& node = mKdNodes[stack[--top]];
                const float radius = aRadii[node.Axis];
                const float coordinate = aOrigin[node.Axis];
                const float lo = coordinate - radius;
                const float hi = coordinate + radius;
                if ( lo < node.Distance )
                {
                    if ( node.NearIndex >= 0 )
                    {
                        assert ( top < kTraversalStackSize && "Kd-tree traversal stack overflow." );
                        stack[top++] = node.NearIndex;
                    }
                    else if ( overlap_leaf ( -1 - node.NearIndex ) )
                    {
                        overlaps = true;
                    }
                }
                if ( !overlaps && hi > node.Distance )
                {
                    if ( node.FarIndex >= 0 )
                    {
                        assert ( top < kTraversalStackSize && "Kd-tree traversal stack overflow." );
                        stack[top++] = node.FarIndex;
                    }
                    else if ( overlap_leaf ( -1 - node.FarIndex ) )
                    {
                        overlaps = true;
                    }
                }
            }
        }
        else if ( !mKdLeaves.empty() )
        {
            overlaps = overlap_leaf ( 0 );
        }
        else
        {
            for ( size_t b = 0; b < mBrushes.size() && !overlaps; ++b )
            {
                overlaps = OverlapBrush ( mBrushes[b], aOrigin, aRadii );
            }
        }
        return overlaps;
    }
}
