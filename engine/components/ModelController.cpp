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
#include "aeongames/CRC.h"

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
        /** @todo Add code to update animations. */
        ( void ) aNode;
        ( void ) aDelta;
    }

    std::vector<PropertyRef> ModelController::GetProperties() const
    {
        return std::vector<PropertyRef>
        {
            {
                {
                    "Model", mModel
                },
                {
                    "Active Animation", mActiveAnimation
                },
                {
                    "Animation Delta", mAnimationDelta
                }
            }
        };
    }

    void ModelController::Render ( const Node& aNode, const Window& aWindow ) const
    {
        /** @todo Incorporate use of skeleton and animations back */
        if ( mModel.GetPath() )
        {
            for ( auto& i : mModel.Cast<Model>()->GetAssemblies() )
            {
                aWindow.Render ( aNode.GetGlobalTransform(),
                                 *std::get<0> ( i ).Cast<Mesh>(),
                                 *std::get<1> ( i ).Cast<Pipeline>(),
                                 std::get<2> ( i ).Cast<Material>() );
            }
        }
    }
}
