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
#ifndef AEONGAMES_VULKANTEXTURE_H
#define AEONGAMES_VULKANTEXTURE_H
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <memory>
#include "aeongames/Image.h"

namespace AeonGames
{
    class Image;
    class VulkanRenderer;
    class VulkanImage : public Image
    {
    public:
        VulkanImage ( const VulkanRenderer&  aVulkanRenderer, uint32_t aPath = 0 );
        ~VulkanImage() final;
        void Load ( const std::string& aPath ) final;
        void Load ( uint32_t aId ) final;
        void Initialize ( uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels = nullptr ) final;
        void BitBlit ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels ) final;
        void Finalize() final;
        uint32_t Width() const final;
        uint32_t Height() const final;
        ImageFormat Format() const final;
        ImageType Type() const final;

        const VkDescriptorImageInfo& GetDescriptorImageInfo() const;
    private:
        void InitializeImage ( uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType );
        void FinalizeImage();
        void InitializeImageView();
        void FinalizeImageView();
        const VulkanRenderer& mVulkanRenderer;
        uint32_t mWidth{};
        uint32_t mHeight{};
        VkImage mVkImage{};
        VkDeviceMemory mImageMemory{};
        VkDescriptorImageInfo mVkDescriptorImageInfo{};
    };
}
#endif
