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
#include "ModelComponent.h"
#include "aeongames/Node.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Utilities.h"
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/Window.h"
#include "aeongames/CRC.h"

namespace AeonGames
{
    ModelComponent::~ModelComponent() = default;

    uint32_t ModelComponent::GetTypeId() const
    {
        return "Model"_crc32;
    }

    std::vector<uint32_t> ModelComponent::GetDependencies() const
    {
        return std::vector<uint32_t> {};
    }

    void ModelComponent::Update ( Node& aNode, double aDelta )
    {
        ///@todo Should Update and Render not be abstract?
        ( void ) aNode;
        ( void ) aDelta;
    }

    void ModelComponent::Render ( const Node& aNode, const Window& aWindow ) const
    {
        for ( auto& i : mMeshes )
        {
            if ( std::get<0> ( i ) && std::get<1> ( i ) )
            {
                aWindow.Render ( aNode.GetGlobalTransform(), *std::get<0> ( i ), *std::get<1> ( i ), std::get<2> ( i ).get() );
            }
        }
    }
    const google::protobuf::Message* ModelComponent::GetProperties() const
    {
        return &mProperties;
    }
}
