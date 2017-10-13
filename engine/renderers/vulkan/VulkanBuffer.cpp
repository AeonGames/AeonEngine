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
#include "aeongames/Matrix4x4.h"
#include "aeongames/ResourceCache.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanBuffer::VulkanBuffer ( const VulkanRenderer & aVulkanRenderer ) : mVulkanRenderer ( aVulkanRenderer )
    {
    }
    VulkanBuffer::VulkanBuffer ( const VulkanRenderer& aVulkanRenderer, const VkDeviceSize aSize, const VkBufferUsageFlags aUsage, const VkMemoryPropertyFlags aProperties, const void *aData ) :
        mVulkanRenderer ( aVulkanRenderer ), mSize ( aSize ), mUsage ( aUsage ), mProperties ( aProperties )
    {
        try
        {
            Initialize ( aData );
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    VulkanBuffer::~VulkanBuffer()
    {
        Finalize();
    }

    void VulkanBuffer::Initialize ( const VkDeviceSize aSize, const VkBufferUsageFlags aUsage, const VkMemoryPropertyFlags aProperties, const void * aData )
    {
        if ( mDeviceMemory != VK_NULL_HANDLE || mBuffer != VK_NULL_HANDLE )
        {
            throw ( std::runtime_error ( "Buffer already initialized." ) );
        }
        mSize = aSize;
        mUsage = aUsage;
        mProperties = aProperties;
        Initialize ( aData );
    }

    const VkBuffer& VulkanBuffer::GetBuffer() const
    {
        return mBuffer;
    }

    void VulkanBuffer::WriteMemory ( const VkDeviceSize aOffset, const VkDeviceSize aSize, const void * aData ) const
    {
        if ( ( aData ) && ( mProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) )
        {
            void* data = Map ( aOffset, aSize );
            memcpy ( data, aData, mSize );
            Unmap();
        }
    }

    void * VulkanBuffer::Map ( const VkDeviceSize aOffset, const VkDeviceSize aSize ) const
    {
        void* data = nullptr;
        if ( VkResult result = vkMapMemory ( mVulkanRenderer.GetDevice(), mDeviceMemory, aOffset, aSize, 0, &data ) )
        {
            std::cout << "vkMapMemory failed for buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
        }
        return data;
    }

    void VulkanBuffer::Unmap() const
    {
        vkUnmapMemory ( mVulkanRenderer.GetDevice(), mDeviceMemory );
    }

    VkDeviceSize VulkanBuffer::GetSize() const
    {
        return mSize;
    }

    void VulkanBuffer::Initialize ( const void* aData )
    {
        if ( !mSize )
        {
            return;
        }
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.pNext = nullptr;
        buffer_create_info.flags = 0;
        buffer_create_info.size = mSize;
        buffer_create_info.usage = mUsage;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.queueFamilyIndexCount = 0;
        buffer_create_info.pQueueFamilyIndices = nullptr;

        if ( VkResult result = vkCreateBuffer ( mVulkanRenderer.GetDevice(), &buffer_create_info, nullptr, &mBuffer ) )
        {
            std::ostringstream stream;
            stream << "vkCreateBuffer failed for vertex buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements ( mVulkanRenderer.GetDevice(), mBuffer, &memory_requirements );

        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.pNext = nullptr;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = mVulkanRenderer.GetMemoryTypeIndex ( mProperties );

        if ( memory_allocate_info.memoryTypeIndex == std::numeric_limits<uint32_t>::max() )
        {
            throw std::runtime_error ( "No suitable memory type found for buffer." );
        }

        if ( VkResult result = vkAllocateMemory ( mVulkanRenderer.GetDevice(), &memory_allocate_info, nullptr, &mDeviceMemory ) )
        {
            std::ostringstream stream;
            stream << "vkAllocateMemory failed for buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        vkBindBufferMemory ( mVulkanRenderer.GetDevice(), mBuffer, mDeviceMemory, 0 );

        WriteMemory ( 0, mSize, aData );
    }

    void VulkanBuffer::Finalize()
    {
        if ( mDeviceMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVulkanRenderer.GetDevice(), mDeviceMemory, nullptr );
            mDeviceMemory = VK_NULL_HANDLE;
        }
        if ( mBuffer != VK_NULL_HANDLE )
        {
            vkDestroyBuffer ( mVulkanRenderer.GetDevice(), mBuffer, nullptr );
            mBuffer = VK_NULL_HANDLE;
        }
    }
}
