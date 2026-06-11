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
#include <array>
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
        /// @brief One frame of the iterative cell-traversal stack.
        struct CellFrame
        {
            uint64_t mLocationCode;
            AABB mBounds;
            uint32_t mDepth;
        };
        /// @brief Upper bound on the traversal stack depth: a depth-first walk
        /// that pushes all eight children on each pop holds at most seven pending
        /// siblings per level plus the eight just pushed, so the frontier never
        /// exceeds 7 * maxDepth + 8 entries. Sized for the deepest possible tree
        /// so the stack can live on the call stack with no heap allocation.
        constexpr size_t kCellStackCapacity = 7u * kMaxLocationCodeDepth + 8u;
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
            const uint8_t octant = bounds.OctantOf ( world.GetCenter() );
            const AABB child = bounds.GetChildOctant ( octant );
            if ( !child.Contains ( world ) )
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
            const uint8_t octant = bounds.OctantOf ( world.GetCenter() );
            const AABB child = bounds.GetChildOctant ( octant );
            if ( !child.Contains ( world ) )
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
        std::array<CellFrame, kCellStackCapacity> stack;
        size_t top = 0;
        stack[top++] = CellFrame{ 1, mRootBounds, 0 };
        while ( top != 0 )
        {
            const CellFrame frame = stack[--top];
            auto cell = mCells.find ( frame.mLocationCode );
            if ( cell == mCells.end() || !aFrustum.Intersects ( frame.mBounds ) )
            {
                continue;
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
                    stack[top++] = CellFrame{ ( frame.mLocationCode << 3 ) | octant, frame.mBounds.GetChildOctant ( octant ), 0 };
                }
            }
        }
    }

    void Octree::QueryAABB ( const AABB& aBox, const std::function<void ( const Node* ) >& aCallback ) const
    {
        std::array<CellFrame, kCellStackCapacity> stack;
        size_t top = 0;
        stack[top++] = CellFrame{ 1, mRootBounds, 0 };
        while ( top != 0 )
        {
            const CellFrame frame = stack[--top];
            auto cell = mCells.find ( frame.mLocationCode );
            if ( cell == mCells.end() || !frame.mBounds.Overlaps ( aBox ) )
            {
                continue;
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
                    stack[top++] = CellFrame{ ( frame.mLocationCode << 3 ) | octant, frame.mBounds.GetChildOctant ( octant ), 0 };
                }
            }
        }
    }

    void Octree::ForEachCell ( const std::function<void ( const AABB&, uint32_t ) >& aCallback ) const
    {
        std::array<CellFrame, kCellStackCapacity> stack;
        size_t top = 0;
        stack[top++] = CellFrame{ 1, mRootBounds, 0 };
        while ( top != 0 )
        {
            const CellFrame frame = stack[--top];
            auto cell = mCells.find ( frame.mLocationCode );
            if ( cell == mCells.end() )
            {
                continue;
            }
            aCallback ( frame.mBounds, frame.mDepth );
            const uint8_t child_exists = cell->second.mChildExists;
            for ( uint8_t octant = 0; octant < 8; ++octant )
            {
                if ( child_exists & static_cast<uint8_t> ( 1u << octant ) )
                {
                    stack[top++] = CellFrame{ ( frame.mLocationCode << 3 ) | octant, frame.mBounds.GetChildOctant ( octant ), frame.mDepth + 1 };
                }
            }
        }
    }

    void Octree::ForEachCell ( const Frustum& aFrustum, const std::function<void ( const AABB&, uint32_t ) >& aCallback ) const
    {
        std::array<CellFrame, kCellStackCapacity> stack;
        size_t top = 0;
        stack[top++] = CellFrame{ 1, mRootBounds, 0 };
        while ( top != 0 )
        {
            const CellFrame frame = stack[--top];
            auto cell = mCells.find ( frame.mLocationCode );
            if ( cell == mCells.end() || !aFrustum.Intersects ( frame.mBounds ) )
            {
                continue;
            }
            aCallback ( frame.mBounds, frame.mDepth );
            const uint8_t child_exists = cell->second.mChildExists;
            for ( uint8_t octant = 0; octant < 8; ++octant )
            {
                if ( child_exists & static_cast<uint8_t> ( 1u << octant ) )
                {
                    stack[top++] = CellFrame{ ( frame.mLocationCode << 3 ) | octant, frame.mBounds.GetChildOctant ( octant ), frame.mDepth + 1 };
                }
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
