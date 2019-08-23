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
#ifndef AEONGAMES_VULKANMATERIAL_H
#define AEONGAMES_VULKANMATERIAL_H
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
    class VulkanTexture;
    class VulkanMaterial : public Material
    {
    public:
        VulkanMaterial ( const VulkanRenderer&  aVulkanRenderer, std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers );
        VulkanMaterial ( const VulkanRenderer&  aVulkanRenderer, uint32_t aPath = 0 );
        /// The Copy Contsructor is used for virtual copying.
        VulkanMaterial ( const VulkanMaterial& aMaterial );
        /// Assignment operator due to rule of zero/three/five.
        VulkanMaterial& operator= ( const VulkanMaterial& aMaterial );
        /// No move assignment allowed
        VulkanMaterial& operator = ( VulkanMaterial&& ) = delete;
        /// No move allowed
        VulkanMaterial ( VulkanMaterial&& ) = delete;

        ~VulkanMaterial() final;
        /// @copydoc Material::Clone()
        std::unique_ptr<Material> Clone() const final;
        ///@name Loaders
        ///@{
        void Load ( const MaterialBuffer& aMaterialBuffer ) final;
        void Load ( std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers );
        void Unload() final;
        ///@}
        ///@name Property and Sampler Setters
        ///@{
        void Set ( const UniformKeyValue& aValue ) final;
        void SetSampler ( const std::string& aName, const ResourceId& aValue ) final;
        ///@}
        ///@name Property and Sampler Getters
        ///@{
        uint32_t GetUint ( const std::string& aName ) final;
        int32_t GetSint ( const std::string& aName ) final;
        float GetFloat ( const std::string& aName ) final;
        Vector2 GetFloatVec2 ( const std::string& aName ) final;
        Vector3 GetFloatVec3 ( const std::string& aName ) final;
        Vector4 GetFloatVec4 ( const std::string& aName ) final;
        ResourceId GetSampler ( const std::string& aName ) final;
        const std::vector<std::tuple<std::string, ResourceId>>& GetSamplers() const final;
        ///@}
        const std::vector<VkDescriptorSet>& GetDescriptorSets() const;
        const Material& GetMaterial() const;
    private:
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        void InitializeDescriptorSets();
        void FinalizeDescriptorSets();
        const VulkanRenderer& mVulkanRenderer;
        VkDescriptorPool mVkDescriptorPool{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> mVkDescriptorSets{};
        VulkanBuffer mUniformBuffer;
    };
}
#endif
