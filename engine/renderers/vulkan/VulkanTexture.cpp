/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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

#include <sstream>
#include <limits>
#include <cstring>
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"
#include "aeongames/Image.h"
#include "aeongames/Utilities.h"

namespace AeonGames
{
    VulkanTexture::VulkanTexture ( const std::shared_ptr<Image> aImage, const VulkanRenderer* aVulkanRenderer ) :
        mVulkanRenderer ( aVulkanRenderer ), mImage ( aImage ),
        mVkImage ( VK_NULL_HANDLE ),
        mImageMemory ( VK_NULL_HANDLE ),
        mVkImageView ( VK_NULL_HANDLE ),
        mVkSampler ( VK_NULL_HANDLE )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    VulkanTexture::~VulkanTexture()
    {
        Finalize();
    }

    void VulkanTexture::Initialize()
    {
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.pNext = nullptr;
        image_create_info.flags = 0;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;

        image_create_info.format = ( mImage->Format() == Image::ImageFormat::RGB ) ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
        image_create_info.extent.width = mImage->Width();
        image_create_info.extent.height = mImage->Height();
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.queueFamilyIndexCount = 0;
        image_create_info.pQueueFamilyIndices = nullptr;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        if ( VkResult result = vkCreateImage ( mVulkanRenderer->GetDevice(), &image_create_info, nullptr, &mVkImage ) )
        {
            std::ostringstream stream;
            stream << "Image creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements memory_requirements{};
        vkGetImageMemoryRequirements ( mVulkanRenderer->GetDevice(), mVkImage, &memory_requirements );
        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.pNext = nullptr;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = mVulkanRenderer->GetMemoryTypeIndex ( memory_requirements.memoryTypeBits | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        if ( memory_allocate_info.memoryTypeIndex == std::numeric_limits<uint32_t>::max() )
        {
            throw std::runtime_error ( "Unable to find a suitable memory type index" );
        }
        if ( VkResult result = vkAllocateMemory ( mVulkanRenderer->GetDevice(), &memory_allocate_info, nullptr, &mImageMemory ) )
        {
            std::ostringstream stream;
            stream << "Image Memory Allocation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        if ( VkResult result = vkBindImageMemory ( mVulkanRenderer->GetDevice(), mVkImage, mImageMemory, 0 ) )
        {
            std::ostringstream stream;
            stream << "Bind Image Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        void* image_memory = nullptr;
        if ( VkResult result = vkMapMemory ( mVulkanRenderer->GetDevice(), mImageMemory, 0, VK_WHOLE_SIZE, 0, &image_memory ) )
        {
            std::ostringstream stream;
            stream << "Map Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        memcpy ( image_memory, mImage->Data(), mImage->DataSize() );
        vkUnmapMemory ( mVulkanRenderer->GetDevice(), mImageMemory );

        VkSamplerCreateInfo sampler_create_info{};
        sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.pNext = nullptr;
        sampler_create_info.flags = 0;
        sampler_create_info.magFilter = VK_FILTER_NEAREST;
        sampler_create_info.minFilter = VK_FILTER_NEAREST;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.mipLodBias = 0.0f;
        sampler_create_info.anisotropyEnable = VK_FALSE;
        sampler_create_info.maxAnisotropy = 0.0f;
        sampler_create_info.compareEnable = VK_FALSE;
        sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
        sampler_create_info.minLod = 0.0f;
        sampler_create_info.maxLod = 1.0f;
        sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        sampler_create_info.unnormalizedCoordinates = VK_FALSE;
        if ( VkResult result = vkCreateSampler ( mVulkanRenderer->GetDevice(), &sampler_create_info, nullptr, &mVkSampler ) )
        {
            std::ostringstream stream;
            stream << "Sampler creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanTexture::Finalize()
    {
        if ( mVkSampler != VK_NULL_HANDLE )
        {
            vkDestroySampler ( mVulkanRenderer->GetDevice(), mVkSampler, nullptr );
            mVkSampler = VK_NULL_HANDLE;
        }
        if ( mVkImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( mVulkanRenderer->GetDevice(), mVkImage, nullptr );
            mVkImage = VK_NULL_HANDLE;
        }
        if ( mImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVulkanRenderer->GetDevice(), mImageMemory, nullptr );
            mImageMemory = VK_NULL_HANDLE;
        }
    }

    const VkSampler& VulkanTexture::GetSampler() const
    {
        return mVkSampler;
    }
}
