/*
Copyright (C) 2017-2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANTEXTURE_H
#define AEONGAMES_VULKANTEXTURE_H
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <memory>
#include "aeongames/Texture.h"

namespace AeonGames
{
    class Texture;
    class VulkanRenderer;
    class VulkanTexture
    {
    public:
        VulkanTexture ( const VulkanRenderer&  aVulkanRenderer, const Texture& aTexture );
        ~VulkanTexture();
        void Resize ( uint32_t aWidth, uint32_t aHeight, const uint8_t* aPixels, Format aFormat = Format::Unknown, Type aType = Type::Unknown ) final;
        void WritePixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Format aFormat, Type aType, const uint8_t* aPixels ) final;
        const VkDescriptorImageInfo& GetDescriptorImageInfo() const;
    private:
        void InitializeTexture ( Format aFormat, Type aType );
        void FinalizeTexture();
        void InitializeTextureView();
        void FinalizeTextureView();
        const VulkanRenderer& mVulkanRenderer;
        VkImage mVkImage{};
        VkDeviceMemory mImageMemory{};
        VkDescriptorImageInfo mVkDescriptorImageInfo{};
    };
}
#endif
