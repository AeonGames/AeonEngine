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

    static Node::PropertyDescriptor PropertyDescriptors[] =
    {
        {"Mesh"_crc32, "Mesh", ""}
    };

    size_t MeshNode::GetPropertyCount() const
    {
        return Node::GetPropertyCount() + ( sizeof ( PropertyDescriptors ) / sizeof ( Node::PropertyDescriptor ) );
    }

    const Node::PropertyDescriptor& MeshNode::GetPropertyDescriptor ( size_t aIndex ) const
    {
        if ( aIndex < Node::GetPropertyCount() )
        {
            return Node::GetPropertyDescriptor ( aIndex );
        }
        return PropertyDescriptors[aIndex - Node::GetPropertyCount()];
    }

    void MeshNode::SetProperty ( uint32_t aPropertyId, const void* aProperty )
    {
        ///@ I'll come back to this one later
        switch ( aPropertyId )
        {
        case "Mesh"_crc32:
            mMesh = reinterpret_cast<Mesh*> ( const_cast<void*> ( aProperty ) );
            break;
        default:
            Node::SetProperty ( aPropertyId, aProperty );
        }
    }

    void MeshNode::GetProperty ( uint32_t aPropertyId, void** aProperty ) const
    {
        switch ( aPropertyId )
        {
        case "Mesh"_crc32:
            ( *aProperty ) = mMesh;
            break;
        default:
            Node::GetProperty ( aPropertyId, aProperty );
        }
    }
}
