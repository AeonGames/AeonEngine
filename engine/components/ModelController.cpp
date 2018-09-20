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

    bool ModelController::EnumerateProperties ( size_t* aPropertyCount, PropertyRecord* aRecords ) const
    {
        if ( !aPropertyCount )
        {
            return false;
        }
        else if ( !aRecords )
        {
            *aPropertyCount = 0;
            return true;
        }
        size_t i{0};
        for ( ; i < *aPropertyCount || i < mProperties.size(); ++i )
        {
            aRecords[i].Name = mProperties[i].GetName();
            aRecords[i].TypeIndex = std::type_index{mProperties[i].GetTypeInfo() };
        }
        *aPropertyCount = i;
        return true;
    }

    void ModelController::SetProperty ( const char* aName, const TypedPointer& aValue )
    {
        auto i = std::find_if ( mProperties.begin(), mProperties.end(), [aName] ( const Property<32>& aProperty )
        {
            return strcmp ( aProperty.GetName(), aName ) == 0;
        } );
        if ( i != mProperties.end() )
        {
            i->Set ( aValue );
            if ( strcmp ( aName, "Model" ) == 0 )
            {
                std::shared_ptr<Model> model{Model::GetModel ( *i->Get().Get<std::string>() ) };
                if ( model != mModel )
                {
                    mModel = model;
                }
            }
        }
    }

    const TypedPointer ModelController::GetProperty ( const char* aName ) const
    {
        auto i = std::find_if ( mProperties.begin(), mProperties.end(), [aName] ( const Property<32>& aProperty )
        {
            return strcmp ( aProperty.GetName(), aName ) == 0;
        } );
        if ( i != mProperties.end() )
        {
            return i->Get();
        }
        return TypedPointer{};
    }

    void ModelController::SetProperty ( size_t aIndex, const TypedPointer& aValue )
    {
        mProperties[aIndex].Set ( aValue );
        if ( strcmp ( mProperties[aIndex].GetName(), "Model" ) == 0 )
        {
            std::shared_ptr<Model> model{Model::GetModel ( *mProperties[aIndex].Get().Get<std::string>() ) };
            if ( model != mModel )
            {
                mModel = model;
            }
        }
    }

    const TypedPointer ModelController::GetProperty ( size_t aIndex ) const
    {
        return mProperties[aIndex].Get();
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
        return &mModelControllerBuffer;
    }
    google::protobuf::Message* ModelController::GetProperties()
    {
        return &mModelControllerBuffer;
    }
}
