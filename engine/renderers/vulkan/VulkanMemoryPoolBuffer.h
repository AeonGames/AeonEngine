/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANMEMORYPOOLBUFFER_H
#define AEONGAMES_VULKANMEMORYPOOLBUFFER_H
#include <cstdint>
#include <string>
#include <vector>
#include <tuple>
#include <initializer_list>
#include <vulkan/vulkan.h>
#include "aeongames/BufferAccessor.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanMemoryPoolBuffer
    {
    public:
        VulkanMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer, size_t aStackSize );
        VulkanMemoryPoolBuffer ( const VulkanRenderer& ) = delete;
        VulkanMemoryPoolBuffer& operator= ( const VulkanMemoryPoolBuffer& ) = delete;
        VulkanMemoryPoolBuffer& operator = ( VulkanMemoryPoolBuffer&& ) = delete;
        VulkanMemoryPoolBuffer ( VulkanMemoryPoolBuffer&& ) = delete;
        ~VulkanMemoryPoolBuffer();
        BufferAccessor Allocate ( size_t aSize );
        void Reset();
        const VkDescriptorSet& GetDescriptorSet() const;
    private:
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        void InitializeDescriptorSet();
        void FinalizeDescriptorSet();
        const VulkanRenderer& mVulkanRenderer;
        size_t mOffset{0};
        VkDescriptorPool mVkDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSet mVkDescriptorSet{ VK_NULL_HANDLE };
        VulkanBuffer mUniformBuffer;
    };
}
#endif
