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
#ifndef AEONGAMES_VULKANMATERIAL_H
#define AEONGAMES_VULKANMATERIAL_H
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "aeongames/Material.h"
#include "aeongames/Memory.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanTexture;
    class VulkanMaterial : public Material::IRenderMaterial
    {
    public:
        VulkanMaterial ( const Material& aMaterial, const VulkanRenderer&  aVulkanRenderer );
        ~VulkanMaterial() final;
        void Update ( const uint8_t* aValue, size_t aOffset = 0, size_t aSize = 0 ) final;
        const VkDescriptorSetLayout& GetPropertiesDescriptorSetLayout() const;
        const VkDescriptorSet GetPropertiesDescriptorSet() const;
        const Material& GetMaterial() const;
    private:
        void Initialize();
        void Finalize();
        void InitializeDescriptorSetLayout();
        void FinalizeDescriptorSetLayout();
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        void InitializePropertiesUniform();
        void FinalizePropertiesUniform();
        void InitializeDescriptorSet();
        void FinalizeDescriptorSet();
        const VulkanRenderer& mVulkanRenderer;
        const Material& mMaterial;
        VkDescriptorSetLayout mVkPropertiesDescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorPool mVkPropertiesDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSet mVkPropertiesDescriptorSet{ VK_NULL_HANDLE };
        VulkanBuffer mPropertiesBuffer;
    };
}
#endif
