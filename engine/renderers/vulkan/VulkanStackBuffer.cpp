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
#include <stdexcept>
#include "VulkanStackBuffer.h"

namespace AeonGames
{
    VulkanStackBuffer::VulkanStackBuffer ( const VulkanRenderer& aVulkanRenderer, const VkDeviceSize aSize, const VkBufferUsageFlags aUsage, const VkMemoryPropertyFlags aProperties ) :
        mBuffer ( aVulkanRenderer, aSize, aUsage, aProperties, nullptr )
    {
    }

    VulkanStackBuffer::~VulkanStackBuffer()
        = default;

    VulkanStackBuffer::VulkanStackMemory VulkanStackBuffer::Push ( const VkDeviceSize aSize )
    {
        if ( mBuffer.GetSize() < mTopOfStack + aSize )
        {
            throw std::runtime_error ( "Not enough memory." );
        }
        VkDeviceSize offset = mTopOfStack;
        mTopOfStack += aSize;
        return VulkanStackMemory{ offset, aSize };
    }
    void VulkanStackBuffer::Pop ( const VulkanStackMemory& aVulkanStackMemory )
    {
        if ( mTopOfStack == 0 || mTopOfStack < aVulkanStackMemory.GetSize() || ( mTopOfStack - aVulkanStackMemory.GetSize() != aVulkanStackMemory.GetOffset() ) )
        {
            throw std::runtime_error ( "Vulkan Stack Memory not at the top of the stack." );
        }
        mTopOfStack = aVulkanStackMemory.GetOffset();
    }
    void VulkanStackBuffer::Reset()
    {
        mTopOfStack = 0;
    }

    VulkanStackBuffer::VulkanStackMemory::VulkanStackMemory ( VkDeviceSize aOffset, VkDeviceSize aSize ) :
        mOffset ( aOffset ), mSize ( aSize )
    {
    }

    VulkanStackBuffer::VulkanStackMemory::~VulkanStackMemory()
        = default;

    VkDeviceSize VulkanStackBuffer::VulkanStackMemory::GetOffset() const
    {
        return mOffset;
    }

    VkDeviceSize VulkanStackBuffer::VulkanStackMemory::GetSize() const
    {
        return mSize;
    }
}
