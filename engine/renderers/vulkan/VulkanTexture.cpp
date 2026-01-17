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

#include <sstream>
#include <limits>
#include <cstring>
#include <utility>
#include "VulkanTexture.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanUtilities.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/LogLevel.hpp"

namespace AeonGames
{
    VulkanTexture::VulkanTexture ( const VulkanRenderer&  aVulkanRenderer, const Texture& aTexture ) :
        mVulkanRenderer (  aVulkanRenderer  ), mTexture{&aTexture}
    {
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.pNext = nullptr;
        image_create_info.flags = 0;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM; // This field contains both format and type, calculate rather than hardcode
        VkFormatProperties format_properties{};
        vkGetPhysicalDeviceFormatProperties ( mVulkanRenderer.GetPhysicalDevice(), image_create_info.format, &format_properties );
        image_create_info.extent.width = aTexture.GetWidth();
        image_create_info.extent.height = aTexture.GetHeight();
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
            std::cout << LogLevel::Error << stream.str() << std::endl;
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
            std::cout << LogLevel::Error << "Unable to find a suitable memory type index" << std::endl;
            throw std::runtime_error ( "Unable to find a suitable memory type index" );
        }
        if ( VkResult result = vkAllocateMemory ( mVulkanRenderer.GetDevice(), &memory_allocate_info, nullptr, &mVkDeviceMemory ) )
        {
            std::ostringstream stream;
            stream << "Image Memory Allocation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        if ( VkResult result = vkBindImageMemory ( mVulkanRenderer.GetDevice(), mVkImage, mVkDeviceMemory, 0 ) )
        {
            std::ostringstream stream;
            stream << "Bind Image Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        /// ----------------------------------------------------------------------------

        mVkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        /*-----------------------------------------------------------------*/
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
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        // -----------------------------
        // Write Image
        VkBuffer image_buffer{};
        VkDeviceMemory image_buffer_memory{};

        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = static_cast<size_t> ( aTexture.GetWidth() ) * static_cast<size_t> ( aTexture.GetHeight() ) * 4; /// @todo Use format and type as well as GetPixelSize()
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if ( VkResult result = vkCreateBuffer ( mVulkanRenderer.GetDevice(), &buffer_create_info, nullptr, &image_buffer ) )
        {
            std::ostringstream stream;
            stream << "Create Buffer failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        memset ( &memory_requirements, 0, sizeof ( VkMemoryRequirements ) );
        vkGetBufferMemoryRequirements ( mVulkanRenderer.GetDevice(), image_buffer, &memory_requirements );
        memset ( &memory_allocate_info, 0, sizeof ( VkMemoryAllocateInfo ) );
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex =  mVulkanRenderer.FindMemoryTypeIndex ( memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

        if ( VkResult result = vkAllocateMemory ( mVulkanRenderer.GetDevice(), &memory_allocate_info, nullptr, &image_buffer_memory ) )
        {
            std::ostringstream stream;
            stream << "Allocate Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        if ( VkResult result = vkBindBufferMemory ( mVulkanRenderer.GetDevice(), image_buffer, image_buffer_memory, 0 ) )
        {
            std::ostringstream stream;
            stream << "Bind Buffer Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        void* image_memory = nullptr;
        if ( VkResult result = vkMapMemory ( mVulkanRenderer.GetDevice(), image_buffer_memory, 0, VK_WHOLE_SIZE, 0, &image_memory ) )
        {
            std::ostringstream stream;
            stream << "Map Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        if ( aTexture.GetFormat() == Texture::Format::RGBA )
        {
            if ( aTexture.GetType() == Texture::Type::UNSIGNED_BYTE )
            {
                memcpy ( image_memory, aTexture.GetPixels().data(), aTexture.GetWidth() * aTexture.GetHeight() * GetPixelSize ( aTexture.GetFormat(), aTexture.GetType() ) );
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
                const auto* read_pointer = reinterpret_cast<const uint16_t*> ( aTexture.GetPixels().data() );
                auto* write_pointer = static_cast<uint8_t*> ( image_memory );
                auto data_size = aTexture.GetWidth() * aTexture.GetHeight() * GetPixelSize ( aTexture.GetFormat(), aTexture.GetType() ) / 2;
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
            if ( aTexture.GetType() == Texture::Type::UNSIGNED_BYTE )
            {
                const uint8_t* read_pointer = aTexture.GetPixels().data();
                auto* write_pointer = static_cast<uint8_t*> ( image_memory );
                auto data_size = aTexture.GetWidth() * aTexture.GetHeight() * GetPixelSize ( aTexture.GetFormat(), aTexture.GetType() );
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
                const auto* read_pointer = reinterpret_cast<const uint16_t*> ( aTexture.GetPixels().data() );
                auto* write_pointer = static_cast<uint8_t*> ( image_memory );
                auto data_size = aTexture.GetWidth() * aTexture.GetHeight() * GetPixelSize ( aTexture.GetFormat(), aTexture.GetType() ) / 2;
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
        image_memory_barrier.srcAccessMask = 0; //VK_ACCESS_HOST_WRITE_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier (
            command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
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
        buffer_image_copy.imageExtent = { aTexture.GetWidth(), aTexture.GetHeight(), 1 };
        vkCmdCopyBufferToImage ( command_buffer, image_buffer, mVkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy );

        image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier (
            command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &image_memory_barrier
        );
        mVulkanRenderer.EndSingleTimeCommands ( command_buffer );
        vkDestroyBuffer ( mVulkanRenderer.GetDevice(), image_buffer, nullptr );
        vkFreeMemory ( mVulkanRenderer.GetDevice(), image_buffer_memory, nullptr );
    }

    VulkanTexture::VulkanTexture ( VulkanTexture&& aVulkanTexture ) :
        mVulkanRenderer (  aVulkanTexture.mVulkanRenderer  ), mTexture{aVulkanTexture.mTexture}
    {
        std::swap ( mVkImage, aVulkanTexture.mVkImage );
        std::swap ( mVkDeviceMemory, aVulkanTexture.mVkDeviceMemory );
        std::swap ( mVkDescriptorImageInfo, aVulkanTexture.mVkDescriptorImageInfo );
    }

    VulkanTexture::~VulkanTexture()
    {
        if ( mVkDescriptorImageInfo.sampler != VK_NULL_HANDLE )
        {
            vkDestroySampler ( mVulkanRenderer.GetDevice(), mVkDescriptorImageInfo.sampler, nullptr );
        }
        if ( mVkDescriptorImageInfo.imageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( mVulkanRenderer.GetDevice(), mVkDescriptorImageInfo.imageView, nullptr );
        }
        if ( mVkImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( mVulkanRenderer.GetDevice(), mVkImage, nullptr );
        }
        if ( mVkDeviceMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVulkanRenderer.GetDevice(), mVkDeviceMemory, nullptr );
        }
    }

    const VkDescriptorImageInfo& VulkanTexture::GetDescriptorImageInfo() const
    {
        return mVkDescriptorImageInfo;
    }
}
