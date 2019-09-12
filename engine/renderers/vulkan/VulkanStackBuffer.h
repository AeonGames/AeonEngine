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
#ifndef AEONGAMES_VULKANSTACKBUFFER_H
#define AEONGAMES_VULKANSTACKBUFFER_H
#include <cstdint>
#include <string>
#include <vector>
#include <tuple>
#include <initializer_list>
#include <vulkan/vulkan.h>
#include "aeongames/Material.h"
#include "aeongames/Memory.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanStackBuffer
    {
    public:
        VulkanStackBuffer ( const VulkanRenderer&  aVulkanRenderer, size_t aStackSize );
        VulkanStackBuffer ( const VulkanRenderer& ) = delete;
        VulkanStackBuffer& operator= ( const VulkanStackBuffer& ) = delete;
        VulkanStackBuffer& operator = ( VulkanStackBuffer&& ) = delete;
        VulkanStackBuffer ( VulkanStackBuffer&& ) = delete;
        ~VulkanStackBuffer();
    private:
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        void InitializeDescriptorSet();
        void FinalizeDescriptorSet();
        const VulkanRenderer& mVulkanRenderer;
        VkDescriptorPool mVkDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSet mVkDescriptorSet{ VK_NULL_HANDLE };
        VulkanBuffer mUniformBuffer;
    };
}
#endif
