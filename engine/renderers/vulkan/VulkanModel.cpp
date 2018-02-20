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
#include <vector>
#include "aeongames/Model.h"
#include "aeongames/ModelInstance.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/CRC.h"
#include "VulkanRenderer.h"
#include "VulkanWindow.h"
#include "VulkanModel.h"
#include "VulkanPipeline.h"
#include "VulkanMaterial.h"
#include "VulkanMesh.h"
#include "VulkanSkeleton.h"

namespace AeonGames
{
    const size_t VulkanModel::TypeId = "VulkanModel"_id;
    VulkanModel::VulkanModel ( const std::shared_ptr<const Model> aModel, const std::shared_ptr<const VulkanRenderer> aVulkanRenderer ) :
        mModel ( aModel ), mVulkanRenderer ( aVulkanRenderer )
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

    VulkanModel::~VulkanModel()
    {
        Finalize();
    }

    void VulkanModel::Render ( const ModelInstance* aInstance, const Matrix4x4& aProjectionMatrix, const Matrix4x4& aViewMatrix ) const
    {
        if ( mSkeleton && ( mModel.get() == aInstance->GetModel().get() ) )
        {
            mSkeleton->SetPose ( aInstance->GetSkeletonAnimation() );
        }

        for ( size_t i = 0; i < mAssemblies.size(); ++i )
        {
            if ( !aInstance->IsAssemblyEnabled ( i ) )
            {
                continue;
            }
            if ( mSkeleton && ( mModel.get() == aInstance->GetModel().get() ) )
            {
                /* This has to be called outside of a render pass, which currently it is not. */
                VkBufferCopy buffer_copy{};
                buffer_copy.dstOffset = 0;
                buffer_copy.srcOffset = 0;
                buffer_copy.size = mSkeleton->GetBufferSize();
                vkCmdCopyBuffer ( mVulkanRenderer->GetCommandBuffer(), mSkeleton->GetBuffer(), std::get<0> ( mAssemblies[i] )->GetSkeletonBuffer(), 1, &buffer_copy );
            }
            std::get<0> ( mAssemblies[i] )->SetProjectionMatrix ( aProjectionMatrix );
            std::get<0> ( mAssemblies[i] )->SetViewMatrix ( aViewMatrix );
            std::get<0> ( mAssemblies[i] )->Use ( std::get<1> ( mAssemblies[i] ) );
            std::get<2> ( mAssemblies[i] )->Render();
        }
    }

    void VulkanModel::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        auto& meshes = mModel->GetMeshes();
        mAssemblies.reserve ( meshes.size() );
        for ( auto& i : meshes )
        {
            mAssemblies.emplace_back (
                Get<VulkanPipeline> ( std::get<0> ( i ).get(), std::get<0> ( i ), mVulkanRenderer ),
                std::get<1> ( i ) ? Get<VulkanMaterial> ( std::get<1> ( i ).get(), std::get<1> ( i ), mVulkanRenderer ) : nullptr,
                Get<VulkanMesh> ( std::get<2> ( i ).get(), std::get<2> ( i ), mVulkanRenderer ) );
        }
        if ( mModel->GetSkeleton() != nullptr )
        {
            mSkeleton = Get<VulkanSkeleton> ( mModel->GetSkeleton().get(), mModel->GetSkeleton(), mVulkanRenderer );
        }
    }

    void VulkanModel::Finalize()
    {
    }
}
