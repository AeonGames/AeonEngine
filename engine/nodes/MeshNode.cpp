/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#include "MeshNode.h"
#include "aeongames/Mesh.h"
#include "aeongames/CRC.h"
namespace AeonGames
{
    MeshNode::MeshNode()
    {
        SetName ( "Mesh" );
    }

    MeshNode::~MeshNode() = default;

    static Node::NodeProperty Properties[] =
    {
        {"Mesh", "Mesh", "", [] ( Node * aNode, const void* aTuple ) {}, [] ( const Node * aNode, void* aTuple ) {}}
    };

    size_t MeshNode::GetPropertyCount() const
    {
        return Node::GetPropertyCount() + ( sizeof ( Properties ) / sizeof ( Node::NodeProperty ) );
    }

    const Node::NodeProperty& MeshNode::GetProperty ( size_t aIndex ) const
    {
        if ( aIndex < Node::GetPropertyCount() )
        {
            return Node::GetProperty ( aIndex );
        }
        return Properties[aIndex - Node::GetPropertyCount()];
    }
}
