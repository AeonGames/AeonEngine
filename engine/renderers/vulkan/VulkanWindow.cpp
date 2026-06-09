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
#include "VulkanWindow.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanMaterial.hpp"
#include "VulkanUtilities.hpp"
#include "VulkanMesh.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanStorageMemoryPoolBuffer.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <utility>
#include <cstring>
#include <cstdlib>
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
#include "aeongames/BufferAccessor.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/ResourceId.hpp"

#if defined(__APPLE__)
// Helper function to get CAMetalLayer from NSView (implemented in MacOSMetalHelper.mm)
extern "C" void* GetMetalLayerFromNSView ( void* nsview_ptr );
#endif

namespace AeonGames
{
    VulkanWindow::VulkanWindow ( VulkanRenderer&  aVulkanRenderer, void* aWindowId ) :
        mVulkanRenderer { aVulkanRenderer }, mWindowId{aWindowId},
        mMemoryPoolBuffer{mVulkanRenderer, 64_kb},
        mStorageMemoryPoolBuffer{mVulkanRenderer, 8_mb},
        mMatrices { aVulkanRenderer },
        mLights { aVulkanRenderer },
        mClusterParams { aVulkanRenderer }
    {
        std::cout << LogLevel::Info << "Creating VulkanWindow." << std::endl;
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
        mStorageMemoryPoolBuffer{std::move ( aVulkanWindow.mStorageMemoryPoolBuffer ) },
        mMatrices{std::move ( aVulkanWindow.mMatrices ) },
        mLights{std::move ( aVulkanWindow.mLights ) },
        mClusterParams{std::move ( aVulkanWindow.mClusterParams ) }
    {
        std::cout << LogLevel::Debug << "Moving VulkanWindow." << std::endl;
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
        std::swap ( mLightsDescriptorPool, aVulkanWindow.mLightsDescriptorPool );
        std::swap ( mLightsDescriptorSet, aVulkanWindow.mLightsDescriptorSet );
        std::swap ( mClusterParamsDescriptorPool, aVulkanWindow.mClusterParamsDescriptorPool );
        std::swap ( mClusterParamsDescriptorSet, aVulkanWindow.mClusterParamsDescriptorSet );
        std::swap ( mFrameLightGrid, aVulkanWindow.mFrameLightGrid );
        std::swap ( mFrameLightIndexList, aVulkanWindow.mFrameLightIndexList );
        std::swap ( mVkCommandPool, aVulkanWindow.mVkCommandPool );
        std::swap ( mVkCommandBuffer, aVulkanWindow.mVkCommandBuffer );
        std::swap ( mVkAcquireSemaphore, aVulkanWindow.mVkAcquireSemaphore );
        std::swap ( mVkFence, aVulkanWindow.mVkFence );
        mVkSwapchainImages.swap ( aVulkanWindow.mVkSwapchainImages );
        mVkSwapchainImageViews.swap ( aVulkanWindow.mVkSwapchainImageViews );
        mVkFramebuffers.swap ( aVulkanWindow.mVkFramebuffers );
        mVkSubmitSemaphores.swap ( aVulkanWindow.mVkSubmitSemaphores );
    }

    VulkanWindow::~VulkanWindow()
    {
        std::cout << LogLevel::Info << "Destroying VulkanWindow." << std::endl;
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
        metal_surface_create_info_ext.pLayer = reinterpret_cast<CAMetalLayer*> ( GetMetalLayerFromNSView ( mWindowId ) );
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
        std::cout << LogLevel::Debug << "Initializing VulkanWindow Swapchain." << std::endl;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( mVulkanRenderer.GetPhysicalDevice(), mVkSurfaceKHR, &mVkSurfaceCapabilitiesKHR );

        if ( mVkSurfaceCapabilitiesKHR.currentExtent.width == 0 ||
             mVkSurfaceCapabilitiesKHR.currentExtent.height == 0 )
        {
            std::ostringstream stream;
            stream << "Cannot create swapchain with zero area. (" << mVkSurfaceCapabilitiesKHR.currentExtent.width << "x" << mVkSurfaceCapabilitiesKHR.currentExtent.height << ")" << std::endl;
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
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

        std::cout << LogLevel::Debug << "VulkanWindow Swapchain created with " << mSwapchainImageCount << " images." << std::endl;
        // Create semaphores for rendering.
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if ( VkResult result = vkCreateSemaphore ( mVulkanRenderer.GetDevice(), &semaphore_create_info, nullptr, &mVkAcquireSemaphore ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer semaphore. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        std::cout << LogLevel::Debug << "Created Acquire Semaphore " << std::hex << mVkAcquireSemaphore << std::endl;
        mVkSubmitSemaphores.resize ( mSwapchainImageCount );
        for ( size_t i = 0; i < mSwapchainImageCount; ++i )
        {
            if ( VkResult result = vkCreateSemaphore ( mVulkanRenderer.GetDevice(), &semaphore_create_info, nullptr, &mVkSubmitSemaphores[i] ) )
            {
                std::ostringstream stream;
                stream << "Could not create VulkanRenderer semaphore. error code: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
            std::cout << LogLevel::Debug << "Created Submit Semaphore " << std::hex << mVkSubmitSemaphores[i] << std::endl;
        }
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence ( mVulkanRenderer.GetDevice(), &fence_create_info, nullptr, &mVkFence );
    }

    void VulkanWindow::InitializeImageViews()
    {
        if ( mVkSwapchainKHR == VK_NULL_HANDLE )
        {
            std::ostringstream stream;
            stream << "Cannot initialize swapchain image views with no swapchain.";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
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
            sizeof ( float ) * 16 * 2,
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
        //matrices_descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        matrices_descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
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

    void VulkanWindow::InitializeLights()
    {
        // Per-frame Lights is a persistent storage buffer (SSBO): the light cap
        // is large (MAX_LIGHTS_PER_FRAME) so it no longer fits a uniform buffer.
        mLights.Initialize (
            GpuLightsBufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            nullptr );
        // Zero the header so a frame that draws before SetLights reads no lights.
        const GpuLightsHeader empty_header{};
        mLights.WriteMemory ( 0, sizeof ( empty_header ), &empty_header );

        VkDescriptorSetLayoutCreateInfo lights_descriptor_set_layout_create_info{};
        lights_descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        lights_descriptor_set_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding lights_descriptor_set_layout_binding{};
        lights_descriptor_set_layout_binding.binding = 0;
        lights_descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        lights_descriptor_set_layout_binding.descriptorCount = 1;
        lights_descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        lights_descriptor_set_layout_binding.pImmutableSamplers = nullptr;
        lights_descriptor_set_layout_create_info.pBindings = &lights_descriptor_set_layout_binding;

        mLightsDescriptorPool = CreateDescriptorPool ( mVulkanRenderer.GetDevice(), {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}} );
        mLightsDescriptorSet = CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mLightsDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( lights_descriptor_set_layout_create_info ) );
        VkDescriptorBufferInfo descriptor_buffer_info{};
        descriptor_buffer_info.buffer = mLights.GetBuffer();
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range = mLights.GetSize();
        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.pNext = nullptr;
        write_descriptor_set.dstSet = mLightsDescriptorSet;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
        write_descriptor_set.pImageInfo = nullptr;
        write_descriptor_set.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), 1, &write_descriptor_set, 0, nullptr );
    }

    void VulkanWindow::FinalizeLights()
    {
        DestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mLightsDescriptorPool );
        mLights.Finalize();
    }

    void VulkanWindow::InitializeClusterParams()
    {
        GpuClusterParams empty{};
        mClusterParams.Initialize (
            sizeof ( GpuClusterParams ),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &empty );

        VkDescriptorSetLayoutCreateInfo cluster_params_descriptor_set_layout_create_info{};
        cluster_params_descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        cluster_params_descriptor_set_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding cluster_params_descriptor_set_layout_binding{};
        cluster_params_descriptor_set_layout_binding.binding = 0;
        cluster_params_descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cluster_params_descriptor_set_layout_binding.descriptorCount = 1;
        cluster_params_descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        cluster_params_descriptor_set_layout_binding.pImmutableSamplers = nullptr;
        cluster_params_descriptor_set_layout_create_info.pBindings = &cluster_params_descriptor_set_layout_binding;

        mClusterParamsDescriptorPool = CreateDescriptorPool ( mVulkanRenderer.GetDevice(), {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}} );
        mClusterParamsDescriptorSet = CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mClusterParamsDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( cluster_params_descriptor_set_layout_create_info ) );
        VkDescriptorBufferInfo descriptor_buffer_info{};
        descriptor_buffer_info.buffer = mClusterParams.GetBuffer();
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range = mClusterParams.GetSize();
        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.pNext = nullptr;
        write_descriptor_set.dstSet = mClusterParamsDescriptorSet;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
        write_descriptor_set.pImageInfo = nullptr;
        write_descriptor_set.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), 1, &write_descriptor_set, 0, nullptr );
    }

    void VulkanWindow::FinalizeClusterParams()
    {
        DestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mClusterParamsDescriptorPool );
        mClusterParams.Finalize();
    }

    void VulkanWindow::UpdateClusterParams()
    {
        GpuClusterParams params{};
        params.inverse_projection = Matrix4x4 ( mProjectionMatrix ).GetInvertedMatrix4x4();
        params.screen[0] = mVkViewport.width;
        params.screen[1] = mVkViewport.height;
        // Debug cluster heatmap toggle, read once from the environment.
        static const bool heatmap = []
        {
            const char* value = std::getenv ( "AEON_CLUSTER_HEATMAP" );
            return value != nullptr && value[0] != '\0' && value[0] != '0';
        } ();
        params.screen[2] = heatmap ? 1.0f : 0.0f;
        // screen.w gates active-cluster culling in the light-cull stage: it is
        // only set once the depth pre-pass mark stage has run this frame.
        params.screen[3] = mActiveCullEnabled ? 1.0f : 0.0f;
        mClusterParams.WriteMemory ( 0, sizeof ( GpuClusterParams ), &params );
    }

    void VulkanWindow::Initialize()
    {
        InitializeMatrices();
        InitializeLights();
        InitializeClusterParams();
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
            std::cerr << LogLevel::Error << "vkQueueWaitIdle failed: " << GetVulkanResultString ( result );
        }
        if ( VkResult result = vkDeviceWaitIdle ( mVulkanRenderer.GetDevice() ) )
        {
            std::cerr << LogLevel::Error << "vkDeviceWaitIdle failed: " << GetVulkanResultString ( result );
        }
        FinalizeCommandBuffer();
        FinalizeFrameBuffers();
        FinalizeDepthStencil();
        FinalizeImageViews();
        FinalizeSwapchain();
        FinalizeRenderPass();
        FinalizeSurface();
        FinalizeClusterParams();
        FinalizeLights();
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
            FinalizeSwapchain();
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
        UpdateClusterParams();
    }

    void VulkanWindow::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mProjectionMatrix = aMatrix;
        mFrustum = mProjectionMatrix * mViewMatrix;
        mMatrices.WriteMemory ( 0, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );
        UpdateClusterParams();
    }

    void VulkanWindow::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mViewMatrix = aMatrix;
        mFrustum = mProjectionMatrix * mViewMatrix;
        mMatrices.WriteMemory ( sizeof ( float ) * 16, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );
    }

    void VulkanWindow::SetLights ( std::span<const GpuLight> aLights )
    {
        // Frustum-cull the lights before uploading: any point/spot light whose
        // bounding sphere lies entirely outside the view frustum cannot affect
        // a visible pixel, so dropping it here shrinks the set the per-cluster
        // light cull has to iterate. Directional lights are always kept.
        mVisibleLights.clear();
        for ( const GpuLight& light : aLights )
        {
            if ( mVisibleLights.size() >= MAX_LIGHTS_PER_FRAME )
            {
                break;
            }
            if ( mFrustum.Intersects ( light ) )
            {
                mVisibleLights.push_back ( light );
            }
        }
        // Stream the light records into the SSBO: write the 16-byte header
        // (count) then the tightly packed array.
        const size_t count = mVisibleLights.size();
        const GpuLightsHeader header{ static_cast<uint32_t> ( count ), { 0, 0, 0 } };
        mLights.WriteMemory ( 0, sizeof ( header ), &header );
        if ( count > 0 )
        {
            mLights.WriteMemory ( sizeof ( GpuLightsHeader ), count * sizeof ( GpuLight ), mVisibleLights.data() );
        }
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

    const BufferAccessor & VulkanWindow::GetFrameLightGrid() const
    {
        return mFrameLightGrid;
    }

    const BufferAccessor & VulkanWindow::GetFrameClusterActive() const
    {
        return mFrameClusterActive;
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

    void VulkanWindow::BeginFrame()
    {
        // Idempotent within a frame: the application may call BeginFrame()
        // explicitly to run a pre-render-pass compute phase (e.g. skinning)
        // before BeginRender(), which also calls BeginFrame().
        if ( mFrameBegun )
        {
            return;
        }
        mFrameBegun = true;
        if ( VkResult result = vkWaitForFences ( mVulkanRenderer.GetDevice(), 1,
                               &mVkFence,
                               VK_TRUE, UINT64_MAX ) )
        {
            std::cout << LogLevel::Error << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        if ( VkResult result = vkResetFences ( mVulkanRenderer.GetDevice(), 1,
                                               &mVkFence ) )
        {
            std::cout << LogLevel::Error << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        if ( VkResult result = vkAcquireNextImageKHR (
                                   mVulkanRenderer.GetDevice(),
                                   mVkSwapchainKHR,
                                   UINT64_MAX,
                                   mVkAcquireSemaphore,
                                   VK_NULL_HANDLE,
                                   const_cast<uint32_t * > ( &mActiveImageIndex ) ) )
        {
            std::cout << LogLevel::Error << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkResetCommandPool ( mVulkanRenderer.GetDevice(), mVkCommandPool, 0 );
        if ( VkResult result = vkBeginCommandBuffer ( mVkCommandBuffer, &command_buffer_begin_info ) )
        {
            std::cout << LogLevel::Error << GetVulkanResultString ( result ) << "  Error Code: " << result << " at " << __func__ << " line " << __LINE__ << " " << std::endl;
        }

        vkCmdSetViewport ( mVkCommandBuffer, 0, 1, &mVkViewport );
        vkCmdSetScissor ( mVkCommandBuffer, 0, 1, &mVkScissor );
    }

    void VulkanWindow::BeginRenderPass()
    {
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

    void VulkanWindow::BeginRender ( const Pipeline* aComputePipeline )
    {
        BeginFrame();
        if ( aComputePipeline != nullptr )
        {
            // Enable active-cluster culling for this frame and refresh the
            // ClusterParams UBO so the light-cull stage sees screen.w = 1.
            mActiveCullEnabled = true;
            UpdateClusterParams();
            // Lazily load the renderer-owned marking pipeline that substitutes
            // the scene's draw pipelines during the depth pre-pass.
            if ( !mClusterMarkLoaded )
            {
                mClusterMarkPipeline.LoadFromId ( "shaders/cluster_mark.txt"_crc32 );
                mClusterMarkLoaded = true;
            }
            // Stage 0: build the cluster AABBs, reset the index allocator and
            // clear the per-cluster active flags before the mark pass runs.
            DispatchClusterBuild ( *aComputePipeline );
            Barrier();
            // Begin the depth pre-pass; the application's first geometry
            // traversal records into it with the marking pipeline substituted.
            BeginRenderPass();
        }
        else
        {
            mActiveCullEnabled = false;
            UpdateClusterParams();
            // No clustering this frame: still hand the clustered fragment
            // shader valid, empty light buffers so it reads zero lights per
            // cluster instead of sampling an unbound buffer.
            mFrameLightGrid = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
            mFrameLightIndexList = mStorageMemoryPoolBuffer.Allocate ( LIGHT_INDEX_LIST_CAPACITY * sizeof ( uint32_t ) );
            if ( void * grid = mFrameLightGrid.Map() )
            {
                std::memset ( grid, 0, CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
                mFrameLightGrid.Unmap();
            }
            BeginRenderPass();
        }
    }

    void VulkanWindow::EndDepthPrePass ( const Pipeline* aComputePipeline )
    {
        // End the depth pre-pass render pass so light culling can run outside
        // any render pass.
        vkCmdEndRenderPass ( mVkCommandBuffer );
        // The mark pass wrote the per-cluster active flags from the fragment
        // shader; make those writes visible to the light-cull compute stage.
        Barrier();
        if ( aComputePipeline != nullptr )
        {
            DispatchLightCull ( *aComputePipeline );
        }
        // Begin the main color pass; the application's second geometry
        // traversal shades normally using the now-populated light grid.
        BeginRenderPass();
    }

    void VulkanWindow::DispatchClusterBuild ( const Pipeline& aComputePipeline )
    {
        // One workgroup per 64 clusters (clustering compute stages use local_size_x=64).
        constexpr uint32_t group_count = ( CLUSTER_COUNT + 63u ) / 64u;

        // Allocate the per-frame clustering buffers. They persist as members so
        // both this build dispatch and the post-mark light-cull dispatch bind
        // the same storage; reflection silently drops the blocks a given stage
        // does not declare.
        mFrameClusterAABBs = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( GpuClusterAABB ) );
        mFrameLightGrid = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( GpuLightGridCell ) );
        mFrameLightIndexList = mStorageMemoryPoolBuffer.Allocate ( LIGHT_INDEX_LIST_CAPACITY * sizeof ( uint32_t ) );
        // Global atomic allocator for the flat LightIndexList (R1). cluster_build
        // zeroes it from invocation 0, so no host-side initialisation is needed.
        mFrameLightIndexCounter = mStorageMemoryPoolBuffer.Allocate ( sizeof ( uint32_t ) );
        // Per-cluster active flags (R2): cluster_build clears them, the mark
        // pass sets them, the light-cull stage reads them.
        mFrameClusterActive = mStorageMemoryPoolBuffer.Allocate ( CLUSTER_COUNT * sizeof ( uint32_t ) );

        const StorageBufferBinding bindings[]
        {
            { Mesh::BindingLocations::CLUSTER_AABBS, &mFrameClusterAABBs },
            { Mesh::BindingLocations::LIGHT_GRID, &mFrameLightGrid },
            { Mesh::BindingLocations::LIGHT_INDEX_LIST, &mFrameLightIndexList },
            { Mesh::BindingLocations::LIGHT_INDEX_COUNTER, &mFrameLightIndexCounter },
            { Mesh::BindingLocations::CLUSTER_ACTIVE, &mFrameClusterActive },
        };

        // Stage 0 only: build AABBs, reset the allocator and clear active flags.
        Dispatch ( aComputePipeline, group_count, 1, 1, bindings, 0 );
    }

    void VulkanWindow::DispatchLightCull ( const Pipeline& aComputePipeline )
    {
        constexpr uint32_t group_count = ( CLUSTER_COUNT + 63u ) / 64u;

        const StorageBufferBinding bindings[]
        {
            { Mesh::BindingLocations::CLUSTER_AABBS, &mFrameClusterAABBs },
            { Mesh::BindingLocations::LIGHT_GRID, &mFrameLightGrid },
            { Mesh::BindingLocations::LIGHT_INDEX_LIST, &mFrameLightIndexList },
            { Mesh::BindingLocations::LIGHT_INDEX_COUNTER, &mFrameLightIndexCounter },
            { Mesh::BindingLocations::CLUSTER_ACTIVE, &mFrameClusterActive },
        };

        // Stages 1..N: cull lights against the AABBs, skipping the clusters the
        // depth pre-pass left inactive. A barrier between stages keeps each
        // stage's writes visible to the next and, after the last stage, to the
        // fragment shader of the main color pass.
        const uint32_t stage_count = aComputePipeline.GetComputeStageCount();
        for ( uint32_t stage = 1; stage < stage_count; ++stage )
        {
            Dispatch ( aComputePipeline, group_count, 1, 1, bindings, stage );
            Barrier();
        }
    }

    void VulkanWindow::EndRender()
    {
        vkCmdEndRenderPass ( mVkCommandBuffer );
        if ( VkResult result = vkEndCommandBuffer ( mVkCommandBuffer ) )
        {
            std::cout << LogLevel::Error << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        VkSubmitInfo submit_info{};
        VkPipelineStageFlags wait_stages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &mVkAcquireSemaphore;
        submit_info.pWaitDstStageMask = &wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &mVkCommandBuffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &mVkSubmitSemaphores[mActiveImageIndex];
        if ( VkResult result = vkQueueSubmit ( mVulkanRenderer.GetQueue(), 1, &submit_info, mVkFence ) )
        {
            std::cout << LogLevel::Error << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        std::array<VkResult, 1> result_array{ { VkResult::VK_SUCCESS } };
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &mVkSubmitSemaphores[mActiveImageIndex];
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &mVkSwapchainKHR;
        present_info.pImageIndices = &mActiveImageIndex;
        present_info.pResults = result_array.data();
        if ( VkResult result = vkQueuePresentKHR ( mVulkanRenderer.GetQueue(), &present_info ) )
        {
            std::cout << LogLevel::Error << GetVulkanResultString ( result ) << "  " << __func__ << " " << __LINE__ << " " << std::endl;
        }
        mMemoryPoolBuffer.Reset();
        mStorageMemoryPoolBuffer.Reset();
        mFrameBegun = false;
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
                                Topology aTopology,
                                uint32_t aVertexStart,
                                uint32_t aVertexCount,
                                uint32_t aInstanceCount,
                                uint32_t aFirstInstance,
                                const BufferAccessor* aSkinnedVertices,
                                RenderPass aRenderPass ) const
    {
        // Resolve the optional pre-skinned vertex buffer produced by the compute
        // skinning pre-pass. When present it is bound as the vertex input in
        // place of the mesh's rest-pose vertices (the index buffer still comes
        // from the mesh).
        VkBuffer skinned_vertex_buffer = VK_NULL_HANDLE;
        VkDeviceSize skinned_vertex_offset = 0;
        if ( aSkinnedVertices != nullptr && aSkinnedVertices->GetMemoryPoolBuffer() != nullptr )
        {
            const VulkanStorageMemoryPoolBuffer* storage_pool_buffer =
                reinterpret_cast<const VulkanStorageMemoryPoolBuffer*> ( aSkinnedVertices->GetMemoryPoolBuffer() );
            skinned_vertex_buffer =
                reinterpret_cast<const VulkanBuffer&> ( storage_pool_buffer->GetBuffer() ).GetBuffer();
            skinned_vertex_offset = aSkinnedVertices->GetOffset();
        }
        // During the depth pre-pass, substitute the renderer-owned marking
        // pipeline: it records only the cluster each fragment occupies into the
        // ClusterActive SSBO and ignores material and lighting state.
        if ( aRenderPass == RenderPass::DepthPrePass )
        {
            const VulkanPipeline* mark_pipeline = mVulkanRenderer.GetVulkanPipeline ( mClusterMarkPipeline );
            vkCmdBindPipeline ( mVkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mark_pipeline->GetVkPipeline() );
            vkCmdSetPrimitiveTopology ( mVkCommandBuffer, TopologyMap.at ( aTopology ) );

            if ( uint32_t matrix_set_index = mark_pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::MATRICES ); matrix_set_index != std::numeric_limits<uint32_t>::max() )
            {
                vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          mark_pipeline->GetPipelineLayout(),
                                          matrix_set_index,
                                          1,
                                          &mMatricesDescriptorSet, 0, nullptr );
            }

            if ( uint32_t cluster_params_set_index = mark_pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::CLUSTER_PARAMS ); cluster_params_set_index != std::numeric_limits<uint32_t>::max() )
            {
                vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          mark_pipeline->GetPipelineLayout(),
                                          cluster_params_set_index,
                                          1,
                                          &mClusterParamsDescriptorSet, 0, nullptr );
            }

            if ( mFrameClusterActive.GetMemoryPoolBuffer() != nullptr )
            {
                if ( uint32_t active_set_index = mark_pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::CLUSTER_ACTIVE ); active_set_index != std::numeric_limits<uint32_t>::max() )
                {
                    const VulkanStorageMemoryPoolBuffer* memory_pool_buffer =
                        reinterpret_cast<const VulkanStorageMemoryPoolBuffer*> ( mFrameClusterActive.GetMemoryPoolBuffer() );
                    size_t offset = mFrameClusterActive.GetOffset();
                    uint32_t dynamic_offset = 0;
                    vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                                              mark_pipeline->GetPipelineLayout(),
                                              active_set_index,
                                              1,
                                              &memory_pool_buffer->GetDescriptorSet ( offset ), 1, &dynamic_offset );
                }
            }

            if ( const VkPushConstantRange& push_constant_model_matrix = mark_pipeline->GetPushConstantModelMatrix() ; push_constant_model_matrix.size != 0 )
            {
                vkCmdPushConstants ( mVkCommandBuffer,
                                     mark_pipeline->GetPipelineLayout(),
                                     push_constant_model_matrix.stageFlags,
                                     push_constant_model_matrix.offset, push_constant_model_matrix.size,
                                     aModelMatrix.GetMatrix4x4() );
            }

            mVulkanRenderer.GetVulkanMesh ( aMesh )->Bind ( mVkCommandBuffer, skinned_vertex_buffer, skinned_vertex_offset );
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
            return;
        }

        const VulkanPipeline* pipeline = mVulkanRenderer.GetVulkanPipeline ( aPipeline );
        vkCmdBindPipeline ( mVkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline() );
        vkCmdSetPrimitiveTopology ( mVkCommandBuffer, TopologyMap.at ( aTopology ) );


        if ( uint32_t matrix_set_index = pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::MATRICES ); matrix_set_index != std::numeric_limits<uint32_t>::max() )
        {
            vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      pipeline->GetPipelineLayout(),
                                      matrix_set_index,
                                      1,
                                      &mMatricesDescriptorSet, 0, nullptr );
        }

        if ( uint32_t lights_set_index = pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::LIGHTS ); lights_set_index != std::numeric_limits<uint32_t>::max() )
        {
            vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      pipeline->GetPipelineLayout(),
                                      lights_set_index,
                                      1,
                                      &mLightsDescriptorSet, 0, nullptr );
        }

        if ( uint32_t cluster_params_set_index = pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::CLUSTER_PARAMS ); cluster_params_set_index != std::numeric_limits<uint32_t>::max() )
        {
            vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      pipeline->GetPipelineLayout(),
                                      cluster_params_set_index,
                                      1,
                                      &mClusterParamsDescriptorSet, 0, nullptr );
        }

        // Clustered Forward+ light lists, produced by the lighting compute
        // pipeline in BeginRender. Bound per draw for pipelines that declare
        // them; mirrors the storage-buffer binding done in Dispatch().
        auto bind_cluster_storage = [&] ( uint32_t aBinding, const BufferAccessor & aAccessor )
        {
            if ( aAccessor.GetMemoryPoolBuffer() == nullptr )
            {
                return;
            }
            uint32_t storage_set_index = pipeline->GetDescriptorSetIndex ( aBinding );
            if ( storage_set_index == std::numeric_limits<uint32_t>::max() )
            {
                return;
            }
            const VulkanStorageMemoryPoolBuffer* memory_pool_buffer =
                reinterpret_cast<const VulkanStorageMemoryPoolBuffer*> ( aAccessor.GetMemoryPoolBuffer() );
            size_t offset = aAccessor.GetOffset();
            uint32_t dynamic_offset = 0;
            vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      pipeline->GetPipelineLayout(),
                                      storage_set_index,
                                      1,
                                      &memory_pool_buffer->GetDescriptorSet ( offset ), 1, &dynamic_offset );
        };
        bind_cluster_storage ( Mesh::BindingLocations::LIGHT_GRID, mFrameLightGrid );
        bind_cluster_storage ( Mesh::BindingLocations::LIGHT_INDEX_LIST, mFrameLightIndexList );

        if ( const VkPushConstantRange& push_constant_model_matrix = pipeline->GetPushConstantModelMatrix() ; push_constant_model_matrix.size != 0 )
        {
            vkCmdPushConstants ( mVkCommandBuffer,
                                 pipeline->GetPipelineLayout(),
                                 push_constant_model_matrix.stageFlags,
                                 push_constant_model_matrix.offset, push_constant_model_matrix.size,
                                 aModelMatrix.GetMatrix4x4() );
        }
        if ( aMaterial != nullptr )
        {
            mVulkanRenderer.GetVulkanMaterial ( *aMaterial )->Bind ( mVkCommandBuffer, *pipeline );
        }
        mVulkanRenderer.GetVulkanMesh ( aMesh )->Bind ( mVkCommandBuffer, skinned_vertex_buffer, skinned_vertex_offset );
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

    void VulkanWindow::Dispatch ( const Pipeline& aPipeline,
                                  uint32_t aGroupCountX,
                                  uint32_t aGroupCountY,
                                  uint32_t aGroupCountZ,
                                  std::span<const StorageBufferBinding> aStorageBuffers,
                                  uint32_t aComputeStageIndex ) const
    {
        const VulkanPipeline* pipeline = mVulkanRenderer.GetVulkanPipeline ( aPipeline );
        vkCmdBindPipeline ( mVkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetVkComputePipeline ( aComputeStageIndex ) );

        if ( uint32_t matrix_set_index = pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::MATRICES ); matrix_set_index != std::numeric_limits<uint32_t>::max() )
        {
            vkCmdBindDescriptorSets ( mVkCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE,
                                      pipeline->GetPipelineLayout(),
                                      matrix_set_index,
                                      1,
                                      &mMatricesDescriptorSet, 0, nullptr );
        }

        if ( uint32_t lights_set_index = pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::LIGHTS ); lights_set_index != std::numeric_limits<uint32_t>::max() )
        {
            vkCmdBindDescriptorSets ( mVkCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE,
                                      pipeline->GetPipelineLayout(),
                                      lights_set_index,
                                      1,
                                      &mLightsDescriptorSet, 0, nullptr );
        }

        if ( uint32_t cluster_params_set_index = pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::CLUSTER_PARAMS ); cluster_params_set_index != std::numeric_limits<uint32_t>::max() )
        {
            vkCmdBindDescriptorSets ( mVkCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE,
                                      pipeline->GetPipelineLayout(),
                                      cluster_params_set_index,
                                      1,
                                      &mClusterParamsDescriptorSet, 0, nullptr );
        }

        for ( const StorageBufferBinding& storage_buffer : aStorageBuffers )
        {
            if ( storage_buffer.mBuffer == nullptr )
            {
                continue;
            }
            uint32_t storage_set_index = pipeline->GetDescriptorSetIndex ( storage_buffer.mBinding );
            if ( storage_set_index == std::numeric_limits<uint32_t>::max() )
            {
                continue;
            }
            const VulkanStorageMemoryPoolBuffer* memory_pool_buffer =
                reinterpret_cast<const VulkanStorageMemoryPoolBuffer*> ( storage_buffer.mBuffer->GetMemoryPoolBuffer() );
            size_t offset = storage_buffer.mBuffer->GetOffset();
            // The allocation's descriptor already covers exactly [offset, offset+size];
            // bind it with a zero dynamic offset.
            uint32_t dynamic_offset = 0;
            vkCmdBindDescriptorSets ( mVkCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE,
                                      pipeline->GetPipelineLayout(),
                                      storage_set_index,
                                      1,
                                      &memory_pool_buffer->GetDescriptorSet ( offset ), 1, &dynamic_offset );
        }

        vkCmdDispatch ( mVkCommandBuffer, aGroupCountX, aGroupCountY, aGroupCountZ );
    }

    void VulkanWindow::Skin ( const Pipeline& aSkinningPipeline,
                              const Mesh& aMesh,
                              const BufferAccessor& aSkinningMatrices,
                              const BufferAccessor& aSkinnedVertices ) const
    {
        if ( aMesh.GetVertexCount() == 0 )
        {
            return;
        }
        const VulkanPipeline* pipeline = mVulkanRenderer.GetVulkanPipeline ( aSkinningPipeline );
        if ( pipeline == nullptr )
        {
            return;
        }
        vkCmdBindPipeline ( mVkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetVkComputePipeline ( 0 ) );

        // SkinningMatrices: per-joint pose*inverse-bind matrices (storage pool allocation).
        if ( uint32_t set_index = pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::SKINNING_MATRICES ); set_index != std::numeric_limits<uint32_t>::max() )
        {
            const VulkanStorageMemoryPoolBuffer* memory_pool_buffer =
                reinterpret_cast<const VulkanStorageMemoryPoolBuffer*> ( aSkinningMatrices.GetMemoryPoolBuffer() );
            uint32_t dynamic_offset = 0;
            vkCmdBindDescriptorSets ( mVkCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE,
                                      pipeline->GetPipelineLayout(),
                                      set_index,
                                      1,
                                      &memory_pool_buffer->GetDescriptorSet ( aSkinningMatrices.GetOffset() ), 1, &dynamic_offset );
        }

        // SourceVertices: the mesh's own rest-pose vertex buffer, exposed as an SSBO.
        if ( uint32_t set_index = pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::SOURCE_VERTICES ); set_index != std::numeric_limits<uint32_t>::max() )
        {
            const VulkanMesh* vulkan_mesh = mVulkanRenderer.GetVulkanMesh ( aMesh );
            if ( vulkan_mesh != nullptr )
            {
                VkDescriptorSet source_set = vulkan_mesh->GetSourceVerticesDescriptorSet();
                if ( source_set != VK_NULL_HANDLE )
                {
                    uint32_t dynamic_offset = 0;
                    vkCmdBindDescriptorSets ( mVkCommandBuffer,
                                              VK_PIPELINE_BIND_POINT_COMPUTE,
                                              pipeline->GetPipelineLayout(),
                                              set_index,
                                              1,
                                              &source_set, 1, &dynamic_offset );
                }
            }
        }

        // SkinnedVertices: output buffer receiving the skinned vertices (storage pool allocation).
        if ( uint32_t set_index = pipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::SKINNED_VERTICES ); set_index != std::numeric_limits<uint32_t>::max() )
        {
            const VulkanStorageMemoryPoolBuffer* memory_pool_buffer =
                reinterpret_cast<const VulkanStorageMemoryPoolBuffer*> ( aSkinnedVertices.GetMemoryPoolBuffer() );
            uint32_t dynamic_offset = 0;
            vkCmdBindDescriptorSets ( mVkCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE,
                                      pipeline->GetPipelineLayout(),
                                      set_index,
                                      1,
                                      &memory_pool_buffer->GetDescriptorSet ( aSkinnedVertices.GetOffset() ), 1, &dynamic_offset );
        }

        uint32_t group_count = ( aMesh.GetVertexCount() + 63u ) / 64u;
        vkCmdDispatch ( mVkCommandBuffer, group_count, 1, 1 );

        // The skinned vertices are consumed by the subsequent draw as a vertex
        // attribute buffer (vertex-input stage), not via a shader read, so the
        // generic shader-read Barrier() does not cover this hazard. Without this
        // barrier the draw can race the compute write and fetch stale/partial
        // data, manifesting as exploded vertices.
        VkMemoryBarrier vertex_barrier{};
        vertex_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        vertex_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        vertex_barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        vkCmdPipelineBarrier ( mVkCommandBuffer,
                               VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                               VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                               0,
                               1, &vertex_barrier,
                               0, nullptr,
                               0, nullptr );
    }

    void VulkanWindow::Barrier() const
    {
        VkMemoryBarrier memory_barrier{};
        memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memory_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        // Source covers both compute (clustering stages) and the graphics
        // stages of the depth pre-pass mark, whose fragment shader writes the
        // per-cluster active flags read by the following light-cull dispatch.
        vkCmdPipelineBarrier ( mVkCommandBuffer,
                               VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               0,
                               1, &memory_barrier,
                               0, nullptr,
                               0, nullptr );
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
        if ( mVkAcquireSemaphore != VK_NULL_HANDLE )
        {
            vkDestroySemaphore ( mVulkanRenderer.GetDevice(), mVkAcquireSemaphore, nullptr );
            std::cout << LogLevel::Debug << "Destroyed Acquire Semaphore " << std::hex << mVkAcquireSemaphore << std::endl;
            mVkAcquireSemaphore = VK_NULL_HANDLE;
        }
        for ( auto& i : mVkSubmitSemaphores )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroySemaphore ( mVulkanRenderer.GetDevice(), i, nullptr );
                std::cout << LogLevel::Debug << "Destroyed Submit Semaphore " << std::hex << i << std::endl;
            }
        }
        mVkSubmitSemaphores.clear();
        if ( mVkFence != VK_NULL_HANDLE )
        {
            vkDestroyFence ( mVulkanRenderer.GetDevice(), mVkFence, nullptr );
            mVkFence = VK_NULL_HANDLE;
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

    BufferAccessor VulkanWindow::AllocateSingleFrameStorageMemory ( size_t aSize )
    {
        return mStorageMemoryPoolBuffer.Allocate ( aSize );
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
