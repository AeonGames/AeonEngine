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
#include "aeongames/Platform.h"
#include "aeongames/Transform.h"
#include "aeongames/Memory.h"
#include "aeongames/DependencyMap.h"

namespace AeonGames
{
    class Renderer;
    class Scene;
    class ModelInstance;
    class AABB;
    class Component;
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
        class PropertyDescriptor
        {
        public:
            PropertyDescriptor (
                const size_t aId,
                const char*  aName,
                const char*  aTupleFormat,
                std::initializer_list<PropertyDescriptor> aSubProperties = {}
            ) :
                mId{aId},
                mName{aName},
                mTupleFormat{aTupleFormat},
                mParent{},
                mSubProperties{std::move ( aSubProperties ) }
            {
                for ( auto& i : mSubProperties )
                {
                    i.mParent = this;
                }
            }
#if 0
            /// @todo Implement these for proper parent assignment
            PropertyDescriptor ( const PropertyDescriptor& ) = delete;
            PropertyDescriptor& operator= ( const PropertyDescriptor& ) = delete;
            /// @todo Implement these just for fun (and completeness)
            PropertyDescriptor ( PropertyDescriptor&& ) = delete;
            PropertyDescriptor& operator= ( PropertyDescriptor&& ) = delete;
#endif
            const size_t GetId() const
            {
                return mId;
            }
            const char*  GetName() const
            {
                return mName;
            }
            const char*  GetTupleFormat() const
            {
                return mTupleFormat;
            }
        private:
            const size_t mId {};
            const char*  mName{};
            const char*  mTupleFormat{};
            PropertyDescriptor* mParent{};
            std::vector<PropertyDescriptor> mSubProperties{};
        };
        DLL Node ( uint32_t aFlags = AllBits );
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
        DLL bool Add ( Node* aNode );
        DLL bool Insert ( size_t aIndex, Node* aNode );
        DLL bool Remove ( Node* );
        DLL bool RemoveByIndex ( size_t aIndex );
        DLL bool Move ( size_t aIndex, Node* aNode );
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
        DLL const Transform& GetLocalTransform() const;
        DLL const Transform& GetGlobalTransform() const;
        DLL void SetLocalTransform ( const Transform& aTransform );
        DLL void SetGlobalTransform ( const Transform& aTransform );
        DLL size_t GetChildrenCount() const;
        DLL Node* GetChild ( size_t aIndex ) const;
        DLL const Node& operator[] ( const std::size_t index ) const;
        DLL Node& operator[] ( const std::size_t index );
        DLL Node* GetParent() const;
        DLL size_t GetIndex() const;
        /** @name Properties */
        /** @{ */
        DLL virtual size_t GetPropertyCount() const;
        DLL virtual const PropertyDescriptor& GetPropertyDescriptor ( size_t aIndex ) const;
        DLL virtual void SetProperty ( uint32_t aPropertyId, const void* aProperty );
        DLL virtual void GetProperty ( uint32_t aPropertyId, void** aProperty ) const;
        /** @} */
    private:
        /** @name Abstract functions */
        /** @{ */
        DLL virtual void Update ( const double delta );
        /** @} */
        friend class Scene;
        std::string mName;
        Node* mParent;
        Transform mLocalTransform;
        Transform mGlobalTransform;
        std::vector<Node*> mNodes;
        /** Tree iteration helper.
            Mutable to allow for constant iterations (EC++ Item 3).*/
        mutable std::vector<Node*>::size_type mIterator{ 0 };
        std::bitset<8> mFlags;
    };
    /**@name Factory Functions */
    /*@{*/
    DLL std::unique_ptr<Node> ConstructNode ( const std::string& aIdentifier );
    /** Registers a renderer loader for a specific identifier.*/
    DLL bool RegisterNodeConstructor ( const std::string& aIdentifier, const std::function<std::unique_ptr<Node>() >& aConstructor );
    /** Unregisters a renderer loader for a specific identifier.*/
    DLL bool UnregisterNodeConstructor ( const std::string& aIdentifier );
    /** Enumerates Node loader identifiers via an enumerator functor.*/
    DLL void EnumerateNodeConstructors ( const std::function<bool ( const std::string& ) >& aEnumerator );
    /*@}*/
}
#endif
