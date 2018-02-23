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
#include <cstring>
#include <utility>
#include <vector>
#include <sstream>
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/ResourceCache.h"
#include "VulkanRenderer.h"
#include "VulkanSkeleton.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanSkeleton::VulkanSkeleton ( const std::shared_ptr<const Skeleton>&  aSkeleton, const std::shared_ptr<const VulkanRenderer>& aVulkanRenderer ) :
        mVulkanRenderer ( aVulkanRenderer ), mSkeleton ( aSkeleton ),
        mSkeletonBuffer ( *aVulkanRenderer,
                          mSkeleton->GetJoints().size() * sizeof ( float ) * 16,
                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT )
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

    VulkanSkeleton::~VulkanSkeleton()
    {
        Finalize();
    }

    VkBuffer VulkanSkeleton::GetBuffer() const
    {
        return mSkeletonBuffer.GetBuffer();
    }

    void VulkanSkeleton::SetPose ( const std::vector<Matrix4x4>& aSkeleton ) const
    {
        mSkeletonBuffer.WriteMemory ( 0, aSkeleton.size() * sizeof ( Matrix4x4 ), aSkeleton.data() );
    }

    size_t VulkanSkeleton::GetBufferSize() const
    {
        return mSkeleton->GetJoints().size() * sizeof ( float ) * 16;
    }

    void VulkanSkeleton::Initialize()
    {
        auto* joint_array = static_cast<float*> ( mSkeletonBuffer.Map ( 0, VK_WHOLE_SIZE ) );
        const float identity[16] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        for ( size_t i = 0; i < mSkeleton->GetJoints().size(); ++i )
        {
            memcpy ( ( joint_array + ( i * 16 ) ), identity, sizeof ( float ) * 16 );
        }
        mSkeletonBuffer.Unmap();
    }

    void VulkanSkeleton::Finalize()
    {
    }
}
