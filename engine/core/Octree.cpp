/*
Copyright (C) 2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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

#include <algorithm>
#include <cmath>
#include "aeongames/Octree.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Transform.hpp"

namespace AeonGames
{
    namespace
    {
        /// @brief Maximum depth a 64-bit location code can represent (63 spare bits / 3 bits per level).
        constexpr uint32_t kMaxLocationCodeDepth = 21u;

        /** @brief Bounds of a child octant of @p aParent.
         *  @param aParent Parent cell bounds.
         *  @param aOctant Octant index: bit0=+X, bit1=+Y, bit2=+Z. */
        AABB ChildBounds ( const AABB& aParent, uint8_t aOctant )
        {
            const Vector3& center = aParent.GetCenter();
            const Vector3& radii = aParent.GetRadii();
            const Vector3 child_radii { radii[0] * 0.5f, radii[1] * 0.5f, radii[2] * 0.5f };
            const Vector3 child_center
            {
                center[0] + ( ( aOctant & 1u ) ? child_radii[0] : -child_radii[0] ),
                center[1] + ( ( aOctant & 2u ) ? child_radii[1] : -child_radii[1] ),
                center[2] + ( ( aOctant & 4u ) ? child_radii[2] : -child_radii[2] )
            };
            return AABB { child_center, child_radii };
        }

        /// @brief True if @p aInner is fully contained within @p aOuter.
        bool Contains ( const AABB& aOuter, const AABB& aInner )
        {
            const Vector3& outer_center = aOuter.GetCenter();
            const Vector3& outer_radii = aOuter.GetRadii();
            const Vector3& inner_center = aInner.GetCenter();
            const Vector3& inner_radii = aInner.GetRadii();
            for ( size_t i = 0; i < 3; ++i )
            {
                if ( std::abs ( inner_center[i] - outer_center[i] ) + inner_radii[i] > outer_radii[i] )
                {
                    return false;
                }
            }
            return true;
        }

        /// @brief Index of the octant of @p aCell that contains @p aPoint.
        uint8_t OctantOf ( const AABB& aCell, const Vector3& aPoint )
        {
            const Vector3& center = aCell.GetCenter();
            uint8_t octant = 0;
            if ( aPoint[0] >= center[0] )
            {
                octant |= 1u;
            }
            if ( aPoint[1] >= center[1] )
            {
                octant |= 2u;
            }
            if ( aPoint[2] >= center[2] )
            {
                octant |= 4u;
            }
            return octant;
        }
    }

    Octree::Octree() = default;

    Octree::Octree ( const AABB& aRootBounds, uint32_t aMaxDepth ) :
        mRootBounds{aRootBounds},
        mMaxDepth{ ( aMaxDepth > kMaxLocationCodeDepth ) ? kMaxLocationCodeDepth : aMaxDepth }
    {
    }

    Octree::~Octree() = default;

    void Octree::AddNode ( const Node* aNode )
    {
        if ( aNode == nullptr )
        {
            return;
        }
        const AABB world = aNode->GetGlobalTransform() * aNode->GetAABB();
        uint64_t location_code = 1;
        AABB bounds = mRootBounds;
        for ( uint32_t depth = 0; depth < mMaxDepth; ++depth )
        {
            const uint8_t octant = OctantOf ( bounds, world.GetCenter() );
            const AABB child = ChildBounds ( bounds, octant );
            if ( !Contains ( child, world ) )
            {
                break;
            }
            mCells[location_code].mChildExists |= static_cast<uint8_t> ( 1u << octant );
            location_code = ( location_code << 3 ) | octant;
            bounds = child;
        }
        mCells[location_code].mObjects.push_back ( aNode );
        ++mSize;
    }

    void Octree::RemoveNode ( const Node* aNode )
    {
        if ( aNode == nullptr )
        {
            return;
        }
        const AABB world = aNode->GetGlobalTransform() * aNode->GetAABB();
        uint64_t location_code = 1;
        AABB bounds = mRootBounds;
        for ( uint32_t depth = 0; depth < mMaxDepth; ++depth )
        {
            const uint8_t octant = OctantOf ( bounds, world.GetCenter() );
            const AABB child = ChildBounds ( bounds, octant );
            if ( !Contains ( child, world ) )
            {
                break;
            }
            location_code = ( location_code << 3 ) | octant;
            bounds = child;
        }
        auto cell = mCells.find ( location_code );
        if ( cell == mCells.end() )
        {
            return;
        }
        std::vector<const Node*>& objects = cell->second.mObjects;
        auto found = std::find ( objects.begin(), objects.end(), aNode );
        if ( found == objects.end() )
        {
            return;
        }
        objects.erase ( found );
        --mSize;
        // Prune cells that have become empty leaves, walking up to the root.
        while ( location_code != 1 )
        {
            const Cell& current = mCells[location_code];
            if ( !current.mObjects.empty() || current.mChildExists != 0 )
            {
                break;
            }
            const uint8_t octant = static_cast<uint8_t> ( location_code & 7u );
            const uint64_t parent = location_code >> 3;
            mCells.erase ( location_code );
            mCells[parent].mChildExists &= static_cast<uint8_t> ( ~ ( 1u << octant ) );
            location_code = parent;
        }
        // Drop the root cell too once it holds nothing, so a fully emptied octree
        // matches the cell count of a freshly constructed one.
        auto root = mCells.find ( 1 );
        if ( root != mCells.end() && root->second.mObjects.empty() && root->second.mChildExists == 0 )
        {
            mCells.erase ( root );
        }
    }

    void Octree::QueryFrustum ( const Frustum& aFrustum, const std::function<void ( const Node* ) >& aCallback ) const
    {
        QueryFrustum ( 1, mRootBounds, aFrustum, aCallback );
    }

    void Octree::QueryFrustum ( uint64_t aLocationCode, const AABB& aBounds, const Frustum& aFrustum, const std::function<void ( const Node* ) >& aCallback ) const
    {
        auto cell = mCells.find ( aLocationCode );
        if ( cell == mCells.end() || !aFrustum.Intersects ( aBounds ) )
        {
            return;
        }
        for ( const Node * node : cell->second.mObjects )
        {
            aCallback ( node );
        }
        const uint8_t child_exists = cell->second.mChildExists;
        for ( uint8_t octant = 0; octant < 8; ++octant )
        {
            if ( child_exists & static_cast<uint8_t> ( 1u << octant ) )
            {
                QueryFrustum ( ( aLocationCode << 3 ) | octant, ChildBounds ( aBounds, octant ), aFrustum, aCallback );
            }
        }
    }

    size_t Octree::GetNodeCount() const
    {
        return mSize;
    }

    size_t Octree::GetCellCount() const
    {
        return mCells.size();
    }

    uint32_t Octree::GetMaxDepth() const
    {
        return mMaxDepth;
    }

    const AABB& Octree::GetRootBounds() const
    {
        return mRootBounds;
    }
}
