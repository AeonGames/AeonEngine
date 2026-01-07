/*
Copyright (C) 2017-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANMATERIAL_H
#define AEONGAMES_VULKANMATERIAL_H
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <memory>
#include "aeongames/Pipeline.hpp"
#include "aeongames/Material.hpp"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanTexture;
    class VulkanPipeline;
    class VulkanMaterial
    {
    public:
        VulkanMaterial ( VulkanRenderer&  aVulkanRenderer, const Material& aMaterial );
        VulkanMaterial ( const VulkanMaterial& aMaterial ) = delete;
        VulkanMaterial& operator= ( const VulkanMaterial& aMaterial ) = delete;
        VulkanMaterial& operator= ( VulkanMaterial&& ) = delete;
        VulkanMaterial ( VulkanMaterial&& aVulkanMaterial );
        ~VulkanMaterial();
        void Bind ( VkCommandBuffer aVkCommandBuffer, const VulkanPipeline& aVulkanPipeline ) const;
    private:
        VulkanRenderer& mVulkanRenderer;
        const Material* mMaterial{nullptr};
        void Finalize ();
        VkDescriptorPool mVkDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mUniformDescriptorSet{VK_NULL_HANDLE};
        VkDescriptorSet mSamplerDescriptorSet{VK_NULL_HANDLE};
        VulkanBuffer mUniformBuffer;
    };
}
#endif
