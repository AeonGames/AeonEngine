/*
Copyright (C) 2018,2019,2021 Rodrigo Jose Hernandez Cordoba

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

#include <array>
#include <cstring>
#include <cmath>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/AeonEngine.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/Window.h"
#include "aeongames/Buffer.h"
#include "aeongames/Renderer.h"
#include "aeongames/Node.h"
#include "aeongames/Model.h"
#include "ModelComponent.h"

namespace AeonGames
{
    static const StringId ModelStringId{"Model Component"};
    const StringId& ModelComponent::GetClassId()
    {
        return ModelStringId;
    }

    ModelComponent::ModelComponent() = default;
    ModelComponent::~ModelComponent() = default;

    const StringId& ModelComponent::GetId() const
    {
        return ModelStringId;
    }

    static constexpr std::array<const StringId, 3> ModelComponentPropertyIds
    {
        {
            {"Model"},
            {"Active Animation"},
            {"Starting Frame"},
        }
    };

    size_t ModelComponent::GetPropertyCount () const
    {
        return ModelComponentPropertyIds.size();
    }

    const StringId* ModelComponent::GetPropertyInfoArray () const
    {
        return ModelComponentPropertyIds.data();
    }

    Property ModelComponent::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case ModelComponentPropertyIds[0]:
            return GetModel().GetPathString();
        case ModelComponentPropertyIds[1]:
            return GetActiveAnimation();
        case ModelComponentPropertyIds[2]:
            return GetStartingFrame();
        }
        return Property{};
    }

    void ModelComponent::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        switch ( aId )
        {
        case ModelComponentPropertyIds[0]:
            if ( std::holds_alternative<std::string> ( aProperty ) )
            {
                SetModel ( {"Model"_crc32, std::get<std::string> ( aProperty ) } );
            }
            else if ( std::holds_alternative<std::filesystem::path> ( aProperty ) )
            {
                SetModel ( {"Model"_crc32, std::get<std::filesystem::path> ( aProperty ).string() } );
            }
            else if ( std::holds_alternative<uint32_t> ( aProperty ) )
            {
                SetModel ( {"Model"_crc32, std::get<uint32_t> ( aProperty ) } );
            }
            break;
        case ModelComponentPropertyIds[1]:
            if ( std::holds_alternative<size_t> ( aProperty ) )
            {
                SetActiveAnimation ( std::get<size_t> ( aProperty ) );
            }
            break;
        case ModelComponentPropertyIds[2]:
            if ( std::holds_alternative<double> ( aProperty ) )
            {
                SetStartingFrame ( std::get<double> ( aProperty ) );
            }
            break;
        }
    }

    void ModelComponent::SetModel ( const ResourceId& aModel ) noexcept
    {
        mModel = aModel;
        mModel.Store();
    }

    const ResourceId& ModelComponent::GetModel() const noexcept
    {
        return mModel;
    }

    void ModelComponent::SetActiveAnimation ( size_t aActiveAnimation ) noexcept
    {
        mActiveAnimation = aActiveAnimation;
        mCurrentSample = mStartingFrame;
    }

    const size_t& ModelComponent::GetActiveAnimation() const noexcept
    {
        return mActiveAnimation;
    }

    void ModelComponent::SetStartingFrame ( double aAnimationDelta ) noexcept
    {
        mStartingFrame = mCurrentSample = aAnimationDelta;
    }

    const double& ModelComponent::GetStartingFrame() const noexcept
    {
        return mStartingFrame;
    }

    void ModelComponent::Update ( Node& aNode, double aDelta, Window* aWindow )
    {
        if ( auto model = mModel.Cast<Model>() )
        {
            AABB aabb;
            for ( auto& i : model->GetAssemblies() )
            {
                if ( Mesh* mesh = std::get<0> ( i ).Cast<Mesh>() )
                {
                    aabb += mesh->GetAABB();
                }
            }
            aNode.SetAABB ( aabb );
            if ( aWindow != nullptr && model->GetSkeleton() && ( model->GetAnimations().size() > mActiveAnimation ) )
            {
                mSkeleton = aWindow->AllocateSingleFrameUniformMemory ( model->GetSkeleton()->GetJoints().size() * sizeof ( float ) * 16 );
                auto* skeleton_buffer = reinterpret_cast<float*> ( mSkeleton.Map ( 0, mSkeleton.GetSize() ) );
                auto animation = model->GetAnimations() [mActiveAnimation].Cast<Animation>();
                mCurrentSample = animation->AddTimeToSample ( mCurrentSample, aDelta );
                for ( size_t i = 0; i < model->GetSkeleton()->GetJoints().size(); ++i )
                {
                    Matrix4x4 matrix{ ( animation->GetTransform ( i, mCurrentSample ) *
                                        model->GetSkeleton()->GetJoints() [i].GetInvertedTransform() ) };
                    memcpy ( skeleton_buffer + ( i * 16 ), matrix.GetMatrix4x4(), sizeof ( float ) * 16 );
                }
                mSkeleton.Unmap();
            }
        }
    }

    void ModelComponent::Render ( const Node& aNode, const Window& aWindow ) const
    {
        if ( auto model = mModel.Cast<Model>() )
        {
            for ( auto& i : model->GetAssemblies() )
            {
                aWindow.Render ( aNode.GetGlobalTransform(),
                                 *std::get<0> ( i ).Cast<Mesh>(),
                                 *std::get<1> ( i ).Cast<Pipeline>(),
                                 std::get<2> ( i ).Cast<Material>(),
                                 ( model->GetSkeleton() && ( model->GetAnimations().size() > mActiveAnimation ) ) ? &mSkeleton : nullptr );
            }
        }
    }

    void ModelComponent::ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData )
    {
    }
}
