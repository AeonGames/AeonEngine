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
#include "aeongames/CRC.h"

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
                const std::string& aName,
                const std::string& aDisplayName,
                const std::string& aFormat,
                const std::function<void ( Node*, const void* ) >& aSetter,
                const std::function<void ( const Node*, void* ) >& aGetter,
                std::initializer_list<PropertyDescriptor> aSubProperties = {}
            ) :
                mId{crc32i ( aName.c_str(), aName.size() ) },
                mName{aName},
                mDisplayName{aDisplayName},
                mFormat{aFormat},
                mSetter{aSetter},
                mGetter{aGetter},
                mParent{},
                mSubProperties{std::move ( aSubProperties ) }
            {
                for ( auto& i : mSubProperties )
                {
                    i.mParent = this;
                }
            }
            PropertyDescriptor ( const PropertyDescriptor& aPropertyDescriptor ) :
                mId{aPropertyDescriptor.mId},
                mName{aPropertyDescriptor.mName},
                mDisplayName{aPropertyDescriptor.mDisplayName},
                mFormat{aPropertyDescriptor.mFormat},
                mSetter{aPropertyDescriptor.mSetter},
                mGetter{aPropertyDescriptor.mGetter},
                mParent{aPropertyDescriptor.mParent},
                mSubProperties{ aPropertyDescriptor.mSubProperties }
            {
                for ( auto& i : mSubProperties )
                {
                    i.mParent = this;
                }
            }
            PropertyDescriptor& operator= ( const PropertyDescriptor& aPropertyDescriptor )
            {
                mId = aPropertyDescriptor.mId;
                mName = aPropertyDescriptor.mName;
                mDisplayName = aPropertyDescriptor.mDisplayName;
                mFormat = aPropertyDescriptor.mFormat;
                mSetter = aPropertyDescriptor.mSetter;
                mGetter = aPropertyDescriptor.mGetter;
                mParent = aPropertyDescriptor.mParent;
                mSubProperties = aPropertyDescriptor.mSubProperties;
                for ( auto& i : mSubProperties )
                {
                    i.mParent = this;
                }
                return *this;
            }
            PropertyDescriptor ( const PropertyDescriptor&& aPropertyDescriptor ) :
                mId{aPropertyDescriptor.mId},
                mName{std::move ( aPropertyDescriptor.mName ) },
                mDisplayName{std::move ( aPropertyDescriptor.mDisplayName ) },
                mFormat{std::move ( aPropertyDescriptor.mFormat ) },
                mSetter{std::move ( aPropertyDescriptor.mSetter ) },
                mGetter{std::move ( aPropertyDescriptor.mGetter ) },
                mParent{aPropertyDescriptor.mParent},
                mSubProperties{ std::move ( aPropertyDescriptor.mSubProperties ) }
            {
                for ( auto& i : mSubProperties )
                {
                    i.mParent = this;
                }
            }
            PropertyDescriptor& operator= ( const PropertyDescriptor&& aPropertyDescriptor )
            {
                mId = aPropertyDescriptor.mId;
                mName = std::move ( aPropertyDescriptor.mName );
                mDisplayName = std::move ( aPropertyDescriptor.mDisplayName );
                mFormat = std::move ( aPropertyDescriptor.mFormat );
                mParent = aPropertyDescriptor.mParent;
                mSubProperties = std::move ( aPropertyDescriptor.mSubProperties );
                for ( auto& i : mSubProperties )
                {
                    i.mParent = this;
                }
                return *this;
            }
            const size_t GetId() const
            {
                return mId;
            }
            const std::string& GetName() const
            {
                return mName;
            }
            const std::string& GetDisplayName() const
            {
                return mDisplayName;
            }
            const std::string& GetFormat() const
            {
                return mFormat;
            }
            void Set ( Node* aNode, const void* aTuple ) const
            {
                mSetter ( aNode, aTuple );
            }
            void Get ( const Node* aNode, void* aTuple ) const
            {
                mGetter ( aNode, aTuple );
            }
            const std::vector<PropertyDescriptor>& GetSubProperties() const
            {
                return mSubProperties;
            }
        private:
            size_t mId {};
            std::string  mName{};
            std::string  mDisplayName{};
            std::string  mFormat{};
            std::function<void ( Node*, const void* ) > mSetter;
            std::function<void ( const Node*, void* ) > mGetter;
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
