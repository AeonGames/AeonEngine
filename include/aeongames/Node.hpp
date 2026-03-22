/*
Copyright (C) 2014-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
    /// @brief Variant type representing a node's parent (either a Node or a Scene).
    using NodeParent = std::variant<Node*, Scene*>;
    /// @brief Extract the Node pointer from a NodeParent variant.
    inline Node* GetNodePtr ( const NodeParent& aNodeParent )
    {
        return std::holds_alternative<Node*> ( aNodeParent ) ? std::get<Node*> ( aNodeParent ) : nullptr;
    }
    /// @brief Extract the Scene pointer from a NodeParent variant.
    inline Scene* GetScenePtr ( const NodeParent& aNodeParent )
    {
        return std::holds_alternative<Scene*> ( aNodeParent ) ? std::get<Scene*> ( aNodeParent ) : nullptr;
    }
    /** Scene graph node representing an entity in the game world.
        Nodes form a hierarchical tree structure and can hold components,
        transforms, and bounding volumes. */
    class Node
    {
    public:
        /** Bitmask values for node flags. */
        enum FlagBits
        {
            EnabledBit = 1,
            VisibleBit = 2,
            AllBits    = 3
        };
        /** Individual flag indices for use with SetFlag/IsFlagEnabled. */
        enum Flags
        {
            Enabled = 0,
            Visible,
            FlagCount
        };
        /** Construct a node with the given initial flags.
            @param aFlags Bitmask of FlagBits to enable. Defaults to AllBits. */
        DLL Node ( uint32_t aFlags = AllBits );
        /** Set the name of this node.
            @param aName The new name string. */
        DLL void SetName ( const std::string& aName );
        /** Get the name of this node.
            @return A const reference to the node name. */
        DLL const std::string& GetName() const;
        /** Serialize this node's state into a protobuf message.
            @param aNodeMsg The message to populate. */
        DLL void Serialize ( NodeMsg& aNodeMsg ) const;
        /** Deserialize this node's state from a protobuf message.
            @param aNodeMsg The message to read from. */
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
        /** Check whether a specific flag is enabled.
            @param aFlag The flag to query.
            @return true if the flag is enabled. */
        DLL bool IsFlagEnabled ( enum Flags aFlag ) const;
        /** Append a child node.
            @param aNode Unique pointer to the node to add.
            @return Raw pointer to the added node. */
        DLL Node* Add ( std::unique_ptr<Node> aNode );
        /** Insert a child node at a specific index.
            @param aIndex Position at which to insert.
            @param aNode Unique pointer to the node to insert.
            @return Raw pointer to the inserted node. */
        DLL Node* Insert ( size_t aIndex, std::unique_ptr<Node> aNode );
        /** Remove a child node by pointer.
            @param aNode Pointer to the child node to remove.
            @return Unique pointer to the removed node, or nullptr if not found. */
        DLL std::unique_ptr<Node> Remove ( Node* );
        /** Remove a child node by index.
            @param aIndex Index of the child to remove.
            @return Unique pointer to the removed node. */
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
        /// @brief Traverse ancestors iteratively, calling an action on each.
        DLL void LoopTraverseAncestors ( const std::function<void ( Node& ) >& aAction );
        /// @brief Traverse ancestors iteratively (const version).
        DLL void LoopTraverseAncestors ( const std::function<void ( const Node& ) >& aAction ) const;
        /// @brief Traverse ancestors recursively, calling an action on each.
        DLL void RecursiveTraverseAncestors ( const std::function<void ( Node& ) >& aAction );
        /// @brief Find a descendant node matching a predicate.
        DLL Node* Find ( const std::function<bool ( const Node& ) >& aUnaryPredicate ) const;
        /** Get the local transform relative to the parent.
            @return A const reference to the local transform. */
        DLL const Transform& GetLocalTransform() const;
        /** Get the global (world-space) transform.
            @return A const reference to the global transform. */
        DLL const Transform& GetGlobalTransform() const;
        /** Get the axis-aligned bounding box.
            @return A const reference to the AABB. */
        DLL const AABB& GetAABB() const;
        /** Set the local transform relative to the parent.
            @param aTransform The new local transform. */
        DLL void SetLocalTransform ( const Transform& aTransform );
        /** Set the global (world-space) transform.
            @param aTransform The new global transform. */
        DLL void SetGlobalTransform ( const Transform& aTransform );
        /** Set the axis-aligned bounding box.
            @param aAABB The new AABB. */
        DLL void SetAABB ( const AABB& aAABB );
        /** Get the number of direct child nodes.
            @return Child count. */
        DLL size_t GetChildrenCount() const;
        /** Get a child node by index.
            @param aIndex Index of the child.
            @return Pointer to the child node, or nullptr if out of range. */
        DLL Node* GetChild ( size_t aIndex ) const;
        /** Access a child node by index (const).
            @param index Index of the child.
            @return Const reference to the child node. */
        DLL const Node& operator[] ( const std::size_t index ) const;
        /** Access a child node by index.
            @param index Index of the child.
            @return Reference to the child node. */
        DLL Node& operator[] ( const std::size_t index );
        /** Get this node's parent.
            @return A NodeParent variant holding either a Node* or Scene*. */
        DLL NodeParent GetParent() const;
        /** Get this node's index within its parent's child list.
            @return The index position. */
        DLL size_t GetIndex() const;
        /** Get the unique identifier of this node.
            @return The node id. */
        DLL uint32_t GetId() const;
        /** @name Node Data */
        /** @{ */
        /** Add a component to this node.
            @param aComponent Unique pointer to the component.
            @return Raw pointer to the added component. */
        DLL Component* AddComponent ( std::unique_ptr<Component> aComponent );
        /** Get the number of components attached to this node.
            @return Component count. */
        DLL size_t GetComponentCount() const;
        /** Get a component by its index.
            @param aIndex Index of the component.
            @return Pointer to the component, or nullptr if out of range. */
        DLL Component* GetComponentByIndex ( size_t aIndex ) const;
        /** Get a component by its type identifier.
            @param aId CRC identifier of the component type.
            @return Pointer to the component, or nullptr if not found. */
        DLL Component* GetComponent ( uint32_t aId ) const;
        /** Remove a component by its type identifier.
            @param aId CRC identifier of the component type.
            @return Unique pointer to the removed component, or nullptr if not found. */
        DLL std::unique_ptr<Component> RemoveComponent ( uint32_t aId );
        /** @} */
        /** Update this node and its children.
            @param delta Elapsed time in seconds since the last update. */
        DLL void Update ( const double delta );
        /** Render this node and its children.
            @param aRenderer The renderer to draw with.
            @param aWindowId Platform-specific window handle. */
        DLL void Render ( Renderer& aRenderer, void* aWindowId ) const;
        /** Deliver a message to this node and its components.
            @param aMessageType Type identifier for the message.
            @param aMessageData Pointer to message-specific data. */
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
