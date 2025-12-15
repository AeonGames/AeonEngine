/*
Copyright (C) 2017-2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include <vulkan/vulkan.h>
#include "aeongames/Pipeline.hpp"
#include "VulkanVariable.h"
#include "VulkanDescriptorSet.h"

// Forward declarations
struct SpvReflectShaderModule;

namespace AeonGames
{
    enum  BindingLocations : uint32_t
    {
        MATRICES = 0,
        MATERIAL,
        SAMPLERS,
        SKELETON,
    };

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
        const std::vector<VulkanVariable>& GetVertexAttributes() const;
        const std::vector<VulkanSamplerLocation>& GetSamplers() const;
        const VkDescriptorSetLayout GetDescriptorSetLayout ( uint32_t name ) const;
        //const VulkanDescriptorSetBinding* GetUniformBlock ( uint32_t name ) const;
        const uint32_t GetSamplerBinding ( uint32_t name_hash ) const;
    private:
        void ReflectAttributes ( SpvReflectShaderModule& module );
        void ReflectDescriptorSets ( SpvReflectShaderModule& module, ShaderType aType );
        const VulkanRenderer& mVulkanRenderer;
        const Pipeline* mPipeline{nullptr};
        VkPipelineLayout mVkPipelineLayout{ VK_NULL_HANDLE };
        VkPipeline mVkPipeline{ VK_NULL_HANDLE };
        uint32_t mDefaultStride{0};
        std::vector<VulkanVariable> mAttributes{};
        std::vector<VulkanVariable> mUniforms{};
        std::vector<VulkanDescriptorSetInfo> mDescriptorSets{};
        std::vector<VulkanSamplerLocation> mSamplerLocations{};
    };
}
#endif
