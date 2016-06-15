/*
Copyright 2014-2016 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <cassert>
#include <algorithm>

namespace AeonGames
{
    /**@todo implement Linear Octrees:
    https://geidav.wordpress.com/2014/08/18/advanced-octrees-2-node-representations/
    */
    class Node;
    Scene::Scene() :
        mName ( "Scene" ),
        mRootNodes(),
        mAllNodes()
    {
    }

    Scene::~Scene()
    {
        for ( auto i = mRootNodes.begin(); i != mRootNodes.end(); ++i )
        {
            ( *i )->mScene = nullptr;
            delete ( *i );
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

    Node* Scene::GetChild ( size_t aIndex ) const
    {
        if ( aIndex < mRootNodes.size() )
        {
            return mRootNodes[aIndex];
        }
        return nullptr;
    }

    void Scene::Update ( double delta )
    {
        for ( auto i = mRootNodes.begin(); i != mRootNodes.end(); ++i )
        {
            ( *i )->LoopTraverseDFSPreOrder ( [delta] ( Node * node )
            {
                if ( node->mFlags[Node::Enabled] )
                {
                    node->Update ( delta );
                }
            } );
        }
    }

    void Scene::LoopTraverseDFSPreOrder ( std::function<void ( Node* ) > aAction )
    {
        for ( auto i = mRootNodes.begin(); i != mRootNodes.end(); ++i )
        {
            ( *i )->LoopTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPreOrder ( std::function<void ( Node* ) > aPreamble, std::function<void ( Node* ) > aPostamble )
    {
        for ( auto i = mRootNodes.begin(); i != mRootNodes.end(); ++i )
        {
            ( *i )->LoopTraverseDFSPreOrder ( aPreamble, aPostamble );
        }
    }

    void Scene::LoopTraverseDFSPreOrder ( std::function<void ( const Node* ) > aAction ) const
    {
        for ( auto i = mRootNodes.cbegin(); i != mRootNodes.cend(); ++i )
        {
            static_cast<const Node*> ( ( *i ) )->LoopTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPostOrder ( std::function<void ( Node* ) > aAction )
    {
        for ( auto i = mRootNodes.begin(); i != mRootNodes.end(); ++i )
        {
            ( *i )->LoopTraverseDFSPostOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPostOrder ( std::function<void ( const Node* ) > aAction ) const
    {
        for ( auto i = mRootNodes.cbegin(); i != mRootNodes.cend(); ++i )
        {
            static_cast<const Node*> ( ( *i ) )->LoopTraverseDFSPostOrder ( aAction );
        }
    }

    void Scene::RecursiveTraverseDFSPreOrder ( std::function<void ( Node* ) > aAction )
    {
        for ( auto i = mRootNodes.begin(); i != mRootNodes.end(); ++i )
        {
            ( *i )->RecursiveTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::RecursiveTraverseDFSPostOrder ( std::function<void ( Node* ) > aAction )
    {
        for ( auto i = mRootNodes.begin(); i != mRootNodes.end(); ++i )
        {
            ( *i )->RecursiveTraverseDFSPostOrder ( aAction );
        }
    }

    bool Scene::InsertNode ( size_t aIndex, Node* aNode )
    {
        // Never append null or this pointers.
        if ( ( aNode != nullptr ) && ( std::find ( mRootNodes.begin(), mRootNodes.end(), aNode ) == mRootNodes.end() ) )
        {
            if ( aNode->mParent != nullptr )
            {
                if ( !aNode->mParent->RemoveNode ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = nullptr;
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
                [this] ( Node * node )
            {
                this->mAllNodes.push_back ( node );
                node->mScene = this;
            } );
            return true;
        }
        return false;
    }

    bool Scene::AddNode ( Node* aNode )
    {
        // Never append null or this pointers.
        if ( ( aNode != nullptr ) && ( std::find ( mRootNodes.begin(), mRootNodes.end(), aNode ) == mRootNodes.end() ) )
        {
            if ( aNode->mParent != nullptr )
            {
                if ( !aNode->mParent->RemoveNode ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = nullptr;
            aNode->mIndex = mRootNodes.size();
            mRootNodes.push_back ( aNode );
            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            aNode->LoopTraverseDFSPreOrder (
                [this] ( Node * node )
            {
                this->mAllNodes.push_back ( node );
                node->mScene = this;
            } );
            return true;
        }
        return false;
    }

    bool Scene::RemoveNode ( Node* aNode )
    {
        assert ( aNode != nullptr );
        // If passed a null or this pointer find SHOULD not find it on release builds.
        /*  While only a single instance should be found and erase does the element shifting
        we're using remove here to do the shifting in order to stablish
        that the erase-remove idiom is what should be used in these situations.*/
        std::vector<Node*>::iterator it = std::remove ( mRootNodes.begin(), mRootNodes.end(), aNode );
        if ( it != mRootNodes.end() )
        {
            mRootNodes.erase ( it );
            for ( auto i = mRootNodes.begin() + aNode->GetIndex(); i != mRootNodes.end(); ++i )
            {
                ( *i )->mIndex = i - mRootNodes.begin();
            }
            aNode->mParent = nullptr;
            aNode->mIndex = Node::kInvalidIndex;
            // Force recalculation of transforms.
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            std::vector<Node*>::iterator it = mAllNodes.end();
            aNode->LoopTraverseDFSPostOrder ( [&it, this] ( Node * node )
            {
                node->mScene = nullptr;
                it = std::remove ( this->mAllNodes.begin(), it, node );
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
