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
#ifndef AEONGAMES_VULKANPIPELINE_H
#define AEONGAMES_VULKANPIPELINE_H
#include <cstdint>
#include <string>
#include <array>
#include <vulkan/vulkan.h>
#include "aeongames/Utilities.h"

namespace AeonGames
{
    class Pipeline;
    class VulkanWindow;
    class VulkanTexture;
    class VulkanRenderer;
    class VulkanMaterial;
    class VulkanPipeline
    {
    public:
        VulkanPipeline ( const std::shared_ptr<const Pipeline> aPipeline, const std::shared_ptr<const VulkanRenderer> aVulkanRenderer );
        ~VulkanPipeline();
        void Use ( const std::shared_ptr<VulkanMaterial>& aMaterial = nullptr ) const;
        VkBuffer GetSkeletonBuffer() const;
    private:
        void InitializePropertiesUniform();
        void FinalizePropertiesUniform();
        void InitializeSkeletonUniform();
        void FinalizeSkeletonUniform();
        void InitializeDescriptorSetLayout();
        void FinalizeDescriptorSetLayout();
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        void InitializeDescriptorSet();
        void FinalizeDescriptorSet();
        void Initialize();
        void Finalize();
        std::shared_ptr<const Pipeline> mPipeline;
        std::shared_ptr<const VulkanRenderer> mVulkanRenderer;
        std::array < VkShaderModule, ffs ( ~VK_SHADER_STAGE_ALL_GRAPHICS ) >
        mVkShaderModules{ {VK_NULL_HANDLE} };
        const std::shared_ptr<VulkanMaterial> mDefaultMaterial;
        VkPipelineLayout mVkPipelineLayout = VK_NULL_HANDLE;
        VkPipeline mVkPipeline = VK_NULL_HANDLE;
        VkBuffer mVkPropertiesUniformBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mVkPropertiesUniformMemory = VK_NULL_HANDLE;
        VkBuffer mVkSkeletonBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mVkSkeletonMemory = VK_NULL_HANDLE;
        VkDescriptorSetLayout mVkDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool mVkDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet mVkDescriptorSet = VK_NULL_HANDLE;
    };
}
#endif
