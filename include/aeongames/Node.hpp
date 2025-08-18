/*
Copyright (C) 2014-2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include <unordered_map>
#include <tuple>
#include "aeongames/Platform.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/Component.hpp"
#include "aeongames/DependencyMap.hpp"

namespace AeonGames
{
    class Window;
    class NodeMsg;
    class Scene;
    using NodeParent = std::variant<Node*, Scene*>;
    inline Node* GetNodePtr ( const NodeParent& aNodeParent )
    {
        return std::holds_alternative<Node*> ( aNodeParent ) ? std::get<Node*> ( aNodeParent ) : nullptr;
    }
    inline Scene* GetScenePtr ( const NodeParent& aNodeParent )
    {
        return std::holds_alternative<Scene*> ( aNodeParent ) ? std::get<Scene*> ( aNodeParent ) : nullptr;
    }
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
        DLL Node ( uint32_t aFlags = AllBits );
        DLL void SetName ( const std::string& aName );
        DLL const std::string& GetName() const;
        DLL void Serialize ( NodeMsg& aNodeMsg ) const;
        DLL void Deserialize ( const NodeMsg& aNodeMsg );
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
        DLL Node* Add ( std::unique_ptr<Node> aNode );
        DLL Node* Insert ( size_t aIndex, std::unique_ptr<Node> aNode );
        DLL std::unique_ptr<Node> Remove ( Node* );
        DLL std::unique_ptr<Node> RemoveByIndex ( size_t aIndex );
        /** Retrieve a pointer to the scene containing the node instance
         * @return A pointer to the scene at the root of the node tree
         *  or nullptr if the node is not contained in a scene.
        */
        DLL Scene* GetScene() const;
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
        DLL Node* Find ( const std::function<bool ( const Node& ) >& aUnaryPredicate ) const;
        DLL const Transform& GetLocalTransform() const;
        DLL const Transform& GetGlobalTransform() const;
        DLL const AABB& GetAABB() const;
        DLL void SetLocalTransform ( const Transform& aTransform );
        DLL void SetGlobalTransform ( const Transform& aTransform );
        DLL void SetAABB ( const AABB& aAABB );
        DLL size_t GetChildrenCount() const;
        DLL Node* GetChild ( size_t aIndex ) const;
        DLL const Node& operator[] ( const std::size_t index ) const;
        DLL Node& operator[] ( const std::size_t index );
        DLL NodeParent GetParent() const;
        DLL size_t GetIndex() const;
        DLL uint32_t GetId() const;
        /** @name Node Data */
        /** @{ */
        DLL Component* AddComponent ( std::unique_ptr<Component> aComponent );
        DLL size_t GetComponentCount() const;
        DLL Component* GetComponentByIndex ( size_t aIndex ) const;
        DLL Component* GetComponent ( uint32_t aId ) const;
        DLL std::unique_ptr<Component> RemoveComponent ( uint32_t aId );
        /** @} */
        DLL void Update ( const double delta );
        DLL void Render ( Renderer& aRenderer, void* aWindowId ) const;
        DLL void ProcessMessage ( uint32_t aMessageType, const void* aMessageData );
    private:
        friend class Scene;
        std::string mName{};
        NodeParent mParent{};
        Transform mLocalTransform{};
        Transform mGlobalTransform{};
        AABB mAABB{};
        std::vector<std::unique_ptr<Node >> mNodes{};
        DependencyMap<uint32_t> mComponentDependencyMap{};
        /**
         * Node component container
        */
        std::vector<std::unique_ptr<Component >> mComponents{};
        /** Tree iteration helper.
            Mutable to allow for constant iterations (EC++ Item 3).*/
        mutable std::vector<Node*>::size_type mIterator{ 0 };
        uint32_t mId{};
        std::bitset<8> mFlags{};
    };
}
#endif
