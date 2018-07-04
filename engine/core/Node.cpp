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
#include "Factory.h"
#include "aeongames/CRC.h"

namespace AeonGames
{
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
        if ( auto parent = mParent )
        {
            if ( !parent->Remove ( this ) )
            {
                std::cerr << "Remove Node Failed" << std::endl;
            }
        }
        for ( auto & mNode : mNodes )
        {
            mNode->mParent = nullptr;
            mNode = nullptr;
        }
    }
    size_t Node::GetChildrenCount() const
    {
        return mNodes.size();
    }
    const Node& Node::operator[] ( const std::size_t index ) const
    {
        return * ( mNodes[index] );
    }

    Node& Node::operator[] ( const std::size_t index )
    {
        return const_cast<Node&> ( static_cast<const Node&> ( *this ) [index] );
    }
    Node* Node::GetChild ( size_t aIndex ) const
    {
        return mNodes.at ( aIndex );
    }
    Node* Node::GetParent() const
    {
        return mParent;
    }
    size_t Node::GetIndex() const
    {
        auto parent = mParent;
        if ( parent )
        {
            auto index = std::find_if ( parent->mNodes.begin(), parent->mNodes.end(),
                                        [this] ( const Node * node )
            {
                return node == this;
            } );
            return index - parent->mNodes.begin();
        }
        throw std::runtime_error ( "Node has no parent and thus no assigned index." );
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

    void Node::SetLocalTransform ( const Transform& aTransform )
    {
        mLocalTransform = aTransform;
        LoopTraverseDFSPreOrder (
            [] ( Node & node )
        {
            if ( auto parent = node.mParent )
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
        if ( auto parent = mParent )
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
                if ( auto parent = node.mParent )
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

    bool Node::Insert ( size_t aIndex, Node* aNode )
    {
        // Never append null or this pointers.
        ///@todo std::find might be slower than removing and reinserting an existing node
        if ( ( aNode != nullptr ) && ( aNode != this ) && ( std::find ( mNodes.begin(), mNodes.end(), aNode ) == mNodes.end() ) )
        {
            if ( auto parent = aNode->mParent )
            {
                if ( !parent->Remove ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = this;
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

    bool Node::Add ( Node* aNode )
    {
        // Never append null or this pointers.
        if ( ( aNode != nullptr ) && ( aNode != this ) && ( std::find ( mNodes.begin(), mNodes.end(), aNode ) == mNodes.end() ) )
        {
            if ( auto parent = aNode->mParent )
            {
                if ( !parent->Remove ( aNode ) )
                {
                    std::cout << LogLevel ( LogLevel::Level::Warning ) << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
            aNode->mParent = this;
            mNodes.push_back ( aNode );
            // Force a recalculation of the LOCAL transform
            // by setting the GLOBAL transform to itself.
            aNode->SetGlobalTransform ( aNode->mGlobalTransform );
            return true;
        }
        return false;
    }

    bool Node::Remove ( Node* aNode )
    {
        assert ( aNode != nullptr );
        assert ( aNode != this );
        // If passed a null or this pointer find SHOULD not find it on release builds.
        auto it = std::find_if (
                      mNodes.begin(),
                      mNodes.end(),
                      [aNode] ( const Node * node )
        {
            return aNode == node;
        } );
        if ( it != mNodes.end() )
        {
            aNode->mParent = nullptr;
            // Force recalculation of transforms.
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            mNodes.erase ( it );
            return true;
        }
        return false;
    }

    bool Node::RemoveByIndex ( size_t aIndex )
    {
        if ( mNodes.size() < aIndex )
        {
            return false;
        }
        mNodes[aIndex]->mParent = nullptr;
        mNodes[aIndex]->SetLocalTransform ( mNodes[aIndex]->mGlobalTransform );
        mNodes.erase ( mNodes.begin() + aIndex );
        return true;
    }

    bool Node::Move ( size_t aIndex, Node* aNode )
    {
        if ( !aNode )
        {
            return false;
        }
        if ( aNode->GetParent() )
        {
            aNode->GetParent()->Remove ( aNode );
        }
        Insert ( aIndex, aNode );
        return true;
    }

    void Node::LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        /** @todo (EC++ Item 3) This code is the same as the constant overload,
        but can't easily be implemented in terms of that because of aAction's node parameter
        need to also be const.
        */
        auto node = this;
        aAction ( *node );
        auto parent = mParent;
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
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
        auto parent = mParent;
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
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

    void Node::LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        auto node = this;
        aAction ( *node );
        auto parent = mParent;
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
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

    void Node::LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        auto node = this;
        auto parent = mParent;
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
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

    void Node::LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        /*
        This code implements a similar solution to this stackoverflow answer:
        http://stackoverflow.com/questions/5987867/traversing-a-n-ary-tree-without-using-recurrsion/5988138#5988138
        */
        auto node = this;
        auto parent = mParent;
        while ( node != parent )
        {
            if ( node->mIterator < node->mNodes.size() )
            {
                auto prev = node;
                node = node->mNodes[node->mIterator];
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
            node = node->mParent;
        }
    }

    void Node::LoopTraverseAncestors ( const std::function<void ( const Node& ) >& aAction ) const
    {
        auto node = this;
        while ( node != nullptr )
        {
            aAction ( *node );
            node = node->mParent;
        }
    }

    void Node::RecursiveTraverseAncestors ( const std::function<void ( Node& ) >& aAction )
    {
        aAction ( *this );
        if ( auto parent = mParent )
        {
            parent->RecursiveTraverseAncestors ( aAction );
        }
    }

    void Node::Update ( const double aDelta )
    {
        ( void ) aDelta;
    }

    static Node::PropertyDescriptor PropertyDescriptors [] =
    {
        {
            "LocalTransform"_crc32, "Local Transform", "10f",
            [] ( Node * aNode, const void* aTuple ) {aNode->SetLocalTransform ( reinterpret_cast<const float*> ( aTuple ) );},
            [] ( const Node * aNode, void* aTuple ) {aNode->GetLocalTransform().Get ( reinterpret_cast<float*> ( aTuple ) );},
            {
                {
                    "LocalScale"_crc32, "Scale", "3f",
                    [] ( Node * aNode, const void* aTuple ) {Transform transform = aNode->GetLocalTransform(); transform.SetScale ( reinterpret_cast<const float*> ( aTuple ) ); aNode->SetLocalTransform ( transform );},
                    [] ( const Node * aNode, void* aTuple ) {aNode->GetLocalTransform().GetScale().Get ( reinterpret_cast<float*> ( aTuple ) );},
                    {
                        {
                            "LocalScaleX"_crc32, "X", "f",
                            [] ( Node * aNode, const void* aTuple ) {Transform transform = aNode->GetLocalTransform(); Vector3 scale = transform.GetScale(); scale[0] = *reinterpret_cast<const float*> ( aTuple ); transform.SetScale ( scale ); aNode->SetLocalTransform ( transform );},
                            [] ( const Node * aNode, void* aTuple )
                            {
                                *reinterpret_cast<float*> ( aTuple )  = aNode->GetLocalTransform().GetScale() [0];
                            }
                        },
                        {"LocalScaleY"_crc32, "Y", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"LocalScaleZ"_crc32, "Z", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}}
                    }
                },
                {
                    "LocalRotation"_crc32, "Rotation", "4f",
                    [] ( Node * aNode, const void* aTuple ) {Transform transform = aNode->GetLocalTransform(); transform.SetRotation ( reinterpret_cast<const float*> ( aTuple ) ); aNode->SetLocalTransform ( transform );},
                    [] ( const Node * aNode, void* aTuple ) {aNode->GetLocalTransform().GetRotation().Get ( reinterpret_cast<float*> ( aTuple ) );},
                    {
                        {"LocalRotationW"_crc32, "W", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"LocalRotationX"_crc32, "X", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"LocalRotationY"_crc32, "Y", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"LocalRotationZ"_crc32, "Z", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}}
                    }
                },
                {
                    "LocalTranslation"_crc32, "Translation", "3f",
                    [] ( Node * aNode, const void* aTuple ) {Transform transform = aNode->GetLocalTransform(); transform.SetTranslation ( reinterpret_cast<const float*> ( aTuple ) ); aNode->SetLocalTransform ( transform );},
                    [] ( const Node * aNode, void* aTuple ) {aNode->GetLocalTransform().GetTranslation().Get ( reinterpret_cast<float*> ( aTuple ) );},
                    {
                        {"LocalTranslationX"_crc32, "X", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"LocalTranslationY"_crc32, "Y", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"LocalTranslationZ"_crc32, "Z", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}}
                    }
                },
            }
        },
        {
            "GlobalTransform"_crc32, "Global Transform", "10f",
            [] ( Node * aNode, const void* aTuple ) {aNode->SetGlobalTransform ( reinterpret_cast<const float*> ( aTuple ) );},
            [] ( const Node * aNode, void* aTuple ) {aNode->GetGlobalTransform().Get ( reinterpret_cast<float*> ( aTuple ) );},
            {
                {
                    "GlobalScale"_crc32, "Scale", "3f",
                    [] ( Node * aNode, const void* aTuple ) {Transform transform = aNode->GetGlobalTransform(); transform.SetScale ( reinterpret_cast<const float*> ( aTuple ) ); aNode->SetGlobalTransform ( transform );},
                    [] ( const Node * aNode, void* aTuple ) {aNode->GetGlobalTransform().GetScale().Get ( reinterpret_cast<float*> ( aTuple ) );},
                    {
                        {"GlobalScaleX"_crc32, "X", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"GlobalScaleY"_crc32, "Y", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"GlobalScaleZ"_crc32, "Z", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}}
                    }
                },
                {
                    "GlobalRotation"_crc32, "Rotation", "4f",
                    [] ( Node * aNode, const void* aTuple ) {Transform transform = aNode->GetGlobalTransform(); transform.SetRotation ( reinterpret_cast<const float*> ( aTuple ) ); aNode->SetGlobalTransform ( transform );},
                    [] ( const Node * aNode, void* aTuple ) {aNode->GetGlobalTransform().GetRotation().Get ( reinterpret_cast<float*> ( aTuple ) );},
                    {
                        {"GlobalRotationW"_crc32, "W", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"GlobalRotationX"_crc32, "X", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"GlobalRotationY"_crc32, "Y", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"GlobalRotationZ"_crc32, "Z", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}}
                    }
                },
                {
                    "GlobalTranslation"_crc32, "Translation", "3f",
                    [] ( Node * aNode, const void* aTuple ) {Transform transform = aNode->GetGlobalTransform(); transform.SetTranslation ( reinterpret_cast<const float*> ( aTuple ) ); aNode->SetGlobalTransform ( transform );},
                    [] ( const Node * aNode, void* aTuple ) {aNode->GetGlobalTransform().GetTranslation().Get ( reinterpret_cast<float*> ( aTuple ) );},
                    {
                        {"GlobalTranslationX"_crc32, "X", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"GlobalTranslationY"_crc32, "Y", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}},
                        {"GlobalTranslationZ"_crc32, "Z", "f", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}}
                    }
                },
            }
        }
    };

    size_t Node::GetPropertyCount() const
    {
        return sizeof ( PropertyDescriptors ) / sizeof ( Node::PropertyDescriptor );
    }

    const Node::PropertyDescriptor& Node::GetPropertyDescriptor ( size_t aIndex ) const
    {
        return PropertyDescriptors[aIndex];
    }

    FactoryImplementation ( Node );
}
