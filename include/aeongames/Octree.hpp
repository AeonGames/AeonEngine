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
#ifndef AEONGAMES_OCTREE_H
#define AEONGAMES_OCTREE_H
#include "aeongames/Platform.hpp"
#include "aeongames/AABB.hpp"
#include <cstdint>
#include <vector>
#include <functional>
#include <unordered_map>
namespace AeonGames
{
    class Node;
    class Frustum;
    /** @brief Linear (hashed) octree for broad-phase spatial queries.
     *
     * Cells are stored in a hash map keyed by a 64-bit @e location @e code rather
     * than as a pointer-linked tree (see
     * https://geidav.wordpress.com/2014/08/18/advanced-octrees-2-node-representations/).
     * The location code is built by concatenating 3-bit octant indices from the
     * root downwards, preceded by a single sentinel bit, so the root cell has the
     * location code @c 1. For a cell with location code @c L:
     *  - its parent is @c L>>3,
     *  - its child in octant @c o (0..7) is @c (L<<3)|o,
     *  - its depth is @c floor(log2(L)/3).
     *
     * Each cell carries a one-byte @c ChildExists mask: bit @c o is set when the
     * child cell in octant @c o is present in the map. Cell bounds are never
     * stored; they are derived on the fly from the root bounds and the location
     * code, which keeps the per-cell footprint to a node list plus a single byte.
     *
     * The root bounds are fixed at construction (the structure does not grow to
     * fit out-of-bounds geometry); objects that do not fit any child octant, or
     * fall outside the root, come to rest in the deepest cell that fully contains
     * them, defaulting to the root. */
    class Octree
    {
    public:
        ///@brief Default constructor. Produces an empty octree with zero-sized root bounds.
        DLL Octree();
        /** @brief Construct an octree spanning the given world bounds.
         *  @param aRootBounds Axis-aligned bounds of the root cell in world space.
         *  @param aMaxDepth Maximum subdivision depth (clamped to the 21 levels a
         *         64-bit location code can represent). */
        DLL Octree ( const AABB& aRootBounds, uint32_t aMaxDepth );
        DLL ~Octree();
        /** @brief Insert a node into the octree.
         *
         * The node's world-space AABB (its global transform applied to its local
         * AABB) determines placement: the node is stored in the deepest cell that
         * fully contains that box.
         *  @param aNode Pointer to the node to add. */
        DLL void AddNode ( const Node* aNode );
        /** @brief Remove a node from the octree.
         *
         * Placement is recomputed from the node's current world-space AABB, so the
         * node must not have moved since it was added. Empty cells are pruned.
         *  @param aNode Pointer to the node to remove. */
        DLL void RemoveNode ( const Node* aNode );
        /** @brief Visit every node whose cell intersects the frustum.
         *
         * Descends from the root, skipping whole subtrees whose cell bounds fall
         * entirely outside the frustum. The callback may be invoked on nodes that
         * are ultimately outside the frustum (this is a conservative broad phase);
         * callers should perform an exact per-node test where needed.
         *  @param aFrustum The frustum to test cell bounds against.
         *  @param aCallback Invoked once per node found inside an intersecting cell. */
        DLL void QueryFrustum ( const Frustum& aFrustum, const std::function<void ( const Node* ) >& aCallback ) const;
        /** @brief Visit every node whose cell intersects the query box.
         *
         * Descends from the root, skipping whole subtrees whose cell bounds fall
         * entirely outside @p aBox. Like QueryFrustum this is a conservative broad
         * phase: the callback may be invoked on nodes whose own AABB does not
         * actually overlap @p aBox, so callers needing an exact result should
         * re-test each node.
         *  @param aBox The query box to test cell bounds against.
         *  @param aCallback Invoked once per node found inside an intersecting cell. */
        DLL void QueryAABB ( const AABB& aBox, const std::function<void ( const Node* ) >& aCallback ) const;
        /** @brief Visit every allocated cell, passing its world-space bounds and depth.
         *
         * Intended for debug visualization of the spatial subdivision (drawing the
         * octree grid). Cells are visited in depth-first order from the root; the
         * root cell has depth 0. Cell bounds are derived on the fly from the root
         * bounds and location code.
         *  @param aCallback Invoked once per allocated cell with its bounds and depth. */
        DLL void ForEachCell ( const std::function<void ( const AABB&, uint32_t ) >& aCallback ) const;
        /** @brief Visit every allocated cell whose bounds intersect the frustum.
         *
         * Like ForEachCell but skips whole subtrees whose bounds fall entirely
         * outside @p aFrustum, so only cells potentially on screen are visited.
         *  @param aFrustum The frustum to test cell bounds against.
         *  @param aCallback Invoked once per intersecting cell with its bounds and depth. */
        DLL void ForEachCell ( const Frustum& aFrustum, const std::function<void ( const AABB&, uint32_t ) >& aCallback ) const;
        ///@brief Number of nodes currently stored.
        DLL size_t GetNodeCount() const;
        ///@brief Number of allocated cells (occupied or on the path to an occupied cell).
        DLL size_t GetCellCount() const;
        ///@brief Maximum subdivision depth.
        DLL uint32_t GetMaxDepth() const;
        ///@brief Root cell bounds in world space.
        DLL const AABB& GetRootBounds() const;
    private:
        struct Cell
        {
            std::vector<const Node*> mObjects{};
            uint8_t mChildExists{0};
        };
        AABB mRootBounds{};
        uint32_t mMaxDepth{0};
        size_t mSize{0};
        std::unordered_map<uint64_t, Cell> mCells{};
    };
}
#endif
