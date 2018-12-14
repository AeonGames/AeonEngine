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
#include "aeongames/StringId.h"
#include "ModelData.h"

namespace AeonGames
{
    ModelController::ModelController() :
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

#if 0
    std::vector<PropertyRef> ModelController::GetProperties() const
    {
        /** @todo This is not going to work,
         * we need to know when a property is being set or retrieved.
         * in this particular case, we need to know when model is set so we can
         * change the skeleton acordingly. */
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
#endif

    void ModelController::Render ( const Node& aNode, const Window& aWindow ) const
    {
        /** @todo Incorporate use of skeleton and animations back */
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
