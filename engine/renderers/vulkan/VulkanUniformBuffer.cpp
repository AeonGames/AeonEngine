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
#include "VulkanUniformBuffer.h"

namespace AeonGames
{
    VulkanUniformBuffer::VulkanUniformBuffer ( const VulkanRenderer& aVulkanRenderer ) : mBuffer{aVulkanRenderer} {}
    VulkanUniformBuffer::VulkanUniformBuffer ( const VulkanRenderer& aVulkanRenderer, const VkDeviceSize aSize, const void *aData ) :
        mBuffer{aVulkanRenderer, aSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, aData} {}
    VulkanUniformBuffer::VulkanUniformBuffer ( const VulkanUniformBuffer& aBuffer ) : mBuffer{aBuffer.mBuffer} {}
    VulkanUniformBuffer& VulkanUniformBuffer::operator= ( const VulkanUniformBuffer& aBuffer )
    {
        mBuffer = aBuffer.mBuffer;
        return *this;
    }

    const VkBuffer& VulkanUniformBuffer::GetBuffer() const
    {
        return mBuffer.GetBuffer();
    }

    void* VulkanUniformBuffer::Map ( size_t aOffset, size_t aSize ) const
    {
        return mBuffer.Map ( aOffset, aSize );
    }
    void VulkanUniformBuffer::Unmap() const
    {
        return mBuffer.Unmap();
    }
    size_t VulkanUniformBuffer::GetSize() const
    {
        return mBuffer.GetSize();
    }
}