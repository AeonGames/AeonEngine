/*
Copyright (C) 2014-2019,2021 Rodrigo Jose Hernandez Cordoba

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

/*! \file
\brief Implementation file for the Scene Node class.
\author Rodrigo Hernandez.
\copy 2014-2018 Rodrigo Hernandez
*/

#include <cassert>
#include <algorithm>
#include <type_traits>
#include "aeongames/Node.h"
#include "aeongames/Scene.h"
#include "aeongames/LogLevel.h"
#include "Factory.h"
#include "aeongames/StringId.h"
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/CRC.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "scene.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    Node::Node ( uint32_t aFlags ) :
        mName ( "Node" ),
        mParent{},
        mLocalTransform(),
        mGlobalTransform(),
        mNodes(),
        mId{crc32i ( mName.c_str(), mName.size() ) },
        mFlags ( aFlags )
    {
    }

    size_t Node::GetChildrenCount() const
    {
        return mNodes.size();
    }
    const Node& Node::operator[] ( const std::size_t index ) const
    {
        return * ( mNodes[index] );
    }

    Node& Node::operator[] ( const std::size_t index )
    {
        return const_cast<Node&> ( static_cast<const Node&> ( *this ) [index] );
    }
    Node* Node::GetChild ( size_t aIndex ) const
    {
        return mNodes.at ( aIndex ).get();
    }
    NodeParent Node::GetParent() const
    {
        return mParent;
    }
    size_t Node::GetIndex() const
    {
        return std::visit ( [this] ( auto&& parent ) -> size_t
        {
            if ( parent == nullptr )
            {
                throw std::runtime_error ( "Node has no parent and thus no assigned index." );
            }
            auto index = std::find_if ( parent->mNodes.begin(), parent->mNodes.end(),
                                        [this] ( const std::unique_ptr<Node>& node )
            {
                return node.get() == this;
            } );
            return index - parent->mNodes.begin();
        },
        mParent );
    }
    Scene* Node::GetScene() const
    {
        const Node* node = this;
        while ( std::holds_alternative<Node*> ( node->mParent ) && std::get<Node*> ( node->mParent ) != nullptr )
        {
            node = std::get<Node*> ( node->mParent );
        }
        return GetScenePtr ( node->mParent );
    }

    void Node::SetFlags ( uint32_t aFlagBits, bool aEnabled )
    {
        ( aEnabled ) ? mFlags |= aFlagBits : mFlags &= static_cast<uint32_t> ( ~aFlagBits );
    }

    void Node::SetFlag ( enum Flags aFlag, bool aEnabled )
    {
        mFlags[aFlag] = aEnabled;
    }

    bool Node::IsFlagEnabled ( enum Flags aFlag ) const
    {
        return mFlags[aFlag];
    }

    const Transform& Node::GetLocalTransform() const
    {
        return mLocalTransform;
    }

    const Transform& Node::GetGlobalTransform() const
    {
        return mGlobalTransform;
    }

    const AABB& Node::GetAABB() const
    {
        /** @todo Decide if transforms should be applied
         *  to the AABB and return the result */
        return mAABB;
    }

    void Node::SetLocalTransform ( const Transform& aTransform )
    {
        mLocalTransform = aTransform;
        LoopTraverseDFSPreOrder (
            [] ( Node & node )
        {
            std::visit ( [&node] ( auto&& parent )
            {
                if ( typeid ( parent ).hash_code() == typeid ( Node* ).hash_code() && parent != nullptr )
                {
                    node.mGlobalTransform = reinterpret_cast<Node*> ( parent )->mGlobalTransform * node.mLocalTransform;
                }
                else
                {
                    node.mGlobalTransform = node.mLocalTransform;
                }
            },
            node.mParent );
        } );
    }

    void Node::SetGlobalTransform ( const Transform& aTransform )
    {
        mGlobalTransform = aTransform;
        // Update the Local transform for this node only
        std::visit ( [this] ( auto&& parent )
        {
            if ( typeid ( parent ).hash_code() == typeid ( Node* ).hash_code() && parent != nullptr )
            {
                mLocalTransform = mGlobalTransform * reinterpret_cast<Node*> ( parent )->mGlobalTransform.GetInverted();
            }
            else
            {
                mLocalTransform = mGlobalTransform;
            }
        },
        mParent );

        // Then Update the Global transform for all children
        LoopTraverseDFSPreOrder (
            [this] ( Node & node )
        {
            /*! @todo Although this->mLocalTransform has already been computed
                      think about removing the check and let it be recomputed,
                      it may be faster than the branch that needs to run
                      for each node and is false only once.*/
            std::visit ( [this, &node] ( auto&& parent )
            {
                if ( &node != this && typeid ( parent ).hash_code() == typeid ( Node* ).hash_code() && parent != nullptr )
                {
                    node.mGlobalTransform = reinterpret_cast<Node*> ( parent )->mGlobalTransform * node.mLocalTransform;
                }
            },
            node.mParent );
        } );
    }

    void Node::SetAABB ( const AABB& aAABB )
    {
        mAABB = aAABB;
    }

    void Node::SetName ( const std::string& aName )
    {
        mId = crc32i ( aName.c_str(), aName.size() );
        mName = aName;
    }

    const std::string& Node::GetName() const
    {
        return mName;
    }

    uint32_t Node::GetId() const
    {
        return mId;
    }

    // Helper for the Property visitor in Serialize/Deserialize
    template<class T> struct always_false : std::false_type {};

    void Node::Serialize ( NodeMsg& aNodeMsg ) const
    {
        *aNodeMsg.mutable_name() = GetName();
        aNodeMsg.mutable_local()->mutable_scale()->set_x ( GetLocalTransform().GetScale() [0] );
        aNodeMsg.mutable_local()->mutable_scale()->set_y ( GetLocalTransform().GetScale() [1] );
        aNodeMsg.mutable_local()->mutable_scale()->set_z ( GetLocalTransform().GetScale() [2] );
        aNodeMsg.mutable_local()->mutable_rotation()->set_w ( GetLocalTransform().GetRotation() [0] );
        aNodeMsg.mutable_local()->mutable_rotation()->set_x ( GetLocalTransform().GetRotation() [1] );
        aNodeMsg.mutable_local()->mutable_rotation()->set_y ( GetLocalTransform().GetRotation() [2] );
        aNodeMsg.mutable_local()->mutable_rotation()->set_z ( GetLocalTransform().GetRotation() [3] );
        aNodeMsg.mutable_local()->mutable_translation()->set_x ( GetLocalTransform().GetTranslation() [0] );
        aNodeMsg.mutable_local()->mutable_translation()->set_y ( GetLocalTransform().GetTranslation() [1] );
        aNodeMsg.mutable_local()->mutable_translation()->set_z ( GetLocalTransform().GetTranslation() [2] );
        for ( auto& i : mComponents )
        {
            ComponentMsg* component_buffer = aNodeMsg.add_component();
            ( *component_buffer->mutable_name() ) = i->GetId().GetString();
            const StringId* PropertyIds = i->GetPropertyInfoArray();
            for ( size_t j = 0; j < i->GetPropertyCount(); ++j )
            {
                ComponentPropertyMsg* component_property_buffer = component_buffer->add_property();
                ( *component_property_buffer->mutable_name() ) = PropertyIds[j].GetString();

                std::visit (
                    [component_property_buffer] ( auto&& arg )
                {
                    using T = std::decay_t<decltype ( arg ) >;
                    if constexpr ( std::is_same_v<T, int> )
                    {
                        component_property_buffer->set_int_ ( arg );
                    }
                    else if constexpr ( std::is_same_v<T, long> )
                    {
                        component_property_buffer->set_long_ ( arg );
                    }
                    else if constexpr ( std::is_same_v<T, long long> )
                    {
                        component_property_buffer->set_long_long ( arg );
                    }
                    else if constexpr ( std::is_same_v<T, unsigned> )
                    {
                        component_property_buffer->set_unsigned_ ( arg );
                    }
                    else if constexpr ( std::is_same_v<T, unsigned long> )
                    {
                        component_property_buffer->set_unsigned_long ( arg );
                    }
                    else if constexpr ( std::is_same_v<T, unsigned long long> )
                    {
                        component_property_buffer->set_unsigned_long_long ( arg );
                    }
                    else if constexpr ( std::is_same_v<T, float> )
                    {
                        component_property_buffer->set_float_ ( arg );
                    }
                    else if constexpr ( std::is_same_v<T, double> )
                    {
                        component_property_buffer->set_double_ ( arg );
                    }
                    else if constexpr ( std::is_same_v<T, std::string> )
                    {
                        component_property_buffer->set_string ( arg );
                    }
                    else if constexpr ( std::is_same_v<T, std::filesystem::path> )
                    {
                        component_property_buffer->set_path ( arg.string() );
                    }
                    else
                    {
                        static_assert ( always_false<T>::value,
                                        "A non-exhaustive visitor is being "
                                        "called on an Property object!" );
                    }
                }, i->GetProperty ( PropertyIds[j] ) );
            }
        }
    }

    void Node::Deserialize ( const NodeMsg& aNodeMsg )
    {
        SetName ( aNodeMsg.name() );
        if ( aNodeMsg.has_local() )
        {
            SetLocalTransform ( GetTransform ( aNodeMsg.local() ) );
        }
        else if ( aNodeMsg.has_global() )
        {
            SetGlobalTransform ( GetTransform ( aNodeMsg.global() ) );
        }
        for ( auto& i : aNodeMsg.component() )
        {
            Component* component = AddComponent ( ConstructComponent ( i.name() ) );
            for ( auto& j : i.property() )
            {
                component->SetProperty ( j.name(), GetProperty ( j ) );
            }
        }
    }

    Node* Node::Insert ( size_t aIndex, std::unique_ptr<Node> aNode )
    {
        // Never append this or null pointers.
        if ( aNode == nullptr || aNode.get() == this )
        {
            return nullptr;
        }
        aNode->mParent = this;
        std::vector<std::unique_ptr<Node>>::iterator it{};
        if ( aIndex < mNodes.size() )
        {
            it = mNodes.insert ( mNodes.begin() + aIndex, std::move ( aNode ) );
        }
        else
        {
            it = mNodes.insert ( mNodes.end(), std::move ( aNode ) );
        }
        // Force a recalculation of the LOCAL transform
        // by setting the GLOBAL transform to itself.
        ( *it )->SetGlobalTransform ( ( *it )->mGlobalTransform );
        return ( *it ).get();
    }

    Node* Node::Add ( std::unique_ptr<Node> aNode )
    {
        // Never append this or null pointers.
        if ( aNode == nullptr || aNode.get() == this )
        {
            return nullptr;
        }
        aNode->mParent = this;
        mNodes.emplace_back ( std::move ( aNode ) );
        // Force a recalculation of the LOCAL transform
        // by setting the GLOBAL transform to itself.
        mNodes.back()->SetGlobalTransform ( mNodes.back()->mGlobalTransform );
        return mNodes.back().get();
    }

    std::unique_ptr<Node> Node::Remove ( Node* aNode )
    {
        assert ( aNode != nullptr );
        assert ( aNode != this );
        // If passed a null or this pointer find SHOULD not find it on release builds.
        auto it = std::find_if (
                      mNodes.begin(),
                      mNodes.end(),
                      [aNode] ( const std::unique_ptr<Node>& node )
        {
            return aNode == node.get();
        } );
        if ( it != mNodes.end() )
        {
            aNode->mParent = static_cast<Node*> ( nullptr );
            // Force recalculation of transforms.
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            std::unique_ptr<Node> removed_node{std::move ( *it ) };
            mNodes.erase ( it );
            return removed_node;
        }
        return nullptr;
    }

    std::unique_ptr<Node> Node::RemoveByIndex ( size_t aIndex )
    {
        if ( aIndex >= mNodes.size() )
        {
            return nullptr;
        }
        mNodes[aIndex]->mParent = static_cast<Node*> ( nullptr );
        mNodes[aIndex]->SetLocalTransform ( mNodes[aIndex]->mGlobalTransform );
        auto it = mNodes.begin() + aIndex;
        std::unique_ptr<Node> removed_node{std::move ( *it ) };
        mNodes.erase ( it );
        return removed_node;
    }

    void Node::LoopTraverseDFSPreOrder (
        const std::function<void ( Node& ) >& aPreamble,
        const std::function<void ( Node& ) >& aPostamble )
    {
        /** @todo (EC++ Item 3) This code is the same as the constant overload,
        but can't easily be implemented in terms of that because of aAction's node parameter
        needs to also be const.
        */
        auto node = this;
        aPreamble ( *node );
        auto parent = GetNodePtr ( mParent );
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator].get();
                aPreamble ( *node );
                ++prev->mIterator;
            }
            else
            {
                aPostamble ( *node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = GetNodePtr ( node->mParent );
            }
        }
    }

    /*  This is ugly, but it is only way to use the same code for the const and the non const version
        without having to add template or friend members to the class declaration. */
#define LoopTraverseDFSPreOrder(...) \
    void Node::LoopTraverseDFSPreOrder ( const std::function<void ( __VA_ARGS__ Node& ) >& aAction ) __VA_ARGS__ \
    {\
        /** @todo (EC++ Item 3) This code is the same as the constant overload,\
        but can't easily be implemented in terms of that because of aAction's node parameter\
        need to also be const.\
        */\
        auto node = this;\
        aAction ( *node );\
        auto parent = GetNodePtr(mParent);\
        while ( node != parent )\
        {\
            if ( node->mIterator < node->mNodes.size() )\
            {\
                auto prev = node;\
                node = node->mNodes[node->mIterator].get();\
                aAction ( *node );\
                prev->mIterator++;\
            }\
            else\
            {\
                node->mIterator = 0; /* Reset counter for next traversal.*/\
                node = GetNodePtr(node->mParent);\
            }\
        }\
    }

    LoopTraverseDFSPreOrder ( const )
    LoopTraverseDFSPreOrder( )
#undef LoopTraverseDFSPreOrder

    Node* Node::Find ( const std::function<bool ( const Node& ) >& aUnaryPredicate ) const
    {
        /* Short Circuit in case this is the node being searched for */
        if ( aUnaryPredicate ( *this ) )
        {
            return const_cast<Node*> ( this );
        };
        auto node = this;
        auto parent = GetNodePtr ( mParent );
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator].get();
                ++prev->mIterator;
            }
            else
            {
                node->mIterator = 0; /* Reset counter for next traversal. */
                if ( aUnaryPredicate ( *node ) )
                {
                    return const_cast<Node*> ( node );
                };
                node = GetNodePtr ( node->mParent );
            }
        }
        return nullptr;
    }

#define LoopTraverseDFSPostOrder(...) \
    void Node::LoopTraverseDFSPostOrder ( const std::function<void ( __VA_ARGS__ Node& ) >& aAction ) __VA_ARGS__ \
    { \
        /* \
        This code implements a similar solution to this stackoverflow answer: \
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138 \
        */ \
        auto node = this; \
        auto parent = GetNodePtr(mParent); \
        while ( node != parent ) \
        { \
            if ( node->mIterator < node->mNodes.size() ) \
            { \
                auto prev = node; \
                node = node->mNodes[node->mIterator].get(); \
                ++prev->mIterator; \
            } \
            else \
            { \
                aAction ( *node ); \
                node->mIterator = 0; /* Reset counter for next traversal. */ \
                node = GetNodePtr(node->mParent); \
            } \
        } \
    }

    LoopTraverseDFSPostOrder ( const )
    LoopTraverseDFSPostOrder( )
#undef LoopTraverseDFSPostOrder

    void Node::RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mNode : mNodes )
        {
            mNode->RecursiveTraverseDFSPostOrder ( aAction );
        }
        aAction ( *this );
    }

    void Node::RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        aAction ( *this );
        for ( auto & mNode : mNodes )
        {
            mNode->RecursiveTraverseDFSPreOrder ( aAction );
        }
    }

    void Node::LoopTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
    {
        auto node = this;
        while ( node != nullptr )
        {
            aAction ( *node );
            node = GetNodePtr ( node->mParent );
        }
    }

    void Node::LoopTraverseAncestors ( const std::function<void ( const Node& ) >& aAction ) const
    {
        auto node = this;
        while ( node != nullptr )
        {
            aAction ( *node );
            node = GetNodePtr ( node->mParent );
        }
    }

    void Node::RecursiveTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
    {
        aAction ( *this );
        if ( auto parent = GetNodePtr ( mParent ) )
        {
            parent->RecursiveTraverseAncestors ( aAction );
        }
    }

    void Node::Update ( const double aDelta )
    {
        for ( auto& i : mComponentDependencyMap )
        {
            GetComponent ( i )->Update ( *this, aDelta );
        }
    }

    void Node::ProcessMessage ( uint32_t aMessageType, const void* aMessageData )
    {
        for ( auto& i : mComponentDependencyMap )
        {
            GetComponent ( i )->ProcessMessage ( *this, aMessageType, aMessageData );
        }
    }

    void Node::Render ( Renderer& aRenderer, void* aWindowId ) const
    {
        for ( auto& i : mComponentDependencyMap )
        {
            GetComponent ( i )->Render ( *this, aRenderer, aWindowId );
        }
    }

    size_t Node::GetComponentCount() const
    {
        return mComponents.size();
    }

    Component* Node::GetComponentByIndex ( size_t aIndex ) const
    {
        return mComponents[aIndex].get();
    }

    Component* Node::AddComponent ( std::unique_ptr<Component> aComponent )
    {
        auto i = std::find_if ( mComponents.begin(), mComponents.end(), [&aComponent] ( const std::unique_ptr<Component>& aIteratorComponent )
        {
            return aComponent->GetId() == aIteratorComponent->GetId();
        } );
        if ( i != mComponents.end() )
        {
            std::cout << "Overwriting node data for " << aComponent->GetId().GetString() << std::endl;
            i->swap ( aComponent );
            return i->get();
        }
        mComponentDependencyMap.Insert ( {aComponent->GetId(),/** @todo Get Node Data dependencies. */{}, aComponent->GetId() } );
        mComponents.emplace_back ( std::move ( aComponent ) );
        return mComponents.back().get();
    }

    Component* Node::GetComponent ( uint32_t aId ) const
    {
        auto i = std::find_if ( mComponents.begin(), mComponents.end(), [aId] ( const std::unique_ptr<Component>& aComponent )
        {
            return aComponent->GetId() == aId;
        } );
        if ( i != mComponents.end() )
        {
            return i->get();
        }
        return nullptr;
    }

    std::unique_ptr<Component> Node::RemoveComponent ( uint32_t aId )
    {
        std::unique_ptr<Component> result{};
        auto i = std::find_if ( mComponents.begin(), mComponents.end(), [aId] ( const std::unique_ptr<Component>& aComponent )
        {
            return aComponent->GetId() == aId;
        } );
        if ( i != mComponents.end() )
        {
            mComponentDependencyMap.Erase ( aId );
            result = std::move ( *i );
            mComponents.erase ( std::remove ( i, mComponents.end(), *i ), mComponents.end() );
        }
        return result;
    }
}
