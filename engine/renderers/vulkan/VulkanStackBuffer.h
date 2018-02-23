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
#ifndef AEONGAMES_VULKANSTACKBUFFER_H
#define AEONGAMES_VULKANSTACKBUFFER_H
#include "aeongames/Memory.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanStackBuffer
    {
    public:
        class VulkanStackMemory
        {
        public:
            VulkanStackMemory ( VkDeviceSize aOffset, VkDeviceSize aSize );
            ~VulkanStackMemory();
            VkDeviceSize GetOffset() const;
            VkDeviceSize GetSize() const;
        private:
            VkDeviceSize mOffset{};
            VkDeviceSize mSize{};
        };
        VulkanStackBuffer ( const VulkanRenderer& aVulkanRenderer, const VkDeviceSize aSize, const VkBufferUsageFlags aUsage, const VkMemoryPropertyFlags aProperties );
        ~VulkanStackBuffer();
        VulkanStackMemory Push ( const VkDeviceSize aSize );
        void Pop ( const VulkanStackMemory& aVulkanStackMemory );
        void Reset();
    private:
        VkDeviceSize mTopOfStack{};
        VulkanBuffer mBuffer;
    };
}
#endif
