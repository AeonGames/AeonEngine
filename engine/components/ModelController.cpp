/*
Copyright (C) 2018,2019 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include "ModelController.h"
#include "aeongames/Node.h"
#include "aeongames/Model.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Utilities.h"
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/Window.h"
#include "ModelData.h"

namespace AeonGames
{
    ModelController::~ModelController() = default;

    const char* ModelController::GetTypeName() const
    {
        return "ModelController";
    }

    uint32_t ModelController::GetTypeId() const
    {
        return "ModelController"_crc32;
    }

    std::vector<uint32_t> ModelController::GetDependencies() const
    {
        return std::vector<uint32_t> {};
    }

    void ModelController::Update ( Node& aNode, double aDelta )
    {
        if ( ModelData* model_data = reinterpret_cast<ModelData*> ( aNode.GetData ( "ModelData"_crc32 ) ) )
        {
            model_data->Update ( aNode, aDelta );
        }
    }

    void ModelController::Render ( const Node& aNode, const Window& aWindow ) const
    {
        if ( ModelData* model_data = reinterpret_cast<ModelData*> ( aNode.GetData ( "ModelData"_crc32 ) ) )
        {
            model_data->Render ( aNode, aWindow );
        }
    }
}
