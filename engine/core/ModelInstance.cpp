/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include <cmath>
#include "aeongames/Model.h"
#include "aeongames/Mesh.h"
#include "aeongames/Material.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Utilities.h"
#include "aeongames/ModelInstance.h"
#include "aeongames/Animation.h"
#include "aeongames/Skeleton.h"
#include "aeongames/Matrix4x4.h"

namespace AeonGames
{
    const size_t ModelInstance::TypeId = std::hash<std::string> {} ( "ModelInstance" );
    ModelInstance::ModelInstance ( const std::shared_ptr<const Model> aModel ) :
        mModel ( aModel ),
        mEnabledAssemblies ( mModel->GetMeshes().size(), true ),
        mAnimationIndex ( 0 ),
        mAnimationTime ( 0.0f ),
        mSkeletonAnimation ( mModel->GetSkeleton() ? mModel->GetSkeleton()->GetJoints().size() : 0 )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    ModelInstance::~ModelInstance()
    {
        Finalize();
    }

    void ModelInstance::EnableAssembly ( size_t aAssemblyIndex, bool aEnabled )
    {
        if ( mEnabledAssemblies.size() < aAssemblyIndex )
        {
            mEnabledAssemblies[aAssemblyIndex] = aEnabled;
        }
    }

    void ModelInstance::EnableAllAssemblies ( bool aEnabled )
    {
        mEnabledAssemblies.assign ( mEnabledAssemblies.size(), aEnabled );
    }

    size_t ModelInstance::GetAnimationIndex () const
    {
        return mAnimationIndex;
    }

    void ModelInstance::SetAnimationIndex ( size_t aAnimationIndex )
    {
        mAnimationIndex = aAnimationIndex;
        // Reset Animation Time
        mAnimationTime = 0.0f;
    }

    double ModelInstance::GetAnimationTime() const
    {
        return mAnimationTime;
    }

    void ModelInstance::SetAnimationTime ( double aTime )
    {
        if ( mModel->GetAnimations().size() > mAnimationIndex )
        {
            double duration = mModel->GetAnimations() [mAnimationIndex]->GetDuration();
            if ( ( aTime <= duration ) && ( aTime >= 0.0 ) )
            {
                mAnimationTime = aTime;
            }
            else if ( duration > aTime )
            {
                mAnimationTime = duration;
            }
            else // if (aTime < 0.0f)
            {
                mAnimationTime = 0.0f;
            }
            UpdateSkeletonAnimation();
        }
    }

    void ModelInstance::StepAnimation ( double aDelta )
    {
        if ( mModel->GetAnimations().size() > mAnimationIndex )
        {
            mAnimationTime = fmod ( mAnimationTime + aDelta, mModel->GetAnimations() [mAnimationIndex]->GetDuration() );
            UpdateSkeletonAnimation();
        }
    }

    const std::vector<Matrix4x4>& ModelInstance::GetSkeletonAnimation() const
    {
        return mSkeletonAnimation;
    }

    const std::shared_ptr<const Model>& ModelInstance::GetModel() const
    {
        return mModel;
    }

    bool ModelInstance::IsAssemblyEnabled ( size_t aAssemblyIndex ) const
    {
        return mEnabledAssemblies[aAssemblyIndex];
    }

    void ModelInstance::Update ( Node* aNode, const double delta )
    {
        StepAnimation ( delta );
    }

    void ModelInstance::Initialize()
    {
    }

    void ModelInstance::Finalize()
    {
    }

    void ModelInstance::UpdateSkeletonAnimation()
    {
        if ( mModel->GetSkeleton() && ( mModel->GetAnimations().size() > mAnimationIndex ) )
        {
            for ( size_t i = 0; i < mModel->GetSkeleton()->GetJoints().size(); ++i )
            {
                mSkeletonAnimation[i] =
                    ( mModel->GetAnimations() [mAnimationIndex]->GetTransform ( i, mAnimationTime ) *
                      mModel->GetSkeleton()->GetJoints() [i].GetInvertedTransform() );
            }
        }
    }
}
