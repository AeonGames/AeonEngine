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
#include "aeongames/Scene.h"
#include "aeongames/Node.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Renderer.h"
#include "ProtoBufHelpers.h"
#include <cstring>
#include <cassert>
#include <algorithm>
#include <sstream>

#include "aeongames/ProtoBufClasses.h"
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
    /**@todo implement Linear Octrees:
    https://geidav.wordpress.com/2014/08/18/advanced-octrees-2-node-representations/
    */
    Scene::Scene() :
        mName{ "Scene" },
        mNodes{}
    {
    }

    Scene::~Scene()
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode = nullptr;
        }
    }

    void Scene::SetName ( const char* aName )
    {
        mName = aName;
    }

    const char* const Scene::GetName() const
    {
        return mName.c_str();
    }

    size_t Scene::GetChildrenCount() const
    {
        return mNodes.size();
    }

    Node* Scene::GetChild ( size_t aIndex ) const
    {
        return mNodes.at ( aIndex );
    }

    size_t Scene::GetChildIndex ( const Node* aNode ) const
    {
        auto index = std::find_if ( mNodes.begin(), mNodes.end(),
                                    [aNode] ( const Node * node )
        {
            return node == aNode;
        } );
        if ( index != mNodes.end() )
        {
            return index - mNodes.begin();
        }
        throw std::runtime_error ( "Node is not a child of this object." );
    }

    const Node& Scene::operator[] ( const std::size_t index ) const
    {
        return * ( mNodes[index] );
    }

    Node& Scene::operator[] ( const std::size_t index )
    {
        return const_cast<Node&> ( static_cast<const Scene&> ( *this ) [index] );
    }

    void Scene::Update ( const double delta )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->LoopTraverseDFSPreOrder ( [delta] ( Node & node )
            {
                if ( node.mFlags[Node::Enabled] )
                {
                    node.Update ( delta );
                }
            } );
        }
    }

    void Scene::LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->LoopTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPreOrder (
        const std::function<void ( Node& ) >& aPreamble,
        const std::function<void ( Node& ) >& aPostamble )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->LoopTraverseDFSPreOrder ( aPreamble, aPostamble );
        }
    }

    void Scene::LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& mRootNode : mNodes )
        {
            static_cast<const Node*> ( mRootNode )->LoopTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->LoopTraverseDFSPostOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& mRootNode : mNodes )
        {
            static_cast<const Node*> ( mRootNode )->LoopTraverseDFSPostOrder ( aAction );
        }
    }

    void Scene::RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->RecursiveTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->RecursiveTraverseDFSPostOrder ( aAction );
        }
    }

    bool Scene::Insert ( size_t aIndex, Node* aNode )
    {
        // Never append null or this pointers.
        if ( ( aNode != nullptr ) && ( std::find ( mNodes.begin(), mNodes.end(), aNode ) == mNodes.end() ) )
        {
            if ( auto parent = aNode->mParent )
            {
                if ( !parent->Remove ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = nullptr;
            if ( aIndex < mNodes.size() )
            {
                mNodes.insert ( mNodes.begin() + aIndex, aNode );
            }
            else
            {
                mNodes.emplace_back ( aNode );
            }
            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            return true;
        }
        return false;
    }

    bool Scene::Add ( Node* aNode )
    {
        // Never append null or this pointers.
        if ( ( aNode != nullptr ) && ( std::find ( mNodes.begin(), mNodes.end(), aNode ) == mNodes.end() ) )
        {
            if ( auto parent = aNode->mParent )
            {
                if ( !parent->Remove ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = nullptr;
            mNodes.emplace_back ( aNode );
            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            return true;
        }
        return false;
    }

    bool Scene::Remove ( Node* aNode )
    {
        if ( aNode == nullptr )
        {
            return false;
        }
        // If passed a null or this pointer find SHOULD not find it on release builds.
        auto it = std::find_if ( mNodes.begin(), mNodes.end(),
                                 [aNode] ( const Node * node )
        {
            return aNode == node;
        } );
        if ( it != mNodes.end() )
        {
            // Force recalculation of transforms.
            aNode->mParent = nullptr;
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            mNodes.erase ( it );
            return true;
        }
        return false;
    }

    bool Scene::RemoveByIndex ( size_t aIndex )
    {
        if ( mNodes.size() < aIndex )
        {
            return false;
        }
        mNodes[aIndex]->mParent = nullptr;
        mNodes[aIndex]->SetLocalTransform ( mNodes[aIndex]->mGlobalTransform );
        mNodes.erase ( mNodes.begin() + aIndex );
        return true;
    }

    bool Scene::Move ( size_t aIndex, Node* aNode )
    {
        if ( !aNode )
        {
            return false;
        }
        if ( aNode->GetParent() )
        {
            aNode->GetParent()->Remove ( aNode );
        }
        Insert ( aIndex, aNode );
        return true;
    }

    std::string Scene::Serialize ( bool aAsBinary ) const
    {
        static SceneBuffer scene_buffer;
        *scene_buffer.mutable_name() = mName;
        std::unordered_map<const Node*, NodeBuffer*> node_map;
        LoopTraverseDFSPreOrder (
            [&node_map] ( const Node & node )
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
            node.Serialize ( *node_buffer );
            node_map.emplace ( std::make_pair ( &node, node_buffer ) );
        } );
        std::stringstream serialization;
        if ( aAsBinary )
        {
            serialization << "AEONSCE" << '\0';
            scene_buffer.SerializeToOstream ( &serialization );
        }
        else
        {
            std::string text;
            serialization << "AEONSCE\n";
            google::protobuf::TextFormat::Printer printer;
            printer.PrintToString ( scene_buffer, &text );
            serialization << text;
        }
        scene_buffer.Clear();
        return serialization.str();
    }
    void Scene::Deserialize ( const std::string& aSerializedScene )
    {
        static SceneBuffer scene_buffer;
        LoadProtoBufObject ( scene_buffer, aSerializedScene.data(), aSerializedScene.size(), "AEONSCE" );
        mName = scene_buffer.name();

        std::unordered_map<const NodeBuffer*, std::tuple<const NodeBuffer*, int, Node*>> node_map;
        for ( auto &i : scene_buffer.node() )
        {
            const NodeBuffer* node = &i;
            node_map[node] = {nullptr, 0, StoreNode ( ConstructNode ( node->type() ) ) };
            Add ( std::get<2> ( node_map[node] ) );
            std::get<2> ( node_map[node] )->Deserialize ( *node );
            while ( node )
            {
                if ( std::get<1> ( node_map[node] ) < node->node().size() )
                {
                    const NodeBuffer* prev = node;
                    node = &node->node ( std::get<1> ( node_map[node] ) );
                    node_map[node] = {prev, 0, StoreNode ( ConstructNode ( node->type() ) ) };
                    std::get<2> ( node_map[prev] )->Add ( std::get<2> ( node_map[node] ) );
                    std::get<2> ( node_map[node] )->Deserialize ( *node );
                    ++std::get<1> ( node_map[prev] );
                }
                else
                {
                    std::get<1> ( node_map[node] ) = 0;
                    node = std::get<0> ( node_map[node] );
                }
            }
        }
        scene_buffer.Clear();
    }
    Node* Scene::StoreNode ( std::unique_ptr<Node> aNode )
    {
        mNodeStorage.emplace_back ( ( aNode ) ? std::move ( aNode ) : std::make_unique<Node>() );
        return mNodeStorage.back().get();
    }
    std::unique_ptr<Node> Scene::DisposeNode ( const Node* aNode )
    {
        std::unique_ptr<Node> result{};
        auto i = std::find_if ( mNodeStorage.begin(), mNodeStorage.end(), [aNode] ( const std::unique_ptr<Node>& node )
        {
            return aNode == node.get();
        } );
        if ( i != mNodeStorage.end() )
        {
            result = std::move ( *i );
            mNodeStorage.erase ( std::remove ( i, mNodeStorage.end(), *i ), mNodeStorage.end() );
        }
        return result;
    }
}
