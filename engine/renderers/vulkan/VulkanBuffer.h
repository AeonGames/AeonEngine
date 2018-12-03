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
#ifndef AEONGAMES_VULKANBUFFER_H
#define AEONGAMES_VULKANBUFFER_H
#include <vulkan/vulkan.h>

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanBuffer
    {
    public:
        VulkanBuffer ( const VulkanRenderer& aVulkanRenderer );
        VulkanBuffer ( const VulkanRenderer& aVulkanRenderer, const VkDeviceSize aSize, const VkBufferUsageFlags aUsage, const VkMemoryPropertyFlags aProperties, const void *aData = nullptr );
        /// Copy contsructor.
        VulkanBuffer ( const VulkanBuffer& aBuffer );
        /// No move allowed
        VulkanBuffer ( VulkanBuffer&& ) = delete;
        /// Assignment operator due to rule of zero/three/five.
        VulkanBuffer& operator= ( const VulkanBuffer& aBuffer );
        /// No move assignment allowed
        VulkanBuffer& operator = ( VulkanBuffer&& ) = delete;
        ~VulkanBuffer();
        void Initialize ( const VkDeviceSize aSize, const VkBufferUsageFlags aUsage, const VkMemoryPropertyFlags aProperties, const void *aData = nullptr );
        void Finalize();
        const VkBuffer& GetBuffer() const;
        void WriteMemory ( const VkDeviceSize aOffset, const  VkDeviceSize aSize, const void *aData = nullptr ) const;
        void* Map ( size_t aOffset, size_t aSize ) const;
        void Unmap() const;
        size_t GetSize() const;
    private:
        void Initialize ( const void *aData );
        void CopyBuffer ( const VkBuffer& aBuffer );
        const VulkanRenderer& mVulkanRenderer;
        VkBuffer mBuffer{ VK_NULL_HANDLE };
        VkDeviceMemory mDeviceMemory{ VK_NULL_HANDLE };
        VkDeviceSize mSize{};
        VkBufferUsageFlags mUsage{};
        VkMemoryPropertyFlags mProperties{};
    };
}
#endif
