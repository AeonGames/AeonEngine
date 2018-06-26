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

    void MeshNode::SetProperty ( uint32_t aPropertyId, void* aProperty )
    {
        switch ( aPropertyId )
        {
        case "Mesh"_crc32:
            mMesh = reinterpret_cast<Mesh*> ( aProperty );
            break;
        }
    }
    const void* MeshNode::GetProperty ( uint32_t aPropertyId ) const
    {
        switch ( aPropertyId )
        {
        case "Mesh"_crc32:
            return mMesh;
            break;
        }
        return nullptr;
    }
}
