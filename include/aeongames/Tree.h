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
#ifndef AEONGAMES_TREE_H
#define AEONGAMES_TREE_H
#include <cstdint>
#include <string>
#include <vector>
#include <bitset>
#include <functional>
#include <limits>
#include <unordered_map>
#include <tuple>
#include "aeongames/Platform.h"
#include "aeongames/Transform.h"

namespace AeonGames
{
    class Tree
    {
    public:
        class Node
        {
            friend class Tree;
            Node* mParent{};
            Tree* mTree{};
            std::vector<Node> mNodes{};
            /** Tree iteration helper.
                Mutable to allow for constant iterations (EC++ Item 3).*/
            mutable std::vector<Node>::size_type mIterator{ 0 };
        public:
            Node() {}
            Node ( const Node & aNode ) : mParent{}, mNodes{aNode.mNodes}, mIterator{}
            {
                for ( auto& i : mNodes )
                {
                    i.mParent = this;
                    i.mTree = mTree;
                }
            }
            Node ( const Node && aNode ) : mParent{}, mNodes{std::move ( aNode.mNodes ) }, mIterator{}
            {
                for ( auto& i : mNodes )
                {
                    i.mParent = this;
                    i.mTree = mTree;
                }
            }
            Node& operator= ( const Node & aNode )
            {
                mNodes = aNode.mNodes;
                mTree = aNode.mTree;
                mIterator = 0;
                for ( auto& i : mNodes )
                {
                    i.mParent = this;
                }
            }
            Node& operator= ( const Node && aNode )
            {
                mNodes = std::move ( aNode.mNodes );
                mTree = aNode.mTree;
                mIterator = 0;
                for ( auto& i : mNodes )
                {
                    i.mParent = this;
                }
            }
            Node ( std::initializer_list<Node> aList ) : mNodes ( aList )
            {
                for ( auto& i : mNodes )
                {
                    i.mParent = this;
                    i.mTree = mTree;
                }
            }
            ~Node() = default;
            void Append ( const Node& aNode )
            {
                mNodes.emplace_back ( aNode );
                mNodes.back().mParent = this;
                mNodes.back().mTree = mTree;
            }
            void Insert ( size_t aIndex, const Node& aNode )
            {
                auto node = mNodes.emplace ( mNodes.begin() + aIndex, aNode );
                node->mParent = this;
                node->mTree = mTree;
            }
            void Erase ( std::vector<Node>::size_type aIndex )
            {
                mNodes.erase ( mNodes.begin() + aIndex );
            }
            void Erase ( const Node& aNode )
            {
                if ( aNode.mParent != this )
                {
                    throw std::runtime_error ( "Node's parent does not match this node." );
                }
                mNodes.erase ( mNodes.begin() + aNode.GetIndex() );
            }

            std::vector<Node>::size_type GetChildrenCount() const
            {
                return mNodes.size();
            }

            const Node& GetChild ( size_t aIndex ) const
            {
                return mNodes.at ( aIndex );
            }

            Node& GetChild ( size_t aIndex )
            {
                return const_cast<Node&> ( static_cast<const Node&> ( *this ).GetChild ( aIndex ) );
            }

            const Node& operator[] ( const std::size_t aIndex ) const
            {
                return mNodes[aIndex];
            }

            Node& operator[] ( const std::size_t aIndex )
            {
                return const_cast<Node&> ( static_cast<const Node&> ( *this ) [aIndex] );
            }

            const Node* GetParent() const
            {
                return mParent;
            }

            Node* GetParent()
            {
                return const_cast<Node*> ( static_cast<const Node&> ( *this ).mParent );
            }

            std::size_t GetIndex() const
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

            /** Iterative depth first search iteration.
            Iterates all descendants without recursion in pre-order.
            This function guarrantees that parents are processed before their children.
            @param aAction a function, function pointer or function like object to proccess each child node.
            @sa Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPreOrder,Node::RecursiveTraverseDFSPostOrder
            */
            void LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
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

            /** Iterative depth first search iteration.
            Iterates all descendants without recursion in pre-order.
            This function guarrantees that parents are processed before their children.
            This function does the same as the single argument version except that it runs two
            functions, a preamble upon reaching a node and a postamble upon leaving it having run the preambles and postambles
            of all of the node's children.
            @param aPreamble a function, function pointer or function like object to run upon reaching each child node.
            @param aPostamble a function, function pointer or function like object to run upon leaving each child node.
            @sa Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPreOrder,Node::RecursiveTraverseDFSPostOrder
            */
            void LoopTraverseDFSPreOrder (
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

            /** Constant version of LoopTraverseDFSPreOrder. */
            void LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
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

            /** Iterative depth first search iteration.
            Iterates all descendants without recursion in post-order.
            This function guarrantees that children are processed before their parent.
            @param aAction a function, function pointer or function like object to proccess each child node.
            @sa Node::LoopTraverseDFSPreOrder,Node::RecursiveTraverseDFSPreOrder,Node::RecursiveTraverseDFSPostOrder
            */
            void LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
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

            /** Constant version of LoopTraverseDFSPostOrder. */
            void LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
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

            /** Recursive depth first search iteration.
            Iterates all descendants with recursion in pre-order.
            This function guarrantees that parents are processed before their children.
            @param aAction a function, function pointer or function like object to proccess each child node.
            @sa Node::LoopTraverseDFSPreOrder,Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPostOrder
            */
            void RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
            {
                aAction ( *this );
                for ( auto & node : mNodes )
                {
                    node.RecursiveTraverseDFSPreOrder ( aAction );
                }
            }

            /** Recursive depth first search iteration.
            Iterates all descendants with recursion in post-order.
            This function guarrantees that children are processed before their parent.
            @param aAction a function, function pointer or function like object to proccess each child node.
            @sa Node::LoopTraverseDFSPreOrder,Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPreOrder
            */
            void RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
            {
                for ( auto & node : mNodes )
                {
                    node.RecursiveTraverseDFSPostOrder ( aAction );
                }
                aAction ( *this );
            }

            void LoopTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
            {
                auto node = this;
                while ( node != nullptr )
                {
                    aAction ( *node );
                    node = node->mParent;
                }
            }

            void LoopTraverseAncestors ( const std::function<void ( const Node& ) >& aAction ) const
            {
                auto node = this;
                while ( node != nullptr )
                {
                    aAction ( *node );
                    node = node->mParent;
                }
            }
            void RecursiveTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
            {
                aAction ( *this );
                if ( mParent )
                {
                    mParent->RecursiveTraverseAncestors ( aAction );
                }
            }
        };

        Tree ( std::initializer_list<Node> aList ) : mNodes ( aList )
        {
            for ( auto& i : mNodes )
            {
                i.mParent = nullptr;
                i.mTree = this;
            }
        }

        ~Tree() = default;

        void Append ( const Node& aNode )
        {
            mNodes.emplace_back ( aNode );
            mNodes.back().mParent = nullptr;
            mNodes.back().mTree = this;
        }
        void Insert ( size_t aIndex, const Node& aNode )
        {
            auto node = mNodes.emplace ( mNodes.begin() + aIndex, aNode );
            node->mParent = nullptr;
            node->mTree = this;
        }
        void Erase ( std::vector<Node>::size_type aIndex )
        {
            mNodes.erase ( mNodes.begin() + aIndex );
        }

        void Erase ( const Node& aNode )
        {
            if ( aNode.mParent != nullptr || aNode.mTree != this )
            {
                throw std::runtime_error ( "Node's parent does not match this node." );
            }
            mNodes.erase ( mNodes.begin() + aNode.GetIndex() );
        }

        std::vector<Node>::size_type GetChildrenCount() const
        {
            return mNodes.size();
        }

        const Node& GetChild ( size_t aIndex ) const
        {
            return mNodes.at ( aIndex );
        }

        Node& GetChild ( size_t aIndex )
        {
            return const_cast<Node&> ( static_cast<const Tree&> ( *this ).GetChild ( aIndex ) );
        }

        const Node& operator[] ( const std::size_t aIndex ) const
        {
            return mNodes[aIndex];
        }

        Node& operator[] ( const std::size_t aIndex )
        {
            return const_cast<Node&> ( static_cast<const Tree&> ( *this ) [aIndex] );
        }

        /** @copydoc Node::LoopTraverseDFSPreOrder(std::function<void(Node&) > aAction)*/
        void LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
        {
            for ( auto & node : mNodes )
            {
                node.LoopTraverseDFSPreOrder ( aAction );
            }
        }
        /** @copydoc Node::LoopTraverseDFSPreOrder(
            std::function<void(Node*) > aPreamble,
            std::function<void(Node*) > aPostamble)*/
        void LoopTraverseDFSPreOrder (
            const std::function<void ( Node& ) >& aPreamble,
            const std::function<void ( Node& ) >& aPostamble )
        {
            for ( auto & node : mNodes )
            {
                node.LoopTraverseDFSPreOrder ( aPreamble, aPostamble );
            }
        }
        /** @copydoc Node::LoopTraverseDFSPreOrder(std::function<void(const std::shared_ptr<const Node>&) > aAction) const */
        void LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
        {
            for ( const auto& node : mNodes )
            {
                node.LoopTraverseDFSPreOrder ( aAction );
            }
        }
        /** @copydoc Node::LoopTraverseDFSPostOrder(std::function<void(const std::shared_ptr<Node>&) > aAction)*/
        void LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
        {
            for ( auto & node : mNodes )
            {
                node.LoopTraverseDFSPostOrder ( aAction );
            }
        }
        /** @copydoc Node::LoopTraverseDFSPostOrder(std::function<void(const std::shared_ptr<const Node>&) > aAction) const */
        void LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
        {
            for ( const auto& node : mNodes )
            {
                node.LoopTraverseDFSPostOrder ( aAction );
            }
        }
        /** @copydoc Node::RecursiveTraverseDFSPreOrder(std::function<void(const std::shared_ptr<Node>&) > aAction)*/
        void RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
        {
            for ( auto & node : mNodes )
            {
                node.RecursiveTraverseDFSPreOrder ( aAction );
            }
        }
        /** @copydoc Node::RecursiveTraverseDFSPostOrder(std::function<void(const std::shared_ptr<Node>&) > aAction)*/
        void RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
        {
            for ( auto & node : mNodes )
            {
                node.RecursiveTraverseDFSPostOrder ( aAction );
            }
        }
    private:
        std::vector<Node> mNodes{};
    };
}
#endif
