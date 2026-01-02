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
#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"
#include "VulkanMaterial.h"
#include "VulkanUtilities.h"
#include "VulkanMesh.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <utility>
#include <cstring>
#include <cassert>
#include "aeongames/Frustum.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/MemoryPool.hpp"

namespace AeonGames
{
    VulkanWindow::VulkanWindow ( VulkanRenderer&  aVulkanRenderer, void* aWindowId ) :
        mVulkanRenderer { aVulkanRenderer }, mWindowId{aWindowId},
        mMemoryPoolBuffer{mVulkanRenderer, 64_kb},
        mMatrices { aVulkanRenderer }
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

    VulkanWindow::VulkanWindow ( VulkanWindow&& aVulkanWindow ) :
        mVulkanRenderer { aVulkanWindow.mVulkanRenderer },
        mMemoryPoolBuffer{std::move ( aVulkanWindow.mMemoryPoolBuffer ) },
        mMatrices{std::move ( aVulkanWindow.mMatrices ) }
    {
        std::swap ( mWindowId, aVulkanWindow.mWindowId );
        std::swap ( mFrustum, aVulkanWindow.mFrustum );
        std::swap ( mProjectionMatrix, aVulkanWindow.mProjectionMatrix );
        std::swap ( mViewMatrix, aVulkanWindow.mViewMatrix );
        std::swap ( mVkSurfaceKHR, aVulkanWindow.mVkSurfaceKHR );
        std::swap ( mVkSurfaceCapabilitiesKHR, aVulkanWindow.mVkSurfaceCapabilitiesKHR );
        std::swap ( mSwapchainImageCount, aVulkanWindow.mSwapchainImageCount );
        std::swap ( mVkSwapchainKHR, aVulkanWindow.mVkSwapchainKHR );
        std::swap ( mVkDepthStencilImage, aVulkanWindow.mVkDepthStencilImage );
        std::swap ( mVkDepthStencilImageMemory, aVulkanWindow.mVkDepthStencilImageMemory );
        std::swap ( mVkDepthStencilImageView, aVulkanWindow.mVkDepthStencilImageView );
        std::swap ( mHasStencil, aVulkanWindow.mHasStencil );
        std::swap ( mActiveImageIndex, aVulkanWindow.mActiveImageIndex );
        std::swap ( mVkViewport, aVulkanWindow.mVkViewport );
        std::swap ( mVkScissor, aVulkanWindow.mVkScissor );
        std::swap ( mVkDepthStencilFormat, aVulkanWindow.mVkDepthStencilFormat );
        std::swap ( mVkSurfaceFormatKHR, aVulkanWindow.mVkSurfaceFormatKHR );
        std::swap ( mVkRenderPass, aVulkanWindow.mVkRenderPass );
        std::swap ( mMatricesDescriptorPool, aVulkanWindow.mMatricesDescriptorPool );
        std::swap ( mMatricesDescriptorSet, aVulkanWindow.mMatricesDescriptorSet );
        std::swap ( mVkCommandPool, aVulkanWindow.mVkCommandPool );
        std::swap ( mVkCommandBuffer, aVulkanWindow.mVkCommandBuffer );
        mVkSwapchainImages.swap ( aVulkanWindow.mVkSwapchainImages );
        mVkSwapchainImageViews.swap ( aVulkanWindow.mVkSwapchainImageViews );
        mVkFramebuffers.swap ( aVulkanWindow.mVkFramebuffers );
    }

    VulkanWindow::~VulkanWindow()
    {
        Finalize();
    }

    void VulkanWindow::InitializeSurface()
    {
#if defined ( VK_USE_PLATFORM_WIN32_KHR )
        VkWin32SurfaceCreateInfoKHR win32_surface_create_info_khr {};
        win32_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create_info_khr.hwnd = reinterpret_cast<HWND> ( mWindowId );
        win32_surface_create_info_khr.hinstance = reinterpret_cast<HINSTANCE> ( GetWindowLongPtr ( win32_surface_create_info_khr.hwnd, GWLP_HINSTANCE ) );
        if ( VkResult result = vkCreateWin32SurfaceKHR ( mVulkanRenderer.GetInstance(), &win32_surface_create_info_khr, nullptr, &mVkSurfaceKHR ) )
        {
            std::ostringstream stream;
            stream << "Call to vkCreateWin32SurfaceKHR failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
#elif defined( VK_USE_PLATFORM_METAL_EXT )
        VkMetalSurfaceCreateInfoEXT metal_surface_create_info_ext {};
        metal_surface_create_info_ext.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        metal_surface_create_info_ext.pLayer = reinterpret_cast<CAMetalLayer*> ( mWindowId );
        if ( VkResult result = vkCreateMetalSurfaceEXT ( mVulkanRenderer.GetInstance(), &metal_surface_create_info_ext, nullptr, &mVkSurfaceKHR ) )
        {
            std::ostringstream stream;
            stream << "Call to vkCreateMetalSurfaceEXT failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
#elif defined( VK_USE_PLATFORM_XLIB_KHR )
        VkXlibSurfaceCreateInfoKHR xlib_surface_create_info_khr {};
        xlib_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        xlib_surface_create_info_khr.dpy = mVulkanRenderer.GetDisplay();
        xlib_surface_create_info_khr.window = reinterpret_cast<::Window> ( mWindowId );
        if ( VkResult result = vkCreateXlibSurfaceKHR ( mVulkanRenderer.GetInstance(), &xlib_surface_create_info_khr, nullptr, &mVkSurfaceKHR ) )
        {
            std::ostringstream stream;
            stream << "Call to vkCreateXlibSurfaceKHR failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
#endif
        VkBool32 wsi_supported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR ( mVulkanRenderer.GetPhysicalDevice(), mVulkanRenderer.GetQueueFamilyIndex(), mVkSurfaceKHR, &wsi_supported );
        if ( !wsi_supported )
        {
            std::ostringstream stream;
            stream << "WSI not supported.";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanWindow::InitializeSwapchain()
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( mVulkanRenderer.GetPhysicalDevice(), mVkSurfaceKHR, &mVkSurfaceCapabilitiesKHR );

        if ( mVkSurfaceCapabilitiesKHR.currentExtent.width == 0 ||
             mVkSurfaceCapabilitiesKHR.currentExtent.height == 0 )
        {
            std::cout << LogLevel::Debug << "Cannot create swapchain with zero area. (" << mVkSurfaceCapabilitiesKHR.currentExtent.width << "x" << mVkSurfaceCapabilitiesKHR.currentExtent.height << ")" << std::endl;
            return;
        }

        if ( mSwapchainImageCount < mVkSurfaceCapabilitiesKHR.minImageCount )
        {
            mSwapchainImageCount = mVkSurfaceCapabilitiesKHR.minImageCount;
        }
        if ( ( mVkSurfaceCapabilitiesKHR.maxImageCount > 0 ) &&
             ( mSwapchainImageCount > mVkSurfaceCapabilitiesKHR.maxImageCount ) )
        {
            mSwapchainImageCount = mVkSurfaceCapabilitiesKHR.maxImageCount;
        }
        VkSwapchainCreateInfoKHR swapchain_create_info{};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = mVkSurfaceKHR;
        swapchain_create_info.minImageCount = mSwapchainImageCount;
        swapchain_create_info.imageFormat = mVkSurfaceFormatKHR.format;
        swapchain_create_info.imageColorSpace = mVkSurfaceFormatKHR.colorSpace;
        swapchain_create_info.imageExtent.width = mVkSurfaceCapabilitiesKHR.currentExtent.width;
        swapchain_create_info.imageExtent.height = mVkSurfaceCapabilitiesKHR.currentExtent.height;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
        swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR; // This may be reset below.
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = mVkSwapchainKHR; // Used for Resizing.
        {
            uint32_t present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR ( mVulkanRenderer.GetPhysicalDevice(), mVkSurfaceKHR, &present_mode_count, nullptr );
            std::vector<VkPresentModeKHR> present_mode_list ( present_mode_count );
            vkGetPhysicalDeviceSurfacePresentModesKHR ( mVulkanRenderer.GetPhysicalDevice(), mVkSurfaceKHR, &present_mode_count, present_mode_list.data() );
            for ( auto& i : present_mode_list )
            {
                if ( i == VK_PRESENT_MODE_MAILBOX_KHR )
                {
                    swapchain_create_info.presentMode = i;
                    break;
                }
            }
        }
        if ( VkResult result = vkCreateSwapchainKHR ( mVulkanRenderer.GetDevice(), &swapchain_create_info, nullptr, &mVkSwapchainKHR ) )
        {
            std::ostringstream stream;
            stream << "Call to vkCreateSwapchainKHR failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        if ( swapchain_create_info.oldSwapchain != VK_NULL_HANDLE )
        {
            vkDestroySwapchainKHR ( mVulkanRenderer.GetDevice(), swapchain_create_info.oldSwapchain, nullptr );
        }
    }

    void VulkanWindow::InitializeImageViews()
    {
        if ( VkResult result = vkGetSwapchainImagesKHR ( mVulkanRenderer.GetDevice(), mVkSwapchainKHR, &mSwapchainImageCount, nullptr ) )
        {
            std::ostringstream stream;
            stream << "Get swapchain image count failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        mVkSwapchainImages.resize ( mSwapchainImageCount );
        mVkSwapchainImageViews.resize ( mSwapchainImageCount );
        if ( VkResult result = vkGetSwapchainImagesKHR ( mVulkanRenderer.GetDevice(),
                               mVkSwapchainKHR,
                               &mSwapchainImageCount,
                               mVkSwapchainImages.data() ) )
        {
            std::ostringstream stream;
            stream << "Get swapchain images failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        for ( uint32_t i = 0; i < mSwapchainImageCount; ++i )
        {
            VkImageViewCreateInfo image_view_create_info{};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.pNext = nullptr;
            image_view_create_info.flags = 0;
            image_view_create_info.image = mVkSwapchainImages[i];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = mVkSurfaceFormatKHR.format;
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            vkCreateImageView ( mVulkanRenderer.GetDevice(), &image_view_create_info, nullptr, &mVkSwapchainImageViews[i] );
        }
    }

    void VulkanWindow::InitializeDepthStencil()
    {
        if ( mVkSurfaceCapabilitiesKHR.currentExtent.width == 0 ||
             mVkSurfaceCapabilitiesKHR.currentExtent.height == 0 )
        {
            std::cout << LogLevel::Debug << "Cannot create depth stencil with zero area. (" << mVkSurfaceCapabilitiesKHR.currentExtent.width << "x" << mVkSurfaceCapabilitiesKHR.currentExtent.height << ")" << std::endl;
            return;
        }

        mHasStencil =
            ( mVkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT );
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.flags = 0;
        image_create_info.format = mVkDepthStencilFormat;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = mVkSurfaceCapabilitiesKHR.currentExtent.width;
        image_create_info.extent.height = mVkSurfaceCapabilitiesKHR.currentExtent.height;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.queueFamilyIndexCount = 0;
        image_create_info.pQueueFamilyIndices = nullptr;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if ( VkResult result = vkCreateImage ( mVulkanRenderer.GetDevice(), &image_create_info, nullptr, &mVkDepthStencilImage ) )
        {
            std::ostringstream stream;
            stream << "Call to vkCreateImage failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements ( mVulkanRenderer.GetDevice(), mVkDepthStencilImage, &memory_requirements );

        auto required_bits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t memory_index = UINT32_MAX;
        for ( uint32_t i = 0; i < mVulkanRenderer.GetPhysicalDeviceMemoryProperties().memoryTypeCount; ++i )
        {
            if ( memory_requirements.memoryTypeBits & ( 1 << i ) )
            {
                if ( ( mVulkanRenderer.GetPhysicalDeviceMemoryProperties().memoryTypes[i].propertyFlags & required_bits ) == required_bits )
                {
                    memory_index = i;
                    break;
                }
            }
        }

        if ( memory_index == UINT32_MAX )
        {
            std::ostringstream stream;
            stream << "Could not find a suitable memory index.";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = memory_index;
        vkAllocateMemory ( mVulkanRenderer.GetDevice(), &memory_allocate_info, nullptr, &mVkDepthStencilImageMemory );
        vkBindImageMemory ( mVulkanRenderer.GetDevice(), mVkDepthStencilImage, mVkDepthStencilImageMemory, 0 );

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = mVkDepthStencilImage;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = mVkDepthStencilFormat;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | ( mHasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        vkCreateImageView ( mVulkanRenderer.GetDevice(), &image_view_create_info, nullptr, &mVkDepthStencilImageView );
    }

    void VulkanWindow::InitializeFrameBuffers()
    {
        if ( mVkSurfaceCapabilitiesKHR.currentExtent.width == 0 ||
             mVkSurfaceCapabilitiesKHR.currentExtent.height == 0 )
        {
            std::cout << LogLevel::Debug << "Cannot create framebuffers with zero area. (" << mVkSurfaceCapabilitiesKHR.currentExtent.width << "x" << mVkSurfaceCapabilitiesKHR.currentExtent.height << ")" << std::endl;
            return;
        }
        mVkFramebuffers.resize ( mSwapchainImageCount );
        for ( uint32_t i = 0; i < mSwapchainImageCount; ++i )
        {
            std::array<VkImageView, 2> attachments
            {
                {
                    mVkSwapchainImageViews[i],
                    mVkDepthStencilImageView
                }
            };
            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = mVkRenderPass;
            framebuffer_create_info.attachmentCount = static_cast<uint32_t> ( attachments.size() );
            framebuffer_create_info.pAttachments = attachments.data();
            framebuffer_create_info.width = mVkSurfaceCapabilitiesKHR.currentExtent.width;
            framebuffer_create_info.height = mVkSurfaceCapabilitiesKHR.currentExtent.height;
            framebuffer_create_info.layers = 1;
            vkCreateFramebuffer ( mVulkanRenderer.GetDevice(), &framebuffer_create_info, nullptr, &mVkFramebuffers[i] );
        }
    }

    void VulkanWindow::InitializeRenderPass()
    {
        uint32_t surface_format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR ( mVulkanRenderer.GetPhysicalDevice(), mVkSurfaceKHR, &surface_format_count, nullptr );
        if ( surface_format_count == 0 )
        {
            std::ostringstream stream;
            stream << "Physical device reports no surface formats.";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        std::vector<VkSurfaceFormatKHR> surface_format_list ( surface_format_count );
        vkGetPhysicalDeviceSurfaceFormatsKHR (
            mVulkanRenderer.GetPhysicalDevice(),
            mVkSurfaceKHR,
            &surface_format_count,
            surface_format_list.data() );

        if ( surface_format_list[0].format == VK_FORMAT_UNDEFINED )
        {
            mVkSurfaceFormatKHR.format = VK_FORMAT_B8G8R8A8_UNORM;
            mVkSurfaceFormatKHR.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
        else
        {
            mVkSurfaceFormatKHR = surface_format_list[0];
        }
        std::array<VkFormat, 5> try_formats
        {
            {
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D16_UNORM
            }
        };
        for ( auto format : try_formats )
        {
            VkFormatProperties format_properties{};
            vkGetPhysicalDeviceFormatProperties ( mVulkanRenderer.GetPhysicalDevice(), format, &format_properties );
            if ( format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
            {
                mVkDepthStencilFormat = format;
                break;
            }
        }

        if ( std::find ( try_formats.begin(), try_formats.end(), mVkDepthStencilFormat ) == try_formats.end() )
        {
            std::ostringstream stream;
            stream << "Unable to find a suitable depth stencil format";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        std::array<VkAttachmentDescription, 2> attachment_descriptions{ {} };
        attachment_descriptions[0].flags = 0;
        attachment_descriptions[0].format = mVkSurfaceFormatKHR.format;
        attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachment_descriptions[1].flags = 0;
        attachment_descriptions[1].format = mVkDepthStencilFormat;
        attachment_descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkAttachmentReference, 1> color_attachment_references{};
        color_attachment_references[0].attachment = 0;
        color_attachment_references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_stencil_attachment_reference{};
        depth_stencil_attachment_reference.attachment = 1;
        depth_stencil_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 1> subpass_descriptions{};
        subpass_descriptions[0].flags = 0;
        subpass_descriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_descriptions[0].inputAttachmentCount = 0;
        subpass_descriptions[0].pInputAttachments = nullptr;
        subpass_descriptions[0].colorAttachmentCount = static_cast<uint32_t> ( color_attachment_references.size() );
        subpass_descriptions[0].pColorAttachments = color_attachment_references.data();
        subpass_descriptions[0].pResolveAttachments = nullptr;
        subpass_descriptions[0].pDepthStencilAttachment = &depth_stencil_attachment_reference;
        subpass_descriptions[0].preserveAttachmentCount = 0;
        subpass_descriptions[0].pPreserveAttachments = nullptr;

        VkSubpassDependency subpass_dependency = {};
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = 0;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = static_cast<uint32_t> ( attachment_descriptions.size() );
        render_pass_create_info.pAttachments = attachment_descriptions.data();
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &subpass_dependency;
        render_pass_create_info.subpassCount = static_cast<uint32_t> ( subpass_descriptions.size() );
        render_pass_create_info.pSubpasses = subpass_descriptions.data();
        vkCreateRenderPass ( mVulkanRenderer.GetDevice(), &render_pass_create_info, nullptr, &mVkRenderPass );
    }

    void VulkanWindow::FinalizeRenderPass()
    {
        if ( mVkRenderPass != VK_NULL_HANDLE )
        {
            vkDestroyRenderPass ( mVulkanRenderer.GetDevice(), mVkRenderPass, nullptr );
        }
    }

    void VulkanWindow::InitializeMatrices()
    {
        mMatrices.Initialize (
            sizeof ( float ) * 16 * 3,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

        VkDescriptorSetLayoutCreateInfo matrices_descriptor_set_layout_create_info{};
        matrices_descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        matrices_descriptor_set_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding matrices_descriptor_set_layout_binding{};
        matrices_descriptor_set_layout_binding.binding = 0;
        matrices_descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        matrices_descriptor_set_layout_binding.descriptorCount = 1;
        // we're assuming only vertex shaders will access matrices for now
        matrices_descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        matrices_descriptor_set_layout_binding.pImmutableSamplers = nullptr;
        matrices_descriptor_set_layout_create_info.pBindings = &matrices_descriptor_set_layout_binding;

        mMatricesDescriptorPool = CreateDescriptorPool ( mVulkanRenderer.GetDevice(), {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}} );
        mMatricesDescriptorSet = CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mMatricesDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( matrices_descriptor_set_layout_create_info ) );
        VkDescriptorBufferInfo descriptor_buffer_info{};
        descriptor_buffer_info.buffer = mMatrices.GetBuffer();
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range = mMatrices.GetSize();
        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.pNext = nullptr;
        write_descriptor_set.dstSet = mMatricesDescriptorSet;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
        write_descriptor_set.pImageInfo = nullptr;
        write_descriptor_set.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), 1, &write_descriptor_set, 0, nullptr );
    }

    void VulkanWindow::FinalizeMatrices()
    {
        DestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mMatricesDescriptorPool );
        mMatrices.Finalize();
    }

    void VulkanWindow::Initialize()
    {
        InitializeMatrices();
        InitializeSurface();
        InitializeRenderPass();
        InitializeSwapchain();
        InitializeImageViews();
        InitializeDepthStencil();
        InitializeFrameBuffers();
        InitializeCommandBuffer();
    }

    void VulkanWindow::Finalize()
    {
        if ( VkResult result = vkQueueWaitIdle ( mVulkanRenderer.GetQueue() ) )
        {
            std::cerr << "vkQueueWaitIdle failed: " << GetVulkanResultString ( result );
        }
        if ( VkResult result = vkDeviceWaitIdle ( mVulkanRenderer.GetDevice() ) )
        {
            std::cerr << "vkDeviceWaitIdle failed: " << GetVulkanResultString ( result );
        }
        FinalizeCommandBuffer();
        FinalizeFrameBuffers();
        FinalizeDepthStencil();
        FinalizeImageViews();
        FinalizeSwapchain();
        FinalizeRenderPass();
        FinalizeSurface();
        FinalizeMatrices();
    }

    void VulkanWindow::ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        VkSurfaceCapabilitiesKHR surface_capabilities{};
        VkResult result {vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( mVulkanRenderer.GetPhysicalDevice(), mVkSurfaceKHR, &surface_capabilities ) };
        if ( result == VK_SUCCESS && std::memcmp ( &surface_capabilities, &mVkSurfaceCapabilitiesKHR, sizeof ( VkSurfaceCapabilitiesKHR ) ) != 0 )
        {
            if ( VK_SUCCESS != ( result = vkQueueWaitIdle ( mVulkanRenderer.GetQueue() ) ) )
            {
                std::ostringstream stream;
                stream << "vkQueueWaitIdle failed: " << GetVulkanResultString ( result );
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }

            if ( VK_SUCCESS != ( result = vkDeviceWaitIdle ( mVulkanRenderer.GetDevice() ) ) )
            {
                std::ostringstream stream;
                stream << "vkDeviceWaitIdle failed: " << GetVulkanResultString ( result );
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }
            FinalizeFrameBuffers();
            FinalizeDepthStencil();
            FinalizeImageViews();
            InitializeSwapchain();
            InitializeImageViews();
            InitializeDepthStencil();
            InitializeFrameBuffers();
        }
        mVkViewport.x = static_cast<float> ( aX );
        mVkViewport.y = static_cast<float> ( aY );
        mVkViewport.width = static_cast<float> ( aWidth );
        mVkViewport.height = static_cast<float> ( aHeight );
        // Clip Scissors to surface extents
        mVkScissor.offset.x = ( aX < 0 ) ? 0 : aX;
        mVkScissor.offset.y = ( aY < 0 ) ? 0 : aY;
        mVkScissor.extent.width = ( aX + aWidth > mVkSurfaceCapabilitiesKHR.currentExtent.width ) ? mVkSurfaceCapabilitiesKHR.currentExtent.width : aX + aWidth;
        mVkScissor.extent.height = ( aY + aHeight > mVkSurfaceCapabilitiesKHR.currentExtent.height ) ? mVkSurfaceCapabilitiesKHR.currentExtent.height : aY + aHeight;
    }

    void VulkanWindow::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mProjectionMatrix = aMatrix;
        mFrustum = mProjectionMatrix * mViewMatrix;
        mMatrices.WriteMemory ( sizeof ( float ) * 16, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );
    }

    void VulkanWindow::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mViewMatrix = aMatrix;
        mFrustum = mProjectionMatrix * mViewMatrix;
        mMatrices.WriteMemory ( sizeof ( float ) * 16 * 2, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );
    }

    const Matrix4x4 & VulkanWindow::GetProjectionMatrix() const
    {
        return mProjectionMatrix;
    }

    const Matrix4x4 & VulkanWindow::GetViewMatrix() const
    {
        return mViewMatrix;
    }

    const Frustum & VulkanWindow::GetFrustum() const
    {
        return mFrustum;
    }

    void VulkanWindow::InitializeCommandBuffer()
    {
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        /** @note Flags should be set to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         * but we get an annoying from the validation layers to reset the pool rather than
         * each command buffer, since we're allocating a single buffer from this pool,
         * lets indulge the layer.
         */
        command_pool_create_info.flags = /*VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT*/ 0;
        command_pool_create_info.queueFamilyIndex = mVulkanRenderer.GetQueueFamilyIndex();
        vkCreateCommandPool ( mVulkanRenderer.GetDevice(), &command_pool_create_info, nullptr, &mVkCommandPool );

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = mVkCommandPool;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1;
        vkAllocateCommandBuffers ( mVulkanRenderer.GetDevice(), &command_buffer_allocate_info, &mVkCommandBuffer );
    }

    void VulkanWindow::FinalizeCommandBuffer()
    {
        if ( mVkCommandPool != VK_NULL_HANDLE )
        {
            vkDestroyCommandPool ( mVulkanRenderer.GetDevice(), mVkCommandPool, nullptr );
            mVkCommandPool = VK_NULL_HANDLE;
        }
    }

    void VulkanWindow::BeginRender()
    {
        if ( VkResult result = vkAcquireNextImageKHR (
                                   mVulkanRenderer.GetDevice(),
                                   mVkSwapchainKHR,
                                   UINT64_MAX, VK_NULL_HANDLE,
                                   mVulkanRenderer.GetFence(),
                                   const_cast<uint32_t * > ( &mActiveImageIndex ) ) )
        {
            std::cout << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }

        if ( VkResult result = vkWaitForFences ( mVulkanRenderer.GetDevice(), 1,
                               &mVulkanRenderer.GetFence(),
                               VK_TRUE, UINT64_MAX ) )
        {
            std::cout << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        if ( VkResult result = vkResetFences ( mVulkanRenderer.GetDevice(), 1,
                                               &mVulkanRenderer.GetFence() ) )
        {
            std::cout << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        if ( VkResult result = vkQueueWaitIdle ( mVulkanRenderer.GetQueue() ) )
        {
            std::cout << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }

        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkResetCommandPool ( mVulkanRenderer.GetDevice(), mVkCommandPool, 0 );
        if ( VkResult result = vkBeginCommandBuffer ( mVkCommandBuffer, &command_buffer_begin_info ) )
        {
            std::cout << GetVulkanResultString ( result ) << "  Error Code: " << result << " at " << __func__ << " line " << __LINE__ << " " << std::endl;
        }

        vkCmdSetViewport ( mVkCommandBuffer, 0, 1, &mVkViewport );
        vkCmdSetScissor ( mVkCommandBuffer, 0, 1, &mVkScissor );

        /* [0] is color, [1] is depth/stencil.*/
        /**@todo Allow for changing the clear values.*/
        std::array<VkClearValue, 2> clear_values{ { { {{0}} }, { {{0}} } } };
        clear_values[0].color.float32[0] = 0.5f;
        clear_values[0].color.float32[1] = 0.5f;
        clear_values[0].color.float32[2] = 0.5f;
        clear_values[0].color.float32[3] = 0.0f;
        clear_values[1].depthStencil.depth = 1.0f;
        clear_values[1].depthStencil.stencil = 0;

        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = mVkRenderPass;
        render_pass_begin_info.framebuffer = mVkFramebuffers[mActiveImageIndex];
        render_pass_begin_info.renderArea = mVkScissor;
        render_pass_begin_info.clearValueCount = static_cast<uint32_t> ( clear_values.size() );
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass ( mVkCommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );
    }

    void VulkanWindow::EndRender()
    {
        vkCmdEndRenderPass ( mVkCommandBuffer );
        if ( VkResult result = vkEndCommandBuffer ( mVkCommandBuffer ) )
        {
            std::cout << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = nullptr;
        submit_info.pWaitDstStageMask = nullptr;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &mVkCommandBuffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &mVulkanRenderer.GetSignalSemaphore();
        if ( VkResult result = vkQueueSubmit ( mVulkanRenderer.GetQueue(), 1, &submit_info, VK_NULL_HANDLE ) )
        {
            std::cout << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        std::array<VkResult, 1> result_array{ { VkResult::VK_SUCCESS } };
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &mVulkanRenderer.GetSignalSemaphore();
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &mVkSwapchainKHR;
        present_info.pImageIndices = &mActiveImageIndex;
        present_info.pResults = result_array.data();
        if ( VkResult result = vkQueuePresentKHR ( mVulkanRenderer.GetQueue(), &present_info ) )
        {
            std::cout << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        mMemoryPoolBuffer.Reset();
    }

    static const std::unordered_map<Topology, VkPrimitiveTopology> TopologyMap
    {
        {POINT_LIST, VK_PRIMITIVE_TOPOLOGY_POINT_LIST},
        {LINE_STRIP, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP},
        {LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
        {TRIANGLE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP},
        {TRIANGLE_FAN, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN},
        {TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
        {LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY},
        {LINE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY},
        {TRIANGLE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY},
        {TRIANGLE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY},
        {PATCH_LIST, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST}
    };

    void VulkanWindow::Render ( const Matrix4x4& aModelMatrix,
                                const Mesh& aMesh,
                                const Pipeline& aPipeline,
                                const Material* aMaterial,
                                const BufferAccessor* aSkeleton,
                                Topology aTopology,
                                uint32_t aVertexStart,
                                uint32_t aVertexCount,
                                uint32_t aInstanceCount,
                                uint32_t aFirstInstance ) const
    {
        const VulkanPipeline* pipeline = mVulkanRenderer.GetVulkanPipeline ( aPipeline );
        assert ( pipeline );
        mMatrices.WriteMemory ( 0, sizeof ( float ) * 16, aModelMatrix.GetMatrix4x4() );
        vkCmdBindPipeline ( mVkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline() );
        vkCmdSetPrimitiveTopology ( mVkCommandBuffer, TopologyMap.at ( aTopology ) );

        if ( uint32_t matrix_set_index = pipeline->GetMatrixDescriptorSet(); matrix_set_index != std::numeric_limits<uint32_t>::max() )
        {
            vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      pipeline->GetPipelineLayout(),
                                      matrix_set_index,
                                      1,
                                      &mMatricesDescriptorSet, 0, nullptr );
        }
#if 0
        vkCmdPushConstants ( mVkCommandBuffer,
                             pipeline->GetPipelineLayout(),
                             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                             0, sizeof ( float ) * 16, aModelMatrix.GetMatrix4x4() );
#endif
        if ( aMaterial != nullptr )
        {
            mVulkanRenderer.GetVulkanMaterial ( *aMaterial )->Bind ( mVkCommandBuffer, *pipeline );
        }
        if ( aSkeleton != nullptr )
        {
            const VulkanMemoryPoolBuffer* memory_pool_buffer =
                reinterpret_cast<const VulkanMemoryPoolBuffer*> ( aSkeleton->GetMemoryPoolBuffer() );
            uint32_t offset = static_cast<uint32_t> ( aSkeleton->GetOffset() );
            if ( uint32_t skeleton_set_index = pipeline->GetSkeletonDescriptorSet(); skeleton_set_index != std::numeric_limits<uint32_t>::max() )
            {
                vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          pipeline->GetPipelineLayout(),
                                          skeleton_set_index,
                                          1,
                                          &memory_pool_buffer->GetDescriptorSet(), 1, &offset );
            }
        }
        mVulkanRenderer.GetVulkanMesh ( aMesh )->Bind ( mVkCommandBuffer );
        if ( aMesh.GetIndexCount() )
        {
            vkCmdDrawIndexed (
                mVkCommandBuffer,
                ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetIndexCount(),
                aInstanceCount,
                aVertexStart,
                0,
                aFirstInstance );
        }
        else
        {
            vkCmdDraw (
                mVkCommandBuffer,
                ( aVertexCount != 0xffffffff ) ? aVertexCount : aMesh.GetVertexCount(),
                aInstanceCount,
                aVertexStart,
                aFirstInstance );
        }
    }

    void VulkanWindow::FinalizeSurface()
    {
        if ( mVkSurfaceKHR != VK_NULL_HANDLE )
        {
            vkDestroySurfaceKHR ( mVulkanRenderer.GetInstance(), mVkSurfaceKHR, nullptr );
            mVkSurfaceKHR = VK_NULL_HANDLE;
        }
    }

    void VulkanWindow::FinalizeSwapchain()
    {
        if ( mVkSwapchainKHR != VK_NULL_HANDLE )
        {
            vkDestroySwapchainKHR ( mVulkanRenderer.GetDevice(), mVkSwapchainKHR, nullptr );
            mVkSwapchainKHR = VK_NULL_HANDLE;
        }
    }

    void VulkanWindow::FinalizeImageViews()
    {
        for ( auto& i : mVkSwapchainImageViews )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyImageView ( mVulkanRenderer.GetDevice(), i, nullptr );
                i = VK_NULL_HANDLE;
            }
        }
    }

    void VulkanWindow::FinalizeDepthStencil()
    {
        if ( mVkDepthStencilImageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( mVulkanRenderer.GetDevice(), mVkDepthStencilImageView, nullptr );
            mVkDepthStencilImageView = VK_NULL_HANDLE;
        }
        if ( mVkDepthStencilImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( mVulkanRenderer.GetDevice(), mVkDepthStencilImage, nullptr );
            mVkDepthStencilImage = VK_NULL_HANDLE;
        }
        if ( mVkDepthStencilImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVulkanRenderer.GetDevice(), mVkDepthStencilImageMemory, nullptr );
            mVkDepthStencilImageMemory = VK_NULL_HANDLE;
        }
    }

    void VulkanWindow::FinalizeFrameBuffers()
    {
        for ( auto& i : mVkFramebuffers )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyFramebuffer ( mVulkanRenderer.GetDevice(), i, nullptr );
                i = VK_NULL_HANDLE;
            }
        }
    }

    BufferAccessor VulkanWindow::AllocateSingleFrameUniformMemory ( size_t aSize )
    {
        return mMemoryPoolBuffer.Allocate ( aSize );
    }

    VkRenderPass VulkanWindow::GetRenderPass() const
    {
        return mVkRenderPass;
    }

    VkCommandBuffer VulkanWindow::GetCommandBuffer() const
    {
        return mVkCommandBuffer;
    }
}
