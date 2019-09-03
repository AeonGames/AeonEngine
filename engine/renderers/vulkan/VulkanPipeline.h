/*
Copyright (C) 2017-2019 Rodrigo Jose Hernandez Cordoba

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
    class VulkanBuffer;
    class VulkanRenderer;
    class VulkanPipeline : public Pipeline
    {
    public:
        VulkanPipeline ( const VulkanRenderer& aVulkanRenderer, uint32_t aPath = 0 );
        ~VulkanPipeline() final;
        ///@name Overrides
        ///@{
        void Load ( const PipelineBuffer& aPipelineBuffer ) final;
        void Unload() final;
        ///@}
        const VkPipelineLayout GetPipelineLayout() const;
        const VkPipeline GetPipeline() const;
        void Use ( const VulkanMaterial* aMaterial,
                   const VulkanMaterial* aProjectionView,
                   const Matrix4x4* aModel,
                   const VulkanBuffer* aSkeleton ) const;
    private:
        const VulkanRenderer& mVulkanRenderer;
        std::array < VkShaderModule, ffs ( ~VK_SHADER_STAGE_ALL_GRAPHICS ) >
        mVkShaderModules{ { VK_NULL_HANDLE } };
        VkPipelineLayout mVkPipelineLayout{ VK_NULL_HANDLE };
        VkPipeline mVkPipeline{ VK_NULL_HANDLE };
    };
}
#endif
