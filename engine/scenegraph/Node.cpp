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

/*! \file
\brief Implementation file for the Scene Node class.
\author Rodrigo Hernandez.
\copy 2014-2016 Rodrigo Hernandez
*/
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <cassert>
#include <algorithm>
#include "aeongames/Node.h"
#include "aeongames/Scene.h"
#include "aeongames/LogLevel.h"

namespace AeonGames
{

    const size_t Node::kInvalidIndex = std::numeric_limits<size_t>::max(); // Use on VS2013

    Node::Node ( uint32_t aFlags ) :
        mName ( "Node" ),
        mParent ( nullptr ),
        mScene ( nullptr ),
        mLocalTransform(),
        mGlobalTransform(),
        //mAxisAlignedBoundingBox(),
        mNodes(),
        mIndex ( kInvalidIndex ),
        mIterator ( 0 ),
        mFlags ( aFlags ),
        mEntity ( nullptr )
    {
    }

    Node::~Node()
    {
        /* Make sure tree is left in a valid state */
        if ( mParent != nullptr )
        {
            mParent->RemoveNode ( this );
        }
        else if ( mScene != nullptr )
        {
            mScene->RemoveNode ( this );
        }
        for ( std::vector<Node*>::iterator i = mNodes.begin(); i != mNodes.end(); ++i )
        {
            ( *i )->mParent = nullptr;
            ( *i )->mScene = nullptr;
            delete ( *i );
        }
    }

    size_t Node::GetChildrenCount() const
    {
        return mNodes.size();
    }

    Node* Node::GetChild ( size_t aIndex ) const
    {
        if ( aIndex < mNodes.size() )
        {
            return mNodes[aIndex];
        }
        return nullptr;
    }

    Node* Node::GetParent() const
    {
        return mParent;
    }

    size_t Node::GetIndex() const
    {
        return mIndex;
    }

    void Node::AttachEntity ( Entity* aEntity )
    {
        mEntity = aEntity;
    }

    Entity* Node::DettachEntity()
    {
        Entity* entity = mEntity;
        mEntity = nullptr;
        return entity;
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

    void Node::SetLocalTransform ( const Transform& aTransform )
    {
        mLocalTransform = aTransform;
        LoopTraverseDFSPreOrder (
            [] ( Node * node )
        {
            if ( node->mParent != nullptr )
            {
                node->mGlobalTransform = node->mParent->mGlobalTransform * node->mLocalTransform;
            }
            else
            {
                node->mGlobalTransform = node->mLocalTransform;
            }
        } );
    }

    void Node::SetGlobalTransform ( const Transform& aTransform )
    {
        mGlobalTransform = aTransform;
        // Update the Local transform for this node only
        if ( mParent != nullptr )
        {
            mLocalTransform = mGlobalTransform * mParent->mGlobalTransform.GetInverted();
        }
        else
        {
            mLocalTransform = mGlobalTransform;
        }
        // Then Update the Global transform for all children
        LoopTraverseDFSPreOrder (
            [this] ( Node * node )
        {
            /*! @todo Although this->mLocalTransform has already been computed
                      think about removing the check and let it be recomputed,
                      it may be faster than the branch that needs to run
                      for each node and is false only once.*/
            if ( node != this )
            {
                node->mGlobalTransform = node->mParent->mGlobalTransform * node->mLocalTransform;
            }
        } );
    }

    void Node::SetName ( const std::string& aName )
    {
        mName = aName;
    }

    const std::string& Node::GetName() const
    {
        return mName;
    }

    bool Node::InsertNode ( size_t aIndex, Node* aNode )
    {
        // Never append null or this pointers.
        ///@todo std::find might be slower than removing and reinserting an existing node
        if ( ( aNode != nullptr ) && ( aNode != this ) && ( std::find ( mNodes.begin(), mNodes.end(), aNode ) == mNodes.end() ) )
        {
            if ( aNode->mParent != nullptr )
            {
                if ( !aNode->mParent->RemoveNode ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = this;
            aNode->mIndex = aIndex;
            mNodes.insert ( mNodes.begin() + aIndex, aNode );
            for ( auto i = mNodes.begin() + aIndex + 1; i != mNodes.end(); ++i )
            {
                ++ ( *i )->mIndex;
            }
            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            aNode->LoopTraverseDFSPreOrder (
                [this] ( Node * node )
            {
                if ( this->mScene != nullptr )
                {
                    this->mScene->mAllNodes.push_back ( node );
                }
                node->mScene = this->mScene;
            } );
            return true;
        }
        return false;
    }

    bool Node::AddNode ( Node* aNode )
    {
        // Never append null or this pointers.
        if ( ( aNode != nullptr ) && ( aNode != this ) && ( std::find ( mNodes.begin(), mNodes.end(), aNode ) == mNodes.end() ) )
        {
            if ( aNode->mParent != nullptr )
            {
                if ( !aNode->mParent->RemoveNode ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = this;
            aNode->mIndex = mNodes.size();
            mNodes.push_back ( aNode );
            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            aNode->LoopTraverseDFSPreOrder (
                [this] ( Node * node )
            {
                if ( this->mScene != nullptr )
                {
                    this->mScene->mAllNodes.push_back ( node );
                }
                node->mScene = this->mScene;
            } );
            return true;
        }
        return false;
    }

    bool Node::RemoveNode ( Node* aNode )
    {
        assert ( aNode != nullptr );
        assert ( aNode != this );
        // If passed a null or this pointer find SHOULD not find it on release builds.
        /*  While only a single instance should be found and erase does the element shifting
            we're using remove here to do the shifting in order to stablish
            that the erase-remove idiom is what should be used in these situations.*/
        std::vector<Node*>::iterator it = std::remove ( mNodes.begin(), mNodes.end(), aNode );
        if ( it != mNodes.end() )
        {
            mNodes.erase ( it );
            for ( auto i = mNodes.begin() + aNode->GetIndex(); i != mNodes.end(); ++i )
            {
                ( *i )->mIndex = i - mNodes.begin();
            }
            aNode->mParent = nullptr;
            aNode->mIndex = Node::kInvalidIndex;
            // Force recalculation of transforms.
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            // Remove node from Scene
            if ( mScene != nullptr )
            {
                std::vector<Node*>::iterator it = mScene->mAllNodes.end();
                aNode->LoopTraverseDFSPostOrder ( [&it, this] ( Node * node )
                {
                    node->mScene = nullptr;
                    it = std::remove ( this->mScene->mAllNodes.begin(), it, node );
                } );
                if ( it != mScene->mAllNodes.end() )
                {
                    mScene->mAllNodes.erase ( it, mScene->mAllNodes.end() );
                }
            }
            return true;
        }
        return false;
    }

    void Node::LoopTraverseDFSPreOrder ( std::function<void ( Node* ) > aAction )
    {
        /** @todo (EC++ Item 3) This code is the same as the constant overload,
        but can't easily be implemented in terms of that because of aAction's node parameter
        need to also be const.
        */
        Node* node = this;
        aAction ( node );
        while ( node != this->mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                Node* prev = node;
                node = node->mNodes[node->mIterator];
                aAction ( node );
                prev->mIterator++;
            }
            else
            {
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Node::LoopTraverseDFSPreOrder ( std::function<void ( Node* ) > aPreamble, std::function<void ( Node* ) > aPostamble )
    {
        /** @todo (EC++ Item 3) This code is the same as the constant overload,
        but can't easily be implemented in terms of that because of aAction's node parameter
        need to also be const.
        */
        Node* node = this;
        aPreamble ( node );
        while ( node != this->mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                Node* prev = node;
                node = node->mNodes[node->mIterator];
                aPreamble ( node );
                prev->mIterator++;
            }
            else
            {
                aPostamble ( node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Node::LoopTraverseDFSPreOrder ( std::function<void ( const Node* ) > aAction ) const
    {
        const Node* node = this;
        aAction ( node );
        while ( node != this->mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                const Node* prev = node;
                node = node->mNodes[node->mIterator];
                aAction ( node );
                prev->mIterator++;
            }
            else
            {
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Node::LoopTraverseDFSPostOrder ( std::function<void ( Node* ) > aAction )
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        Node* node = this;
        while ( node != this->mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                Node* prev = node;
                node = node->mNodes[node->mIterator];
                prev->mIterator++;
            }
            else
            {
                aAction ( node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Node::LoopTraverseDFSPostOrder ( std::function<void ( const Node* ) > aAction ) const
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        const Node* node = this;
        while ( node != this->mParent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                const Node* prev = node;
                node = node->mNodes[node->mIterator];
                prev->mIterator++;
            }
            else
            {
                aAction ( node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent;
            }
        }
    }

    void Node::RecursiveTraverseDFSPostOrder ( std::function<void ( Node* ) > aAction )
    {
        for ( std::vector<Node*>::iterator i = mNodes.begin(); i != mNodes.end(); ++i )
        {
            ( *i )->RecursiveTraverseDFSPostOrder ( aAction );
        }
        aAction ( this );
    }

    void Node::RecursiveTraverseDFSPreOrder ( std::function<void ( Node* ) > aAction )
    {
        aAction ( this );
        for ( std::vector<Node*>::iterator i = mNodes.begin(); i != mNodes.end(); ++i )
        {
            ( *i )->RecursiveTraverseDFSPreOrder ( aAction );
        }
    }

    void Node::LoopTraverseAncestors ( std::function<void ( Node* ) > aAction )
    {
        Node* node = this;
        while ( node != nullptr )
        {
            aAction ( node );
            node = node->mParent;
        }
    }

    void Node::LoopTraverseAncestors ( std::function<void ( const Node* ) > aAction ) const
    {
        const Node* node = this;
        while ( node != nullptr )
        {
            aAction ( node );
            node = node->mParent;
        }
    }

    void Node::RecursiveTraverseAncestors ( std::function<void ( Node* ) > aAction )
    {
        aAction ( this );
        if ( mParent != nullptr )
        {
            mParent->RecursiveTraverseAncestors ( aAction );
        }
    }

    void Node::Update ( const double delta )
    {
    }
}
