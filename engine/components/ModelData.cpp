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

#include <array>
#include "ModelData.h"
#include "aeongames/AeonEngine.h"
#include "aeongames/Model.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/Window.h"
#include "aeongames/UniformBuffer.h"
#include "aeongames/Renderer.h"
#include "aeongames/Node.h"

namespace AeonGames
{
    static const StringId ModelStringId{"ModelData"};

    ModelData::ModelData() : NodeData{},
        /// @todo We're hardcoding the skeleton buffer here to the max size, but should be set based on what the model requires.
        mSkeletonBuffer{GetRenderer()->CreateUniformBuffer ( sizeof ( float ) * 16 /*(16 floats in a matrix)*/ * 256 /*(256 maximum bones)*/ ) }
    {
        const float identity[16] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        float* skeleton_buffer = reinterpret_cast<float*> ( mSkeletonBuffer->Map ( 0, mSkeletonBuffer->GetSize() ) );
        for ( size_t i = 0; i < 256; ++i )
        {
            memcpy ( ( skeleton_buffer + ( i * 16 ) ), identity, sizeof ( float ) * 16 );
        }
        mSkeletonBuffer->Unmap();
    }

    ModelData::~ModelData() = default;

    const StringId& ModelData::GetId() const
    {
        return ModelStringId;
    }

    static const std::array<const StringId, 3> ModelDataPropertyIds
    {
        {
            {"Model"},
            {"Active Animation"},
            {"Animation Delta"},
        }
    };

    size_t ModelData::GetPropertyCount () const
    {
        return ModelDataPropertyIds.size();
    }

    const StringId* ModelData::GetPropertyInfoArray () const
    {
        return ModelDataPropertyIds.data();
    }


    Property ModelData::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case "Model"_crc32:
            return GetModel().GetPathString();
        case "Active Animation"_crc32:
            return GetActiveAnimation();
        case "Animation Delta"_crc32:
            return GetAnimationDelta();
        }
        return Property{};
    }

    void ModelData::SetProperty ( const StringId& aId, const Property& aProperty )
    {
        switch ( aId )
        {
        case "Model"_crc32:
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
        case "Active Animation"_crc32:
            if ( std::holds_alternative<size_t> ( aProperty ) )
            {
                SetActiveAnimation ( std::get<size_t> ( aProperty ) );
            }
            break;
        case "Animation Delta"_crc32:
            if ( std::holds_alternative<double> ( aProperty ) )
            {
                SetAnimationDelta ( std::get<double> ( aProperty ) );
            }
            break;
        }
    }

    void ModelData::SetModel ( const ResourceId& aModel ) noexcept
    {
        mModel = aModel;
        mModel.Store();
    }

    const ResourceId& ModelData::GetModel() const noexcept
    {
        return mModel;
    }

    void ModelData::SetActiveAnimation ( size_t aActiveAnimation ) noexcept
    {
        mActiveAnimation = aActiveAnimation;
        mAnimationDelta = 0.0;
    }

    const size_t& ModelData::GetActiveAnimation() const noexcept
    {
        return mActiveAnimation;
    }

    void ModelData::SetAnimationDelta ( double aAnimationDelta ) noexcept
    {
        mAnimationDelta = aAnimationDelta;
    }

    const double& ModelData::GetAnimationDelta() const noexcept
    {
        return mAnimationDelta;
    }

    void ModelData::Update ( Node& aNode, double aDelta )
    {
        /** @todo Add code to update animations. */
        ( void ) aNode;
        mAnimationDelta += aDelta;
        if ( auto model = mModel.Cast<Model>() )
        {
            if ( model->GetSkeleton() && ( model->GetAnimations().size() > mActiveAnimation ) )
            {
                float* skeleton_buffer = reinterpret_cast<float*> ( mSkeletonBuffer->Map ( 0, mSkeletonBuffer->GetSize() ) );
                auto animation = model->GetAnimations() [mActiveAnimation].Cast<Animation>();
                for ( size_t i = 0; i < model->GetSkeleton()->GetJoints().size(); ++i )
                {
                    Matrix4x4 matrix{ ( animation->GetTransform ( i, mAnimationDelta ) *
                                        model->GetSkeleton()->GetJoints() [i].GetInvertedTransform() ) };
                    memcpy ( skeleton_buffer + ( i * 16 ), matrix.GetMatrix4x4(), sizeof ( float ) * 16 );
                }
                mSkeletonBuffer->Unmap();
            }
        }
    }

    void ModelData::Render ( const Node& aNode, const Window& aWindow ) const
    {
        if ( auto model = mModel.Cast<Model>() )
        {
            for ( auto& i : model->GetAssemblies() )
            {
                aWindow.Render ( aNode.GetGlobalTransform(),
                                 *std::get<0> ( i ).Cast<Mesh>(),
                                 *std::get<1> ( i ).Cast<Pipeline>(),
                                 std::get<2> ( i ).Cast<Material>(),
                                 mSkeletonBuffer.get() );
            }
        }
    }
}
