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

#include <sstream>
#include <limits>
#include <cstring>
#include <utility>
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"
#include "aeongames/Image.h"
#include "aeongames/Utilities.h"

namespace AeonGames
{
    VulkanTexture::VulkanTexture ( const Image& aImage, const VulkanRenderer&  aVulkanRenderer ) :
        mVulkanRenderer (  aVulkanRenderer  ), mImage (  aImage ),
        mVkImage ( VK_NULL_HANDLE ),
        mImageMemory ( VK_NULL_HANDLE )
    {
        mVkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        mVkDescriptorImageInfo.imageView = VK_NULL_HANDLE;
        mVkDescriptorImageInfo.sampler = VK_NULL_HANDLE;
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
        InitializeImage();
        InitializeImageView();
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
        sampler_create_info.maxAnisotropy = 1.0f;
        sampler_create_info.compareEnable = VK_FALSE;
        sampler_create_info.compareOp = VK_COMPARE_OP_NEVER;
        sampler_create_info.minLod = 0.0f;
        sampler_create_info.maxLod = 1.0f;
        sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        sampler_create_info.unnormalizedCoordinates = VK_FALSE;
        if ( VkResult result = vkCreateSampler ( mVulkanRenderer.GetDevice(), &sampler_create_info, nullptr, &mVkDescriptorImageInfo.sampler ) )
        {
            std::ostringstream stream;
            stream << "Sampler creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanTexture::Finalize()
    {
        if ( mVkDescriptorImageInfo.sampler != VK_NULL_HANDLE )
        {
            vkDestroySampler ( mVulkanRenderer.GetDevice(), mVkDescriptorImageInfo.sampler, nullptr );
            mVkDescriptorImageInfo.sampler = VK_NULL_HANDLE;
        }
        FinalizeImageView();
        FinalizeImage();
    }

    void VulkanTexture::InitializeImage()
    {
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.pNext = nullptr;
        image_create_info.flags = 0;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        VkFormatProperties format_properties{};
        vkGetPhysicalDeviceFormatProperties ( mVulkanRenderer.GetPhysicalDevice(), image_create_info.format, &format_properties );
        image_create_info.extent.width = mImage.Width();
        image_create_info.extent.height = mImage.Height();
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
        if ( VkResult result = vkCreateImage ( mVulkanRenderer.GetDevice(), &image_create_info, nullptr, &mVkImage ) )
        {
            std::ostringstream stream;
            stream << "Image creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements memory_requirements{};
        vkGetImageMemoryRequirements ( mVulkanRenderer.GetDevice(), mVkImage, &memory_requirements );
        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.pNext = nullptr;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = mVulkanRenderer.FindMemoryTypeIndex ( memory_requirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        if ( memory_allocate_info.memoryTypeIndex == std::numeric_limits<uint32_t>::max() )
        {
            throw std::runtime_error ( "Unable to find a suitable memory type index" );
        }
        if ( VkResult result = vkAllocateMemory ( mVulkanRenderer.GetDevice(), &memory_allocate_info, nullptr, &mImageMemory ) )
        {
            std::ostringstream stream;
            stream << "Image Memory Allocation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        if ( VkResult result = vkBindImageMemory ( mVulkanRenderer.GetDevice(), mVkImage, mImageMemory, 0 ) )
        {
            std::ostringstream stream;
            stream << "Bind Image Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        Update();
    }

    void VulkanTexture::Update()
    {
        // -----------------------------
        // Write Memory
        VkBuffer image_buffer{};
        VkDeviceMemory image_buffer_memory{};
        VkMemoryRequirements memory_requirements{};
        VkMemoryAllocateInfo memory_allocate_info{};

        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = mImage.Width() * mImage.Height() * 4;
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if ( VkResult result = vkCreateBuffer ( mVulkanRenderer.GetDevice(), &buffer_create_info, nullptr, &image_buffer ) )
        {
            std::ostringstream stream;
            stream << "Create Buffer failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        vkGetBufferMemoryRequirements ( mVulkanRenderer.GetDevice(), image_buffer, &memory_requirements );

        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = mVulkanRenderer.FindMemoryTypeIndex ( memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

        if ( VkResult result = vkAllocateMemory ( mVulkanRenderer.GetDevice(), &memory_allocate_info, nullptr, &image_buffer_memory ) )
        {
            std::ostringstream stream;
            stream << "Allocate Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        if ( VkResult result = vkBindBufferMemory ( mVulkanRenderer.GetDevice(), image_buffer, image_buffer_memory, 0 ) )
        {
            std::ostringstream stream;
            stream << "Bind Buffer Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        void* image_memory = nullptr;
        if ( VkResult result = vkMapMemory ( mVulkanRenderer.GetDevice(), image_buffer_memory, 0, VK_WHOLE_SIZE, 0, &image_memory ) )
        {
            std::ostringstream stream;
            stream << "Map Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        if ( mImage.Format() == Image::ImageFormat::RGBA )
        {
            if ( mImage.Type() == Image::ImageType::UNSIGNED_BYTE )
            {
                memcpy ( image_memory, mImage.Pixels(), mImage.PixelsSize() );
            }
            else
            {
                // Is this a temporary fix?
                /** @note This code and the one bellow for RGB is too redundant,
                    I do not want to have to add more and more cases for each format,
                    specially when Vulkan is so picky about what format it supports
                    and which one it doesn't.
                    So, perhaps support only RGBA and let the Image class
                    handle any conversions?
                    We'll have to see when it comes to handling compressed and
                    "hardware accelerated" formats.*/
                const auto* read_pointer = reinterpret_cast<const uint16_t*> ( mImage.Pixels() );
                auto* write_pointer = static_cast<uint8_t*> ( image_memory );
                auto data_size = mImage.PixelsSize() / 2;
                for ( uint32_t i = 0; i < data_size; i += 4 )
                {
                    write_pointer[i] = read_pointer[i] / 256;
                    write_pointer[i + 1] = read_pointer[i + 1] / 256;
                    write_pointer[i + 2] = read_pointer[i + 2] / 256;
                    write_pointer[i + 3] = read_pointer[i + 3] / 256;
                }
            }
        }
        else
        {
            if ( mImage.Type() == Image::ImageType::UNSIGNED_BYTE )
            {
                const uint8_t* read_pointer = mImage.Pixels();
                auto* write_pointer = static_cast<uint8_t*> ( image_memory );
                auto data_size = mImage.PixelsSize();
                for ( uint32_t i = 0; i < data_size; i += 3 )
                {
                    write_pointer[0] = read_pointer[i];
                    write_pointer[1] = read_pointer[i + 1];
                    write_pointer[2] = read_pointer[i + 2];
                    write_pointer[3] = 255;
                    write_pointer += 4;
                }
            }
            else
            {
                // Is this a temporary fix?
                const auto* read_pointer = reinterpret_cast<const uint16_t*> ( mImage.Pixels() );
                auto* write_pointer = static_cast<uint8_t*> ( image_memory );
                auto data_size = mImage.PixelsSize() / 2;
                for ( uint32_t i = 0; i < data_size; i += 3 )
                {
                    write_pointer[0] = read_pointer[i] / 256;
                    write_pointer[1] = read_pointer[i + 1] / 256;
                    write_pointer[2] = read_pointer[i + 2] / 256;
                    write_pointer[3] = 255;
                    write_pointer += 4;
                }
            }
        }
        vkUnmapMemory ( mVulkanRenderer.GetDevice(), image_buffer_memory );
        //--------------------------------------------------------
        // Transition Image Layout
        VkCommandBuffer command_buffer = mVulkanRenderer.BeginSingleTimeCommands();
        VkImageMemoryBarrier image_memory_barrier{};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.image = mVkImage;
        image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_memory_barrier.subresourceRange.baseMipLevel = 0;
        image_memory_barrier.subresourceRange.levelCount = 1;
        image_memory_barrier.subresourceRange.baseArrayLayer = 0;
        image_memory_barrier.subresourceRange.layerCount = 1;
        image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier (
            command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &image_memory_barrier
        );
        VkBufferImageCopy buffer_image_copy{};
        buffer_image_copy.bufferOffset = 0;
        buffer_image_copy.bufferRowLength = 0;
        buffer_image_copy.bufferImageHeight = 0;
        buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        buffer_image_copy.imageSubresource.mipLevel = 0;
        buffer_image_copy.imageSubresource.baseArrayLayer = 0;
        buffer_image_copy.imageSubresource.layerCount = 1;
        buffer_image_copy.imageOffset = { 0, 0, 0 };
        buffer_image_copy.imageExtent = { mImage.Width(), mImage.Height(), 1 };
        vkCmdCopyBufferToImage ( command_buffer, image_buffer, mVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy );

        image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier (
            command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &image_memory_barrier
        );
        mVulkanRenderer.EndSingleTimeCommands ( command_buffer );
        vkDestroyBuffer ( mVulkanRenderer.GetDevice(), image_buffer, nullptr );
        vkFreeMemory ( mVulkanRenderer.GetDevice(), image_buffer_memory, nullptr );
    }

    void VulkanTexture::FinalizeImage()
    {
        if ( mVkImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( mVulkanRenderer.GetDevice(), mVkImage, nullptr );
            mVkImage = VK_NULL_HANDLE;
        }
        if ( mImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVulkanRenderer.GetDevice(), mImageMemory, nullptr );
            mImageMemory = VK_NULL_HANDLE;
        }
    }

    void VulkanTexture::InitializeImageView()
    {
        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = mVkImage;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        if ( VkResult result = vkCreateImageView ( mVulkanRenderer.GetDevice(), &image_view_create_info, nullptr, &mVkDescriptorImageInfo.imageView ) )
        {
            std::ostringstream stream;
            stream << "Create Image View failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanTexture::FinalizeImageView()
    {
        if ( mVkDescriptorImageInfo.imageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( mVulkanRenderer.GetDevice(), mVkDescriptorImageInfo.imageView, nullptr );
            mVkDescriptorImageInfo.imageView = VK_NULL_HANDLE;
        }
    }

    const VkDescriptorImageInfo& VulkanTexture::GetDescriptorImageInfo() const
    {
        return mVkDescriptorImageInfo;
    }
}
