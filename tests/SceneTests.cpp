/*
Copyright (C) 2015,2018 Rodrigo Jose Hernandez Cordoba

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
#include "gtest/gtest.h"
#include "aeongames/CRC.h"
#include "aeongames/Node.h"
#include "aeongames/Scene.h"
#include "aeongames/Memory.h"

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
            mScene.Add ( &a );
            mScene.Add ( &b );
            a.Add ( &c );
            a.Add ( &d );
            b.Add ( &e );
            b.Add ( &f );
            c.Add ( &g );
            c.Add ( &h );
            d.Add ( &i );
            d.Add ( &j );
            e.Add ( &k );
            e.Add ( &l );
            f.Add ( &m );
            f.Add ( &n );
            a.SetName ( "a" );
            b.SetName ( "b" );
            c.SetName ( "c" );
            d.SetName ( "d" );
            e.SetName ( "e" );
            f.SetName ( "f" );
            g.SetName ( "g" );
            h.SetName ( "h" );
            i.SetName ( "i" );
            j.SetName ( "j" );
            k.SetName ( "k" );
            l.SetName ( "l" );
            m.SetName ( "m" );
            n.SetName ( "n" );
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
        Node a{};
        Node b{};
        Node c{};
        Node d{};
        Node e{};
        Node f{};
        Node g{};
        Node h{};
        Node i{};
        Node j{};
        Node k{};
        Node l{};
        Node m{};
        Node n{};
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
        Node node;
        node.SetGlobalTransform ( transform );
        transform.SetTranslation ( {1, 2, 3} );
        mScene[0].SetLocalTransform ( transform );
        mScene[0].Add ( &node );
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
        Node node1;
        Node node2;
        Node node3;
        Node node;
        node.Add ( &node1 );
        node.Add ( &node2 );
        node.Add ( &node3 );
        size_t count = node.GetChildrenCount();
        for ( size_t i = 0; i < count; ++i )
        {
            EXPECT_EQ ( node.GetChild ( i )->GetIndex(), i );
        }
    }
    TEST_F ( SceneTest, IndicesAreContiguousAfterRemovingFirstNode )
    {
        Node node1;
        Node node2;
        Node node3;
        Node node;
        node.Add ( &node1 );
        node.Add ( &node2 );
        node.Add ( &node3 );
        EXPECT_EQ ( node.GetChildrenCount(), 3u );
        Node* removedNode = node.GetChild ( 0 );
        node.Remove ( removedNode );
        for ( size_t i = 0; i < node.GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node.GetChild ( i )->GetIndex(), i );
        }
    }
    TEST_F ( SceneTest, IndicesAreContiguousAfterRemovingMiddleNode )
    {
        Node node1;
        Node node2;
        Node node3;
        Node node;
        node.Add ( &node1 );
        node.Add ( &node2 );
        node.Add ( &node3 );
        EXPECT_EQ ( node.GetChildrenCount(), 3u );
        Node* removedNode = node.GetChild ( 1 );
        node[0].Remove ( removedNode );
        for ( size_t i = 0; i < node.GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node.GetChild ( i )->GetIndex(), i );
        }
    }
    TEST_F ( SceneTest, IndicesAreContiguousAfterInsertingNodeOnFront )
    {
        Node node1;
        Node node2;
        Node node3;
        Node node;
        node.Add ( &node1 );
        node.Add ( &node2 );
        node.Insert ( 0, &node3 );
        EXPECT_EQ ( node.GetChildrenCount(), 3u );
        for ( size_t i = 0; i < node.GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node.GetChild ( i )->GetIndex(), i );
        }
        EXPECT_EQ ( &node[0], node.GetChild ( 0 ) );
    }
    TEST_F ( SceneTest, IndicesAreContiguousAfterInsertingNodeAtMiddle )
    {
        Node node1;
        Node node2;
        Node node3;
        Node node;
        node.Add ( &node1 );
        node.Add ( &node2 );
        node.Insert ( 0, &node3 );
        EXPECT_EQ ( node.GetChildrenCount(), 3u );
        for ( size_t i = 0; i < node.GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node.GetChild ( i )->GetIndex(), i );
        }
        EXPECT_EQ ( &node[1], node.GetChild ( 1 ) );
    }
    TEST_F ( SceneTest, SerializeAsTextHasProperHeader )
    {
        std::string serialized = mScene.Serialize ( false );
        EXPECT_EQ ( serialized.substr ( 0, 8 ), "AEONSCE\n" );
    }

    TEST_F ( SceneTest, SerializeAsBinaryHasProperHeader )
    {
        std::string serialized = mScene.Serialize ( true );
        EXPECT_EQ ( serialized.substr ( 0, 7 ), "AEONSCE" );
        EXPECT_EQ ( serialized[7], 0 );
    }
    TEST ( Scene, StoreDispose )
    {
        Scene scene;
        Node* node = scene.StoreNode ( std::make_unique<Node>() );
        EXPECT_NE ( node, nullptr );
        std::unique_ptr<Node> unique_ptr_node = scene.DisposeNode ( node );
        EXPECT_EQ ( unique_ptr_node.get(), node );
    }
    TEST_F ( SceneTest, SerializeDeserializeText )
    {
        std::string serialized = mScene.Serialize ( false );
        Scene scene;
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
}
