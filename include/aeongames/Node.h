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
#ifndef AEONGAMES_NODE_H
#define AEONGAMES_NODE_H
#include <cstdint>
#include <string>
#include <vector>
#include <bitset>
#include <functional>
#include <limits>
#include "aeongames/Platform.h"
#include "aeongames/Transform.h"
#include "aeongames/Memory.h"

namespace AeonGames
{
    class Renderer;
    class Scene;
    class ModelInstance;
    class AABB;
    /// Scene Graph Node
    class Node : public std::enable_shared_from_this<Node>
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
        DLL virtual ~Node();
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
        DLL bool AddNode ( const std::shared_ptr<Node>& aNode );
        DLL bool InsertNode ( size_t aIndex, const std::shared_ptr<Node>& aNode );
        DLL bool RemoveNode ( const std::shared_ptr<Node>& );
        /** Iterative depth first search iteration.
        Iterates all descendants without recursion in pre-order.
        This function guarrantees that parents are processed before their children.
        @param aAction a function, function pointer or function like object to proccess each child node.
        @sa Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPreOrder,Node::RecursiveTraverseDFSPostOrder
        */
        DLL void LoopTraverseDFSPreOrder ( std::function<void ( const std::shared_ptr<Node>& ) > aAction );
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
            std::function<void ( const std::shared_ptr<Node>& ) > aPreamble,
            std::function<void ( const std::shared_ptr<Node>& ) > aPostamble );
        /** Constant version of LoopTraverseDFSPreOrder. */
        DLL void LoopTraverseDFSPreOrder ( std::function<void ( const std::shared_ptr<const Node>& ) > aAction ) const;
        /** Iterative depth first search iteration.
        Iterates all descendants without recursion in post-order.
        This function guarrantees that children are processed before their parent.
        @param aAction a function, function pointer or function like object to proccess each child node.
        @sa Node::LoopTraverseDFSPreOrder,Node::RecursiveTraverseDFSPreOrder,Node::RecursiveTraverseDFSPostOrder
        */
        DLL void LoopTraverseDFSPostOrder ( std::function<void ( const std::shared_ptr<Node>& ) > aAction );
        /** Constant version of LoopTraverseDFSPostOrder. */
        DLL void LoopTraverseDFSPostOrder ( std::function<void ( const std::shared_ptr<const Node>& ) > aAction ) const;
        /** Recursive depth first search iteration.
        Iterates all descendants with recursion in pre-order.
        This function guarrantees that parents are processed before their children.
        @param aAction a function, function pointer or function like object to proccess each child node.
        @sa Node::LoopTraverseDFSPreOrder,Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPostOrder
        */
        DLL void RecursiveTraverseDFSPreOrder ( std::function<void ( const std::shared_ptr<Node>& ) > aAction );
        /** Recursive depth first search iteration.
        Iterates all descendants with recursion in post-order.
        This function guarrantees that children are processed before their parent.
        @param aAction a function, function pointer or function like object to proccess each child node.
        @sa Node::LoopTraverseDFSPreOrder,Node::LoopTraverseDFSPostOrder,Node::RecursiveTraverseDFSPreOrder
        */
        DLL void RecursiveTraverseDFSPostOrder ( std::function<void ( const std::shared_ptr<Node>& ) > aAction );
        DLL void LoopTraverseAncestors ( std::function<void ( const std::shared_ptr<Node>& ) > aAction );
        DLL void LoopTraverseAncestors ( std::function<void ( const std::shared_ptr<const Node>& ) > aAction ) const;
        DLL void RecursiveTraverseAncestors ( std::function<void ( const std::shared_ptr<Node>& ) > aAction );
        DLL const Transform& GetLocalTransform() const;
        DLL const Transform& GetGlobalTransform() const;
        DLL const AABB GetLocalAABB() const;
        DLL const AABB GetGlobalAABB() const;
        DLL void SetLocalTransform ( const Transform& aTransform );
        DLL void SetGlobalTransform ( const Transform& aTransform );
        DLL size_t GetChildrenCount() const;
        DLL const std::shared_ptr<Node>& GetChild ( size_t aIndex ) const;
        DLL const std::shared_ptr<Node> GetParent() const;
        DLL size_t GetIndex() const;
        // ModelInstance itself should probably BE a node and implement its own Update function.
        DLL const std::shared_ptr<ModelInstance>& GetModelInstance() const;
        DLL void SetModelInstance ( const std::shared_ptr<ModelInstance>& aModelInstance );
    protected:
        //virtual void Update(const double delta) = 0;
    private:
        // This Update function should be temporary.
        void Update ( const double delta );
        static const std::shared_ptr<Node> mNullNode;
        friend class Scene;
        std::string mName;
        std::weak_ptr<Node> mParent;
        std::weak_ptr<Scene> mScene;
        std::shared_ptr<ModelInstance> mModelInstance;
        Transform mLocalTransform;
        Transform mGlobalTransform;
        std::vector<std::shared_ptr<Node>> mNodes;
        size_t mIndex;
        /** Tree iteration helper.
            Mutable to allow for constant iterations (EC++ Item 3).*/
        mutable std::vector<std::shared_ptr<Node>>::size_type mIterator;
        std::bitset<8> mFlags;
    };
}
#endif
