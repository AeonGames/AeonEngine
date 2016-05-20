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
#ifndef AEONGAMES_NODE_H
#define AEONGAMES_NODE_H
#include <cstdint>
#include <string>
#include <vector>
#include <bitset>
#include <functional>
#include <limits>
#include "aeongames/Platform.h"
#include "math/Transform.h"

namespace AeonGames
{
    class Geometry;
    class Scene;
    class Entity;
    /// Scene Graph Node
    class Node
    {
    public:
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
        DLL static const size_t kInvalidIndex;
        DLL explicit Node ( uint32_t aFlags = AllBits );
        DLL ~Node();
        DLL void SetName ( const std::string& aName );
        DLL const std::string& GetName() const;
        /** Enables or disables a set of node flags.
        @param aFlagBits Flag bits to enable/disable.
        @param aEnabled true to enable, false to disable.
        */
        DLL void SetFlags ( uint32_t aFlagBits, bool aEnabled = true );
        /** Enables or disables a single node flag.
        @param aFlag Flag to enable/disable.
        @param aEnabled true to enable, false to disable.
        */
        DLL void SetFlag ( enum Flags aFlag, bool aEnabled = true );
        DLL bool IsFlagEnabled ( enum Flags aFlag ) const;
        DLL bool AddNode ( Node* aNode );
        DLL bool InsertNode ( size_t aIndex, Node* aNode );
        DLL bool RemoveNode ( Node* aNode );
        /** Iterative depth first search iteration.
        Iterates all descendants without recursion in pre-order.
        This function guarrantees that parents are processed before their children.
        @param aAction a function, function pointer or function like object to proccess each child node.
        @sa Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPreOrder,Node::RecursiveTraverseDFSPostOrder
        */
        DLL void LoopTraverseDFSPreOrder ( std::function<void ( Node* ) > aAction );
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
            std::function<void ( Node* ) > aPreamble,
            std::function<void ( Node* ) > aPostamble );
        /** Constant version of LoopTraverseDFSPreOrder. */
        DLL void LoopTraverseDFSPreOrder ( std::function<void ( const Node* ) > aAction ) const;
        /** Iterative depth first search iteration.
        Iterates all descendants without recursion in post-order.
        This function guarrantees that children are processed before their parent.
        @param aAction a function, function pointer or function like object to proccess each child node.
        @sa Node::LoopTraverseDFSPreOrder,Node::RecursiveTraverseDFSPreOrder,Node::RecursiveTraverseDFSPostOrder
        */
        DLL void LoopTraverseDFSPostOrder ( std::function<void ( Node* ) > aAction );
        /** Constant version of LoopTraverseDFSPostOrder. */
        DLL void LoopTraverseDFSPostOrder ( std::function<void ( const Node* ) > aAction ) const;
        /** Recursive depth first search iteration.
        Iterates all descendants with recursion in pre-order.
        This function guarrantees that parents are processed before their children.
        @param aAction a function, function pointer or function like object to proccess each child node.
        @sa Node::LoopTraverseDFSPreOrder,Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPostOrder
        */
        DLL void RecursiveTraverseDFSPreOrder ( std::function<void ( Node* ) > aAction );
        /** Recursive depth first search iteration.
        Iterates all descendants with recursion in post-order.
        This function guarrantees that children are processed before their parent.
        @param aAction a function, function pointer or function like object to proccess each child node.
        @sa Node::LoopTraverseDFSPreOrder,Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPreOrder
        */
        DLL void RecursiveTraverseDFSPostOrder ( std::function<void ( Node* ) > aAction );
        DLL void LoopTraverseAncestors ( std::function<void ( Node* ) > aAction );
        DLL void LoopTraverseAncestors ( std::function<void ( const Node* ) > aAction ) const;
        DLL void RecursiveTraverseAncestors ( std::function<void ( Node* ) > aAction );
        DLL const Transform& GetLocalTransform() const;
        DLL const Transform& GetGlobalTransform() const;
        DLL void SetLocalTransform ( const Transform& aTransform );
        DLL void SetGlobalTransform ( const Transform& aTransform );
        DLL void AttachEntity ( Entity* aEntity );
        DLL Entity* DettachEntity();
        DLL size_t GetChildrenCount() const;
        DLL Node* GetChild ( size_t aIndex ) const;
        DLL Node* GetParent() const;
        DLL size_t GetIndex() const;
    private:
        friend class Scene;
        DLL void Update ( const double delta );
        std::string mName;
        Node* mParent;
        Scene* mScene;
        Transform mLocalTransform;
        Transform mGlobalTransform;
        std::vector<Node*> mNodes;
        size_t mIndex;
        /** Tree iteration helper.
            Mutable to allow for constant iterations (EC++ Item 3).*/
        mutable std::vector<Node*>::size_type mIterator;
        std::bitset<8> mFlags;
        Entity* mEntity;
    };
}
#endif
