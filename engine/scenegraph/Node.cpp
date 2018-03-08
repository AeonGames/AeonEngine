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

/*! \file
\brief Implementation file for the Scene Node class.
\author Rodrigo Hernandez.
\copy 2014-2018 Rodrigo Hernandez
*/

#include <cassert>
#include <algorithm>
#include "aeongames/Node.h"
#include "aeongames/Scene.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Model.h"
#include "aeongames/ModelInstance.h"
#include "aeongames/AABB.h"
#include "aeongames/Component.h"

namespace AeonGames
{
    const std::shared_ptr<Node> Node::mNullNode{};
    const size_t Node::kInvalidIndex = std::numeric_limits<size_t>::max(); // Use on VS2013

    Node::Node ( uint32_t aFlags ) :
        mName ( "Node" ),
        mParent{},
        mLocalTransform(),
        mGlobalTransform(),
        mNodes(),
        mFlags ( aFlags )
    {
    }

    Node::~Node()
    {
        /* Make sure tree is left in a valid state */
        if ( auto parent = mParent.lock() )
        {
            try
            {
                if ( !parent->RemoveNode ( shared_from_this() ) )
                {
                    std::cerr << "Remove Node Failed" << std::endl;
                }
            }
            catch ( std::bad_weak_ptr e )
            {
                std::cerr << "Remove Node Failed " << e.what() << std::endl;
            }
        }
        for ( auto & mNode : mNodes )
        {
            mNode->mParent.reset();
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
        auto parent = mParent.lock();
        if ( parent )
        {
            auto index = std::find_if ( parent->mNodes.begin(), parent->mNodes.end(),
                                        [this] ( const std::shared_ptr<Node>& node )
            {
                return node.get() == this;
            } );
            return index - parent->mNodes.begin();
        }
        return kInvalidIndex;
    }
    const Component* Node::GetComponent ( std::size_t aComponentId ) const
    {
        auto i = mComponents.Find ( aComponentId );
        if ( i != mComponents.end() )
        {
            return ( *i ).get();
        }
        return nullptr;
    }

    Component* Node::GetComponent ( std::size_t aComponentId )
    {
        // EC++ Item 3
        return const_cast<Component*> ( static_cast<const Node&> ( *this ).GetComponent ( aComponentId ) );
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

    const AABB Node::GetLocalAABB() const
    {
        return AABB{};
    }

    const AABB Node::GetGlobalAABB() const
    {
        ///@todo Remove dependency on ModelInstance::TypeId
        const auto* model_instance = reinterpret_cast<const ModelInstance*> ( GetComponent ( ModelInstance::TypeId ) );
        return ( model_instance ) ? mGlobalTransform * model_instance->GetModel()->GetCenterRadii() : AABB{};
    }

    void Node::SetLocalTransform ( const Transform& aTransform )
    {
        mLocalTransform = aTransform;
        LoopTraverseDFSPreOrder (
            [] ( Node & node )
        {
            if ( auto parent = node.mParent.lock() )
            {
                node.mGlobalTransform = parent->mGlobalTransform * node.mLocalTransform;
            }
            else
            {
                node.mGlobalTransform = node.mLocalTransform;
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
            [this] ( Node & node )
        {
            /*! @todo Although this->mLocalTransform has already been computed
                      think about removing the check and let it be recomputed,
                      it may be faster than the branch that needs to run
                      for each node and is false only once.*/
            if ( &node != this )
            {
                if ( auto parent = node.mParent.lock() )
                {
                    node.mGlobalTransform = parent->mGlobalTransform * node.mLocalTransform;
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
            mNodes.push_back ( aNode );
            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            return true;
        }
        return false;
    }

    bool Node::RemoveNode ( const std::shared_ptr<Node>& aNode )
    {
        return RemoveNode ( aNode.get() );
    }

    bool Node::RemoveNode ( Node* aNode )
    {
        assert ( aNode != nullptr );
        assert ( aNode != this );
        // If passed a null or this pointer find SHOULD not find it on release builds.
        auto it = std::find_if (
                      mNodes.begin(),
                      mNodes.end(),
                      [aNode] ( const std::shared_ptr<Node>& node )
        {
            return aNode == node.get();
        } );
        if ( it != mNodes.end() )
        {
            aNode->mParent.reset();
            // Force recalculation of transforms.
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            mNodes.erase ( it );
            return true;
        }
        return false;
    }

    void Node::LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        /** @todo (EC++ Item 3) This code is the same as the constant overload,
        but can't easily be implemented in terms of that because of aAction's node parameter
        need to also be const.
        */
        auto node = this;
        aAction ( *node );
        auto parent = mParent.lock().get();
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator].get();
                aAction ( *node );
                prev->mIterator++;
            }
            else
            {
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock().get();
            }
        }
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
        auto parent = mParent.lock().get();
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator].get();
                aPreamble ( *node );
                prev->mIterator++;
            }
            else
            {
                aPostamble ( *node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock().get();
            }
        }
    }

    void Node::LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        auto node = this;
        aAction ( *node );
        auto parent = mParent.lock().get();
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator].get();
                aAction ( *node );
                ++prev->mIterator;
            }
            else
            {
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock().get();
            }
        }
    }

    void Node::LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        auto node = this;
        auto parent = mParent.lock().get();
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
                aAction ( *node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock().get();
            }
        }
    }

    void Node::LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        auto node = this;
        auto parent = mParent.lock().get();
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
                aAction ( *node );
                node->mIterator = 0; // Reset counter for next traversal.
                node = node->mParent.lock().get();
            }
        }
    }

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
            node = node->mParent.lock().get();
        }
    }

    void Node::LoopTraverseAncestors ( const std::function<void ( const Node& ) >& aAction ) const
    {
        auto node = this;
        while ( node != nullptr )
        {
            aAction ( *node );
            node = node->mParent.lock().get();
        }
    }

    void Node::RecursiveTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
    {
        aAction ( *this );
        if ( auto parent = mParent.lock().get() )
        {
            parent->RecursiveTraverseAncestors ( aAction );
        }
    }

    void Node::AttachComponent (
        std::size_t aId,
        const std::vector<std::size_t>& aDependencies,
        const std::shared_ptr<Component>& aComponent )
    {
        mComponents.Insert ( DependencyMap<std::size_t, std::shared_ptr<Component>>::triple {aId, aDependencies, aComponent} );
    }

    void Node::DettachComponent ( std::size_t aId )
    {
        mComponents.Erase ( aId );
    }

    void Node::Update ( const double delta )
    {
        for ( auto& i : mComponents )
        {
            i->Update ( *this, delta );
        }
    }
}
