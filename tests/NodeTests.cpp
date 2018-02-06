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
#include "aeongames/Node.h"
#include "aeongames/Memory.h"

using namespace ::testing;
namespace AeonGames
{
    class NodeTest : public ::testing::Test
    {
    protected:
        NodeTest()
        {
        }
        virtual ~NodeTest()
        {
        }
        virtual void SetUp()
        {
            root = std::make_shared<Node>();
            a = std::make_shared<Node>();
            b = std::make_shared<Node>();
            c = std::make_shared<Node>();
            d = std::make_shared<Node>();
            e = std::make_shared<Node>();
            f = std::make_shared<Node>();
            g = std::make_shared<Node>();
            h = std::make_shared<Node>();
            i = std::make_shared<Node>();
            j = std::make_shared<Node>();
            k = std::make_shared<Node>();
            l = std::make_shared<Node>();
            m = std::make_shared<Node>();
            n = std::make_shared<Node>();
            root->SetName ( "root" );
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

            /*
            This tree should look like this:
                                                    root
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

            root->AddNode ( a );
            root->AddNode ( b );
            a->AddNode ( c );
            a->AddNode ( d );
            b->AddNode ( e );
            b->AddNode ( f );
            c->AddNode ( g );
            c->AddNode ( h );
            d->AddNode ( i );
            d->AddNode ( j );
            e->AddNode ( k );
            e->AddNode ( l );
            f->AddNode ( m );
            f->AddNode ( n );
        }
        virtual void TearDown()
        {
        }
        std::shared_ptr<Node> root;
        std::shared_ptr<Node> a;
        std::shared_ptr<Node> b;
        std::shared_ptr<Node> c;
        std::shared_ptr<Node> d;
        std::shared_ptr<Node> e;
        std::shared_ptr<Node> f;
        std::shared_ptr<Node> g;
        std::shared_ptr<Node> h;
        std::shared_ptr<Node> i;
        std::shared_ptr<Node> j;
        std::shared_ptr<Node> k;
        std::shared_ptr<Node> l;
        std::shared_ptr<Node> m;
        std::shared_ptr<Node> n;
    };
    TEST_F ( NodeTest, LoopTraverseDFSPostOrderTraversesAllNodes )
    {
        std::vector < Node* > nodeVector;
        root->LoopTraverseDFSPostOrder ( [&nodeVector] ( Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 15u );
    }

    TEST_F ( NodeTest, RecursiveTraverseDFSPostOrderTraversesAllNodes )
    {
        std::vector < Node* > nodeVector;
        root->LoopTraverseDFSPostOrder ( [&nodeVector] ( Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 15u );
    }

    TEST_F ( NodeTest, LoopTraverseDFSPostOrderIsEquivalentToRecursiveTraverseDFS )
    {
        std::vector < Node* > loopNodeVector;
        std::vector < Node* > recurseNodeVector;
        root->LoopTraverseDFSPostOrder ( [&loopNodeVector] ( Node & aNode )
        {
            loopNodeVector.push_back ( &aNode );
        } );
        root->RecursiveTraverseDFSPostOrder ( [&recurseNodeVector] ( Node & aNode )
        {
            recurseNodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( loopNodeVector, recurseNodeVector );
    }

    TEST_F ( NodeTest, CallingLoopTraverseDFSPostOrderMoreThanOnceHasSameResult )
    {
        std::vector < Node* > nodeVectorA;
        std::vector < Node* > nodeVectorB;
        root->LoopTraverseDFSPostOrder ( [&nodeVectorA] ( Node & aNode )
        {
            nodeVectorA.push_back ( &aNode );
        } );
        root->LoopTraverseDFSPostOrder ( [&nodeVectorB] ( Node & aNode )
        {
            nodeVectorB.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVectorA, nodeVectorB );
    }

    TEST_F ( NodeTest, LoopTraverseDFSPreOrderTraversesAllNodes )
    {
        std::vector < Node* > nodeVector;
        root->LoopTraverseDFSPreOrder ( [&nodeVector] ( Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 15u );
    }

    TEST_F ( NodeTest, RecursiveTraverseDFSPreOrderTraversesAllNodes )
    {
        std::vector < Node* > nodeVector;
        root->LoopTraverseDFSPreOrder ( [&nodeVector] ( Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 15u );
    }

    TEST_F ( NodeTest, LoopTraverseDFSPreOrderIsEquivalentToRecursiveTraverseDFS )
    {
        std::vector < Node* > loopNodeVector;
        std::vector < Node* > recurseNodeVector;
        root->LoopTraverseDFSPreOrder ( [&loopNodeVector] ( Node & aNode )
        {
            loopNodeVector.push_back ( &aNode );
        } );
        root->RecursiveTraverseDFSPreOrder ( [&recurseNodeVector] ( Node & aNode )
        {
            recurseNodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( loopNodeVector, recurseNodeVector );
    }

    TEST_F ( NodeTest, CallingLoopTraverseDFSPreOrderMoreThanOnceHasSameResult )
    {
        std::vector < Node* > nodeVectorA;
        std::vector < Node* > nodeVectorB;
        root->LoopTraverseDFSPreOrder ( [&nodeVectorA] ( Node & aNode )
        {
            nodeVectorA.push_back ( &aNode );
        } );
        root->LoopTraverseDFSPreOrder ( [&nodeVectorB] ( Node & aNode )
        {
            nodeVectorB.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVectorA, nodeVectorB );
    }

    TEST_F ( NodeTest, RemoveNodeWorks )
    {
        EXPECT_EQ ( root->RemoveNode ( a ), true );
    }

    TEST_F ( NodeTest, LocalTransformSyncsGlobalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        root->SetLocalTransform ( transform );
        EXPECT_EQ ( root->GetLocalTransform(), root->GetGlobalTransform() );
    }

    TEST_F ( NodeTest, GlobalTransformSyncsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        root->SetGlobalTransform ( transform );
        EXPECT_EQ ( root->GetLocalTransform(), root->GetGlobalTransform() );
    }

    TEST_F ( NodeTest, AddNodeSynchsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{2, 4, 6}};
        root->RemoveNode ( a );
        a->SetGlobalTransform ( transform );
        transform.SetTranslation ( {1, 2, 3} );
        root->SetLocalTransform ( transform );
        root->AddNode ( a );
        EXPECT_EQ ( transform, a->GetLocalTransform() );
    }

    TEST_F ( NodeTest, RemoveNodeSynchsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{}, Vector3{1, 2, 3}};
        root->SetLocalTransform ( transform );
        a->SetLocalTransform ( transform );
        transform.SetTranslation ( {2, 4, 6} );
        EXPECT_EQ ( transform, a->GetGlobalTransform() );
        root->RemoveNode ( a );
        EXPECT_EQ ( transform, a->GetLocalTransform() );
    }

    TEST_F ( NodeTest, LocalTransformSyncsGlobalTransformInChildNode )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        root->SetLocalTransform ( transform );
        EXPECT_EQ ( root->GetLocalTransform(), a->GetGlobalTransform() );
    }

    TEST_F ( NodeTest, GlobalTransformSyncsGlobalTransformInChildNode )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        root->SetGlobalTransform ( transform );
        EXPECT_EQ ( root->GetLocalTransform(), a->GetGlobalTransform() );
    }

    TEST_F ( NodeTest, LocalTransformLeavesLocalTransformInChildNodeUntoched )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        root->SetLocalTransform ( transform );
        transform.SetTranslation ( {0, 0, 0} );
        EXPECT_EQ ( transform, a->GetLocalTransform() );
    }

    TEST_F ( NodeTest, GlobalTransformLeavesLocalTransformInChildNodeUntoched )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        root->SetGlobalTransform ( transform );
        transform.SetTranslation ( {0, 0, 0} );
        EXPECT_EQ ( transform, a->GetLocalTransform() );
    }

    TEST_F ( NodeTest, DefaultIndexIsInvalid )
    {
        std::shared_ptr<Node> node ( std::make_shared<Node>() );
        EXPECT_EQ ( node->GetIndex(), Node::kInvalidIndex );
    }

    TEST_F ( NodeTest, IndicesAreContiguous )
    {
        std::shared_ptr<Node> node ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        size_t count = node->GetChildrenCount();
        for ( size_t i = 0; i < count; ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
    }

    TEST_F ( NodeTest, IndicesAreContiguousAfterRemovingFirstNode )
    {
        std::shared_ptr<Node> node ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        EXPECT_EQ ( node->GetChildrenCount(), 3u );
        std::shared_ptr<Node> removedNode = node->GetChild ( 0 );
        node->RemoveNode ( removedNode );
        for ( size_t i = 0; i < node->GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
    }

    TEST_F ( NodeTest, IndicesAreContiguousAfterRemovingMiddleNode )
    {
        std::shared_ptr<Node> node ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        EXPECT_EQ ( node->GetChildrenCount(), 3u );
        std::shared_ptr<Node> removedNode = node->GetChild ( 1 );
        node->RemoveNode ( removedNode );
        for ( size_t i = 0; i < node->GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
    }

    TEST_F ( NodeTest, IndicesAreContiguousAfterInsertingNodeOnFront )
    {
        std::shared_ptr<Node> node ( std::make_shared<Node>() );
        std::shared_ptr<Node> inserted;
        node->AddNode ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        node->InsertNode ( 0, inserted = std::make_shared<Node>() );
        EXPECT_EQ ( node->GetChildrenCount(), 3u );
        for ( size_t i = 0; i < node->GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
        EXPECT_EQ ( inserted, node->GetChild ( 0 ) );
    }

    TEST_F ( NodeTest, IndicesAreContiguousAfterInsertingNodeAtMiddle )
    {
        std::shared_ptr<Node> node ( std::make_shared<Node>() );
        std::shared_ptr<Node> inserted = nullptr;
        node->AddNode ( std::make_shared<Node>() );
        node->AddNode ( std::make_shared<Node>() );
        node->InsertNode ( 1, inserted = std::make_shared<Node>() );
        EXPECT_EQ ( node->GetChildrenCount(), 3u );
        for ( size_t i = 0; i < node->GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node->GetChild ( i )->GetIndex(), i );
        }
        EXPECT_EQ ( inserted, node->GetChild ( 1 ) );
    }
}
