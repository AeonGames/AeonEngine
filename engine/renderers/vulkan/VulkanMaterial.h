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
#include "aeongames/Material.h"
#include "aeongames/Memory.h"
#include <vulkan/vulkan.h>

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanTexture;
    class VulkanMaterial : public Material::IRenderMaterial
    {
    public:
        VulkanMaterial ( const Material& aMaterial, const std::shared_ptr<const VulkanRenderer>&  aVulkanRenderer );
        ~VulkanMaterial() final;
        void Update ( size_t aOffset, size_t aSize, uint8_t aValue ) final;
        const std::vector<uint8_t>& GetUniformData() const;
        const std::vector<std::shared_ptr<VulkanTexture>>& GetTextures() const;
        const VkDescriptorSet& GetDescriptorSet() const;
        const VkDescriptorSetLayout& GetDescriptorSetLayout() const;
    private:
        void Initialize();
        void Finalize();
        void InitializeDescriptorSetLayout();
        void FinalizeDescriptorSetLayout();
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        void InitializeDescriptorSet();
        void FinalizeDescriptorSet();
        std::shared_ptr<const VulkanRenderer> mVulkanRenderer;
        const Material& mMaterial;
        std::vector<uint8_t> mUniformData;
        VkDescriptorSetLayout mVkDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool mVkDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet mVkDescriptorSet = VK_NULL_HANDLE;
        std::vector<std::shared_ptr<VulkanTexture>> mTextures;
    };
}
#endif
