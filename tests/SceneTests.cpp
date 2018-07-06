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
    TEST ( Node, GetLocalTransformProperty )
    {
        Node node;
        float result[10] {};
        float value[10] {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
        const Node::Property& local_transform_descriptor = node.GetProperties() [ 0 ].get();
        EXPECT_EQ ( local_transform_descriptor.GetDisplayName(), "Local Transform" );
        EXPECT_EQ ( local_transform_descriptor.GetName(), "LocalTransform" );
        EXPECT_EQ ( local_transform_descriptor.GetId(), "LocalTransform"_crc32 );
        EXPECT_EQ ( local_transform_descriptor.GetFormat(), "10f" );
        local_transform_descriptor.Set ( &node, value );
        local_transform_descriptor.Get ( &node, result );
        for ( size_t i = 0; i < 10; ++i )
        {
            EXPECT_EQ ( result[i] , static_cast<float> ( i ) );
        }
    }
    TEST ( Node, GetLocalTransformScaleProperty )
    {
        Node node;
        float result[3] {};
        float value[3] {0.0f, 1.0f, 2.0f};
        const Node::Property& local_transform_scale_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [0];
        EXPECT_EQ ( local_transform_scale_descriptor.GetDisplayName(), "Scale" );
        EXPECT_EQ ( local_transform_scale_descriptor.GetName(), "LocalScale" );
        EXPECT_EQ ( local_transform_scale_descriptor.GetId(), "LocalScale"_crc32 );
        EXPECT_EQ ( local_transform_scale_descriptor.GetFormat(), "3f" );
        local_transform_scale_descriptor.Set ( &node, value );
        local_transform_scale_descriptor.Get ( &node, result );
        for ( size_t i = 0; i < 3; ++i )
        {
            EXPECT_EQ ( result[i] , static_cast<float> ( i ) );
        }
    }
    TEST ( Node, GetLocalTransformScaleXProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_scale_x_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [0].GetSubProperties() [0];
        EXPECT_EQ ( local_transform_scale_x_descriptor.GetDisplayName(), "X" );
        EXPECT_EQ ( local_transform_scale_x_descriptor.GetName(), "LocalScaleX" );
        EXPECT_EQ ( local_transform_scale_x_descriptor.GetId(), "LocalScaleX"_crc32 );
        EXPECT_EQ ( local_transform_scale_x_descriptor.GetFormat(), "f" );
        local_transform_scale_x_descriptor.Set ( &node, &value );
        local_transform_scale_x_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalTransformScaleYProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_scale_y_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [0].GetSubProperties() [1];
        EXPECT_EQ ( local_transform_scale_y_descriptor.GetDisplayName(), "Y" );
        EXPECT_EQ ( local_transform_scale_y_descriptor.GetName(), "LocalScaleY" );
        EXPECT_EQ ( local_transform_scale_y_descriptor.GetId(), "LocalScaleY"_crc32 );
        EXPECT_EQ ( local_transform_scale_y_descriptor.GetFormat(), "f" );
        local_transform_scale_y_descriptor.Set ( &node, &value );
        local_transform_scale_y_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalTransformScaleZProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_scale_z_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [0].GetSubProperties() [2];
        EXPECT_EQ ( local_transform_scale_z_descriptor.GetDisplayName(), "Z" );
        EXPECT_EQ ( local_transform_scale_z_descriptor.GetName(), "LocalScaleZ" );
        EXPECT_EQ ( local_transform_scale_z_descriptor.GetId(), "LocalScaleZ"_crc32 );
        EXPECT_EQ ( local_transform_scale_z_descriptor.GetFormat(), "f" );
        local_transform_scale_z_descriptor.Set ( &node, &value );
        local_transform_scale_z_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalTransformRotationWProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_rotation_x_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [1].GetSubProperties() [0];
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetDisplayName(), "W" );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetName(), "LocalRotationW" );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetId(), "LocalRotationW"_crc32 );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetFormat(), "f" );
        local_transform_rotation_x_descriptor.Set ( &node, &value );
        local_transform_rotation_x_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalTransformRotationXProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_rotation_x_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [1].GetSubProperties() [1];
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetDisplayName(), "X" );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetName(), "LocalRotationX" );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetId(), "LocalRotationX"_crc32 );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetFormat(), "f" );
        local_transform_rotation_x_descriptor.Set ( &node, &value );
        local_transform_rotation_x_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalTransformRotationYProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_rotation_y_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [1].GetSubProperties() [2];
        EXPECT_EQ ( local_transform_rotation_y_descriptor.GetDisplayName(), "Y" );
        EXPECT_EQ ( local_transform_rotation_y_descriptor.GetName(), "LocalRotationY" );
        EXPECT_EQ ( local_transform_rotation_y_descriptor.GetId(), "LocalRotationY"_crc32 );
        EXPECT_EQ ( local_transform_rotation_y_descriptor.GetFormat(), "f" );
        local_transform_rotation_y_descriptor.Set ( &node, &value );
        local_transform_rotation_y_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalTransformRotationZProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_rotation_z_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [1].GetSubProperties() [3];
        EXPECT_EQ ( local_transform_rotation_z_descriptor.GetDisplayName(), "Z" );
        EXPECT_EQ ( local_transform_rotation_z_descriptor.GetName(), "LocalRotationZ" );
        EXPECT_EQ ( local_transform_rotation_z_descriptor.GetId(), "LocalRotationZ"_crc32 );
        EXPECT_EQ ( local_transform_rotation_z_descriptor.GetFormat(), "f" );
        local_transform_rotation_z_descriptor.Set ( &node, &value );
        local_transform_rotation_z_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalTransformTranslationXProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_translation_x_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [2].GetSubProperties() [0];
        EXPECT_EQ ( local_transform_translation_x_descriptor.GetDisplayName(), "X" );
        EXPECT_EQ ( local_transform_translation_x_descriptor.GetName(), "LocalTranslationX" );
        EXPECT_EQ ( local_transform_translation_x_descriptor.GetId(), "LocalTranslationX"_crc32 );
        EXPECT_EQ ( local_transform_translation_x_descriptor.GetFormat(), "f" );
        local_transform_translation_x_descriptor.Set ( &node, &value );
        local_transform_translation_x_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalTransformTranslationYProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_translation_y_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [2].GetSubProperties() [1];
        EXPECT_EQ ( local_transform_translation_y_descriptor.GetDisplayName(), "Y" );
        EXPECT_EQ ( local_transform_translation_y_descriptor.GetName(), "LocalTranslationY" );
        EXPECT_EQ ( local_transform_translation_y_descriptor.GetId(), "LocalTranslationY"_crc32 );
        EXPECT_EQ ( local_transform_translation_y_descriptor.GetFormat(), "f" );
        local_transform_translation_y_descriptor.Set ( &node, &value );
        local_transform_translation_y_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalTransformTranslationZProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_translation_z_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [2].GetSubProperties() [2];
        EXPECT_EQ ( local_transform_translation_z_descriptor.GetDisplayName(), "Z" );
        EXPECT_EQ ( local_transform_translation_z_descriptor.GetName(), "LocalTranslationZ" );
        EXPECT_EQ ( local_transform_translation_z_descriptor.GetId(), "LocalTranslationZ"_crc32 );
        EXPECT_EQ ( local_transform_translation_z_descriptor.GetFormat(), "f" );
        local_transform_translation_z_descriptor.Set ( &node, &value );
        local_transform_translation_z_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetLocalRotationTranslationProperty )
    {
        Node node;
        float result[4] {};
        float value[4] {0.0f, 1.0f, 2.0f, 3.0f};
        const Node::Property& local_transform_rotation_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [1];
        EXPECT_EQ ( local_transform_rotation_descriptor.GetDisplayName(), "Rotation" );
        EXPECT_EQ ( local_transform_rotation_descriptor.GetName(), "LocalRotation" );
        EXPECT_EQ ( local_transform_rotation_descriptor.GetId(), "LocalRotation"_crc32 );
        EXPECT_EQ ( local_transform_rotation_descriptor.GetFormat(), "4f" );
        local_transform_rotation_descriptor.Set ( &node, value );
        local_transform_rotation_descriptor.Get ( &node, result );
        for ( size_t i = 0; i < 4; ++i )
        {
            EXPECT_EQ ( result[i] , static_cast<float> ( i ) );
        }
    }
    TEST ( Node, GetLocalTransformTranslationProperty )
    {
        Node node;
        float result[3] {};
        float value[3] {0.0f, 1.0f, 2.0f};
        const Node::Property& local_transform_translation_descriptor = node.GetProperties() [ 0 ].get().GetSubProperties() [2];
        EXPECT_EQ ( local_transform_translation_descriptor.GetDisplayName(), "Translation" );
        EXPECT_EQ ( local_transform_translation_descriptor.GetName(), "LocalTranslation" );
        EXPECT_EQ ( local_transform_translation_descriptor.GetId(), "LocalTranslation"_crc32 );
        EXPECT_EQ ( local_transform_translation_descriptor.GetFormat(), "3f" );
        local_transform_translation_descriptor.Set ( &node, value );
        local_transform_translation_descriptor.Get ( &node, result );
        for ( size_t i = 0; i < 3; ++i )
        {
            EXPECT_EQ ( result[i] , static_cast<float> ( i ) );
        }
    }
    TEST ( Node, GetGlobalTransformProperty )
    {
        Node node;
        float result[10] {};
        float value[10] {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
        const Node::Property& global_transform_descriptor = node.GetProperties() [ 1 ].get();
        EXPECT_EQ ( global_transform_descriptor.GetDisplayName(), "Global Transform" );
        EXPECT_EQ ( global_transform_descriptor.GetName(), "GlobalTransform" );
        EXPECT_EQ ( global_transform_descriptor.GetId(), "GlobalTransform"_crc32 );
        EXPECT_EQ ( global_transform_descriptor.GetFormat(), "10f" );
        global_transform_descriptor.Set ( &node, value );
        global_transform_descriptor.Get ( &node, result );
        for ( size_t i = 0; i < 10; ++i )
        {
            EXPECT_EQ ( result[i] , static_cast<float> ( i ) );
        }
    }
    TEST ( Node, GetGlobalTransformScaleProperty )
    {
        Node node;
        float result[3] {};
        float value[3] {0.0f, 1.0f, 2.0f};
        const Node::Property& global_transform_scale_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [0];
        EXPECT_EQ ( global_transform_scale_descriptor.GetDisplayName(), "Scale" );
        EXPECT_EQ ( global_transform_scale_descriptor.GetName(), "GlobalScale" );
        EXPECT_EQ ( global_transform_scale_descriptor.GetId(), "GlobalScale"_crc32 );
        EXPECT_EQ ( global_transform_scale_descriptor.GetFormat(), "3f" );
        global_transform_scale_descriptor.Set ( &node, value );
        global_transform_scale_descriptor.Get ( &node, result );
        for ( size_t i = 0; i < 3; ++i )
        {
            EXPECT_EQ ( result[i] , static_cast<float> ( i ) );
        }
    }
    TEST ( Node, GetGlobalRotationTranslationProperty )
    {
        Node node;
        float result[4] {};
        float value[4] {0.0f, 1.0f, 2.0f, 3.0f};
        const Node::Property& global_transform_rotation_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [1];
        EXPECT_EQ ( global_transform_rotation_descriptor.GetDisplayName(), "Rotation" );
        EXPECT_EQ ( global_transform_rotation_descriptor.GetName(), "GlobalRotation" );
        EXPECT_EQ ( global_transform_rotation_descriptor.GetId(), "GlobalRotation"_crc32 );
        EXPECT_EQ ( global_transform_rotation_descriptor.GetFormat(), "4f" );
        global_transform_rotation_descriptor.Set ( &node, value );
        global_transform_rotation_descriptor.Get ( &node, result );
        for ( size_t i = 0; i < 4; ++i )
        {
            EXPECT_EQ ( result[i] , static_cast<float> ( i ) );
        }
    }
    TEST ( Node, GetGlobalTransformTranslationProperty )
    {
        Node node;
        float result[3] {};
        float value[3] {0.0f, 1.0f, 2.0f};
        const Node::Property& global_transform_translation_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [2];
        EXPECT_EQ ( global_transform_translation_descriptor.GetDisplayName(), "Translation" );
        EXPECT_EQ ( global_transform_translation_descriptor.GetName(), "GlobalTranslation" );
        EXPECT_EQ ( global_transform_translation_descriptor.GetId(), "GlobalTranslation"_crc32 );
        EXPECT_EQ ( global_transform_translation_descriptor.GetFormat(), "3f" );
        global_transform_translation_descriptor.Set ( &node, value );
        global_transform_translation_descriptor.Get ( &node, result );
        for ( size_t i = 0; i < 3; ++i )
        {
            EXPECT_EQ ( result[i] , static_cast<float> ( i ) );
        }
    }
    TEST ( Node, GetGlobalTransformScaleXProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_scale_x_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [0].GetSubProperties() [0];
        EXPECT_EQ ( local_transform_scale_x_descriptor.GetDisplayName(), "X" );
        EXPECT_EQ ( local_transform_scale_x_descriptor.GetName(), "GlobalScaleX" );
        EXPECT_EQ ( local_transform_scale_x_descriptor.GetId(), "GlobalScaleX"_crc32 );
        EXPECT_EQ ( local_transform_scale_x_descriptor.GetFormat(), "f" );
        local_transform_scale_x_descriptor.Set ( &node, &value );
        local_transform_scale_x_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetGlobalTransformScaleYProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_scale_y_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [0].GetSubProperties() [1];
        EXPECT_EQ ( local_transform_scale_y_descriptor.GetDisplayName(), "Y" );
        EXPECT_EQ ( local_transform_scale_y_descriptor.GetName(), "GlobalScaleY" );
        EXPECT_EQ ( local_transform_scale_y_descriptor.GetId(), "GlobalScaleY"_crc32 );
        EXPECT_EQ ( local_transform_scale_y_descriptor.GetFormat(), "f" );
        local_transform_scale_y_descriptor.Set ( &node, &value );
        local_transform_scale_y_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetGlobalTransformScaleZProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_scale_z_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [0].GetSubProperties() [2];
        EXPECT_EQ ( local_transform_scale_z_descriptor.GetDisplayName(), "Z" );
        EXPECT_EQ ( local_transform_scale_z_descriptor.GetName(), "GlobalScaleZ" );
        EXPECT_EQ ( local_transform_scale_z_descriptor.GetId(), "GlobalScaleZ"_crc32 );
        EXPECT_EQ ( local_transform_scale_z_descriptor.GetFormat(), "f" );
        local_transform_scale_z_descriptor.Set ( &node, &value );
        local_transform_scale_z_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetGlobalTransformRotationWProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_rotation_x_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [1].GetSubProperties() [0];
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetDisplayName(), "W" );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetName(), "GlobalRotationW" );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetId(), "GlobalRotationW"_crc32 );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetFormat(), "f" );
        local_transform_rotation_x_descriptor.Set ( &node, &value );
        local_transform_rotation_x_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetGlobalTransformRotationXProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_rotation_x_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [1].GetSubProperties() [1];
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetDisplayName(), "X" );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetName(), "GlobalRotationX" );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetId(), "GlobalRotationX"_crc32 );
        EXPECT_EQ ( local_transform_rotation_x_descriptor.GetFormat(), "f" );
        local_transform_rotation_x_descriptor.Set ( &node, &value );
        local_transform_rotation_x_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetGlobalTransformRotationYProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_rotation_y_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [1].GetSubProperties() [2];
        EXPECT_EQ ( local_transform_rotation_y_descriptor.GetDisplayName(), "Y" );
        EXPECT_EQ ( local_transform_rotation_y_descriptor.GetName(), "GlobalRotationY" );
        EXPECT_EQ ( local_transform_rotation_y_descriptor.GetId(), "GlobalRotationY"_crc32 );
        EXPECT_EQ ( local_transform_rotation_y_descriptor.GetFormat(), "f" );
        local_transform_rotation_y_descriptor.Set ( &node, &value );
        local_transform_rotation_y_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetGlobalTransformRotationZProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_rotation_z_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [1].GetSubProperties() [3];
        EXPECT_EQ ( local_transform_rotation_z_descriptor.GetDisplayName(), "Z" );
        EXPECT_EQ ( local_transform_rotation_z_descriptor.GetName(), "GlobalRotationZ" );
        EXPECT_EQ ( local_transform_rotation_z_descriptor.GetId(), "GlobalRotationZ"_crc32 );
        EXPECT_EQ ( local_transform_rotation_z_descriptor.GetFormat(), "f" );
        local_transform_rotation_z_descriptor.Set ( &node, &value );
        local_transform_rotation_z_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetGlobalTransformTranslationXProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_translation_x_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [2].GetSubProperties() [0];
        EXPECT_EQ ( local_transform_translation_x_descriptor.GetDisplayName(), "X" );
        EXPECT_EQ ( local_transform_translation_x_descriptor.GetName(), "GlobalTranslationX" );
        EXPECT_EQ ( local_transform_translation_x_descriptor.GetId(), "GlobalTranslationX"_crc32 );
        EXPECT_EQ ( local_transform_translation_x_descriptor.GetFormat(), "f" );
        local_transform_translation_x_descriptor.Set ( &node, &value );
        local_transform_translation_x_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetGlobalTransformTranslationYProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_translation_y_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [2].GetSubProperties() [1];
        EXPECT_EQ ( local_transform_translation_y_descriptor.GetDisplayName(), "Y" );
        EXPECT_EQ ( local_transform_translation_y_descriptor.GetName(), "GlobalTranslationY" );
        EXPECT_EQ ( local_transform_translation_y_descriptor.GetId(), "GlobalTranslationY"_crc32 );
        EXPECT_EQ ( local_transform_translation_y_descriptor.GetFormat(), "f" );
        local_transform_translation_y_descriptor.Set ( &node, &value );
        local_transform_translation_y_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
    TEST ( Node, GetGlobalTransformTranslationZProperty )
    {
        Node node;
        float result{};
        float value{3.14f};
        const Node::Property& local_transform_translation_z_descriptor = node.GetProperties() [ 1 ].get().GetSubProperties() [2].GetSubProperties() [2];
        EXPECT_EQ ( local_transform_translation_z_descriptor.GetDisplayName(), "Z" );
        EXPECT_EQ ( local_transform_translation_z_descriptor.GetName(), "GlobalTranslationZ" );
        EXPECT_EQ ( local_transform_translation_z_descriptor.GetId(), "GlobalTranslationZ"_crc32 );
        EXPECT_EQ ( local_transform_translation_z_descriptor.GetFormat(), "f" );
        local_transform_translation_z_descriptor.Set ( &node, &value );
        local_transform_translation_z_descriptor.Get ( &node, &result );
        EXPECT_EQ ( result , 3.14f );
    }
}
