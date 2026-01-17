/*
Copyright (C) 2017-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANTEXTURE_HPP
#define AEONGAMES_VULKANTEXTURE_HPP
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <memory>
#include "aeongames/Texture.hpp"

namespace AeonGames
{
    class Texture;
    class VulkanRenderer;
    class VulkanTexture
    {
    public:
        VulkanTexture ( const VulkanRenderer&  aVulkanRenderer, const Texture& aTexture );
        ~VulkanTexture();
        VulkanTexture ( VulkanTexture&& aVulkanTexture );
        VulkanTexture ( const VulkanTexture& ) = delete;
        VulkanTexture& operator= ( const VulkanTexture& ) = delete;
        VulkanTexture& operator= ( VulkanTexture&& ) = delete;
        const VkDescriptorImageInfo& GetDescriptorImageInfo() const;
    private:
        const VulkanRenderer& mVulkanRenderer;
        const Texture* mTexture{nullptr};
        VkImage mVkImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkDeviceMemory{VK_NULL_HANDLE};
        VkDescriptorImageInfo mVkDescriptorImageInfo{};
    };
}
#endif
