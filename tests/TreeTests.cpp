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
#include "aeongames/Tree.h"
#include "aeongames/Memory.h"

using namespace ::testing;
namespace AeonGames
{
    class TreeTest : public ::testing::Test
    {
    protected:
        TreeTest()
        {
        }
        virtual ~TreeTest()
        {
        }
        virtual void SetUp()
        {
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
        Tree mTree
        {
            {
                // a
                {
                    // c
                    {
                        //g
                    },
                    {
                        // h
                    },
                },
                {
                    // d
                    {
                        //i
                    },
                    {
                        // j
                    },
                }
            },
            {
                // b
                {
                    // e
                    {
                        //k
                    },
                    {
                        // l
                    },
                },
                {
                    // f
                    {
                        //m
                    },
                    {
                        // n
                    },
                }
            }
        };
    };

    TEST_F ( TreeTest, LoopTraverseDFSPostOrderTraversesAllNodes )
    {
        std::vector < Tree::Node* > nodeVector;
        mTree.LoopTraverseDFSPostOrder ( [&nodeVector] ( Tree::Node & aNode )
        {
            std::cout << aNode.GetChildrenCount() << std::endl;
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }

    TEST_F ( TreeTest, RecursiveTraverseDFSPostOrderTraversesAllNodes )
    {
        std::vector < Tree::Node* > nodeVector;
        mTree.LoopTraverseDFSPostOrder ( [&nodeVector] ( Tree::Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }


    TEST_F ( TreeTest, LoopTraverseDFSPostOrderIsEquivalentToRecursiveTraverseDFS )
    {
        std::vector < Tree::Node* > loopNodeVector;
        std::vector < Tree::Node* > recurseNodeVector;
        mTree.LoopTraverseDFSPostOrder ( [&loopNodeVector] ( Tree::Node & aNode )
        {
            loopNodeVector.push_back ( &aNode );
        } );
        mTree.RecursiveTraverseDFSPostOrder ( [&recurseNodeVector] ( Tree::Node & aNode )
        {
            recurseNodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( loopNodeVector, recurseNodeVector );
    }

    TEST_F ( TreeTest, CallingLoopTraverseDFSPostOrderMoreThanOnceHasSameResult )
    {
        std::vector < Tree::Node* > nodeVectorA;
        std::vector < Tree::Node* > nodeVectorB;
        mTree.LoopTraverseDFSPostOrder ( [&nodeVectorA] ( Tree::Node & aNode )
        {
            nodeVectorA.push_back ( &aNode );
        } );
        mTree.LoopTraverseDFSPostOrder ( [&nodeVectorB] ( Tree::Node & aNode )
        {
            nodeVectorB.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVectorA, nodeVectorB );
    }

    TEST_F ( TreeTest, LoopTraverseDFSPreOrderTraversesAllNodes )
    {
        std::vector < Tree::Node* > nodeVector;
        mTree.LoopTraverseDFSPreOrder ( [&nodeVector] ( Tree::Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }

    TEST_F ( TreeTest, RecursiveTraverseDFSPreOrderTraversesAllNodes )
    {
        std::vector < Tree::Node* > nodeVector;
        mTree.LoopTraverseDFSPreOrder ( [&nodeVector] ( Tree::Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }

    TEST_F ( TreeTest, LoopTraverseDFSPreOrderIsEquivalentToRecursiveTraverseDFS )
    {
        std::vector < Tree::Node* > loopNodeVector;
        std::vector < Tree::Node* > recurseNodeVector;
        mTree.LoopTraverseDFSPreOrder ( [&loopNodeVector] ( Tree::Node & aNode )
        {
            loopNodeVector.push_back ( &aNode );
        } );
        mTree.RecursiveTraverseDFSPreOrder ( [&recurseNodeVector] ( Tree::Node & aNode )
        {
            recurseNodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( loopNodeVector, recurseNodeVector );
    }

    TEST_F ( TreeTest, CallingLoopTraverseDFSPreOrderMoreThanOnceHasSameResult )
    {
        std::vector < Tree::Node* > nodeVectorA;
        std::vector < Tree::Node* > nodeVectorB;
        mTree.LoopTraverseDFSPreOrder ( [&nodeVectorA] ( Tree::Node & aNode )
        {
            nodeVectorA.push_back ( &aNode );
        } );
        mTree.LoopTraverseDFSPreOrder ( [&nodeVectorB] ( Tree::Node & aNode )
        {
            nodeVectorB.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVectorA, nodeVectorB );
    }

    TEST_F ( TreeTest, EraseWorks )
    {
        EXPECT_EQ ( mTree.GetChildrenCount(), 2u );
        mTree.Erase ( 0 );
        EXPECT_EQ ( mTree.GetChildrenCount(), 1u );
        mTree.Erase ( mTree[0] );
        EXPECT_EQ ( mTree.GetChildrenCount(), 0u );
    }

#if 0
    TEST_F ( TreeTest, LocalTransformSyncsGlobalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree.SetLocalTransform ( transform );
        EXPECT_EQ ( mTree.GetLocalTransform(), mTree.GetGlobalTransform() );
    }

    TEST_F ( TreeTest, GlobalTransformSyncsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree.SetGlobalTransform ( transform );
        EXPECT_EQ ( mTree.GetLocalTransform(), mTree.GetGlobalTransform() );
    }

    TEST_F ( TreeTest, AddNodeSynchsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{2, 4, 6}};
        mTree.RemoveNode ( a );
        a->SetGlobalTransform ( transform );
        transform.SetTranslation ( {1, 2, 3} );
        mTree.SetLocalTransform ( transform );
        mTree.AddNode ( a );
        EXPECT_EQ ( transform, a->GetLocalTransform() );
    }

    TEST_F ( TreeTest, RemoveNodeSynchsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{}, Vector3{1, 2, 3}};
        mTree.SetLocalTransform ( transform );
        a->SetLocalTransform ( transform );
        transform.SetTranslation ( {2, 4, 6} );
        EXPECT_EQ ( transform, a->GetGlobalTransform() );
        mTree.RemoveNode ( a );
        EXPECT_EQ ( transform, a->GetLocalTransform() );
    }

    TEST_F ( TreeTest, LocalTransformSyncsGlobalTransformInChildNode )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree.SetLocalTransform ( transform );
        EXPECT_EQ ( mTree.GetLocalTransform(), a->GetGlobalTransform() );
    }

    TEST_F ( TreeTest, GlobalTransformSyncsGlobalTransformInChildNode )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree.SetGlobalTransform ( transform );
        EXPECT_EQ ( mTree.GetLocalTransform(), a->GetGlobalTransform() );
    }

    TEST_F ( TreeTest, LocalTransformLeavesLocalTransformInChildNodeUntoched )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree.SetLocalTransform ( transform );
        transform.SetTranslation ( {0, 0, 0} );
        EXPECT_EQ ( transform, a->GetLocalTransform() );
    }

    TEST_F ( TreeTest, GlobalTransformLeavesLocalTransformInChildNodeUntoched )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree.SetGlobalTransform ( transform );
        transform.SetTranslation ( {0, 0, 0} );
        EXPECT_EQ ( transform, a->GetLocalTransform() );
    }

    TEST_F ( TreeTest, DefaultIndexIsInvalid )
    {
        std::shared_ptr<Node> node ( std::make_shared<Node>() );
        EXPECT_THROW ( node->GetIndex(), std::runtime_error );
    }

    TEST_F ( TreeTest, IndicesAreContiguous )
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

    TEST_F ( TreeTest, IndicesAreContiguousAfterRemovingFirstNode )
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

    TEST_F ( TreeTest, IndicesAreContiguousAfterRemovingMiddleNode )
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

    TEST_F ( TreeTest, IndicesAreContiguousAfterInsertingNodeOnFront )
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

    TEST_F ( TreeTest, IndicesAreContiguousAfterInsertingNodeAtMiddle )
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
#endif
}
