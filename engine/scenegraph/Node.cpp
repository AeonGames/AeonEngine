/*
Copyright (C) 2014-2017 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ModelInstance.h"

namespace AeonGames
{

    const std::shared_ptr<Node> Node::mNullNode{};
    const size_t Node::kInvalidIndex = std::numeric_limits<size_t>::max(); // Use on VS2013

    Node::Node ( uint32_t aFlags ) :
        mName ( "Node" ),
        mParent{},
        mScene{},
        mLocalTransform(),
        mGlobalTransform(),
        mNodes(),
        mIndex ( kInvalidIndex ),
        mIterator ( 0 ),
        mFlags ( aFlags )
    {
    }

    Node::~Node()
    {
        /* Make sure tree is left in a valid state */
        if ( auto parent = mParent.lock() )
        {
            if ( !parent->RemoveNode ( shared_from_this() ) )
            {
                std::cerr << "Remove Node Failed" << std::endl;
            }
        }
        else if ( auto scene = mScene.lock() )
        {
            if ( !scene->RemoveNode ( shared_from_this() ) )
            {
                std::cerr << "Remove Node Failed" << std::endl;
            }
        }
        for ( auto & mNode : mNodes )
        {
            mNode->mParent.reset();
            mNode->mScene.reset();
            mNode.reset();
        }
    }

    size_t Node::GetChildrenCount() const
    {
        return mNodes.size();
    }

    const std::shared_ptr<Node>& Node::GetChild ( size_t aIndex ) const
    {
        if ( aIndex < mNodes.size() )
        {
            return mNodes[aIndex];
        }
        return mNullNode;
    }

    const std::shared_ptr<Node> Node::GetParent() const
    {
        return mParent.lock();
    }

    size_t Node::GetIndex() const
    {
        return mIndex;
    }

    const std::shared_ptr<ModelInstance>& Node::GetModelInstance() const
    {
        return mModelInstance;
    }

    void Node::SetModelInstance ( const std::shared_ptr<ModelInstance>& aModelInstance )
    {
        mModelInstance = aModelInstance;
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
            [] ( const std::shared_ptr<Node>& node )
        {
            if ( auto parent = node->mParent.lock() )
            {
                parent->mGlobalTransform * node->mLocalTransform;
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
        if ( auto parent = mParent.lock() )
        {
            mLocalTransform = mGlobalTransform * parent->mGlobalTransform.GetInverted();
        }
        else
        {
            mLocalTransform = mGlobalTransform;
        }
        // Then Update the Global transform for all children
        LoopTraverseDFSPreOrder (
            [this] ( const std::shared_ptr<Node>& node )
        {
            /*! @todo Although this->mLocalTransform has already been computed
                      think about removing the check and let it be recomputed,
                      it may be faster than the branch that needs to run
                      for each node and is false only once.*/
            if ( node.get() != this )
            {
                if ( auto parent = node->mParent.lock() )
                {
                    node->mGlobalTransform = parent->mGlobalTransform * node->mLocalTransform;
                }
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

    bool Node::InsertNode ( size_t aIndex, const std::shared_ptr<Node>& aNode )
    {
        // Never append null or this pointers.
        ///@todo std::find might be slower than removing and reinserting an existing node
        if ( ( aNode != nullptr ) && ( aNode.get() != this ) && ( std::find ( mNodes.begin(), mNodes.end(), aNode ) == mNodes.end() ) )
        {
            if ( auto parent = aNode->mParent.lock() )
            {
                if ( !parent->RemoveNode ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = shared_from_this();
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
                [this] ( const std::shared_ptr<Node>& node )
            {
                if ( auto scene = mScene.lock() )
                {
                    scene->mAllNodes.push_back ( node );
                }
                node->mScene = this->mScene;
            } );
            return true;
        }
        return false;
    }

    bool Node::AddNode ( const std::shared_ptr<Node>& aNode )
    {
        // Never append null or this pointers.
        if ( ( aNode != nullptr ) && ( aNode.get() != this ) && ( std::find ( mNodes.begin(), mNodes.end(), aNode ) == mNodes.end() ) )
        {
            if ( auto parent = aNode->mParent.lock() )
            {
                if ( !parent->RemoveNode ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = shared_from_this();
            aNode->mIndex = mNodes.size();
            mNodes.push_back ( aNode );
            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            aNode->LoopTraverseDFSPreOrder (
                [this] ( const std::shared_ptr<Node>& node )
            {
                if ( auto scene = mScene.lock() )
                {
                    scene->mAllNodes.push_back ( node );
                }
                node->mScene = mScene;
            } );
            return true;
        }
        return false;
    }

    bool Node::RemoveNode ( const std::shared_ptr<Node>& aNode )
    {
        assert ( aNode != nullptr );
        assert ( aNode.get() != this );
        // If passed a null or this pointer find SHOULD not find it on release builds.
        /*  While only a single instance should be found and erase does the element shifting
            we're using remove here to do the shifting in order to stablish
            that the erase-remove idiom is what should be used in these situations.*/
        auto it = std::remove ( mNodes.begin(), mNodes.end(), aNode );
        if ( it != mNodes.end() )
        {
            mNodes.erase ( it );
            for ( auto i = mNodes.begin() + aNode->GetIndex(); i != mNodes.end(); ++i )
            {
                ( *i )->mIndex = i - mNodes.begin();
            }
            aNode->mParent.reset();
            aNode->mIndex = Node::kInvalidIndex;
            // Force recalculation of transforms.
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            // Remove node from Scene
            if ( auto scene = mScene.lock() )
            {
                auto it = scene->mAllNodes.end();
                aNode->LoopTraverseDFSPostOrder ( [&scene, &it, this] ( const std::shared_ptr<Node>& node )
                {
                    node->mScene.reset();
                    it = std::remove ( scene->mAllNodes.begin(), it, node );
                } );
                if ( it != scene->mAllNodes.end() )
                {
                    scene->mAllNodes.erase ( it, scene->mAllNodes.end() );
                }
            }
            return true;
        }
        return false;
    }

    void Node::LoopTraverseDFSPreOrder ( std::function<void ( const std::shared_ptr<Node>& ) > aAction )
    {
        /** @todo (EC++ Item 3) This code is the same as the constant overload,
        but can't easily be implemented in terms of that because of aAction's node parameter
        need to also be const.
        */
        auto node = shared_from_this();
        aAction ( node );
        auto parent = mParent.lock();
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
                aAction ( node );
                prev->mIterator++;
            }
            else
            {
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock();
            }
        }
    }

    void Node::LoopTraverseDFSPreOrder ( std::function<void ( const std::shared_ptr<Node>& ) > aPreamble, std::function<void ( const std::shared_ptr<Node>& ) > aPostamble )
    {
        /** @todo (EC++ Item 3) This code is the same as the constant overload,
        but can't easily be implemented in terms of that because of aAction's node parameter
        need to also be const.
        */
        auto node = shared_from_this();
        aPreamble ( node );
        auto parent = mParent.lock();
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
                aPreamble ( node );
                prev->mIterator++;
            }
            else
            {
                aPostamble ( node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock();
            }
        }
    }

    void Node::LoopTraverseDFSPreOrder ( std::function<void ( const std::shared_ptr<const Node>& ) > aAction ) const
    {
        auto node = shared_from_this();
        aAction ( node );
        auto parent = mParent.lock();
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
                aAction ( node );
                prev->mIterator++;
            }
            else
            {
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock();
            }
        }
    }

    void Node::LoopTraverseDFSPostOrder ( std::function<void ( const std::shared_ptr<Node>& ) > aAction )
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        auto node = shared_from_this();
        auto parent = mParent.lock();
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
                prev->mIterator++;
            }
            else
            {
                aAction ( node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock();
            }
        }
    }

    void Node::LoopTraverseDFSPostOrder ( std::function<void ( const std::shared_ptr<const Node>& ) > aAction ) const
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        auto node = shared_from_this();
        auto parent = mParent.lock();
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
                prev->mIterator++;
            }
            else
            {
                aAction ( node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock();
            }
        }
    }

    void Node::RecursiveTraverseDFSPostOrder ( std::function<void ( const std::shared_ptr<Node>& ) > aAction )
    {
        for ( auto & mNode : mNodes )
        {
            mNode->RecursiveTraverseDFSPostOrder ( aAction );
        }
        aAction ( shared_from_this() );
    }

    void Node::RecursiveTraverseDFSPreOrder ( std::function<void ( const std::shared_ptr<Node>& ) > aAction )
    {
        aAction ( shared_from_this() );
        for ( auto & mNode : mNodes )
        {
            mNode->RecursiveTraverseDFSPreOrder ( aAction );
        }
    }

    void Node::LoopTraverseAncestors ( std::function<void ( const std::shared_ptr<Node>& ) > aAction )
    {
        auto node = shared_from_this();
        while ( node != nullptr )
        {
            aAction ( node );
            node = node->mParent.lock();
        }
    }

    void Node::LoopTraverseAncestors ( std::function<void ( const std::shared_ptr<const Node>& ) > aAction ) const
    {
        auto node = shared_from_this();
        while ( node != nullptr )
        {
            aAction ( node );
            node = node->mParent.lock();
        }
    }

    void Node::RecursiveTraverseAncestors ( std::function<void ( const std::shared_ptr<Node>& ) > aAction )
    {
        aAction ( shared_from_this() );
        if ( auto parent = mParent.lock() )
        {
            parent->RecursiveTraverseAncestors ( aAction );
        }
    }

    void Node::Update ( const double delta )
    {
    }
}
