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
#include "aeongames/Tree.h"

namespace AeonGames
{
    Tree::Node::Node() = default;
    Tree::Node::Node ( const Node & aNode ) :
        mParent{},
        mIterator{},
        mFlags{aNode.mFlags},
        mGlobalTransform{aNode.mGlobalTransform},
        mNodes{aNode.mNodes}
    {
        for ( auto& i : mNodes )
        {
            i.mParent = this;
            i.mTree = mTree;
        }
        SetGlobalTransform ( mGlobalTransform );
    }
    Tree::Node::Node ( const Node && aNode ) :
        mParent{}, mIterator{},
        mFlags{std::move ( aNode.mFlags ) },
        mGlobalTransform{aNode.mGlobalTransform},
        mNodes{std::move ( aNode.mNodes ) }
    {
        for ( auto& i : mNodes )
        {
            i.mParent = this;
            i.mTree = mTree;
        }
        SetGlobalTransform ( mGlobalTransform );
    }
    Tree::Node& Tree::Node::operator= ( const Tree::Node & aNode )
    {
        mFlags = aNode.mFlags;
        mNodes = aNode.mNodes;
        mTree = aNode.mTree;
        mIterator = 0;
        mGlobalTransform = aNode.mGlobalTransform;
        for ( auto& i : mNodes )
        {
            i.mParent = this;
        }
        SetGlobalTransform ( mGlobalTransform );
        return *this;
    }
    Tree::Node& Tree::Node::operator= ( const Tree::Node && aNode )
    {
        mFlags = std::move ( aNode.mFlags );
        mNodes = std::move ( aNode.mNodes );
        mTree = aNode.mTree;
        mIterator = 0;
        mGlobalTransform = aNode.mGlobalTransform;
        for ( auto& i : mNodes )
        {
            i.mParent = this;
        }
        SetGlobalTransform ( mGlobalTransform );
        return *this;
    }
    Tree::Node::Node ( std::initializer_list<Tree::Node> aList ) : mNodes ( aList )
    {
        for ( auto& i : mNodes )
        {
            i.mParent = this;
            i.mTree = mTree;
        }
        SetGlobalTransform ( mGlobalTransform );
    }

    Tree::Node::~Node() = default;

    void Tree::Node::Append ( const Tree::Node& aNode )
    {
        mNodes.emplace_back ( aNode );
        mNodes.back().mParent = this;
        mNodes.back().mTree = mTree;
        mNodes.back().SetGlobalTransform ( mNodes.back().mGlobalTransform );
    }

    void Tree::Node::Insert ( size_t aIndex, const Tree::Node& aNode )
    {
        auto node = mNodes.emplace ( mNodes.begin() + aIndex, aNode );
        node->mParent = this;
        node->mTree = mTree;
        node->SetGlobalTransform ( node->mGlobalTransform );
    }

    void Tree::Node::Erase ( std::vector<Node>::size_type aIndex )
    {
        mNodes.erase ( mNodes.begin() + aIndex );
    }

    void Tree::Node::Erase ( const Node& aNode )
    {
        if ( aNode.mParent != this )
        {
            throw std::runtime_error ( "Node's parent does not match this node." );
        }
        mNodes.erase ( mNodes.begin() + aNode.GetIndex() );
    }

    std::vector<Tree::Node>::size_type Tree::Node::GetChildrenCount() const
    {
        return mNodes.size();
    }

    const Tree::Node& Tree::Node::GetChild ( size_t aIndex ) const
    {
        return mNodes.at ( aIndex );
    }

    Tree::Node& Tree::Node::GetChild ( size_t aIndex )
    {
        return const_cast<Tree::Node&> ( static_cast<const Tree::Node&> ( *this ).GetChild ( aIndex ) );
    }

    const Tree::Node& Tree::Node::operator[] ( const std::size_t aIndex ) const
    {
        return mNodes[aIndex];
    }

    Tree::Node& Tree::Node::operator[] ( const std::size_t aIndex )
    {
        return const_cast<Tree::Node&> ( static_cast<const Tree::Node&> ( *this ) [aIndex] );
    }

    const Tree::Node* Tree::Node::GetParent() const
    {
        return mParent;
    }

    Tree::Node* Tree::Node::GetParent()
    {
        return const_cast<Tree::Node*> ( static_cast<const Node&> ( *this ).mParent );
    }

    std::size_t Tree::Node::GetIndex() const
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
        else if ( mTree )
        {
            auto index = std::find_if ( mTree->mNodes.begin(), mTree->mNodes.end(),
                                        [this] ( const Node & node )
            {
                return &node == this;
            } );
            return index - mTree->mNodes.begin();
        }
        throw std::runtime_error ( "Node has no parent and thus no assigned index." );
    }

    void Tree::Node::LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
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

    void Tree::Node::LoopTraverseDFSPreOrder (
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

    void Tree::Node::LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
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

    void Tree::Node::LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
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

    void Tree::Node::LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
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

    void Tree::Node::RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        aAction ( *this );
        for ( auto & node : mNodes )
        {
            node.RecursiveTraverseDFSPreOrder ( aAction );
        }
    }

    void Tree::Node::RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.RecursiveTraverseDFSPostOrder ( aAction );
        }
        aAction ( *this );
    }

    void Tree::Node::LoopTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
    {
        auto node = this;
        while ( node != nullptr )
        {
            aAction ( *node );
            node = node->mParent;
        }
    }

    void Tree::Node::LoopTraverseAncestors ( const std::function<void ( const Node& ) >& aAction ) const
    {
        auto node = this;
        while ( node != nullptr )
        {
            aAction ( *node );
            node = node->mParent;
        }
    }

    void Tree::Node::RecursiveTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
    {
        aAction ( *this );
        if ( mParent )
        {
            mParent->RecursiveTraverseAncestors ( aAction );
        }
    }

    void Tree::Node::SetFlags ( size_t aFlagBits, bool aEnabled )
    {
        ( aEnabled ) ? mFlags |= aFlagBits : mFlags &= static_cast<uint32_t> ( ~aFlagBits );
    }

    void Tree::Node::SetFlag ( enum Flags aFlag, bool aEnabled )
    {
        mFlags[aFlag] = aEnabled;
    }

    bool Tree::Node::IsFlagEnabled ( enum Flags aFlag ) const
    {
        return mFlags[aFlag];
    }

    const Transform& Tree::Node::GetLocalTransform() const
    {
        return mLocalTransform;
    }

    const Transform& Tree::Node::GetGlobalTransform() const
    {
        return mGlobalTransform;
    }
    void Tree::Node::SetLocalTransform ( const Transform& aTransform )
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

    void Tree::Node::SetGlobalTransform ( const Transform& aTransform )
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

    Tree::Tree ( std::initializer_list<Tree::Node> aList ) : mNodes ( aList )
    {
        for ( auto& i : mNodes )
        {
            i.mParent = nullptr;
            i.mTree = this;
        }
    }

    Tree::~Tree() = default;

    void Tree::Append ( const Node& aNode )
    {
        mNodes.emplace_back ( aNode );
        mNodes.back().mParent = nullptr;
        mNodes.back().mTree = this;
    }
    void Tree::Insert ( size_t aIndex, const Node& aNode )
    {
        auto node = mNodes.emplace ( mNodes.begin() + aIndex, aNode );
        node->mParent = nullptr;
        node->mTree = this;
    }
    void Tree::Erase ( std::vector<Node>::size_type aIndex )
    {
        mNodes.erase ( mNodes.begin() + aIndex );
    }

    void Tree::Erase ( const Node& aNode )
    {
        if ( aNode.mParent != nullptr || aNode.mTree != this )
        {
            throw std::runtime_error ( "Node's parent does not match this node." );
        }
        mNodes.erase ( mNodes.begin() + aNode.GetIndex() );
    }

    std::vector<Tree::Node>::size_type Tree::GetChildrenCount() const
    {
        return mNodes.size();
    }

    const Tree::Node& Tree::GetChild ( size_t aIndex ) const
    {
        return mNodes.at ( aIndex );
    }

    Tree::Node& Tree::GetChild ( size_t aIndex )
    {
        return const_cast<Tree::Node&> ( static_cast<const Tree&> ( *this ).GetChild ( aIndex ) );
    }

    const Tree::Node& Tree::operator[] ( const std::size_t aIndex ) const
    {
        return mNodes[aIndex];
    }

    Tree::Node& Tree::operator[] ( const std::size_t aIndex )
    {
        return const_cast<Tree::Node&> ( static_cast<const Tree&> ( *this ) [aIndex] );
    }

    void Tree::LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.LoopTraverseDFSPreOrder ( aAction );
        }
    }
    void Tree::LoopTraverseDFSPreOrder (
        const std::function<void ( Node& ) >& aPreamble,
        const std::function<void ( Node& ) >& aPostamble )
    {
        for ( auto & node : mNodes )
        {
            node.LoopTraverseDFSPreOrder ( aPreamble, aPostamble );
        }
    }
    void Tree::LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& node : mNodes )
        {
            node.LoopTraverseDFSPreOrder ( aAction );
        }
    }
    void Tree::LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.LoopTraverseDFSPostOrder ( aAction );
        }
    }
    void Tree::LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& node : mNodes )
        {
            node.LoopTraverseDFSPostOrder ( aAction );
        }
    }
    void Tree::RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.RecursiveTraverseDFSPreOrder ( aAction );
        }
    }
    void Tree::RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & node : mNodes )
        {
            node.RecursiveTraverseDFSPostOrder ( aAction );
        }
    }
}
