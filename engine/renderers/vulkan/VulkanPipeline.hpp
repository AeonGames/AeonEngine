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
#ifndef AEONGAMES_VULKANPIPELINE_HPP
#define AEONGAMES_VULKANPIPELINE_HPP
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <limits>
#include <vulkan/vulkan.h>
#include "aeongames/Pipeline.hpp"
#include "VulkanDescriptorSet.hpp"

// Forward declarations
struct SpvReflectShaderModule;

namespace AeonGames
{
    class VulkanRenderer;
    /** @brief Vulkan graphics pipeline with descriptor set and push constant reflection. */
    class VulkanPipeline
    {
    public:
        /// @brief Construct from a renderer and pipeline resource.
        VulkanPipeline ( const VulkanRenderer&  aVulkanRenderer, const Pipeline& aPipeline );
        ~VulkanPipeline();
        /// @brief Move constructor.
        VulkanPipeline ( VulkanPipeline&& aVulkanPipeline );
        VulkanPipeline ( const VulkanPipeline& ) = delete;
        VulkanPipeline& operator= ( const VulkanPipeline& ) = delete;
        VulkanPipeline& operator= ( VulkanPipeline&& ) = delete;
        /// @brief Get the Vulkan pipeline layout handle.
        const VkPipelineLayout GetPipelineLayout() const;
        /// @brief Get the Vulkan pipeline handle.
        const VkPipeline GetVkPipeline() const;
        /// @brief Get the source Pipeline resource.
        const Pipeline* GetPipeline() const;
        /// @brief Get a descriptor set layout by its name hash.
        const VkDescriptorSetLayout GetDescriptorSetLayout ( uint32_t name ) const;
        /// @brief Get the descriptor set index for a given hash.
        uint32_t GetDescriptorSetIndex ( uint32_t hash ) const;
        /// @brief Get the push constant range used for the model matrix.
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
