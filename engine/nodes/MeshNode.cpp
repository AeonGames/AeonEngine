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
#include "aeongames/Utilities.h"
#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "scene.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    MeshNode::MeshNode() : mMeshId{}
    {
        SetName ( "Mesh" );
    }

    MeshNode::~MeshNode() = default;

    uint32_t MeshNode::GetMesh() const
    {
        return mMeshId;
    }

    void MeshNode::SetMesh ( uint32_t aMeshId )
    {
        mMeshId = aMeshId;
    }

    static std::vector<Node::Property> MeshNodeProperties
    {
        {
            "Mesh", "Mesh", "I",
            [] ( Node * aNode, const void* aTuple )
            {
                reinterpret_cast<MeshNode*> ( aNode )->SetMesh ( *reinterpret_cast<const uint32_t*> ( aTuple ) );
            },
            [] ( const Node * aNode, void* aTuple )
            {
                *reinterpret_cast<uint32_t*> ( aTuple ) = reinterpret_cast<const MeshNode*> ( aNode )->GetMesh();
            }
        }
    };

    static const std::vector<std::reference_wrapper<Node::Property>> MeshNodePropertyRefs ( Concatenate ( Node::Properties(), MeshNodeProperties ) );

    const std::vector<std::reference_wrapper<Node::Property>>& MeshNode::GetProperties() const
    {
        return MeshNodePropertyRefs;
    }

    const std::string& MeshNode::GetType() const
    {
        const static std::string type ( "Mesh" );
        return type;
    }
}
