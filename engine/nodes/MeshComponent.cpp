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
#include "MeshComponent.h"
#include "aeongames/Node.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Utilities.h"
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/Window.h"
#include "aeongames/CRC.h"
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

    MeshComponent::~MeshComponent() = default;

    uint32_t MeshComponent::GetMesh() const
    {
        return mMeshId;
    }

    void MeshComponent::SetMesh ( uint32_t aMeshId )
    {
        mMeshId = aMeshId;
        mMesh = Mesh::GetMesh ( mMeshId );
    }

    uint32_t MeshComponent::GetPipeline() const
    {
        return mPipelineId;
    }

    void MeshComponent::SetPipeline ( uint32_t aPipelineId )
    {
        mPipelineId = aPipelineId;
        mPipeline = Pipeline::GetPipeline ( mPipelineId );
    }

    uint32_t MeshComponent::GetTypeId() const
    {
        return "Mesh"_crc32;
    }

    std::vector<uint32_t> MeshComponent::GetDependencies() const
    {
        return std::vector<uint32_t> {};
    }

    void MeshComponent::Update ( Node& aNode, double aDelta )
    {
        ///@todo Should Update and Render not be abstract?
        ( void ) aNode;
        ( void ) aDelta;
    }

    void MeshComponent::Render ( const Node& aNode, const Window& aWindow ) const
    {
        if ( mMesh && mPipeline )
        {
            aWindow.Render ( aNode.GetGlobalTransform(), *mMesh, *mPipeline );
        }
    }
}
