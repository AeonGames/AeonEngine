/*
Copyright (C) 2015,2018,2019 Rodrigo Jose Hernandez Cordoba

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
        EXPECT_EQ ( serialized.substr ( 0, 8 ), "AEONSCE\n" );
    }

    TEST_F ( SceneTest, SerializeAsBinaryHasProperHeader )
    {
        std::string serialized = mScene.Serialize ( true );
        EXPECT_EQ ( serialized.substr ( 0, 7 ), "AEONSCE" );
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
}
