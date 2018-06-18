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
        Scene mScene
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

    TEST_F ( SceneTest, LoopTraverseDFSPostOrderTraversesAllNodes )
    {
        std::vector < Scene::Node* > nodeVector;
        mScene.LoopTraverseDFSPostOrder ( [&nodeVector] ( Scene::Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }

    TEST_F ( SceneTest, RecursiveTraverseDFSPostOrderTraversesAllNodes )
    {
        std::vector < Scene::Node* > nodeVector;
        mScene.LoopTraverseDFSPostOrder ( [&nodeVector] ( Scene::Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }


    TEST_F ( SceneTest, LoopTraverseDFSPostOrderIsEquivalentToRecursiveTraverseDFS )
    {
        std::vector < Scene::Node* > loopNodeVector;
        std::vector < Scene::Node* > recurseNodeVector;
        mScene.LoopTraverseDFSPostOrder ( [&loopNodeVector] ( Scene::Node & aNode )
        {
            loopNodeVector.push_back ( &aNode );
        } );
        mScene.RecursiveTraverseDFSPostOrder ( [&recurseNodeVector] ( Scene::Node & aNode )
        {
            recurseNodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( loopNodeVector, recurseNodeVector );
    }

    TEST_F ( SceneTest, CallingLoopTraverseDFSPostOrderMoreThanOnceHasSameResult )
    {
        std::vector < Scene::Node* > nodeVectorA;
        std::vector < Scene::Node* > nodeVectorB;
        mScene.LoopTraverseDFSPostOrder ( [&nodeVectorA] ( Scene::Node & aNode )
        {
            nodeVectorA.push_back ( &aNode );
        } );
        mScene.LoopTraverseDFSPostOrder ( [&nodeVectorB] ( Scene::Node & aNode )
        {
            nodeVectorB.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVectorA, nodeVectorB );
    }

    TEST_F ( SceneTest, LoopTraverseDFSPreOrderTraversesAllNodes )
    {
        std::vector < Scene::Node* > nodeVector;
        mScene.LoopTraverseDFSPreOrder ( [&nodeVector] ( Scene::Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }

    TEST_F ( SceneTest, RecursiveTraverseDFSPreOrderTraversesAllNodes )
    {
        std::vector < Scene::Node* > nodeVector;
        mScene.LoopTraverseDFSPreOrder ( [&nodeVector] ( Scene::Node & aNode )
        {
            nodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVector.size(), 14u );
    }

    TEST_F ( SceneTest, LoopTraverseDFSPreOrderIsEquivalentToRecursiveTraverseDFS )
    {
        std::vector < Scene::Node* > loopNodeVector;
        std::vector < Scene::Node* > recurseNodeVector;
        mScene.LoopTraverseDFSPreOrder ( [&loopNodeVector] ( Scene::Node & aNode )
        {
            loopNodeVector.push_back ( &aNode );
        } );
        mScene.RecursiveTraverseDFSPreOrder ( [&recurseNodeVector] ( Scene::Node & aNode )
        {
            recurseNodeVector.push_back ( &aNode );
        } );
        EXPECT_EQ ( loopNodeVector, recurseNodeVector );
    }

    TEST_F ( SceneTest, CallingLoopTraverseDFSPreOrderMoreThanOnceHasSameResult )
    {
        std::vector < Scene::Node* > nodeVectorA;
        std::vector < Scene::Node* > nodeVectorB;
        mScene.LoopTraverseDFSPreOrder ( [&nodeVectorA] ( Scene::Node & aNode )
        {
            nodeVectorA.push_back ( &aNode );
        } );
        mScene.LoopTraverseDFSPreOrder ( [&nodeVectorB] ( Scene::Node & aNode )
        {
            nodeVectorB.push_back ( &aNode );
        } );
        EXPECT_EQ ( nodeVectorA, nodeVectorB );
    }

    TEST_F ( SceneTest, EraseWorks )
    {
        EXPECT_EQ ( mScene.GetChildrenCount(), 2u );
        mScene.Erase ( 0 );
        EXPECT_EQ ( mScene.GetChildrenCount(), 1u );
        mScene.Erase ( mScene[0] );
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

    TEST_F ( SceneTest, AppendSynchsLocalTransform )
    {
        Transform transform{Vector3{1, 1, 1}, Quaternion{1, 0, 0, 0}, Vector3{2, 4, 6}};
        Scene::Node node;
        node.SetGlobalTransform ( transform );
        transform.SetTranslation ( {1, 2, 3} );
        mScene[0].SetLocalTransform ( transform );
        mScene[0].Append ( node );
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
        Scene::Node node;
        EXPECT_THROW ( node.GetIndex(), std::runtime_error );
    }

    TEST_F ( SceneTest, IndicesAreContiguous )
    {
        Scene::Node node{{}};
        node[0].Append ( {} );
        node[0].Append ( {} );
        node[0].Append ( {} );
        size_t count = node[0].GetChildrenCount();
        for ( size_t i = 0; i < count; ++i )
        {
            EXPECT_EQ ( node[0].GetChild ( i ).GetIndex(), i );
        }
    }

    TEST_F ( SceneTest, IndicesAreContiguousAfterRemovingFirstNode )
    {
        Scene::Node node {{}};
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

    TEST_F ( SceneTest, IndicesAreContiguousAfterRemovingMiddleNode )
    {
        Scene::Node node {{}};
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

    TEST_F ( SceneTest, IndicesAreContiguousAfterInsertingNodeOnFront )
    {
        Scene::Node node {{}};
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

    TEST_F ( SceneTest, IndicesAreContiguousAfterInsertingNodeAtMiddle )
    {
        Scene::Node node {{}};
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
}
