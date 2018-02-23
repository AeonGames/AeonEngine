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
#include <cstring>
#include <cassert>
#include <algorithm>

namespace AeonGames
{
    /**@todo implement Linear Octrees:
    https://geidav.wordpress.com/2014/08/18/advanced-octrees-2-node-representations/
    */
    Scene::Scene() :
        mName{ "Scene" },
        mRootNodes{},
        mAllNodes{}
    {
    }

    Scene::~Scene()
    {
        for ( auto & mRootNode : mRootNodes )
        {
            mRootNode->mScene.reset();
            mRootNode.reset();
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

    size_t Scene::GetNodeCount() const
    {
        return mAllNodes.size();
    }

    size_t Scene::GetChildrenCount() const
    {
        return mRootNodes.size();
    }

    const std::shared_ptr<Node>& Scene::GetChild ( size_t aIndex ) const
    {
        if ( aIndex < mRootNodes.size() )
        {
            return mRootNodes[aIndex];
        }
        return Node::mNullNode;
    }

    void Scene::Update ( const double delta )
    {
        for ( auto & mRootNode : mRootNodes )
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
        for ( auto & mRootNode : mRootNodes )
        {
            mRootNode->LoopTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPreOrder (
        const std::function<void ( Node& ) >& aPreamble,
        const std::function<void ( Node& ) >& aPostamble )
    {
        for ( auto & mRootNode : mRootNodes )
        {
            mRootNode->LoopTraverseDFSPreOrder ( aPreamble, aPostamble );
        }
    }

    void Scene::LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& mRootNode : mRootNodes )
        {
            static_cast<const Node*> ( mRootNode.get() )->LoopTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mRootNodes )
        {
            mRootNode->LoopTraverseDFSPostOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& mRootNode : mRootNodes )
        {
            static_cast<const Node*> ( mRootNode.get() )->LoopTraverseDFSPostOrder ( aAction );
        }
    }

    void Scene::RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mRootNodes )
        {
            mRootNode->RecursiveTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mRootNodes )
        {
            mRootNode->RecursiveTraverseDFSPostOrder ( aAction );
        }
    }

    bool Scene::InsertNode ( size_t aIndex, const std::shared_ptr<Node>& aNode )
    {
        // Never append null or this pointers.
        if ( ( aNode != nullptr ) && ( std::find ( mRootNodes.begin(), mRootNodes.end(), aNode ) == mRootNodes.end() ) )
        {
            if ( auto parent = aNode->mParent.lock() )
            {
                if ( !parent->RemoveNode ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent.reset();
            aNode->mIndex = ( aIndex > mRootNodes.size() ) ? mRootNodes.size() : aIndex;
            mRootNodes.insert ( mRootNodes.begin() + aNode->mIndex, aNode );
            for ( auto i = mRootNodes.begin() + aNode->mIndex + 1; i != mRootNodes.end(); ++i )
            {
                ++ ( *i )->mIndex;
            }

            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            aNode->LoopTraverseDFSPreOrder (
                [this] ( Node & node )
            {
                ///@todo Evaluate wether we really need the mAllNodes vector.
                mAllNodes.push_back ( node.shared_from_this() );
                node.mScene = shared_from_this();
            } );
            return true;
        }
        return false;
    }

    bool Scene::AddNode ( const std::shared_ptr<Node>& aNode )
    {
        // Never append null or this pointers.
        if ( ( aNode != nullptr ) && ( std::find ( mRootNodes.begin(), mRootNodes.end(), aNode ) == mRootNodes.end() ) )
        {
            if ( auto parent = aNode->mParent.lock() )
            {
                if ( !parent->RemoveNode ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent.reset();
            aNode->mIndex = mRootNodes.size();
            mRootNodes.push_back ( aNode );
            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            aNode->LoopTraverseDFSPreOrder (
                [this] ( Node & node )
            {
                mAllNodes.push_back ( node.shared_from_this() );
                node.mScene = shared_from_this();
            } );
            return true;
        }
        return false;
    }

    bool Scene::RemoveNode ( const std::shared_ptr<Node>& aNode )
    {
        if ( aNode == nullptr )
        {
            return false;
        }
        // If passed a null or this pointer find SHOULD not find it on release builds.
        /*  While only a single instance should be found and erase does the element shifting
        we're using remove here to do the shifting in order to stablish
        that the erase-remove idiom is what should be used in these situations.*/
        auto it = std::remove ( mRootNodes.begin(), mRootNodes.end(), aNode );
        if ( it != mRootNodes.end() )
        {
            mRootNodes.erase ( it );
            for ( auto i = mRootNodes.begin() + aNode->GetIndex(); i != mRootNodes.end(); ++i )
            {
                ( *i )->mIndex = i - mRootNodes.begin();
            }
            aNode->mParent.reset();
            aNode->mIndex = Node::kInvalidIndex;
            // Force recalculation of transforms.
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            auto it = mAllNodes.end();
            aNode->LoopTraverseDFSPostOrder ( [&it, this] ( Node & node )
            {
                node.mScene.reset();
                it = std::remove ( this->mAllNodes.begin(), it, node.shared_from_this() );
            } );
            if ( it != mAllNodes.end() )
            {
                mAllNodes.erase ( it, mAllNodes.end() );
            }
            return true;
        }
        return false;
    }
}
