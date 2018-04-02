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

    TEST_F ( TreeTest, LocalTransformSyncsGlobalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree[0].SetLocalTransform ( transform );
        EXPECT_EQ ( mTree[0].GetLocalTransform(), mTree[0].GetGlobalTransform() );
    }

    TEST_F ( TreeTest, GlobalTransformSyncsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree[0].SetGlobalTransform ( transform );
        EXPECT_EQ ( mTree[0].GetLocalTransform(), mTree[0].GetGlobalTransform() );
    }

    TEST_F ( TreeTest, AppendSynchsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{2, 4, 6}};
        Tree::Node node;
        node.SetGlobalTransform ( transform );
        transform.SetTranslation ( {1, 2, 3} );
        mTree[0].SetLocalTransform ( transform );
        mTree[0].Append ( node );
        EXPECT_EQ ( transform, mTree[0][mTree[0].GetChildrenCount() - 1].GetLocalTransform() );
    }

    TEST_F ( TreeTest, LocalTransformSyncsGlobalTransformInChildNode )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree[0].SetLocalTransform ( transform );
        EXPECT_EQ ( mTree[0].GetLocalTransform(), mTree[0][0].GetGlobalTransform() );
    }

    TEST_F ( TreeTest, GlobalTransformSyncsGlobalTransformInChildNode )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree[0].SetGlobalTransform ( transform );
        EXPECT_EQ ( mTree[0].GetLocalTransform(), mTree[0][0].GetGlobalTransform() );
    }

    TEST_F ( TreeTest, LocalTransformLeavesLocalTransformInChildNodeUntoched )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree[0].SetLocalTransform ( transform );
        transform.SetTranslation ( {0, 0, 0} );
        EXPECT_EQ ( transform, mTree[0][0].GetLocalTransform() );
    }

    TEST_F ( TreeTest, GlobalTransformLeavesLocalTransformInChildNodeUntoched )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{1, 2, 3}};
        mTree[0].SetGlobalTransform ( transform );
        transform.SetTranslation ( {0, 0, 0} );
        EXPECT_EQ ( transform, mTree[0][0].GetLocalTransform() );
    }

    TEST_F ( TreeTest, DefaultIndexIsInvalid )
    {
        Tree::Node node;
        EXPECT_THROW ( node.GetIndex(), std::runtime_error );
    }

    TEST_F ( TreeTest, IndicesAreContiguous )
    {
        Tree::Node node{{}};
        node[0].Append ( {} );
        node[0].Append ( {} );
        node[0].Append ( {} );
        size_t count = node[0].GetChildrenCount();
        for ( size_t i = 0; i < count; ++i )
        {
            EXPECT_EQ ( node[0].GetChild ( i ).GetIndex(), i );
        }
    }

    TEST_F ( TreeTest, IndicesAreContiguousAfterRemovingFirstNode )
    {
        Tree::Node node {{}};
        node[0].Append ( {} );
        node[0].Append ( {} );
        node[0].Append ( {} );
        EXPECT_EQ ( node[0].GetChildrenCount(), 3u );
        auto& removedNode = node[0].GetChild ( 0 );
        node[0].Erase ( removedNode );
        for ( size_t i = 0; i < node[0].GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node[0].GetChild ( i ).GetIndex(), i );
        }
    }

    TEST_F ( TreeTest, IndicesAreContiguousAfterRemovingMiddleNode )
    {
        Tree::Node node {{}};
        node[0].Append ( {} );
        node[0].Append ( {} );
        node[0].Append ( {} );
        EXPECT_EQ ( node[0].GetChildrenCount(), 3u );
        auto& removedNode = node[0].GetChild ( 1 );
        node[0].Erase ( removedNode );
        for ( size_t i = 0; i < node[0].GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node[0].GetChild ( i ).GetIndex(), i );
        }
    }

    TEST_F ( TreeTest, IndicesAreContiguousAfterInsertingNodeOnFront )
    {
        Tree::Node node {{}};
        node[0].Append ( {} );
        node[0].Append ( {} );
        node[0].Insert ( 0, {} );
        EXPECT_EQ ( node[0].GetChildrenCount(), 3u );
        for ( size_t i = 0; i < node[0].GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node[0].GetChild ( i ).GetIndex(), i );
        }
        EXPECT_EQ ( &node[0][0], &node[0].GetChild ( 0 ) );
    }

    TEST_F ( TreeTest, IndicesAreContiguousAfterInsertingNodeAtMiddle )
    {
        Tree::Node node {{}};
        node[0].Append ( {} );
        node[0].Append ( {} );
        node[0].Insert ( 1, {} );
        EXPECT_EQ ( node[0].GetChildrenCount(), 3u );
        for ( size_t i = 0; i < node[0].GetChildrenCount(); ++i )
        {
            EXPECT_EQ ( node[0].GetChild ( i ).GetIndex(), i );
        }
        EXPECT_EQ ( &node[0][1], &node[0].GetChild ( 1 ) );
    }

    TEST_F ( TreeTest, SerializeAsTextHasProperHeader )
    {
        std::string serialized = mTree.Serialize ( false );
        EXPECT_EQ ( serialized.substr ( 0, 8 ), "AEONTRE\n" );
    }

    TEST_F ( TreeTest, SerializeAsBinaryHasProperHeader )
    {
        std::string serialized = mTree.Serialize ( true );
        EXPECT_EQ ( serialized.substr ( 0, 7 ), "AEONTRE" );
        EXPECT_EQ ( serialized[7], 0 );
    }
}
