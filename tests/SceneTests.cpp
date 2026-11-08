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
#include <span>
#include <vector>
#include <iostream>
#include <sstream>
#include "gtest/gtest.h"
#include "aeongames/CRC.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Component.hpp"
#include "aeongames/ComponentProperties.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/ProtoBufClasses.hpp"
#include "pipeline.pb.h"
#include "scene.pb.h"

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
            a->SetId ( "a" );
            b->SetId ( "b" );
            c->SetId ( "c" );
            d->SetId ( "d" );
            e->SetId ( "e" );
            f->SetId ( "f" );
            g->SetId ( "g" );
            h->SetId ( "h" );
            i->SetId ( "i" );
            j->SetId ( "j" );
            k->SetId ( "k" );
            l->SetId ( "l" );
            m->SetId ( "m" );
            n->SetId ( "n" );
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

    class DescriptorTestComponent final : public ComponentProperties<DescriptorTestComponent>
    {
    public:
        float GetValue() const
        {
            return mValue;
        }
        void SetValue ( float aValue )
        {
            mValue = aValue;
            ++mSetterCalls;
        }
        const std::string& GetText() const
        {
            return mText;
        }
        void SetText ( const std::string& aText )
        {
            mText = aText;
        }
        void SetFlexibleText ( const Property& aProperty )
        {
            if ( const auto * text = std::get_if<std::string> ( &aProperty ) )
            {
                SetText ( *text );
            }
            else if ( const auto * path = std::get_if<std::filesystem::path> ( &aProperty ) )
            {
                SetText ( path->string() );
            }
        }
        size_t GetSetterCalls() const
        {
            return mSetterCalls;
        }
        const StringId& GetClassId() const final
        {
            static constexpr StringId identifier{"Descriptor Test"};
            return identifier;
        }
        void Update ( Node&, double ) final {}
        void ProcessMessage ( Node&, uint32_t, const void* ) final {}
    private:
        friend class ComponentProperties<DescriptorTestComponent>;
        inline static constexpr std::array<ComponentPropertyDescriptor<DescriptorTestComponent>, 3> kPropertyDescriptors
        {
            {
                ComponentPropertyDescriptor<DescriptorTestComponent>::Create<&DescriptorTestComponent::GetValue, &DescriptorTestComponent::SetValue> ( "Value" ),
                ComponentPropertyDescriptor<DescriptorTestComponent>::Create<&DescriptorTestComponent::GetText, &DescriptorTestComponent::SetText> ( "Text" ),
                ComponentPropertyDescriptor<DescriptorTestComponent>::Create<&DescriptorTestComponent::GetText, &DescriptorTestComponent::SetFlexibleText> ( "Flexible Text" )
            }};
        float mValue{};
        std::string mText{};
        size_t mSetterCalls{};
    };

    TEST ( ComponentPropertiesTest, MetadataPreservesDeclarationOrder )
    {
        DescriptorTestComponent component;
        ASSERT_EQ ( component.GetPropertyCount(), 3u );
        const StringId* identifiers = component.GetPropertyInfoArray();
        ASSERT_NE ( identifiers, nullptr );
        EXPECT_EQ ( identifiers[0], "Value"_crc32 );
        EXPECT_EQ ( identifiers[1], "Text"_crc32 );
        EXPECT_EQ ( identifiers[2], "Flexible Text"_crc32 );
        EXPECT_EQ ( component.GetPropertyInfoArray(), identifiers );
    }

    TEST ( ComponentPropertiesTest, DispatchesTypedGettersAndSetters )
    {
        DescriptorTestComponent component;
        component.SetProperty ( "Value"_crc32, 3.5f );
        component.SetProperty ( std::string{"Text"}, std::string{"example"} );
        EXPECT_EQ ( std::get<float> ( component.GetProperty ( "Value" ) ), 3.5f );
        EXPECT_EQ ( std::get<std::string> ( component.GetProperty ( "Text" ) ), "example" );
        EXPECT_EQ ( component.GetSetterCalls(), 1u );
    }

    TEST ( ComponentPropertiesTest, IgnoresUnknownIdsAndMismatchedTypes )
    {
        DescriptorTestComponent component;
        component.SetProperty ( "Value"_crc32, 3.5f );
        component.SetProperty ( "Value"_crc32, 7.0 );
        component.SetProperty ( "Value"_crc32, std::string{"wrong type"} );
        component.SetProperty ( "Unknown"_crc32, 8.0f );
        component.SetProperty ( "Text"_crc32, 42 );
        EXPECT_EQ ( component.GetValue(), 3.5f );
        EXPECT_EQ ( component.GetSetterCalls(), 1u );
        EXPECT_TRUE ( component.GetText().empty() );
        EXPECT_EQ ( component.GetProperty ( "Unknown" ), Property{} );
    }

    TEST ( ComponentPropertiesTest, CustomSettersAcceptMultiplePropertyAlternatives )
    {
        DescriptorTestComponent component;
        component.SetProperty ( "Flexible Text"_crc32, std::filesystem::path{"models/example.txt"} );
        EXPECT_EQ ( component.GetText(), "models/example.txt" );
        component.SetProperty ( "Flexible Text"_crc32, std::string{"second"} );
        EXPECT_EQ ( component.GetText(), "second" );
        component.SetProperty ( "Flexible Text"_crc32, 12 );
        EXPECT_EQ ( component.GetText(), "second" );
    }

    class EmptyDescriptorTestComponent final : public ComponentProperties<EmptyDescriptorTestComponent>
    {
    public:
        const StringId& GetClassId() const final
        {
            static constexpr StringId identifier{"Empty Descriptor Test"};
            return identifier;
        }
        void Update ( Node&, double ) final {}
        void ProcessMessage ( Node&, uint32_t, const void* ) final {}
    private:
        friend class ComponentProperties<EmptyDescriptorTestComponent>;
        inline static constexpr std::array<ComponentPropertyDescriptor<EmptyDescriptorTestComponent>, 0> kPropertyDescriptors{};
    };

    TEST ( ComponentPropertiesTest, EmptyTablePreservesEmptyComponentContract )
    {
        EmptyDescriptorTestComponent component;
        EXPECT_EQ ( component.GetPropertyCount(), 0u );
        EXPECT_EQ ( component.GetPropertyInfoArray(), nullptr );
        EXPECT_EQ ( component.GetProperty ( "Unknown" ), Property{} );
        component.SetProperty ( "Unknown"_crc32, 1.0f );
    }

    TEST ( ComponentPropertiesTest, FactoryMetadataAndTypeCheckingRemainStable )
    {
        auto check_properties = [] ( const StringId & aType, std::initializer_list<const char*> aNames )
        {
            SCOPED_TRACE ( aType.GetString() );
            auto component = ConstructComponent ( aType );
            ASSERT_NE ( component, nullptr );
            EXPECT_EQ ( component->GetClassId(), aType );
            ASSERT_EQ ( component->GetPropertyCount(), aNames.size() );
            const StringId* identifiers = component->GetPropertyInfoArray();
            ASSERT_NE ( identifiers, nullptr );
            EXPECT_EQ ( component->GetPropertyInfoArray(), identifiers );
            size_t index = 0;
            for ( const char * name : aNames )
            {
                EXPECT_STREQ ( identifiers[index].GetString(), name );
                const Property initial = component->GetProperty ( identifiers[index] );
                const Property mismatch = std::holds_alternative<float> ( initial ) ? Property{std::string{"wrong type"}} :
                                          Property{2.0f};
                component->SetProperty ( identifiers[index], mismatch );
                EXPECT_EQ ( component->GetProperty ( identifiers[index] ), initial );
                ++index;
            }
            EXPECT_EQ ( component->GetProperty ( "Unknown" ), Property{} );
        };
        check_properties ( "Point Light", {"Color R", "Color G", "Color B", "Intensity", "Radius"} );
        check_properties ( "Spot Light", {"Color R", "Color G", "Color B", "Intensity", "Radius", "Inner Angle", "Outer Angle"} );
        check_properties ( "Directional Light", {"Color R", "Color G", "Color B", "Intensity"} );
        check_properties ( "Marker", {"Type"} );
        check_properties ( "Camera", {"Field of View", "Near Plane", "Far Plane"} );
        check_properties ( "Character Controller", {"Move Speed", "Turn Speed"} );
        check_properties ( "Free Camera", {"Move Speed", "Boost Multiplier", "Look Speed", "Field of View", "Near Plane", "Far Plane"} );
        check_properties ( "OrbitalCamera", {"Distance", "Azimuth", "Elevation", "Height Offset", "Field of View", "Near Plane", "Far Plane"} );
        check_properties ( "OverTheShoulderCamera", {"Shoulder Offset", "Height Offset", "Distance Behind", "Field of View", "Near Plane", "Far Plane"} );
        check_properties ( "Model Component", {"Model", "Active Animation", "Starting Frame"} );
        check_properties ( "Collision Component", {"Collision"} );
    }

    TEST ( ComponentPropertiesTest, SpotLightRetainsAngleClamping )
    {
        auto component = ConstructComponent ( std::string{"Spot Light"} );
        ASSERT_NE ( component, nullptr );
        component->SetProperty ( "Inner Angle"_crc32, 80.0f );
        EXPECT_EQ ( std::get<float> ( component->GetProperty ( "Inner Angle" ) ), 30.0f );
        component->SetProperty ( "Outer Angle"_crc32, 10.0f );
        EXPECT_EQ ( std::get<float> ( component->GetProperty ( "Inner Angle" ) ), 10.0f );
        EXPECT_EQ ( std::get<float> ( component->GetProperty ( "Outer Angle" ) ), 10.0f );
    }

    TEST ( ComponentPropertiesTest, ResourcePropertiesRetainSupportedAlternatives )
    {
        auto check_resource = [] ( const StringId & aType, const StringId & aPropertyId, const std::string & aPath )
        {
            SCOPED_TRACE ( aType.GetString() );
            auto component = ConstructComponent ( aType );
            ASSERT_NE ( component, nullptr );
            const std::array<Property, 3> alternatives
            {{aPath, std::filesystem::path{aPath}, crc32i ( aPath.c_str(), aPath.size() ) }};
            for ( const Property& value : alternatives )
            {
                component->SetProperty ( aPropertyId, value );
                EXPECT_EQ ( std::get<std::string> ( component->GetProperty ( aPropertyId ) ), aPath );
            }
        };
        check_resource ( "Model Component", "Model", "models/simple_phong_cube.txt" );
        check_resource ( "Collision Component", "Collision", "meshes/cube_collision.cln" );
    }

    TEST ( ComponentPropertiesTest, DescriptorComponentsRoundTripThroughScene )
    {
        for ( bool as_binary :
              {
                  false, true
              } )
        {
            Scene source;
            Node* node = source.Add ( std::make_unique<Node>() );
            Component* light = node->AddComponent ( ConstructComponent ( std::string{"Point Light"} ) );
            ASSERT_NE ( light, nullptr );
            light->SetProperty ( "Radius"_crc32, 7.5f );
            Component* marker = node->AddComponent ( ConstructComponent ( std::string{"Marker"} ) );
            ASSERT_NE ( marker, nullptr );
            marker->SetProperty ( "Type"_crc32, std::string{"Spawn"} );
            const std::string serialized = source.Serialize ( as_binary );
            Scene restored;
            restored.Deserialize ( serialized );
            ASSERT_EQ ( restored.GetChildrenCount(), 1u );
            EXPECT_EQ ( restored.Serialize ( as_binary ), serialized );
        }
    }

    TEST_F ( SceneTest, TacticalGridFactoryConstructsAndDestroys )
    {
        auto component = ConstructComponent ( std::string{"Tactical Grid"} );
        ASSERT_NE ( component, nullptr );
        EXPECT_EQ ( component->GetClassId(), "Tactical Grid"_crc32 );
        EXPECT_EQ ( component->GetPropertyCount(), 0u );
        EXPECT_EQ ( component->GetPropertyInfoArray(), nullptr );
        component.reset();
        EXPECT_EQ ( component, nullptr );
    }

    TEST_F ( SceneTest, TacticalGridSceneLoadsRegisteredComponent )
    {
        Scene scene;
        std::ostringstream errors;
        std::streambuf* previous = std::cerr.rdbuf ( errors.rdbuf() );
        EXPECT_NO_THROW ( scene.Deserialize ( "AEONSCN\nnode { component { type: \"Tactical Grid\" } }\n" ) );
        std::cerr.rdbuf ( previous );
        EXPECT_TRUE ( errors.str().empty() );
        ASSERT_EQ ( scene.GetChildrenCount(), 1u );
        ASSERT_EQ ( scene[0].GetComponentCount(), 1u );
        EXPECT_NE ( scene[0].GetComponent ( "Tactical Grid"_crc32 ), nullptr );
    }

    TEST_F ( SceneTest, UnknownComponentLogsErrorAndContinuesLoading )
    {
        constexpr char kScene[] =
            "AEONSCN\n"
            "node {\n"
            "  id: \"Mixed\"\n"
            "  component {\n"
            "    type: \"Unregistered Component Test\"\n"
            "    property { name: \"Ignored\" }\n"
            "  }\n"
            "  component {\n"
            "    type: \"Marker\"\n"
            "    property { name: \"Type\" string: \"Spawn\" }\n"
            "  }\n"
            "}\n"
            "node { id: \"Following\" }\n";
        Scene scene;
        std::ostringstream errors;
        std::streambuf* previous = std::cerr.rdbuf ( errors.rdbuf() );
        EXPECT_NO_THROW ( scene.Deserialize ( kScene ) );
        std::cerr.rdbuf ( previous );
        EXPECT_NE ( errors.str().find ( "Unregistered Component Test" ), std::string::npos );
        EXPECT_NE ( errors.str().find ( "Mixed" ), std::string::npos );
        ASSERT_EQ ( scene.GetChildrenCount(), 2u );
        EXPECT_EQ ( scene[1].GetStringId(), "Following" );
        ASSERT_EQ ( scene[0].GetComponentCount(), 1u );
        const Component* marker = scene[0].GetComponent ( "Marker"_crc32 );
        ASSERT_NE ( marker, nullptr );
        EXPECT_EQ ( std::get<std::string> ( marker->GetProperty ( "Type" ) ), "Spawn" );
    }

    TEST_F ( SceneTest, MissingComponentTypeLogsErrorAndIsSkipped )
    {
        NodeMsg message;
        message.add_component();
        Node node;
        std::ostringstream errors;
        std::streambuf* previous = std::cerr.rdbuf ( errors.rdbuf() );
        EXPECT_NO_THROW ( node.Deserialize ( message ) );
        std::cerr.rdbuf ( previous );
        EXPECT_NE ( errors.str().find ( "Unable to construct component" ), std::string::npos );
        EXPECT_EQ ( node.GetComponentCount(), 0u );
    }

    TEST_F ( SceneTest, ComponentTypeRetainsWireCompatibility )
    {
        const std::string legacy_binary{"\x0a\x06Marker", 8};
        ComponentMsg message;
        ASSERT_TRUE ( message.ParseFromString ( legacy_binary ) );
        EXPECT_EQ ( message.type(), "Marker" );
        EXPECT_EQ ( message.SerializeAsString(), legacy_binary );
        NodeMsg node_message;
        *node_message.add_component() = message;
        Node node;
        node.Deserialize ( node_message );
        ASSERT_EQ ( node.GetComponentCount(), 1u );
        EXPECT_EQ ( node.GetComponentByIndex ( 0 )->GetClassId(), "Marker"_crc32 );
    }

    TEST_F ( SceneTest, MissingTransformResetsToIdentity )
    {
        Node node;
        node.SetLocalTransform ( Transform{Vector3{2, 3, 4}, Quaternion{0, 1, 0, 0}, Vector3{5, 6, 7}} );
        NodeMsg message;
        node.Deserialize ( message );
        EXPECT_EQ ( node.GetLocalTransform().GetScale(), ( Vector3{1, 1, 1} ) );
        EXPECT_EQ ( node.GetLocalTransform().GetRotation(), ( Quaternion{1, 0, 0, 0} ) );
        EXPECT_EQ ( node.GetLocalTransform().GetTranslation(), Vector3{} );
        EXPECT_EQ ( node.GetGlobalTransform(), Transform{} );
    }

    TEST_F ( SceneTest, MissingTransformInheritsParentTransform )
    {
        const Transform parent_transform{Vector3{2, 3, 4}, Quaternion{1, 0, 0, 0}, Vector3{5, 6, 7}};
        a->SetLocalTransform ( parent_transform );
        Node* child = a->Add ( std::make_unique<Node>() );
        NodeMsg message;
        child->Deserialize ( message );
        EXPECT_EQ ( child->GetLocalTransform(), Transform{} );
        EXPECT_EQ ( child->GetGlobalTransform(), parent_transform );
    }

    TEST_F ( SceneTest, NodeDefaultsToNoId )
    {
        Node node;
        EXPECT_TRUE ( node.GetStringId().empty() );
        EXPECT_EQ ( node.GetId(), 0u );
        NodeMsg message;
        message.set_id ( "stale" );
        node.Serialize ( message );
        EXPECT_FALSE ( message.has_id() );
    }

    TEST_F ( SceneTest, NodeIdRoundTrips )
    {
        Node node;
        node.SetId ( "Camera" );
        EXPECT_EQ ( node.GetStringId(), "Camera" );
        EXPECT_EQ ( node.GetId(), "Camera"_crc32 );
        NodeMsg message;
        node.Serialize ( message );
        ASSERT_TRUE ( message.has_id() );
        EXPECT_EQ ( message.id(), "Camera" );
        Node restored;
        restored.Deserialize ( message );
        EXPECT_EQ ( restored.GetStringId(), "Camera" );
        EXPECT_EQ ( restored.GetId(), "Camera"_crc32 );
    }

    TEST_F ( SceneTest, NodeEmptyIdClearsIdentity )
    {
        Node node;
        node.SetId ( "Camera" );
        node.SetId ( "" );
        EXPECT_TRUE ( node.GetStringId().empty() );
        EXPECT_EQ ( node.GetId(), 0u );
        NodeMsg message;
        node.Serialize ( message );
        EXPECT_FALSE ( message.has_id() );
    }

    TEST_F ( SceneTest, NodeDeserializeOptionalIdClearsPreviousIdentity )
    {
        for ( bool has_id :
              {
                  false, true
              } )
        {
            NodeMsg message;
            if ( has_id )
            {
                message.set_id ( "" );
            }
            Node node;
            node.SetId ( "Camera" );
            node.Deserialize ( message );
            EXPECT_TRUE ( node.GetStringId().empty() );
            EXPECT_EQ ( node.GetId(), 0u );
            node.Serialize ( message );
            EXPECT_FALSE ( message.has_id() );
        }
    }

    TEST_F ( SceneTest, ZeroCameraIdClearsCamera )
    {
        mScene.Add ( std::make_unique<Node>() );
        mScene.SetCamera ( a );
        mScene.SetCamera ( uint32_t{0} );
        EXPECT_EQ ( mScene.GetCamera(), nullptr );
        mScene.SetCamera ( a );
        mScene.SetCamera ( std::string{} );
        EXPECT_EQ ( mScene.GetCamera(), nullptr );
    }

    TEST_F ( SceneTest, AnonymousNodesRoundTripWithoutCamera )
    {
        for ( bool as_binary :
              {
                  false, true
              } )
        {
            Scene source;
            source.Add ( std::make_unique<Node>() );
            source.Add ( std::make_unique<Node>() )->SetId ( "Named" );
            std::string serialized = source.Serialize ( as_binary );
            Scene restored;
            restored.Deserialize ( serialized );
            ASSERT_EQ ( restored.GetChildrenCount(), 2u );
            EXPECT_TRUE ( restored[0].GetStringId().empty() );
            EXPECT_EQ ( restored[0].GetId(), 0u );
            EXPECT_EQ ( restored[1].GetStringId(), "Named" );
            EXPECT_EQ ( restored.GetCamera(), nullptr );
            EXPECT_EQ ( restored.Serialize ( as_binary ), serialized );
        }
    }

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

    /** What the Blender scene exporter emits for an empty parented to a mesh:
        a marker node nested under its parent, addressable by name. */
    TEST_F ( SceneTest, DeserializesANestedMarkerNode )
    {
        constexpr char kSceneWithMarker[] =
            "AEONSCN\n"
            "name: \"markers\"\n"
            "node {\n"
            "  id: \"Anchor\"\n"
            "  local {\n"
            "    Scale { x: 1 y: 1 z: 1 }\n"
            "    Rotation { w: 1 }\n"
            "    Translation { x: 3 }\n"
            "  }\n"
            "  node {\n"
            "    id: \"Mark\"\n"
            "    local {\n"
            "      Scale { x: 1 y: 1 z: 1 }\n"
            "      Rotation { w: 1 }\n"
            "      Translation { y: 2 }\n"
            "    }\n"
            "    component {\n"
            "      type: \"Marker\"\n"
            "      property { name: \"Type\" string: \"Spawn\" }\n"
            "    }\n"
            "  }\n"
            "}\n";
        Scene scene;
        scene.Deserialize ( kSceneWithMarker );

        const Node* mark = scene.Find ( [] ( const Node & aNode )
        {
            return aNode.GetStringId() == "Mark";
        } );
        ASSERT_NE ( mark, nullptr );
        EXPECT_EQ ( mark->GetGlobalTransform().GetTranslation(), ( Vector3{3, 2, 0} ) );

        ASSERT_EQ ( mark->GetComponentCount(), 1u );
        const Component* marker = mark->GetComponent ( "Marker"_crc32 );
        ASSERT_NE ( marker, nullptr );
        EXPECT_EQ ( std::get<std::string> ( marker->GetProperty ( "Type" ) ), "Spawn" );
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

        // Brute-force reference: the exact set of nodes whose world AABB overlaps
        // the query box, gathered by a plain traversal.
        std::vector<const Node*> BruteForceOverlap ( const Scene& aScene, const AABB& aBox )
        {
            std::vector<const Node*> hits;
            aScene.LoopTraverseDFSPreOrder ( [&aBox, &hits] ( const Node & aNode )
            {
                const AABB world = aNode.GetGlobalTransform() * aNode.GetAABB();
                if ( aBox.Overlaps ( world ) )
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

    // ---- BuildRenderQueue / ForEachRenderBatch (render-queue batching) --------

    namespace
    {
        // Distinct non-null sentinels standing in for loaded resources. The
        // render queue only compares and stores these pointers (the submit phase,
        // which would dereference them, is exercised by the GPU backends), so
        // fake addresses are enough to test collection, sorting and batching.
        const Mesh* const kMeshA = reinterpret_cast<const Mesh*> ( 0x1000 );
        const Mesh* const kMeshB = reinterpret_cast<const Mesh*> ( 0x2000 );
        const Pipeline kOpaquePipeline = [] ()
        {
            Pipeline pipeline;
            PipelineMsg msg;
            msg.mutable_blend()->set_enabled ( PipelineMsg_Toggle_DISABLED );
            pipeline.LoadFromPBMsg ( msg );
            return pipeline;
        }();
        const Pipeline* const kPipeline = &kOpaquePipeline;
        const Material* const kMaterial = reinterpret_cast<const Material*> ( 0x4000 );

        // Minimal component that declares a single draw, standing in for
        // ModelComponent without needing a loaded model asset. Collect appends
        // one RenderItem built from the fixed resources and the node transform.
        class FakeRenderComponent : public Component
        {
        public:
            FakeRenderComponent ( const Mesh* aMesh, const Pipeline* aPipeline,
                                  const Material* aMaterial, const BufferAccessor* aSkinned,
                                  Topology aTopology )
                : mMesh{aMesh}, mPipeline{aPipeline}, mMaterial{aMaterial}, mSkinned{aSkinned}, mTopology{aTopology} {}
            const StringId& GetClassId() const final
            {
                static const StringId id{ "FakeRenderComponent" };
                return id;
            }
            size_t GetPropertyCount() const final
            {
                return 0;
            }
            const StringId* GetPropertyInfoArray() const final
            {
                return nullptr;
            }
            Property GetProperty ( const StringId& ) const final
            {
                return Property{};
            }
            void SetProperty ( uint32_t, const Property& ) final {}
            void Update ( Node&, double ) final {}
            void Collect ( const Node& aNode, std::vector<RenderItem>& aQueue ) const final
            {
                aQueue.push_back ( RenderItem{ mMesh, mPipeline, mMaterial, mSkinned, aNode.GetGlobalTransform(), mTopology } );
            }
            void ProcessMessage ( Node&, uint32_t, const void* ) final {}
        private:
            const Mesh* mMesh;
            const Pipeline* mPipeline;
            const Material* mMaterial;
            const BufferAccessor* mSkinned;
            Topology mTopology;
        };

        // Add a visible unit-AABB node that declares one draw of the given
        // resources. Mirrors AddPositioned but attaches the fake component.
        Node* AddDrawable ( Scene& aScene, const Vector3& aPosition, const Mesh* aMesh,
                            const Pipeline* aPipeline, const Material* aMaterial,
                            const BufferAccessor* aSkinned = nullptr,
                            Topology aTopology = Topology::TRIANGLE_LIST )
        {
            Node* node = AddPositioned ( aScene, aPosition );
            node->AddComponent ( std::make_unique<FakeRenderComponent> ( aMesh, aPipeline, aMaterial, aSkinned, aTopology ) );
            return node;
        }

        // Collect the sizes of the batches ForEachRenderBatch produces from the
        // built queue, sorted ascending so comparisons are order-independent.
        std::vector<size_t> RenderBatchSizes ( const Scene& aScene )
        {
            std::vector<size_t> sizes;
            aScene.ForEachRenderBatch ( [&sizes] ( std::span<const RenderItem> aBatch )
            {
                sizes.push_back ( aBatch.size() );
            } );
            std::sort ( sizes.begin(), sizes.end() );
            return sizes;
        }
    }

    TEST ( SceneRenderQueue, RenderItemsDefaultToTriangleList )
    {
        const RenderItem item{ kMeshA, kPipeline, kMaterial, nullptr, Matrix4x4{} };
        EXPECT_EQ ( item.mTopology, Topology::TRIANGLE_LIST );
    }

    TEST ( SceneRenderQueue, CollectsVisibleNodesIntoQueue )
    {
        Scene scene;
        AddDrawable ( scene, Vector3 { 0.0f, 50.0f, 0.0f }, kMeshA, kPipeline, kMaterial );
        AddDrawable ( scene, Vector3 { 10.0f, 50.0f, 5.0f }, kMeshA, kPipeline, kMaterial );
        AddDrawable ( scene, Vector3 { -8.0f, 50.0f, -4.0f }, kMeshA, kPipeline, kMaterial );
        scene.BuildRenderQueue ( MakeCullFrustum() );
        EXPECT_EQ ( scene.GetRenderQueue().size(), 3u );
        // All three share geometry, pipeline and material: one instanced batch.
        EXPECT_EQ ( RenderBatchSizes ( scene ), ( std::vector<size_t> { 3u } ) );
    }

    TEST ( SceneRenderQueue, SeparatesDistinctGeometryIntoBatches )
    {
        Scene scene;
        AddDrawable ( scene, Vector3 { 0.0f, 50.0f, 0.0f }, kMeshA, kPipeline, kMaterial );
        AddDrawable ( scene, Vector3 { 10.0f, 50.0f, 5.0f }, kMeshA, kPipeline, kMaterial );
        AddDrawable ( scene, Vector3 { -8.0f, 50.0f, -4.0f }, kMeshB, kPipeline, kMaterial );
        scene.BuildRenderQueue ( MakeCullFrustum() );
        EXPECT_EQ ( scene.GetRenderQueue().size(), 3u );
        // Two share kMeshA (batched), the kMeshB item draws alone.
        EXPECT_EQ ( RenderBatchSizes ( scene ), ( std::vector<size_t> { 1u, 2u } ) );
    }

    TEST ( SceneRenderQueue, SeparatesDistinctTopologiesIntoBatches )
    {
        Scene scene;
        for ( Topology topology :
              {
                  Topology::LINE_LIST, Topology::TRIANGLE_LIST, Topology::LINE_STRIP,
                  Topology::LINE_LIST, Topology::TRIANGLE_LIST
              } )
        {
            AddDrawable ( scene, Vector3 { 0.0f, 50.0f, 0.0f }, kMeshA, kPipeline, kMaterial, nullptr, topology );
        }
        scene.BuildRenderQueue ( MakeCullFrustum() );
        ASSERT_EQ ( scene.GetRenderQueue().size(), 5u );
        EXPECT_EQ ( RenderBatchSizes ( scene ), ( std::vector<size_t> { 1u, 2u, 2u } ) );
        const auto& queue = scene.GetRenderQueue();
        EXPECT_TRUE ( std::is_sorted ( queue.begin(), queue.end(), [] ( const RenderItem & aLhs, const RenderItem & aRhs )
        {
            return aLhs.mTopology < aRhs.mTopology;
        } ) );
        scene.ForEachRenderBatch ( [] ( std::span<const RenderItem> aBatch )
        {
            for ( const RenderItem& item : aBatch )
            {
                EXPECT_EQ ( item.mTopology, aBatch.front().mTopology );
            }
        } );
    }

    TEST ( SceneRenderQueue, CullsItemsOutsideTheFrustum )
    {
        Scene scene;
        AddDrawable ( scene, Vector3 { 0.0f, 50.0f, 0.0f }, kMeshA, kPipeline, kMaterial );
        AddDrawable ( scene, Vector3 { 0.0f, -50.0f, 0.0f }, kMeshA, kPipeline, kMaterial );   // behind camera
        AddDrawable ( scene, Vector3 { 1000.0f, 50.0f, 0.0f }, kMeshA, kPipeline, kMaterial ); // far to the side
        scene.BuildRenderQueue ( MakeCullFrustum() );
        EXPECT_EQ ( scene.GetRenderQueue().size(), 1u );
        EXPECT_EQ ( RenderBatchSizes ( scene ), ( std::vector<size_t> { 1u } ) );
    }

    TEST ( SceneRenderQueue, SkinnedItemsAreNeverBatched )
    {
        Scene scene;
        // Distinct skinned-vertex pointers stand in for per-node posed buffers.
        const BufferAccessor* const skinnedA = reinterpret_cast<const BufferAccessor*> ( 0x5000 );
        const BufferAccessor* const skinnedB = reinterpret_cast<const BufferAccessor*> ( 0x6000 );
        AddDrawable ( scene, Vector3 { 0.0f, 50.0f, 0.0f }, kMeshA, kPipeline, kMaterial, skinnedA );
        AddDrawable ( scene, Vector3 { 10.0f, 50.0f, 5.0f }, kMeshA, kPipeline, kMaterial, skinnedB );
        // A non-skinned node sharing the same geometry must not absorb the
        // skinned ones into a batch.
        AddDrawable ( scene, Vector3 { -8.0f, 50.0f, -4.0f }, kMeshA, kPipeline, kMaterial );
        scene.BuildRenderQueue ( MakeCullFrustum() );
        EXPECT_EQ ( scene.GetRenderQueue().size(), 3u );
        EXPECT_EQ ( RenderBatchSizes ( scene ), ( std::vector<size_t> { 1u, 1u, 1u } ) );
    }

    TEST ( SceneRenderQueue, TransparentItemsAreSortedBackToFrontAndNeverInstanced )
    {
        Scene scene;
        Node* camera = scene.Add ( std::make_unique<Node>() );
        camera->SetGlobalTransform ( Transform { Vector3 { 1.0f, 1.0f, 1.0f }, Quaternion {}, Vector3 { 0.0f, 0.0f, 0.0f } } );
        scene.SetCamera ( camera );

        PipelineMsg opaque_msg;
        opaque_msg.mutable_blend()->set_enabled ( PipelineMsg_Toggle_DISABLED );
        Pipeline opaque_pipeline;
        opaque_pipeline.LoadFromPBMsg ( opaque_msg );

        PipelineMsg transparent_msg;
        transparent_msg.mutable_blend()->set_enabled ( PipelineMsg_Toggle_ENABLED );
        transparent_msg.mutable_blend()->set_source_color ( PipelineMsg_BlendFactor_BLEND_SOURCE_ALPHA );
        transparent_msg.mutable_blend()->set_destination_color ( PipelineMsg_BlendFactor_BLEND_ONE_MINUS_SOURCE_ALPHA );
        transparent_msg.mutable_blend()->set_source_alpha ( PipelineMsg_BlendFactor_BLEND_SOURCE_ALPHA );
        transparent_msg.mutable_blend()->set_destination_alpha ( PipelineMsg_BlendFactor_BLEND_ONE_MINUS_SOURCE_ALPHA );
        Pipeline transparent_pipeline;
        transparent_pipeline.LoadFromPBMsg ( transparent_msg );

        AddDrawable ( scene, Vector3 { 0.0f, 50.0f, -3.0f }, kMeshA, &opaque_pipeline, kMaterial );
        AddDrawable ( scene, Vector3 { 0.0f, 50.0f, -8.0f }, kMeshA, &transparent_pipeline, kMaterial );
        AddDrawable ( scene, Vector3 { 0.0f, 50.0f, -16.0f }, kMeshA, &transparent_pipeline, kMaterial );

        scene.BuildRenderQueue ( MakeCullFrustum() );
        ASSERT_EQ ( scene.GetRenderQueue().size(), 3u );
        EXPECT_EQ ( RenderBatchSizes ( scene ), ( std::vector<size_t> { 1u, 1u, 1u } ) );

        const auto& queue = scene.GetRenderQueue();
        EXPECT_EQ ( queue[0].mPipeline, &opaque_pipeline );
        EXPECT_EQ ( queue[1].mPipeline, &transparent_pipeline );
        EXPECT_EQ ( queue[2].mPipeline, &transparent_pipeline );
        EXPECT_GT ( ( Vector3 { queue[1].mTransform[12], queue[1].mTransform[13], queue[1].mTransform[14] } -
                      camera->GetGlobalTransform().GetTranslation() ).GetLengthSquared(),
                    ( Vector3 { queue[2].mTransform[12], queue[2].mTransform[13], queue[2].mTransform[14] } -
                      camera->GetGlobalTransform().GetTranslation() ).GetLengthSquared() );
    }

    TEST ( SceneRenderQueue, EmptySceneProducesEmptyQueue )
    {
        Scene scene;
        scene.BuildRenderQueue ( MakeCullFrustum() );
        EXPECT_TRUE ( scene.GetRenderQueue().empty() );
        size_t count = 0;
        scene.ForEachRenderBatch ( [&count] ( std::span<const RenderItem> )
        {
            ++count;
        } );
        EXPECT_EQ ( count, 0u );
    }
}
