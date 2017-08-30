/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#include <vector>
#include <sstream>
#include "aeongames/Skeleton.h"
#include "aeongames/Animation.h"
#include "aeongames/ResourceCache.h"
#include "VulkanRenderer.h"
#include "VulkanSkeleton.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanSkeleton::VulkanSkeleton ( const std::shared_ptr<const Skeleton> aSkeleton, const std::shared_ptr<const VulkanRenderer> aVulkanRenderer ) :
        mSkeleton ( aSkeleton ), mVulkanRenderer ( aVulkanRenderer )
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
        return mSkeletonBuffer;
    }

    void VulkanSkeleton::SetPose ( const std::shared_ptr<const Animation> aAnimation, float aTime ) const
    {
        float* joint_array = nullptr;
        if ( VkResult result = vkMapMemory ( mVulkanRenderer->GetDevice(), mSkeletonMemory, 0, VK_WHOLE_SIZE, 0, reinterpret_cast<void**> ( &joint_array ) ) )
        {
            std::cout << "vkMapMemory failed for uniform buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
        }
        for ( size_t i = 0; i < mSkeleton->GetJoints().size(); ++i )
        {
            ( aAnimation->GetTransform ( i, aTime ) * mSkeleton->GetJoints() [i].GetInvertedTransform() ).GetMatrix ( joint_array + ( i * 16 ) );
        }
        vkUnmapMemory ( mVulkanRenderer->GetDevice(), mSkeletonMemory );
    }

    size_t VulkanSkeleton::GetBufferSize() const
    {
        return mSkeleton->GetJoints().size() * sizeof ( float ) * 16;
    }

    void VulkanSkeleton::Initialize()
    {
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.pNext = nullptr;
        buffer_create_info.flags = 0;
        buffer_create_info.size = GetBufferSize();
        buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.queueFamilyIndexCount = 0;
        buffer_create_info.pQueueFamilyIndices = nullptr;

        if ( VkResult result = vkCreateBuffer ( mVulkanRenderer->GetDevice(), &buffer_create_info, nullptr, &mSkeletonBuffer ) )
        {
            std::ostringstream stream;
            stream << "vkCreateBuffer failed for vertex buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements ( mVulkanRenderer->GetDevice(), mSkeletonBuffer, &memory_requirements );

        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.pNext = nullptr;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = mVulkanRenderer->GetMemoryTypeIndex ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

        if ( memory_allocate_info.memoryTypeIndex == std::numeric_limits<uint32_t>::max() )
        {
            throw std::runtime_error ( "No suitable memory type found for mesh buffers" );
        }

        if ( VkResult result = vkAllocateMemory ( mVulkanRenderer->GetDevice(), &memory_allocate_info, nullptr, &mSkeletonMemory ) )
        {
            std::ostringstream stream;
            stream << "vkAllocateMemory failed for vertex buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        vkBindBufferMemory ( mVulkanRenderer->GetDevice(), mSkeletonBuffer, mSkeletonMemory, 0 );

        float* joint_array = nullptr;
        if ( VkResult result = vkMapMemory ( mVulkanRenderer->GetDevice(), mSkeletonMemory, 0, VK_WHOLE_SIZE, 0, reinterpret_cast<void**> ( &joint_array ) ) )
        {
            std::cout << "vkMapMemory failed for uniform buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
        }

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
        vkUnmapMemory ( mVulkanRenderer->GetDevice(), mSkeletonMemory );
    }

    void VulkanSkeleton::Finalize()
    {
        if ( mSkeletonMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVulkanRenderer->GetDevice(), mSkeletonMemory, nullptr );
            mSkeletonMemory = VK_NULL_HANDLE;
        }
        if ( mSkeletonBuffer != VK_NULL_HANDLE )
        {
            vkDestroyBuffer ( mVulkanRenderer->GetDevice(), mSkeletonBuffer, nullptr );
            mSkeletonBuffer = VK_NULL_HANDLE;
        }
    }
}
