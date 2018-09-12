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

#include "aeongames/ProtoBufClasses.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "reference.pb.h"
#include "model.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace AeonGames
{
    ModelController::~ModelController() = default;

    const std::string& ModelController::GetTypeName() const
    {
        static std::string type_name{"ModelController"};
        return type_name;
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

    void ModelController::CommitPropertyChanges()
    {
        if ( mProperties.has_model() )
        {
            std::shared_ptr<Model> model{Model::GetModel ( GetReferenceBufferId ( mProperties.model() ) ) };
            if ( model != mModel )
            {
                mModel = model;
            }
        }
        else
        {
            mModel.reset();
        }
    }

    void ModelController::Render ( const Node& aNode, const Window& aWindow ) const
    {
        /** @todo Incorporate use of skeleton and animations back */
        if ( mModel )
        {
            for ( auto& i : mModel->GetAssemblies() )
            {
                if ( std::get<0> ( i ) && std::get<1> ( i ) )
                {
                    aWindow.Render ( aNode.GetGlobalTransform(), *std::get<0> ( i ), *std::get<1> ( i ), std::get<2> ( i ).get() );
                }
            }
        }
    }

    const google::protobuf::Message* ModelController::GetProperties() const
    {
        return &mProperties;
    }
    google::protobuf::Message* ModelController::GetProperties()
    {
        return &mProperties;
    }
}
