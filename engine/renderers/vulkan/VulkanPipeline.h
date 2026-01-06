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
#ifndef AEONGAMES_VULKANPIPELINE_H
#define AEONGAMES_VULKANPIPELINE_H
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <limits>
#include <vulkan/vulkan.h>
#include "aeongames/Pipeline.hpp"
#include "VulkanDescriptorSet.h"

// Forward declarations
struct SpvReflectShaderModule;

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanPipeline
    {
    public:
        VulkanPipeline ( const VulkanRenderer&  aVulkanRenderer, const Pipeline& aPipeline );
        ~VulkanPipeline();
        VulkanPipeline ( VulkanPipeline&& aVulkanPipeline );
        VulkanPipeline ( const VulkanPipeline& ) = delete;
        VulkanPipeline& operator= ( const VulkanPipeline& ) = delete;
        VulkanPipeline& operator= ( VulkanPipeline&& ) = delete;
        const VkPipelineLayout GetPipelineLayout() const;
        const VkPipeline GetVkPipeline() const;
        const Pipeline* GetPipeline() const;
        const VkDescriptorSetLayout GetDescriptorSetLayout ( uint32_t name ) const;
        uint32_t GetDescriptorSetIndex ( uint32_t hash ) const;
        const VkPushConstantRange& GetPushConstantModelMatrix() const;
    private:
        void ReflectAttributes ( SpvReflectShaderModule& module );
        void ReflectDescriptorSets ( SpvReflectShaderModule& module, ShaderType aType );
        void ReflectPushConstants ( SpvReflectShaderModule& module, ShaderType aType );
        const VulkanRenderer& mVulkanRenderer;
        const Pipeline* mPipeline{nullptr};
        VkPipelineLayout mVkPipelineLayout{ VK_NULL_HANDLE };
        VkPipeline mVkPipeline{ VK_NULL_HANDLE };
        uint32_t mVertexStride{0};
        std::vector<VkVertexInputAttributeDescription> mVertexAttributes{};
        std::vector<VulkanDescriptorSetInfo> mDescriptorSets{};
        VkPushConstantRange mPushConstantModelMatrix{};
    };
}
#endif
