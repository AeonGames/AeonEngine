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
        public:
            DLL Node();
            DLL Node ( const Node & aNode );
            DLL Node ( const Node && aNode );
            DLL Node& operator= ( const Node & aNode );
            DLL Node& operator= ( const Node && aNode );
            DLL Node ( std::initializer_list<Node> aList );
            DLL ~Node();
            ///@name Tree Functions
            ///@{
            /** Append a child node at the end of the child vector. */
            DLL void Append ( const Node& aNode );
            /** Insert a child node at the specified index on the child vector. */
            DLL void Insert ( size_t aIndex, const Node& aNode );
            /** Reparent a node at the specified index on this node's child vector. */
            DLL void Move ( size_t aIndex, Node&& aNode );
            /** Delete a child node given its index.
             * This function deletes the child node and recursivelly all further descendants.
             */
            DLL void Erase ( std::vector<Node>::size_type aIndex );
            /** Delete a child node given a reference to it.
             * This function deletes the child node and recursivelly all further descendants.
             * @note the provided reference MUST be to the specific node to be removed,
             * passing a copy of the node will likely make the operation fail.
             * @todo Decide if having iterators is a better idea to delete by reference.
             */
            DLL void Erase ( const Node& aNode );
            /** Get the current size of the children node vector.
             * This is the count of direct descendants.
             */
            DLL std::vector<Node>::size_type GetChildrenCount() const;
            /** Get a const reference to a direct descendant by index.
             * May throw an std::out_of_range exception.
             */
            DLL const Node& GetChild ( size_t aIndex ) const;
            /** Get a eference to a direct descendant by index.
             * May throw an std::out_of_range exception.
             */
            DLL Node& GetChild ( size_t aIndex );
            /** Get a const reference to a direct descendant by index.
             * Does NOT do bounds checking.
             */
            DLL const Node& operator[] ( const std::size_t aIndex ) const;
            /** Get a eference to a direct descendant by index.
             * Does NOT do bounds checking.
             */
            DLL Node& operator[] ( const std::size_t aIndex );
            /** Get a const pointer to the node's parent.
             * Returned pointer may be nullptr.
             */
            DLL const Node* GetParent() const;
            /** Get a pointer to the node's parent.
             * Returned pointer may be nullptr.
             */
            DLL Node* GetParent();
            /** Get The index of the node in its parent or tree child node vector.
             * If the parent is nullptr this function throws an std::runtime_error exception.
             */
            DLL std::size_t GetIndex() const;
            /** Iterative depth first search iteration.
            Iterates all descendants without recursion in pre-order.
            This function guarrantees that parents are processed before their children.
            @param aAction a function, function pointer or function like object to proccess each child node.
            @sa Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPreOrder,Node::RecursiveTraverseDFSPostOrder
            */
            DLL void LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction );
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
            DLL void LoopTraverseDFSPreOrder (
                const std::function<void ( Node& ) >& aPreamble,
                const std::function<void ( Node& ) >& aPostamble );
            /** Constant version of LoopTraverseDFSPreOrder. */
            DLL void LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const;
            /** Iterative depth first search iteration.
            Iterates all descendants without recursion in post-order.
            This function guarrantees that children are processed before their parent.
            @param aAction a function, function pointer or function like object to proccess each child node.
            @sa Node::LoopTraverseDFSPreOrder,Node::RecursiveTraverseDFSPreOrder,Node::RecursiveTraverseDFSPostOrder
            */
            DLL void LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction );
            /** Constant version of LoopTraverseDFSPostOrder. */
            DLL void LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const;
            /** Recursive depth first search iteration.
            Iterates all descendants with recursion in pre-order.
            This function guarrantees that parents are processed before their children.
            @param aAction a function, function pointer or function like object to proccess each child node.
            @sa Node::LoopTraverseDFSPreOrder,Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPostOrder
            */
            DLL void RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction );
            /** Recursive depth first search iteration.
            Iterates all descendants with recursion in post-order.
            This function guarrantees that children are processed before their parent.
            @param aAction a function, function pointer or function like object to proccess each child node.
            @sa Node::LoopTraverseDFSPreOrder,Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPreOrder
            */
            DLL void RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction );

            DLL void LoopTraverseAncestors ( const std::function<void ( Node& ) >& aAction );

            DLL void LoopTraverseAncestors ( const std::function<void ( const Node& ) >& aAction ) const;

            DLL void RecursiveTraverseAncestors ( const std::function<void ( Node& ) >& aAction );
            ///@}
            ///@name Flag Functions
            ///@{
            enum FlagBits
            {
                EnabledBit = 1,
                VisibleBit = 2,
                AllBits    = 3
            };
            enum Flags
            {
                Enabled = 0,
                Visible,
                FlagCount
            };
            /** Enables or disables a set of node flags.
            @param aFlagBits Flag bits to enable/disable.
            @param aEnabled true to enable, false to disable.
            */
            DLL void SetFlags ( size_t aFlagBits, bool aEnabled = true );
            /** Enables or disables a single node flag.
            @param aFlag Flag to enable/disable.
            @param aEnabled true to enable, false to disable.
            */
            DLL void SetFlag ( enum Flags aFlag, bool aEnabled = true );
            DLL bool IsFlagEnabled ( enum Flags aFlag ) const;
            ///@}
            ///@name Transform Functions
            ///@{
            DLL const Transform& GetLocalTransform() const;
            DLL const Transform& GetGlobalTransform() const;
            DLL void SetLocalTransform ( const Transform& aTransform );
            DLL void SetGlobalTransform ( const Transform& aTransform );
            ///@}
            ///@name Node Name Functions
            ///@{
            DLL const std::string& GetName() const;
            DLL void SetName ( const std::string& aName );
            ///@}
        private:
            friend class Tree;
            std::string mName{};
            Node* mParent{};
            Tree* mTree{};
            /** Tree iteration helper.
                Mutable to allow for constant iterations (EC++ Item 3).*/
            mutable std::vector<Node>::size_type mIterator{ 0 };
            std::bitset<sizeof ( size_t ) > mFlags{AllBits};
            Transform mLocalTransform{};
            Transform mGlobalTransform{};
            std::vector<Node> mNodes{};
        };

        ///@name Tree Functions
        ///@{
        DLL Tree ( std::initializer_list<Node> aList );
        DLL ~Tree();
        DLL void Append ( const Node& aNode );
        DLL void Insert ( size_t aIndex, const Node& aNode );
        /** Reparent a node at the specified index on this tree's child vector. */
        DLL void Move ( size_t aIndex, Node&& aNode );
        DLL void Erase ( std::vector<Node>::size_type aIndex );
        DLL void Erase ( const Node& aNode );
        DLL std::vector<Node>::size_type GetChildrenCount() const;
        DLL const Node& GetChild ( size_t aIndex ) const;
        DLL Node& GetChild ( size_t aIndex );
        DLL const Node& operator[] ( const std::size_t aIndex ) const;
        DLL Node& operator[] ( const std::size_t aIndex );
        /** @copydoc Node::LoopTraverseDFSPreOrder(std::function<void(Node&) > aAction)*/
        DLL void LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction );
        /** @copydoc Node::LoopTraverseDFSPreOrder(
            std::function<void(Node*) > aPreamble,
            std::function<void(Node*) > aPostamble)*/
        DLL void LoopTraverseDFSPreOrder (
            const std::function<void ( Node& ) >& aPreamble,
            const std::function<void ( Node& ) >& aPostamble );
        /** @copydoc Node::LoopTraverseDFSPreOrder(std::function<void(const std::shared_ptr<const Node>&) > aAction) const */
        DLL void LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const;
        /** @copydoc Node::LoopTraverseDFSPostOrder(std::function<void(const std::shared_ptr<Node>&) > aAction)*/
        DLL void LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction );
        /** @copydoc Node::LoopTraverseDFSPostOrder(std::function<void(const std::shared_ptr<const Node>&) > aAction) const */
        DLL void LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const;
        /** @copydoc Node::RecursiveTraverseDFSPreOrder(std::function<void(const std::shared_ptr<Node>&) > aAction)*/
        DLL void RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction );
        /** @copydoc Node::RecursiveTraverseDFSPostOrder(std::function<void(const std::shared_ptr<Node>&) > aAction)*/
        DLL void RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction );
        ///@}
        DLL std::string Serialize ( bool aAsBinary = true ) const;
    private:
        std::vector<Node> mNodes{};
    };
}
#endif
