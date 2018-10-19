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
#ifndef AEONGAMES_VULKANPIPELINE_H
#define AEONGAMES_VULKANPIPELINE_H
#include <cstdint>
#include <string>
#include <array>
#include <vulkan/vulkan.h>
#include "aeongames/Utilities.h"
#include "aeongames/Pipeline.h"
#include "VulkanMaterial.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class Matrix4x4;
    class VulkanWindow;
    class VulkanTexture;
    class VulkanRenderer;
    class VulkanPipeline : public Pipeline
    {
    public:
        VulkanPipeline ( const VulkanRenderer& aVulkanRenderer );
        ~VulkanPipeline() final;
        ///@name Overrides
        ///@{
        void Load ( const std::string& aFilename ) final;
        void Load ( uint32_t aId ) final;
        void Load ( const void* aBuffer, size_t aBufferSize ) final;
        void Load ( const PipelineBuffer& aPipelineBuffer ) final;
        void Unload() final;
        const VulkanMaterial& GetDefaultMaterial() const final;
        ///@}
        const VkPipelineLayout GetPipelineLayout() const;
        const VkPipeline GetPipeline() const;
    private:
        void InitializeDescriptorSetLayout();
        void FinalizeDescriptorSetLayout();
        const VulkanRenderer& mVulkanRenderer;
        VulkanMaterial mDefaultMaterial;
        std::array < VkShaderModule, ffs ( ~VK_SHADER_STAGE_ALL_GRAPHICS ) >
        mVkShaderModules{ { VK_NULL_HANDLE } };
        /** Local DescriptorSetLayout,
         * This layout should exactly match the default material's layout.
         * @note Keeping a copy of the default material layout inside the pipeline class
         * may seem wasteful, but using the one at the default material
         * may in the end be even more wasteful since the default material will need to
         * be initialized even if it is not used and initializing a material just for the
         * DSL means wasting uniforms, textures, a descriptorset and a descriptorpool.
         */
        VkDescriptorSetLayout mVkPropertiesDescriptorSetLayout{ VK_NULL_HANDLE };
        VkPipelineLayout mVkPipelineLayout{ VK_NULL_HANDLE };
        VkPipeline mVkPipeline{ VK_NULL_HANDLE };
    };
}
#endif
