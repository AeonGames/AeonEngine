/*
Copyright (C) 2017-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include <sstream>
#include <iostream>
#include <limits>
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanBuffer::VulkanBuffer ( const VulkanRenderer & aVulkanRenderer ) : mVulkanRenderer ( aVulkanRenderer )
    {
    }
    VulkanBuffer::VulkanBuffer ( const VulkanRenderer& aVulkanRenderer, const VkDeviceSize aSize, const VkBufferUsageFlags aUsage, const VkMemoryPropertyFlags aProperties, const void *aData ) :
        mVulkanRenderer { aVulkanRenderer },
        mSize { aSize },
        mUsage { aUsage },
        mProperties { aProperties }
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

    void VulkanBuffer::CopyBuffer ( const VkBuffer& aBuffer )
    {
        VkCommandBuffer command_buffer = mVulkanRenderer.BeginSingleTimeCommands();
        VkBufferCopy copy_region = {};
        copy_region.size = mSize;
        copy_region.srcOffset  = copy_region.dstOffset = 0;
        vkCmdCopyBuffer ( command_buffer, aBuffer, mBuffer, 1, &copy_region );
        mVulkanRenderer.EndSingleTimeCommands ( command_buffer );
    }

    VulkanBuffer::VulkanBuffer ( const VulkanBuffer& aBuffer ) :
        mVulkanRenderer { aBuffer.mVulkanRenderer },
        mSize{aBuffer.mSize},
        mUsage{aBuffer.mUsage},
        mProperties{aBuffer.mProperties}
    {
        Initialize ( nullptr );
        if ( !mSize )
        {
            return;
        }
        CopyBuffer ( aBuffer.mBuffer );
    }

    VulkanBuffer& VulkanBuffer::operator= ( const VulkanBuffer& aBuffer )
    {
        if ( &mVulkanRenderer != &aBuffer.mVulkanRenderer )
        {
            throw std::runtime_error ( "Assigning buffers from different renderer instances." );
        }
        Finalize();
        mSize = aBuffer.mSize;
        mUsage = aBuffer.mUsage;
        mProperties = aBuffer.mProperties;
        if ( !mSize )
        {
            return *this;
        }
        Initialize ( nullptr );
        CopyBuffer ( aBuffer.mBuffer );
        return *this;
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
        if ( aData )
        {
            if ( ( mProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) )
            {
                void* data = Map ( aOffset, aSize );
                memcpy ( data, aData, aSize );
                Unmap();
            }
            else if ( ( mProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) && ( mUsage & VK_BUFFER_USAGE_TRANSFER_DST_BIT ) )
            {
                VulkanBuffer source ( mVulkanRenderer, aSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, aData );
                VkCommandBuffer command_buffer = mVulkanRenderer.BeginSingleTimeCommands();
                VkBufferCopy copy_region = {};
                copy_region.size = aSize;
                copy_region.srcOffset  = copy_region.dstOffset = aOffset;
                vkCmdCopyBuffer ( command_buffer, source.GetBuffer(), mBuffer, 1, &copy_region );
                mVulkanRenderer.EndSingleTimeCommands ( command_buffer );
            }
        }
    }

    void* VulkanBuffer::Map ( size_t aOffset, size_t aSize ) const
    {
        void* data = nullptr;
        if ( mProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT )
        {
            if ( VkResult result = vkMapMemory ( mVulkanRenderer.GetDevice(), mDeviceMemory, aOffset, aSize, 0, &data ) )
            {
                std::ostringstream stream;
                stream << "vkMapMemory failed for buffer. error code: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        else
        {
            throw std::runtime_error ( "The VkBuffer VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT property must be set to be able to map buffer memory." );
        }
        return data;
    }

    void VulkanBuffer::Unmap() const
    {
        vkUnmapMemory ( mVulkanRenderer.GetDevice(), mDeviceMemory );
    }

    size_t VulkanBuffer::GetSize() const
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
        if ( aData )
        {
            WriteMemory ( 0, mSize, aData );
        }
    }

    void VulkanBuffer::Finalize()
    {
        if ( ( mDeviceMemory != VK_NULL_HANDLE ) || ( mBuffer != VK_NULL_HANDLE ) )
        {
            if ( VkResult result = vkQueueWaitIdle ( mVulkanRenderer.GetQueue() ) )
            {
                std::cout << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
            }
        }
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
