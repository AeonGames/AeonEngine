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

#include <algorithm>
#include <cstdint>
#include <vector>
#include "aeongames/Octree.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Vector3.hpp"
#include "gtest/gtest.h"

using namespace ::testing;

namespace AeonGames
{
    // A node whose identity (global) transform leaves its local AABB unchanged in
    // world space, so the octree places it by the AABB we give it directly.
    static Node MakeNode ( const Vector3& aCenter, const Vector3& aRadii )
    {
        Node node;
        node.SetAABB ( AABB { aCenter, aRadii } );
        return node;
    }

    // Perspective view-projection looking down +Y (matches FrustumTests), near 1
    // far 100. Geometry centred around (0,50,0) is comfortably inside it.
    static Frustum MakeFrustum()
    {
        Matrix4x4 projection {};
        projection.Perspective ( 60.0f, 4.0f / 3.0f, 1.0f, 100.0f );
        return Frustum { projection };
    }

    TEST ( OctreeTest, DefaultConstructorIsEmpty )
    {
        Octree octree;
        EXPECT_EQ ( octree.GetNodeCount(), 0u );
        EXPECT_EQ ( octree.GetCellCount(), 0u );
        EXPECT_EQ ( octree.GetMaxDepth(), 0u );
    }

    TEST ( OctreeTest, ConstructorStoresRootBoundsAndDepth )
    {
        const AABB bounds { Vector3 { 1.0f, 2.0f, 3.0f }, Vector3 { 8.0f, 8.0f, 8.0f } };
        Octree octree { bounds, 5 };
        EXPECT_EQ ( octree.GetMaxDepth(), 5u );
        EXPECT_EQ ( octree.GetRootBounds().GetCenter() [0], 1.0f );
        EXPECT_EQ ( octree.GetRootBounds().GetCenter() [1], 2.0f );
        EXPECT_EQ ( octree.GetRootBounds().GetCenter() [2], 3.0f );
        EXPECT_EQ ( octree.GetRootBounds().GetRadii() [0], 8.0f );
    }

    TEST ( OctreeTest, MaxDepthIsClampedToLocationCodeLimit )
    {
        const AABB bounds { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } };
        Octree octree { bounds, 1000 };
        // A 64-bit location code can only represent 21 levels below the root.
        EXPECT_EQ ( octree.GetMaxDepth(), 21u );
    }

    TEST ( OctreeTest, AddNullNodeIsIgnored )
    {
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        octree.AddNode ( nullptr );
        EXPECT_EQ ( octree.GetNodeCount(), 0u );
        EXPECT_EQ ( octree.GetCellCount(), 0u );
    }

    TEST ( OctreeTest, SmallNodeDescendsToDeepestCell )
    {
        // Root spans [-8,8]^3, depth 3. A tiny box near the +X+Y+Z corner fits in
        // a child at every level, so it lands maxDepth levels down and creates one
        // cell per level (root + 3 descendants).
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        Node node = MakeNode ( Vector3 { 7.0f, 7.0f, 7.0f }, Vector3 { 0.1f, 0.1f, 0.1f } );
        octree.AddNode ( &node );
        EXPECT_EQ ( octree.GetNodeCount(), 1u );
        EXPECT_EQ ( octree.GetCellCount(), 4u );
    }

    TEST ( OctreeTest, StraddlingNodeStaysAtRoot )
    {
        // A box straddling the origin does not fit in any single child octant, so
        // it stays in the root cell: exactly one cell, no subdivision.
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        Node node = MakeNode ( Vector3 {}, Vector3 { 4.0f, 4.0f, 4.0f } );
        octree.AddNode ( &node );
        EXPECT_EQ ( octree.GetNodeCount(), 1u );
        EXPECT_EQ ( octree.GetCellCount(), 1u );
    }

    TEST ( OctreeTest, NodesInDifferentOctantsCreateSeparateBranches )
    {
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 2 };
        Node positive = MakeNode ( Vector3 { 6.0f, 6.0f, 6.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        Node negative = MakeNode ( Vector3 { -6.0f, -6.0f, -6.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        octree.AddNode ( &positive );
        octree.AddNode ( &negative );
        EXPECT_EQ ( octree.GetNodeCount(), 2u );
        // Shared root + two distinct depth-2 paths: 1 + 2 + 2 = 5 cells.
        EXPECT_EQ ( octree.GetCellCount(), 5u );
    }

    TEST ( OctreeTest, RemoveNodePrunesEmptyCells )
    {
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        Node node = MakeNode ( Vector3 { 7.0f, 7.0f, 7.0f }, Vector3 { 0.1f, 0.1f, 0.1f } );
        octree.AddNode ( &node );
        ASSERT_EQ ( octree.GetCellCount(), 4u );
        octree.RemoveNode ( &node );
        EXPECT_EQ ( octree.GetNodeCount(), 0u );
        EXPECT_EQ ( octree.GetCellCount(), 0u );
    }

    TEST ( OctreeTest, RemoveKeepsCellsSharedWithOtherNodes )
    {
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        // Two nodes that share the first descent step (both in the +X+Y+Z octant)
        // but diverge afterwards.
        Node deep = MakeNode ( Vector3 { 7.0f, 7.0f, 7.0f }, Vector3 { 0.1f, 0.1f, 0.1f } );
        Node shallow = MakeNode ( Vector3 { 5.0f, 5.0f, 5.0f }, Vector3 { 3.0f, 3.0f, 3.0f } );
        octree.AddNode ( &deep );
        octree.AddNode ( &shallow );
        const size_t populated = octree.GetCellCount();
        octree.RemoveNode ( &deep );
        EXPECT_EQ ( octree.GetNodeCount(), 1u );
        // Some cells were pruned, but the structure is not empty and the shared
        // branch survives.
        EXPECT_GT ( octree.GetCellCount(), 0u );
        EXPECT_LT ( octree.GetCellCount(), populated );
    }

    TEST ( OctreeTest, RemoveUnknownNodeIsNoOp )
    {
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        Node present = MakeNode ( Vector3 { 7.0f, 7.0f, 7.0f }, Vector3 { 0.1f, 0.1f, 0.1f } );
        Node absent = MakeNode ( Vector3 { 7.0f, 7.0f, 7.0f }, Vector3 { 0.1f, 0.1f, 0.1f } );
        octree.AddNode ( &present );
        octree.RemoveNode ( &absent );
        EXPECT_EQ ( octree.GetNodeCount(), 1u );
    }

    TEST ( OctreeTest, QueryFrustumReturnsAllVisibleNodes )
    {
        // Root sits in front of the camera (around +Y 50), so every cell intersects
        // the frustum and the query yields all stored nodes.
        Octree octree { AABB { Vector3 { 0.0f, 50.0f, 0.0f }, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        Node a = MakeNode ( Vector3 { 3.0f, 50.0f, 3.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        Node b = MakeNode ( Vector3 { -3.0f, 50.0f, -3.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        Node c = MakeNode ( Vector3 { 0.0f, 53.0f, 0.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        octree.AddNode ( &a );
        octree.AddNode ( &b );
        octree.AddNode ( &c );

        std::vector<const Node*> visited;
        octree.QueryFrustum ( MakeFrustum(), [&visited] ( const Node * node )
        {
            visited.push_back ( node );
        } );

        EXPECT_EQ ( visited.size(), 3u );
        EXPECT_NE ( std::find ( visited.begin(), visited.end(), &a ), visited.end() );
        EXPECT_NE ( std::find ( visited.begin(), visited.end(), &b ), visited.end() );
        EXPECT_NE ( std::find ( visited.begin(), visited.end(), &c ), visited.end() );
    }

    TEST ( OctreeTest, QueryFrustumCullsNodesBehindCamera )
    {
        // Root sits behind the camera (-Y), entirely outside the frustum, so the
        // whole subtree is skipped without visiting any node.
        Octree octree { AABB { Vector3 { 0.0f, -50.0f, 0.0f }, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        Node a = MakeNode ( Vector3 { 3.0f, -50.0f, 3.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        Node b = MakeNode ( Vector3 { -3.0f, -50.0f, -3.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        octree.AddNode ( &a );
        octree.AddNode ( &b );

        size_t count = 0;
        octree.QueryFrustum ( MakeFrustum(), [&count] ( const Node * )
        {
            ++count;
        } );
        EXPECT_EQ ( count, 0u );
    }

    TEST ( OctreeTest, QueryEmptyOctreeVisitsNothing )
    {
        Octree octree { AABB { Vector3 { 0.0f, 50.0f, 0.0f }, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        size_t count = 0;
        octree.QueryFrustum ( MakeFrustum(), [&count] ( const Node * )
        {
            ++count;
        } );
        EXPECT_EQ ( count, 0u );
    }

    TEST ( OctreeTest, QueryAABBReturnsOverlappingNodes )
    {
        // A query box covering the whole root reaches every stored node.
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        Node a = MakeNode ( Vector3 { 6.0f, 6.0f, 6.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        Node b = MakeNode ( Vector3 { -6.0f, -6.0f, -6.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        Node c = MakeNode ( Vector3 {}, Vector3 { 0.5f, 0.5f, 0.5f } );
        octree.AddNode ( &a );
        octree.AddNode ( &b );
        octree.AddNode ( &c );

        std::vector<const Node*> visited;
        octree.QueryAABB ( AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, [&visited] ( const Node * node )
        {
            visited.push_back ( node );
        } );

        EXPECT_EQ ( visited.size(), 3u );
        EXPECT_NE ( std::find ( visited.begin(), visited.end(), &a ), visited.end() );
        EXPECT_NE ( std::find ( visited.begin(), visited.end(), &b ), visited.end() );
        EXPECT_NE ( std::find ( visited.begin(), visited.end(), &c ), visited.end() );
    }

    TEST ( OctreeTest, QueryAABBSkipsDisjointSubtrees )
    {
        // A small query box in the +X+Y+Z octant must not visit a node living in
        // the opposite octant, since that whole subtree is skipped.
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        Node positive = MakeNode ( Vector3 { 6.0f, 6.0f, 6.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        Node negative = MakeNode ( Vector3 { -6.0f, -6.0f, -6.0f }, Vector3 { 0.5f, 0.5f, 0.5f } );
        octree.AddNode ( &positive );
        octree.AddNode ( &negative );

        std::vector<const Node*> visited;
        octree.QueryAABB ( AABB { Vector3 { 6.0f, 6.0f, 6.0f }, Vector3 { 0.5f, 0.5f, 0.5f } }, [&visited] ( const Node * node )
        {
            visited.push_back ( node );
        } );

        EXPECT_NE ( std::find ( visited.begin(), visited.end(), &positive ), visited.end() );
        EXPECT_EQ ( std::find ( visited.begin(), visited.end(), &negative ), visited.end() );
    }

    TEST ( OctreeTest, QueryAABBEmptyOctreeVisitsNothing )
    {
        Octree octree { AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, 3 };
        size_t count = 0;
        octree.QueryAABB ( AABB { Vector3 {}, Vector3 { 8.0f, 8.0f, 8.0f } }, [&count] ( const Node * )
        {
            ++count;
        } );
        EXPECT_EQ ( count, 0u );
    }
}
