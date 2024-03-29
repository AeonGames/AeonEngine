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
#include "aeongames/AeonEngine.h"
#include "aeongames/Scene.h"
#include "aeongames/Node.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Renderer.h"
#include "aeongames/ProtoBufHelpers.h"
#include <cstring>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <variant>

#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include <google/protobuf/text_format.h>
#include "scene.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/Resource.h"

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
            Remove ( mRootNode.get() );
        }
    }

    void Scene::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }
    void Scene::Load ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        try
        {
            Load ( buffer.data(), buffer.size() );
        }
        catch ( ... )
        {
            throw;
        }
    }
    void Scene::Load ( const void* aBuffer, size_t aBufferSize )
    {
        Deserialize ( {reinterpret_cast<const char*> ( aBuffer ), aBufferSize} );
    }

    void Scene::SetName ( const char* aName )
    {
        mName = aName;
    }

    const char* const Scene::GetName() const
    {
        return mName.c_str();
    }

    void Scene::SetCamera ( Node* aNode )
    {
        mCamera = aNode;
    }

    void Scene::SetCamera ( uint32_t aNodeId )
    {
        SetCamera ( Find ( [aNodeId] ( const Node & aNode ) -> bool { return aNode.GetId() == aNodeId; } ) );
    }

    void Scene::SetCamera ( const std::string& aNodeName )
    {
        SetCamera ( crc32i ( aNodeName.c_str(), aNodeName.size() ) );
    }

    const Node* Scene::GetCamera() const
    {
        return mCamera;
    }

    void Scene::SetFieldOfView ( float aFieldOfView )
    {
        mFieldOfView = aFieldOfView;
    }

    void Scene::SetNear ( float aNear )
    {
        mNear = aNear;
    }

    void Scene::SetFar ( float aFar )
    {
        mFar = aFar;
    }

    float Scene::GetFieldOfView() const
    {
        return mFieldOfView;
    }
    float Scene::GetNear() const
    {
        return mNear;
    }
    float Scene::GetFar() const
    {
        return mFar;
    }

    void Scene::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mViewMatrix = aMatrix;
    }

    const Matrix4x4& Scene::GetViewMatrix() const
    {
        return mViewMatrix;
    }

    size_t Scene::GetChildrenCount() const
    {
        return mNodes.size();
    }

    Node* Scene::GetChild ( size_t aIndex ) const
    {
        return mNodes.at ( aIndex ).get();
    }

    size_t Scene::GetChildIndex ( const Node* aNode ) const
    {
        auto index = std::find_if ( mNodes.begin(), mNodes.end(),
                                    [aNode] ( const std::unique_ptr<Node>& node )
        {
            return node.get() == aNode;
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
        LoopTraverseDFSPreOrder ( [delta] ( Node & aNode )
        {
            aNode.Update ( delta );
        } );
    }

    void Scene::BroadcastMessage ( uint32_t aMessageType, const void* aMessageData )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->LoopTraverseDFSPreOrder ( [aMessageType, aMessageData] ( Node & node )
            {
                if ( node.mFlags[Node::Enabled] )
                {
                    node.ProcessMessage ( aMessageType, aMessageData );
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
            static_cast<const Node*> ( mRootNode.get() )->LoopTraverseDFSPreOrder ( aAction );
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
            static_cast<const Node*> ( mRootNode.get() )->LoopTraverseDFSPostOrder ( aAction );
        }
    }

    Node* Scene::Find ( const std::function<bool ( const Node& ) >& aUnaryPredicate ) const
    {
        for ( const auto& mRootNode : mNodes )
        {
            if ( Node* node = static_cast<Node*> ( mRootNode.get() )->Find ( aUnaryPredicate ) )
            {
                return node;
            }
        }
        return nullptr;
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

    Node* Scene::Insert ( size_t aIndex, std::unique_ptr<Node> aNode )
    {
        // Never append null pointers.
        if ( aNode == nullptr )
        {
            return nullptr;
        }
        std::visit ( [&aNode] ( auto&& parent )
        {
            if ( parent != nullptr )
            {
                if ( !parent->Remove ( aNode.get() ) )
                {
                    std::cout << LogLevel::Warning << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
        },
        aNode->mParent );
        aNode->mParent = this;
        std::vector<std::unique_ptr<Node>>::iterator inserted_node;
        if ( aIndex < mNodes.size() )
        {
            inserted_node = mNodes.insert ( mNodes.begin() + aIndex, std::move ( aNode ) );
        }
        else
        {
            inserted_node = mNodes.insert ( mNodes.end(), std::move ( aNode ) );
        }
        // Force a recalculation of the LOCAL transform
        // by setting the GLOBAL transform to itself.
        ( *inserted_node )->SetGlobalTransform ( ( *inserted_node )->mGlobalTransform );
        return ( *inserted_node ).get();
    }

    Node* Scene::Add ( std::unique_ptr<Node> aNode )
    {
        // Never append null pointers.
        if ( aNode == nullptr )
        {
            return nullptr;
        }
        std::visit ( [&aNode] ( auto&& parent )
        {
            if ( parent != nullptr )
            {
                if ( !parent->Remove ( aNode.get() ) )
                {
                    std::cout << LogLevel::Warning << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
        },
        aNode->mParent );
        aNode->mParent = this;
        mNodes.emplace_back ( std::move ( aNode ) );
        // Force a recalculation of the LOCAL transform
        // by setting the GLOBAL transform to itself.
        mNodes.back()->SetGlobalTransform ( mNodes.back()->mGlobalTransform );
        return mNodes.back().get();
    }

    std::unique_ptr<Node> Scene::Remove ( Node* aNode )
    {
        if ( aNode == nullptr )
        {
            return nullptr;
        }
        // If passed a null or this pointer find SHOULD not find it on release builds.
        auto it = std::find_if ( mNodes.begin(), mNodes.end(),
                                 [aNode] ( const std::unique_ptr<Node>& node )
        {
            return aNode == node.get();
        } );
        if ( it != mNodes.end() )
        {
            // Force recalculation of transforms.
            aNode->mParent = static_cast<Node*> ( nullptr );
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            std::unique_ptr<Node> removed_node{std::move ( * ( it ) ) };
            mNodes.erase ( it );
            return removed_node;
        }
        return nullptr;
    }

    std::unique_ptr<Node> Scene::RemoveByIndex ( size_t aIndex )
    {
        if ( aIndex >= mNodes.size() )
        {
            return nullptr;
        }
        mNodes[aIndex]->mParent = static_cast<Node*> ( nullptr );
        mNodes[aIndex]->SetLocalTransform ( mNodes[aIndex]->mGlobalTransform );
        auto it = mNodes.begin() + aIndex;
        std::unique_ptr<Node> removed_node{std::move ( * ( it ) ) };
        mNodes.erase ( it );
        return removed_node;
    }

    std::string Scene::Serialize ( bool aAsBinary ) const
    {
        static SceneMsg scene_buffer;
        *scene_buffer.mutable_name() = mName;
        if ( mCamera )
        {
            *scene_buffer.mutable_camera()->mutable_node() = mCamera->GetName();
            scene_buffer.mutable_camera()->set_field_of_view ( mFieldOfView );
            scene_buffer.mutable_camera()->set_near_plane ( mNear );
            scene_buffer.mutable_camera()->set_far_plane ( mFar );
        }
        std::unordered_map<const Node*, NodeMsg*> node_map;
        LoopTraverseDFSPreOrder (
            [&node_map] ( const Node & node )
        {
            NodeMsg* node_buffer;
            auto parent = node_map.find ( GetNodePtr ( node.GetParent() ) );
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
        static std::mutex m{};
        static SceneMsg scene_buffer{};
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject ( scene_buffer, aSerializedScene.data(), aSerializedScene.size(), "AEONSCE"_mgk );
        mName = scene_buffer.name();

        std::unordered_map<const NodeMsg*, std::tuple<const NodeMsg*, int, Node*>> node_map;
        for ( auto &i : scene_buffer.node() )
        {
            const NodeMsg* node = &i;
            node_map[node] = std::tuple<const NodeMsg*, int, Node*> {nullptr, 0, Add ( std::make_unique<Node>() ) };
            std::get<2> ( node_map[node] )->Deserialize ( *node );
            while ( node )
            {
                if ( std::get<1> ( node_map[node] ) < node->node().size() )
                {
                    const NodeMsg* prev = node;
                    node = &node->node ( std::get<1> ( node_map[node] ) );
                    node_map[node] = std::tuple<const NodeMsg*, int, Node*> {prev, 0, std::get<2> ( node_map[prev] )->Add ( std::make_unique<Node>() ) };
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
        SetCamera ( scene_buffer.camera().node() );
        mFieldOfView = scene_buffer.camera().field_of_view();
        mFieldOfView = ( mFieldOfView == 0.0f ) ? 60.0f : mFieldOfView;
        mNear = scene_buffer.camera().near_plane();
        mNear = ( mNear == 0.0f ) ? 1.0f : mNear;
        mFar = scene_buffer.camera().far_plane();
        mFar = ( mFar == 0.0f ) ? 1600.0f : mFar;
        if ( mCamera )
        {
            mViewMatrix = mCamera->GetGlobalTransform().GetInvertedMatrix();
        }
        scene_buffer.Clear();
    }
}
