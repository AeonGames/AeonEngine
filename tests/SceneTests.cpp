/*
Copyright (C) 2015,2018,2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <memory>
#include <algorithm>
#include <cmath>
#include <vector>
#include "gtest/gtest.h"
#include "aeongames/CRC.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Transform.hpp"

using namespace ::testing;
namespace AeonGames
{
    class SceneTest : public ::testing::Test
    {
    protected:
        SceneTest()
        {
        }
        virtual ~SceneTest()
        {
        }
        virtual void SetUp()
        {
            a = mScene.Add ( std::make_unique<Node>() );
            b = mScene.Add ( std::make_unique<Node>() );
            c = a->Add ( std::make_unique<Node>() );
            d = a->Add ( std::make_unique<Node>() );
            e = b->Add ( std::make_unique<Node>() );
            f = b->Add ( std::make_unique<Node>() );
            g = c->Add ( std::make_unique<Node>() );
            h = c->Add ( std::make_unique<Node>() );
            i = d->Add ( std::make_unique<Node>() );
            j = d->Add ( std::make_unique<Node>() );
            k = e->Add ( std::make_unique<Node>() );
            l = e->Add ( std::make_unique<Node>() );
            m = f->Add ( std::make_unique<Node>() );
            n = f->Add ( std::make_unique<Node>() );
            a->SetName ( "a" );
            b->SetName ( "b" );
            c->SetName ( "c" );
            d->SetName ( "d" );
            e->SetName ( "e" );
            f->SetName ( "f" );
            g->SetName ( "g" );
            h->SetName ( "h" );
            i->SetName ( "i" );
            j->SetName ( "j" );
            k->SetName ( "k" );
            l->SetName ( "l" );
            m->SetName ( "m" );
            n->SetName ( "n" );
        }
        virtual void TearDown()
        {
        }
        /*
        This tree should look like this:
                                                scene
                                                 /\
                                                /  \
                                               /    \
                                              a      b
                                             /\      /\
                                            /  \    /  \
                                           /    \  /    \
                                           c    d  e     f
                                          /\   /\  /\   /\
                                          g h i  j k l  m n
        */
        Node* a{};
        Node* b{};
        Node* c{};
        Node* d{};
        Node* e{};
        Node* f{};
        Node* g{};
        Node* h{};
        Node* i{};
        Node* j{};
        Node* k{};
        Node* l{};
        Node* m{};
        Node* n{};
        Scene mScene{};
    };

    TEST_F ( SceneTest, LoopTraverseDFSPostOrderTraversesAllNodes )
    {
        std::vector < Node* > nodeVector;
        mScene.LoopTraverseDFSPostOrder ( [&nodeVector] ( Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }
    TEST_F ( SceneTest, RecursiveTraverseDFSPostOrderTraversesAllNodes )
    {
        std::vector < Node* > nodeVector;
        mScene.LoopTraverseDFSPostOrder ( [&nodeVector] ( Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }
    TEST_F ( SceneTest, LoopTraverseDFSPostOrderIsEquivalentToRecursiveTraverseDFS )
    {
        std::vector < Node* > loopNodeVector;
        std::vector < Node* > recurseNodeVector;
        mScene.LoopTraverseDFSPostOrder ( [&loopNodeVector] ( Node & aNode )
        {
            loopNodeVector.push_back ( &aNode );
        } );
        mScene.RecursiveTraverseDFSPostOrder ( [&recurseNodeVector] ( Node & aNode )
        {
            recurseNodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( loopNodeVector, recurseNodeVector );
    }
    TEST_F ( SceneTest, CallingLoopTraverseDFSPostOrderMoreThanOnceHasSameResult )
    {
        std::vector < Node* > nodeVectorA;
        std::vector < Node* > nodeVectorB;
        mScene.LoopTraverseDFSPostOrder ( [&nodeVectorA] ( Node & aNode )
        {
            nodeVectorA.push_back ( &aNode );
        } );
        mScene.LoopTraverseDFSPostOrder ( [&nodeVectorB] ( Node & aNode )
        {
            nodeVectorB.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVectorA, nodeVectorB );
    }
    TEST_F ( SceneTest, LoopTraverseDFSPreOrderTraversesAllNodes )
    {
        std::vector < Node* > nodeVector;
        mScene.LoopTraverseDFSPreOrder ( [&nodeVector] ( Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }
    TEST_F ( SceneTest, RecursiveTraverseDFSPreOrderTraversesAllNodes )
    {
        std::vector < Node* > nodeVector;
        mScene.LoopTraverseDFSPreOrder ( [&nodeVector] ( Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }
    TEST_F ( SceneTest, LoopTraverseDFSPreOrderIsEquivalentToRecursiveTraverseDFS )
    {
        std::vector < Node* > loopNodeVector;
        std::vector < Node* > recurseNodeVector;
        mScene.LoopTraverseDFSPreOrder ( [&loopNodeVector] ( Node & aNode )
        {
            loopNodeVector.push_back ( &aNode );
        } );
        mScene.RecursiveTraverseDFSPreOrder ( [&recurseNodeVector] ( Node & aNode )
        {
            recurseNodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( loopNodeVector, recurseNodeVector );
    }
    TEST_F ( SceneTest, CallingLoopTraverseDFSPreOrderMoreThanOnceHasSameResult )
    {
        std::vector < Node* > nodeVectorA;
        std::vector < Node* > nodeVectorB;
        mScene.LoopTraverseDFSPreOrder ( [&nodeVectorA] ( Node & aNode )
        {
            nodeVectorA.push_back ( &aNode );
        } );
        mScene.LoopTraverseDFSPreOrder ( [&nodeVectorB] ( Node & aNode )
        {
            nodeVectorB.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVectorA, nodeVectorB );
    }
    TEST_F ( SceneTest, RemoveWorks )
    {
        EXPECT_EQ ( mScene.GetChildrenCount(), 2u );
        mScene.RemoveByIndex ( 0u );
        EXPECT_EQ ( mScene.GetChildrenCount(), 1u );
        mScene.Remove ( &mScene[0] );
        EXPECT_EQ ( mScene.GetChildrenCount(), 0u );
    }
    TEST_F ( SceneTest, LocalTransformSyncsGlobalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mScene[0].SetLocalTransform ( transform );
        EXPECT_EQ ( mScene[0].GetLocalTransform(), mScene[0].GetGlobalTransform() );
    }
    TEST_F ( SceneTest, GlobalTransformSyncsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mScene[0].SetGlobalTransform ( transform );
        EXPECT_EQ ( mScene[0].GetLocalTransform(), mScene[0].GetGlobalTransform() );
    }
    TEST_F ( SceneTest, AddSynchsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{2, 4, 6}};
        std::unique_ptr<Node> node{std::make_unique<Node>() };
        node->SetGlobalTransform ( transform );
        transform.SetTranslation ( {1, 2, 3} );
        mScene[0].SetLocalTransform ( transform );
        mScene[0].Add ( std::move ( node ) );
        EXPECT_EQ ( transform, mScene[0][mScene[0].GetChildrenCount() - 1].GetLocalTransform() );
    }
    TEST_F ( SceneTest, LocalTransformSyncsGlobalTransformInChildNode )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mScene[0].SetLocalTransform ( transform );
        EXPECT_EQ ( mScene[0].GetLocalTransform(), mScene[0][0].GetGlobalTransform() );
    }
    TEST_F ( SceneTest, GlobalTransformSyncsGlobalTransformInChildNode )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mScene[0].SetGlobalTransform ( transform );
        EXPECT_EQ ( mScene[0].GetLocalTransform(), mScene[0][0].GetGlobalTransform() );
    }
    TEST_F ( SceneTest, LocalTransformLeavesLocalTransformInChildNodeUntoched )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mScene[0].SetLocalTransform ( transform );
        transform.SetTranslation ( {0, 0, 0} );
        EXPECT_EQ ( transform, mScene[0][0].GetLocalTransform() );
    }
    TEST_F ( SceneTest, GlobalTransformLeavesLocalTransformInChildNodeUntoched )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mScene[0].SetGlobalTransform ( transform );
        transform.SetTranslation ( {0, 0, 0} );
        EXPECT_EQ ( transform, mScene[0][0].GetLocalTransform() );
    }
    TEST_F ( SceneTest, DefaultIndexIsInvalid )
    {
        Node node;
        EXPECT_THROW ( node.GetIndex(), std::runtime_error );
    }
    TEST_F ( SceneTest, IndicesAreContiguous )
    {
        std::unique_ptr<Node> node{std::make_unique<Node>() };
        node->Add ( std::make_unique<Node>() );
        node->Add ( std::make_unique<Node>() );
        node->Add ( std::make_unique<Node>() );
        size_t count = node->GetChildrenCount();
        for ( size_t i = 0; i < count; ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
    }
    TEST_F ( SceneTest, IndicesAreContiguousAfterRemovingFirstNode )
    {
        std::unique_ptr<Node> node{std::make_unique<Node>() };
        node->Add ( std::make_unique<Node>() );
        node->Add ( std::make_unique<Node>() );
        node->Add ( std::make_unique<Node>() );
        EXPECT_EQ ( node->GetChildrenCount(), 3u );
        node->Remove ( node->GetChild ( 0 ) );
        for ( size_t i = 0; i < node->GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
    }
    TEST_F ( SceneTest, IndicesAreContiguousAfterRemovingMiddleNode )
    {
        std::unique_ptr<Node> node{std::make_unique<Node>() };
        node->Add ( std::make_unique<Node>() );
        node->Add ( std::make_unique<Node>() );
        node->Add ( std::make_unique<Node>() );
        EXPECT_EQ ( node->GetChildrenCount(), 3u );
        Node* removedNode = node->GetChild ( 1 );
        ( *node ) [0].Remove ( removedNode );
        for ( size_t i = 0; i < node->GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
    }
    TEST_F ( SceneTest, IndicesAreContiguousAfterInsertingNodeOnFront )
    {
        std::unique_ptr<Node> node{std::make_unique<Node>() };
        node->Add ( std::make_unique<Node>() );
        node->Add ( std::make_unique<Node>() );
        node->Insert ( 0, std::make_unique<Node>() );
        EXPECT_EQ ( node->GetChildrenCount(), 3u );
        for ( size_t i = 0; i < node->GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
        EXPECT_EQ ( & ( *node ) [0], node->GetChild ( 0 ) );
    }
    TEST_F ( SceneTest, IndicesAreContiguousAfterInsertingNodeAtMiddle )
    {
        std::unique_ptr<Node> node{std::make_unique<Node>() };
        node->Add ( std::make_unique<Node>() );
        node->Add ( std::make_unique<Node>() );
        node->Insert ( 0, std::make_unique<Node>() );
        EXPECT_EQ ( node->GetChildrenCount(), 3u );
        for ( size_t i = 0; i < node->GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
        EXPECT_EQ ( & ( *node ) [1], node->GetChild ( 1 ) );
    }
    TEST_F ( SceneTest, SerializeAsTextHasProperHeader )
    {
        std::string serialized = mScene.Serialize ( false );
        EXPECT_EQ ( serialized.substr ( 0, 8 ), "AEONSCN\n" );
    }

    TEST_F ( SceneTest, SerializeAsBinaryHasProperHeader )
    {
        std::string serialized = mScene.Serialize ( true );
        EXPECT_EQ ( serialized.substr ( 0, 7 ), "AEONSCN" );
        EXPECT_EQ ( serialized[7], 0 );
    }
    TEST_F ( SceneTest, SerializeDeserializeText )
    {
        Scene scene;
        std::string serialized = mScene.Serialize ( false );
        scene.Deserialize ( serialized );
        EXPECT_EQ ( scene.Serialize ( false ), serialized );
    }
    TEST_F ( SceneTest, SerializeDeserializeBinary )
    {
        std::string serialized = mScene.Serialize ();
        Scene scene;
        scene.Deserialize ( serialized );
        EXPECT_EQ ( scene.Serialize(), serialized );
    }

    TEST_F ( SceneTest, GetScene )
    {
        EXPECT_EQ ( a->GetScene(), &mScene );
        EXPECT_EQ ( b->GetScene(), &mScene );
        EXPECT_EQ ( c->GetScene(), &mScene );
        EXPECT_EQ ( d->GetScene(), &mScene );
        EXPECT_EQ ( e->GetScene(), &mScene );
        EXPECT_EQ ( f->GetScene(), &mScene );
        EXPECT_EQ ( g->GetScene(), &mScene );
        EXPECT_EQ ( h->GetScene(), &mScene );
        EXPECT_EQ ( i->GetScene(), &mScene );
        EXPECT_EQ ( j->GetScene(), &mScene );
        EXPECT_EQ ( k->GetScene(), &mScene );
        EXPECT_EQ ( l->GetScene(), &mScene );
        EXPECT_EQ ( m->GetScene(), &mScene );
        EXPECT_EQ ( n->GetScene(), &mScene );
    }

    // ---- CullVisible (octree-backed visibility query) -------------------------

    namespace
    {
        // Perspective view-projection looking down +Y (matches FrustumTests), near
        // 1, far 100. Geometry around (0,50,0) is comfortably inside it.
        Frustum MakeCullFrustum()
        {
            Matrix4x4 projection {};
            projection.Perspective ( 60.0f, 4.0f / 3.0f, 1.0f, 100.0f );
            return Frustum { projection };
        }

        // Add a unit-AABB node positioned at a world location via its global
        // transform (exercising the transform path that invalidates the index).
        Node* AddPositioned ( Scene& aScene, const Vector3& aPosition )
        {
            Node* node = aScene.Add ( std::make_unique<Node>() );
            node->SetAABB ( AABB { Vector3 {}, Vector3 { 1.0f, 1.0f, 1.0f } } );
            node->SetGlobalTransform ( Transform { Vector3 { 1.0f, 1.0f, 1.0f }, Quaternion {}, aPosition } );
            return node;
        }

        // Brute-force reference: the exact set of nodes whose world AABB hits the
        // frustum, gathered by a plain traversal.
        std::vector<const Node*> BruteForceVisible ( const Scene& aScene, const Frustum& aFrustum )
        {
            std::vector<const Node*> visible;
            aScene.LoopTraverseDFSPreOrder ( [&aFrustum, &visible] ( const Node & aNode )
            {
                const AABB world = aNode.GetGlobalTransform() * aNode.GetAABB();
                if ( aFrustum.Intersects ( world ) )
                {
                    visible.push_back ( &aNode );
                }
            } );
            std::sort ( visible.begin(), visible.end() );
            return visible;
        }

        std::vector<const Node*> CullVisibleSet ( const Scene& aScene, const Frustum& aFrustum )
        {
            std::vector<const Node*> visible;
            aScene.CullVisible ( aFrustum, [&visible] ( const Node & aNode )
            {
                visible.push_back ( &aNode );
            } );
            std::sort ( visible.begin(), visible.end() );
            return visible;
        }

        // True if the two axis-aligned boxes overlap (touching counts), matching
        // Scene::QueryAABB's predicate.
        bool BoxesOverlap ( const AABB& aLhs, const AABB& aRhs )
        {
            const Vector3& lc = aLhs.GetCenter();
            const Vector3& lr = aLhs.GetRadii();
            const Vector3& rc = aRhs.GetCenter();
            const Vector3& rr = aRhs.GetRadii();
            for ( int i = 0; i < 3; ++i )
            {
                if ( std::abs ( lc[i] - rc[i] ) > lr[i] + rr[i] )
                {
                    return false;
                }
            }
            return true;
        }

        // Brute-force reference: the exact set of nodes whose world AABB overlaps
        // the query box, gathered by a plain traversal.
        std::vector<const Node*> BruteForceOverlap ( const Scene& aScene, const AABB& aBox )
        {
            std::vector<const Node*> hits;
            aScene.LoopTraverseDFSPreOrder ( [&aBox, &hits] ( const Node & aNode )
            {
                const AABB world = aNode.GetGlobalTransform() * aNode.GetAABB();
                if ( BoxesOverlap ( aBox, world ) )
                {
                    hits.push_back ( &aNode );
                }
            } );
            std::sort ( hits.begin(), hits.end() );
            return hits;
        }

        std::vector<const Node*> QueryAABBSet ( const Scene& aScene, const AABB& aBox )
        {
            std::vector<const Node*> hits;
            aScene.QueryAABB ( aBox, [&hits] ( const Node & aNode )
            {
                hits.push_back ( &aNode );
            } );
            std::sort ( hits.begin(), hits.end() );
            return hits;
        }
    }

    TEST ( SceneCullVisible, MatchesBruteForceMixedScene )
    {
        Scene scene;
        AddPositioned ( scene, Vector3 { 0.0f, 50.0f, 0.0f } );   // inside
        AddPositioned ( scene, Vector3 { 10.0f, 50.0f, 5.0f } );  // inside
        AddPositioned ( scene, Vector3 { 0.0f, -50.0f, 0.0f } );  // behind camera
        AddPositioned ( scene, Vector3 { 1000.0f, 50.0f, 0.0f } ); // far to the side
        AddPositioned ( scene, Vector3 { 0.0f, 90.0f, 0.0f } );   // near the far plane
        const Frustum frustum = MakeCullFrustum();
        EXPECT_EQ ( CullVisibleSet ( scene, frustum ), BruteForceVisible ( scene, frustum ) );
    }

    TEST ( SceneCullVisible, EmptySceneVisitsNothing )
    {
        Scene scene;
        size_t count = 0;
        scene.CullVisible ( MakeCullFrustum(), [&count] ( const Node& )
        {
            ++count;
        } );
        EXPECT_EQ ( count, 0u );
    }

    TEST ( SceneCullVisible, RebuildsAfterNodeMoves )
    {
        Scene scene;
        Node* mover = AddPositioned ( scene, Vector3 { 0.0f, -50.0f, 0.0f } ); // behind
        const Frustum frustum = MakeCullFrustum();
        // Initially behind the camera: not visible.
        EXPECT_TRUE ( CullVisibleSet ( scene, frustum ).empty() );
        // Move it in front: the index must rebuild and now report it visible.
        mover->SetGlobalTransform ( Transform { Vector3 { 1.0f, 1.0f, 1.0f }, Quaternion {}, Vector3 { 0.0f, 50.0f, 0.0f } } );
        const std::vector<const Node*> visible = CullVisibleSet ( scene, frustum );
        ASSERT_EQ ( visible.size(), 1u );
        EXPECT_EQ ( visible.front(), mover );
    }

    TEST ( SceneCullVisible, MatchesBruteForceAfterRemoval )
    {
        Scene scene;
        AddPositioned ( scene, Vector3 { 0.0f, 50.0f, 0.0f } );
        Node* removable = AddPositioned ( scene, Vector3 { 10.0f, 50.0f, 5.0f } );
        AddPositioned ( scene, Vector3 { -8.0f, 50.0f, -4.0f } );
        const Frustum frustum = MakeCullFrustum();
        scene.Remove ( removable );
        EXPECT_EQ ( CullVisibleSet ( scene, frustum ), BruteForceVisible ( scene, frustum ) );
    }

    // ---- QueryAABB (octree-backed broad-phase for collision) ------------------

    TEST ( SceneQueryAABB, MatchesBruteForceMixedScene )
    {
        Scene scene;
        AddPositioned ( scene, Vector3 { 0.0f, 0.0f, 0.0f } );    // inside the box
        AddPositioned ( scene, Vector3 { 3.0f, 0.0f, 0.0f } );    // overlapping edge
        AddPositioned ( scene, Vector3 { 50.0f, 0.0f, 0.0f } );   // far away
        AddPositioned ( scene, Vector3 { 0.0f, 50.0f, 0.0f } );   // far above
        AddPositioned ( scene, Vector3 { -2.0f, -2.0f, -2.0f } ); // inside the box
        const AABB box { Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 4.0f, 4.0f, 4.0f } };
        EXPECT_EQ ( QueryAABBSet ( scene, box ), BruteForceOverlap ( scene, box ) );
    }

    TEST ( SceneQueryAABB, EmptySceneVisitsNothing )
    {
        Scene scene;
        size_t count = 0;
        scene.QueryAABB ( AABB { Vector3 {}, Vector3 { 1.0f, 1.0f, 1.0f } }, [&count] ( const Node& )
        {
            ++count;
        } );
        EXPECT_EQ ( count, 0u );
    }

    TEST ( SceneQueryAABB, RebuildsAfterNodeMoves )
    {
        Scene scene;
        Node* mover = AddPositioned ( scene, Vector3 { 50.0f, 0.0f, 0.0f } ); // outside
        const AABB box { Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 4.0f, 4.0f, 4.0f } };
        // Initially outside the query box.
        EXPECT_TRUE ( QueryAABBSet ( scene, box ).empty() );
        // Move it into the box: the index must rebuild and now report it.
        mover->SetGlobalTransform ( Transform { Vector3 { 1.0f, 1.0f, 1.0f }, Quaternion {}, Vector3 { 0.0f, 0.0f, 0.0f } } );
        const std::vector<const Node*> hits = QueryAABBSet ( scene, box );
        ASSERT_EQ ( hits.size(), 1u );
        EXPECT_EQ ( hits.front(), mover );
    }

    TEST ( SceneQueryAABB, MatchesBruteForceAfterRemoval )
    {
        Scene scene;
        AddPositioned ( scene, Vector3 { 0.0f, 0.0f, 0.0f } );
        Node* removable = AddPositioned ( scene, Vector3 { 2.0f, 0.0f, 0.0f } );
        AddPositioned ( scene, Vector3 { -2.0f, 1.0f, 0.0f } );
        const AABB box { Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 4.0f, 4.0f, 4.0f } };
        scene.Remove ( removable );
        EXPECT_EQ ( QueryAABBSet ( scene, box ), BruteForceOverlap ( scene, box ) );
    }
}
