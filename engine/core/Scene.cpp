/*
Copyright (C) 2014-2018 Rodrigo Jose Hernandez Cordoba

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
#include <algorithm>
#include <sstream>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/Scene.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include <google/protobuf/text_format.h>
#include "scene.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    Scene::Node::Node() = default;
    Scene::Node::Node ( const Node & aNode ) :
        mName{ aNode.mName },
        mParent{aNode.mParent},
        mIterator{},
        mFlags{aNode.mFlags},
        mGlobalTransform{aNode.mGlobalTransform},
        mNodes{aNode.mNodes}
    {
        for ( auto& i : mNodes )
        {
            i.mParent = this;
            i.mScene = mScene;
        }
        SetGlobalTransform ( mGlobalTransform );
    }
    Scene::Node::Node ( const Node && aNode ) :
        mName{ std::move ( aNode.mName ) },
        mParent{aNode.mParent},
        mIterator{},
        mFlags{std::move ( aNode.mFlags ) },
        mGlobalTransform{aNode.mGlobalTransform},
        mNodes{std::move ( aNode.mNodes ) }
    {
        for ( auto& i : mNodes )
        {
            i.mParent = this;
            i.mScene = mScene;
        }
        SetGlobalTransform ( mGlobalTransform );
    }
    Scene::Node& Scene::Node::operator= ( const Scene::Node & aNode )
    {
        mName = aNode.mName;
        mFlags = aNode.mFlags;
        mNodes = aNode.mNodes;
        mScene = aNode.mScene;
        mIterator = 0;
        mGlobalTransform = aNode.mGlobalTransform;
        for ( auto& i : mNodes )
        {
            i.mParent = this;
        }
        SetGlobalTransform ( mGlobalTransform );
        return *this;
    }
    Scene::Node& Scene::Node::operator= ( const Scene::Node && aNode )
    {
        mName = std::move ( aNode.mName );
        mFlags = std::move ( aNode.mFlags );
        mNodes = std::move ( aNode.mNodes );
        mScene = aNode.mScene;
        mIterator = 0;
        mGlobalTransform = aNode.mGlobalTransform;
        for ( auto& i : mNodes )
        {
            i.mParent = this;
        }
        SetGlobalTransform ( mGlobalTransform );
        return *this;
    }
    Scene::Node::Node ( std::initializer_list<Scene::Node> aList ) : mNodes ( aList )
    {
        for ( auto& i : mNodes )
        {
            i.mParent = this;
            i.mScene = mScene;
        }
        SetGlobalTransform ( mGlobalTransform );
    }

    Scene::Node::~Node() = default;

    const std::string& Scene::Node::GetName() const
    {
        return mName;
    }

    void Scene::Node::SetName ( const std::string& aName )
    {
        mName = aName;
    }

    void Scene::Node::Append ( const Scene::Node& aNode )
    {
        mNodes.emplace_back ( aNode );
        mNodes.back().mParent = this;
        mNodes.back().mScene = mScene;
        mNodes.back().SetGlobalTransform ( mNodes.back().mGlobalTransform );
    }

    void Scene::Node::Insert ( size_t aIndex, const Scene::Node& aNode )
    {
        auto node = mNodes.emplace ( mNodes.begin() + aIndex, aNode );
        node->mParent = this;
        node->mScene = mScene;
        node->SetGlobalTransform ( node->mGlobalTransform );
    }

    void Scene::Node::Move ( size_t aIndex, Scene::Node&& aNode )
    {
        auto node = mNodes.emplace ( mNodes.begin() + aIndex, std::move ( aNode ) );
        node->mParent = this;
        node->mScene = mScene;
        node->SetGlobalTransform ( node->mGlobalTransform );
        if ( aNode.GetParent() )
        {
            aNode.GetParent()->Erase ( aNode );
        }
    }

    void Scene::Node::Erase ( std::vector<Node>::size_type aIndex )
    {
        mNodes.erase ( mNodes.begin() + aIndex );
    }

    void Scene::Node::Erase ( const Node& aNode )
    {
        if ( aNode.mParent != this )
        {
            throw std::runtime_error ( "Node's parent does not match this node." );
        }
        mNodes.erase ( mNodes.begin() + aNode.GetIndex() );
    }

    std::vector<Scene::Node>::size_type Scene::Node::GetChildrenCount() const
    {
        return mNodes.size();
    }

    const Scene::Node& Scene::Node::GetChild ( size_t aIndex ) const
    {
        return mNodes.at ( aIndex );
    }

    Scene::Node& Scene::Node::GetChild ( size_t aIndex )
    {
        return const_cast<Scene::Node&> ( static_cast<const Scene::Node&> ( *this ).GetChild ( aIndex ) );
    }

    const Scene::Node& Scene::Node::operator[] ( const std::size_t aIndex ) const
    {
        return mNodes[aIndex];
    }

    Scene::Node& Scene::Node::operator[] ( const std::size_t aIndex )
    {
        return const_cast<Scene::Node&> ( static_cast<const Scene::Node&> ( *this ) [aIndex] );
    }

    const Scene::Node* Scene::Node::GetParent() const
    {
        return mParent;
    }

    Scene::Node* Scene::Node::GetParent()
    {
        return const_cast<Scene::Node*> ( static_cast<const Node&> ( *this ).mParent );
    }

    std::size_t Scene::Node::GetIndex() const
    {
        if ( mParent )
        {
            auto index = std::find_if ( mParent->mNodes.begin(), mParent->mNodes.end(),
                                        [this] ( const Node & node )
            {
                return &node == this;
            } );
            return index - mParent->mNodes.begin();
        }
        else if ( mScene )
        {
            auto index = std::find_if ( mScene->mNodes.begin(), mScene->mNodes.end(),
                                        [this] ( const Node & node )
            {
                return &node == this;
            } );
            return index - mScene->mNodes.begin();
        }
        throw std::runtime_error ( "Node has no parent and thus no assigned index." );
    }

    void Scene::Node::LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        /** @todo (EC++ Item 3) This code is the same as the constant overload,
        but can't easily be implemented in terms of that because of aAction's node parameter
        need to also be const.
        */
        auto node = this;
        aAction ( *node );
        while ( node != mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = &node->mNodes[node->mIterator];
                aAction ( *node );
                prev->mIterator++;
            }
            else
            {
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Scene::Node::LoopTraverseDFSPreOrder (
        const std::function<void ( Node& ) >& aPreamble,
        const std::function<void ( Node& ) >& aPostamble )
    {
        /** @todo (EC++ Item 3) This code is the same as the constant overload,
        but can't easily be implemented in terms of that because of aAction's node parameter
        needs to also be const.
        */
        Node* node = this;
        aPreamble ( *node );
        while ( node != mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = &node->mNodes[node->mIterator];
                aPreamble ( *node );
                prev->mIterator++;
            }
            else
            {
                aPostamble ( *node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Scene::Node::LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        auto node = this;
        aAction ( *node );
        while ( node != mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = &node->mNodes[node->mIterator];
                aAction ( *node );
                ++prev->mIterator;
            }
            else
            {
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Scene::Node::LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        Node* node = this;
        while ( node != mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = &node->mNodes[node->mIterator];
                ++prev->mIterator;
            }
            else
            {
                aAction ( *node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Scene::Node::LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        const Node* node = this;
        while ( node != mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = &node->mNodes[node->mIterator];
                ++prev->mIterator;
            }
            else
            {
                aAction ( *node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Scene::Node::RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        aAction ( *this );
        for ( auto & node : mNodes )
        {
            node.RecursiveTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::Node::RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.RecursiveTraverseDFSPostOrder ( aAction );
        }
        aAction ( *this );
    }

    void Scene::Node::LoopTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
    {
        auto node = this;
        while ( node != nullptr )
        {
            aAction ( *node );
            node = node->mParent;
        }
    }

    void Scene::Node::LoopTraverseAncestors ( const std::function<void ( const Node& ) >& aAction ) const
    {
        auto node = this;
        while ( node != nullptr )
        {
            aAction ( *node );
            node = node->mParent;
        }
    }

    void Scene::Node::RecursiveTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
    {
        aAction ( *this );
        if ( mParent )
        {
            mParent->RecursiveTraverseAncestors ( aAction );
        }
    }

    void Scene::Node::SetFlags ( size_t aFlagBits, bool aEnabled )
    {
        ( aEnabled ) ? mFlags |= aFlagBits : mFlags &= static_cast<uint32_t> ( ~aFlagBits );
    }

    void Scene::Node::SetFlag ( enum Flags aFlag, bool aEnabled )
    {
        mFlags[aFlag] = aEnabled;
    }

    bool Scene::Node::IsFlagEnabled ( enum Flags aFlag ) const
    {
        return mFlags[aFlag];
    }

    const Transform& Scene::Node::GetLocalTransform() const
    {
        return mLocalTransform;
    }

    const Transform& Scene::Node::GetGlobalTransform() const
    {
        return mGlobalTransform;
    }
    void Scene::Node::SetLocalTransform ( const Transform& aTransform )
    {
        mLocalTransform = aTransform;
        LoopTraverseDFSPreOrder (
            [] ( Node & node )
        {
            if ( node.mParent )
            {
                node.mGlobalTransform = node.mParent->mGlobalTransform * node.mLocalTransform;
            }
            else
            {
                node.mGlobalTransform = node.mLocalTransform;
            }
        } );
    }

    void Scene::Node::SetGlobalTransform ( const Transform& aTransform )
    {
        mGlobalTransform = aTransform;
        // Update the Local transform for this node only
        if ( mParent )
        {
            mLocalTransform = mGlobalTransform * mParent->mGlobalTransform.GetInverted();
        }
        else
        {
            mLocalTransform = mGlobalTransform;
        }
        // Then Update the Global transform for all children
        LoopTraverseDFSPreOrder (
            [this] ( Node & node )
        {
            /*! @todo Although this->mLocalTransform has already been computed
                      think about removing the check and let it be recomputed,
                      it may be faster than the branch that needs to run
                      for each node and is false only once.*/
            if ( &node != this )
            {
                if ( node.mParent )
                {
                    node.mGlobalTransform = node.mParent->mGlobalTransform * node.mLocalTransform;
                }
            }
        } );
    }

    Scene::Scene ( std::initializer_list<Scene::Node> aList ) : mNodes ( aList )
    {
        for ( auto& i : mNodes )
        {
            i.mParent = nullptr;
            i.mScene = this;
        }
    }

    Scene::~Scene() = default;

    void Scene::Append ( const Node& aNode )
    {
        mNodes.emplace_back ( aNode );
        mNodes.back().mParent = nullptr;
        mNodes.back().mScene = this;
    }
    void Scene::Insert ( size_t aIndex, const Node& aNode )
    {
        auto node = mNodes.emplace ( mNodes.begin() + aIndex, aNode );
        node->mParent = nullptr;
        node->mScene = this;
    }

    void Scene::Move ( size_t aIndex, Scene::Node&& aNode )
    {
        auto node = mNodes.emplace ( mNodes.begin() + aIndex, std::move ( aNode ) );
        node->mParent = nullptr;
        node->mScene = this;
        node->SetGlobalTransform ( node->mGlobalTransform );
        if ( aNode.GetParent() )
        {
            aNode.GetParent()->Erase ( aNode );
        }
    }

    void Scene::Erase ( std::vector<Node>::size_type aIndex )
    {
        mNodes.erase ( mNodes.begin() + aIndex );
    }

    void Scene::Erase ( const Node& aNode )
    {
        if ( aNode.mParent != nullptr || aNode.mScene != this )
        {
            throw std::runtime_error ( "Node's parent does not match this node." );
        }
        mNodes.erase ( mNodes.begin() + aNode.GetIndex() );
    }

    std::vector<Scene::Node>::size_type Scene::GetChildrenCount() const
    {
        return mNodes.size();
    }

    const Scene::Node& Scene::GetChild ( size_t aIndex ) const
    {
        return mNodes.at ( aIndex );
    }

    Scene::Node& Scene::GetChild ( size_t aIndex )
    {
        return const_cast<Scene::Node&> ( static_cast<const Scene&> ( *this ).GetChild ( aIndex ) );
    }

    const Scene::Node& Scene::operator[] ( const std::size_t aIndex ) const
    {
        return mNodes[aIndex];
    }

    Scene::Node& Scene::operator[] ( const std::size_t aIndex )
    {
        return const_cast<Scene::Node&> ( static_cast<const Scene&> ( *this ) [aIndex] );
    }

    void Scene::LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.LoopTraverseDFSPreOrder ( aAction );
        }
    }
    void Scene::LoopTraverseDFSPreOrder (
        const std::function<void ( Node& ) >& aPreamble,
        const std::function<void ( Node& ) >& aPostamble )
    {
        for ( auto & node : mNodes )
        {
            node.LoopTraverseDFSPreOrder ( aPreamble, aPostamble );
        }
    }
    void Scene::LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& node : mNodes )
        {
            node.LoopTraverseDFSPreOrder ( aAction );
        }
    }
    void Scene::LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.LoopTraverseDFSPostOrder ( aAction );
        }
    }
    void Scene::LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& node : mNodes )
        {
            node.LoopTraverseDFSPostOrder ( aAction );
        }
    }
    void Scene::RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.RecursiveTraverseDFSPreOrder ( aAction );
        }
    }
    void Scene::RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.RecursiveTraverseDFSPostOrder ( aAction );
        }
    }

    std::string Scene::Serialize ( bool aAsBinary ) const
    {
        static SceneBuffer scene_buffer;
        std::unordered_map<const Scene::Node*, NodeBuffer*> node_map;
        LoopTraverseDFSPreOrder (
            [&node_map] ( const Scene::Node & node )
        {
            NodeBuffer* node_buffer;
            auto parent = node_map.find ( node.GetParent() );
            if ( parent != node_map.end() )
            {
                node_buffer = ( *parent ).second->add_node();
            }
            else
            {
                node_buffer = scene_buffer.add_node();
            }
            node_buffer->mutable_local()->mutable_scale()->set_x ( node.GetLocalTransform().GetScale() [0] );
            node_buffer->mutable_local()->mutable_scale()->set_y ( node.GetLocalTransform().GetScale() [1] );
            node_buffer->mutable_local()->mutable_scale()->set_z ( node.GetLocalTransform().GetScale() [2] );
            node_buffer->mutable_local()->mutable_rotation()->set_w ( node.GetLocalTransform().GetRotation() [0] );
            node_buffer->mutable_local()->mutable_rotation()->set_x ( node.GetLocalTransform().GetRotation() [1] );
            node_buffer->mutable_local()->mutable_rotation()->set_y ( node.GetLocalTransform().GetRotation() [2] );
            node_buffer->mutable_local()->mutable_rotation()->set_z ( node.GetLocalTransform().GetRotation() [3] );
            node_buffer->mutable_local()->mutable_translation()->set_x ( node.GetLocalTransform().GetTranslation() [0] );
            node_buffer->mutable_local()->mutable_translation()->set_y ( node.GetLocalTransform().GetTranslation() [1] );
            node_buffer->mutable_local()->mutable_translation()->set_z ( node.GetLocalTransform().GetTranslation() [2] );
            node_map.emplace ( std::make_pair ( &node, node_buffer ) );
        } );
        std::stringstream serialization;
        if ( aAsBinary )
        {
            serialization << "AEONTRE" << '\0';
            scene_buffer.SerializeToOstream ( &serialization );
        }
        else
        {
            std::string text;
            serialization << "AEONTRE\n";
            google::protobuf::TextFormat::Printer printer;
            printer.PrintToString ( scene_buffer, &text );
            serialization << text;
        }
        scene_buffer.Clear();
        return serialization.str();
    }
}
