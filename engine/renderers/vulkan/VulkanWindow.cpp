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
#include <initializer_list>
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
#include "aeongames/GpuShadowParams.hpp"

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
        mClusterParams { aVulkanRenderer },
        mGlobals { aVulkanRenderer },
        mShadowParams { aVulkanRenderer },
        mSpotShadowParams { aVulkanRenderer },
        mSpotShadowDepthMatrices { aVulkanRenderer },
        mPointShadowParams { aVulkanRenderer },
        mPointShadowDepthMatrices { aVulkanRenderer }
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
        mClusterParams{std::move ( aVulkanWindow.mClusterParams ) },
        mGlobals{std::move ( aVulkanWindow.mGlobals ) },
        mShadowParams{std::move ( aVulkanWindow.mShadowParams ) },
        mSpotShadowParams{std::move ( aVulkanWindow.mSpotShadowParams ) },
        mSpotShadowDepthMatrices{std::move ( aVulkanWindow.mSpotShadowDepthMatrices ) },
        mPointShadowParams{std::move ( aVulkanWindow.mPointShadowParams ) },
        mPointShadowDepthMatrices{std::move ( aVulkanWindow.mPointShadowDepthMatrices ) }
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
        std::swap ( mVkHdrColorImage, aVulkanWindow.mVkHdrColorImage );
        std::swap ( mVkHdrColorImageMemory, aVulkanWindow.mVkHdrColorImageMemory );
        std::swap ( mVkHdrColorImageView, aVulkanWindow.mVkHdrColorImageView );
        std::swap ( mVkHdrFramebuffer, aVulkanWindow.mVkHdrFramebuffer );
        std::swap ( mVkHdrSampler, aVulkanWindow.mVkHdrSampler );
        std::swap ( mVkTonemapRenderPass, aVulkanWindow.mVkTonemapRenderPass );
        std::swap ( mTonemapDescriptorPool, aVulkanWindow.mTonemapDescriptorPool );
        std::swap ( mTonemapDescriptorSet, aVulkanWindow.mTonemapDescriptorSet );
        std::swap ( mMatricesDescriptorPool, aVulkanWindow.mMatricesDescriptorPool );
        std::swap ( mMatricesDescriptorSet, aVulkanWindow.mMatricesDescriptorSet );
        std::swap ( mLightsDescriptorPool, aVulkanWindow.mLightsDescriptorPool );
        std::swap ( mLightsDescriptorSet, aVulkanWindow.mLightsDescriptorSet );
        std::swap ( mClusterParamsDescriptorPool, aVulkanWindow.mClusterParamsDescriptorPool );
        std::swap ( mClusterParamsDescriptorSet, aVulkanWindow.mClusterParamsDescriptorSet );
        std::swap ( mGlobalsDescriptorPool, aVulkanWindow.mGlobalsDescriptorPool );
        std::swap ( mGlobalsDescriptorSet, aVulkanWindow.mGlobalsDescriptorSet );
        std::swap ( mVkShadowDepthImage, aVulkanWindow.mVkShadowDepthImage );
        std::swap ( mVkShadowDepthImageMemory, aVulkanWindow.mVkShadowDepthImageMemory );
        std::swap ( mVkShadowDepthImageView, aVulkanWindow.mVkShadowDepthImageView );
        std::swap ( mVkShadowColorImage, aVulkanWindow.mVkShadowColorImage );
        std::swap ( mVkShadowColorImageMemory, aVulkanWindow.mVkShadowColorImageMemory );
        std::swap ( mVkShadowColorImageView, aVulkanWindow.mVkShadowColorImageView );
        std::swap ( mVkShadowSampler, aVulkanWindow.mVkShadowSampler );
        std::swap ( mVkShadowRenderPass, aVulkanWindow.mVkShadowRenderPass );
        std::swap ( mVkShadowFramebuffer, aVulkanWindow.mVkShadowFramebuffer );
        std::swap ( mShadowParamsDescriptorPool, aVulkanWindow.mShadowParamsDescriptorPool );
        std::swap ( mShadowParamsDescriptorSet, aVulkanWindow.mShadowParamsDescriptorSet );
        std::swap ( mShadowMapDescriptorPool, aVulkanWindow.mShadowMapDescriptorPool );
        std::swap ( mShadowMapDescriptorSet, aVulkanWindow.mShadowMapDescriptorSet );
        std::swap ( mVkSpotShadowDepthImage, aVulkanWindow.mVkSpotShadowDepthImage );
        std::swap ( mVkSpotShadowDepthImageMemory, aVulkanWindow.mVkSpotShadowDepthImageMemory );
        std::swap ( mVkSpotShadowDepthArrayView, aVulkanWindow.mVkSpotShadowDepthArrayView );
        std::swap ( mVkSpotShadowDepthLayerViews, aVulkanWindow.mVkSpotShadowDepthLayerViews );
        std::swap ( mVkSpotShadowColorImage, aVulkanWindow.mVkSpotShadowColorImage );
        std::swap ( mVkSpotShadowColorImageMemory, aVulkanWindow.mVkSpotShadowColorImageMemory );
        std::swap ( mVkSpotShadowColorImageView, aVulkanWindow.mVkSpotShadowColorImageView );
        std::swap ( mVkSpotShadowFramebuffers, aVulkanWindow.mVkSpotShadowFramebuffers );
        std::swap ( mSpotShadowParamsDescriptorPool, aVulkanWindow.mSpotShadowParamsDescriptorPool );
        std::swap ( mSpotShadowParamsDescriptorSet, aVulkanWindow.mSpotShadowParamsDescriptorSet );
        std::swap ( mSpotShadowMapDescriptorPool, aVulkanWindow.mSpotShadowMapDescriptorPool );
        std::swap ( mSpotShadowMapDescriptorSet, aVulkanWindow.mSpotShadowMapDescriptorSet );
        std::swap ( mSpotShadowDepthMatricesDescriptorPool, aVulkanWindow.mSpotShadowDepthMatricesDescriptorPool );
        std::swap ( mSpotShadowDepthMatricesDescriptorSets, aVulkanWindow.mSpotShadowDepthMatricesDescriptorSets );
        std::swap ( mSpotShadowDepthMatrixStride, aVulkanWindow.mSpotShadowDepthMatrixStride );
        std::swap ( mVkPointShadowDepthImage, aVulkanWindow.mVkPointShadowDepthImage );
        std::swap ( mVkPointShadowDepthImageMemory, aVulkanWindow.mVkPointShadowDepthImageMemory );
        std::swap ( mVkPointShadowDepthArrayView, aVulkanWindow.mVkPointShadowDepthArrayView );
        std::swap ( mVkPointShadowDepthCasterViews, aVulkanWindow.mVkPointShadowDepthCasterViews );
        std::swap ( mVkPointShadowColorImage, aVulkanWindow.mVkPointShadowColorImage );
        std::swap ( mVkPointShadowColorImageMemory, aVulkanWindow.mVkPointShadowColorImageMemory );
        std::swap ( mVkPointShadowColorImageView, aVulkanWindow.mVkPointShadowColorImageView );
        std::swap ( mVkPointShadowRenderPass, aVulkanWindow.mVkPointShadowRenderPass );
        std::swap ( mVkPointShadowFramebuffers, aVulkanWindow.mVkPointShadowFramebuffers );
        std::swap ( mPointShadowParamsDescriptorPool, aVulkanWindow.mPointShadowParamsDescriptorPool );
        std::swap ( mPointShadowParamsDescriptorSet, aVulkanWindow.mPointShadowParamsDescriptorSet );
        std::swap ( mPointShadowMapDescriptorPool, aVulkanWindow.mPointShadowMapDescriptorPool );
        std::swap ( mPointShadowMapDescriptorSet, aVulkanWindow.mPointShadowMapDescriptorSet );
        std::swap ( mPointShadowDepthMatricesDescriptorPool, aVulkanWindow.mPointShadowDepthMatricesDescriptorPool );
        std::swap ( mPointShadowDepthMatricesDescriptorSets, aVulkanWindow.mPointShadowDepthMatricesDescriptorSets );
        std::swap ( mPointShadowDepthMatrixStride, aVulkanWindow.mPointShadowDepthMatrixStride );
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

        // Seed the viewport and scissor from the surface extent so the very
        // first frame has a valid (non-zero) render area even before the
        // platform delivers its initial resize. On Win32 WM_SIZE arrives before
        // the first paint, but on X11 the render loop can draw a frame before
        // the asynchronous Expose event calls ResizeViewport; without this the
        // first frame's mVkScissor is 0x0 (VUID-VkRenderPassBeginInfo-None-08996).
        // ResizeViewport overwrites these with the real values when it runs.
        mVkViewport.x = 0.0f;
        mVkViewport.y = 0.0f;
        mVkViewport.width = static_cast<float> ( mVkSurfaceCapabilitiesKHR.currentExtent.width );
        mVkViewport.height = static_cast<float> ( mVkSurfaceCapabilitiesKHR.currentExtent.height );
        mVkViewport.minDepth = 0.0f;
        mVkViewport.maxDepth = 1.0f;
        mVkScissor.offset = { 0, 0 };
        mVkScissor.extent = mVkSurfaceCapabilitiesKHR.currentExtent;

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
        const VkDevice device = mVulkanRenderer.GetDevice();
        const uint32_t width = mVkSurfaceCapabilitiesKHR.currentExtent.width;
        const uint32_t height = mVkSurfaceCapabilitiesKHR.currentExtent.height;

        // HDR offscreen colour image the scene renders linear radiance into,
        // sampled by the fullscreen tonemap pass. Extent-dependent, so it is
        // (re)created here alongside the framebuffers.
        VkImageCreateInfo hdr_image_create_info{};
        hdr_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        hdr_image_create_info.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        hdr_image_create_info.imageType = VK_IMAGE_TYPE_2D;
        hdr_image_create_info.extent = { width, height, 1 };
        hdr_image_create_info.mipLevels = 1;
        hdr_image_create_info.arrayLayers = 1;
        hdr_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        hdr_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        hdr_image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        hdr_image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        hdr_image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkCreateImage ( device, &hdr_image_create_info, nullptr, &mVkHdrColorImage );
        VkMemoryRequirements hdr_memory_requirements;
        vkGetImageMemoryRequirements ( device, mVkHdrColorImage, &hdr_memory_requirements );
        VkMemoryAllocateInfo hdr_memory_allocate_info{};
        hdr_memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        hdr_memory_allocate_info.allocationSize = hdr_memory_requirements.size;
        hdr_memory_allocate_info.memoryTypeIndex = mVulkanRenderer.FindMemoryTypeIndex ( hdr_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        vkAllocateMemory ( device, &hdr_memory_allocate_info, nullptr, &mVkHdrColorImageMemory );
        vkBindImageMemory ( device, mVkHdrColorImage, mVkHdrColorImageMemory, 0 );

        VkImageViewCreateInfo hdr_view_create_info{};
        hdr_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        hdr_view_create_info.image = mVkHdrColorImage;
        hdr_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        hdr_view_create_info.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        hdr_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        hdr_view_create_info.subresourceRange.levelCount = 1;
        hdr_view_create_info.subresourceRange.layerCount = 1;
        vkCreateImageView ( device, &hdr_view_create_info, nullptr, &mVkHdrColorImageView );

        // Scene framebuffer: HDR colour + the shared depth-stencil, drawn with
        // the main render pass.
        std::array<VkImageView, 2> hdr_attachments { { mVkHdrColorImageView, mVkDepthStencilImageView } };
        VkFramebufferCreateInfo hdr_framebuffer_create_info{};
        hdr_framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        hdr_framebuffer_create_info.renderPass = mVkRenderPass;
        hdr_framebuffer_create_info.attachmentCount = static_cast<uint32_t> ( hdr_attachments.size() );
        hdr_framebuffer_create_info.pAttachments = hdr_attachments.data();
        hdr_framebuffer_create_info.width = width;
        hdr_framebuffer_create_info.height = height;
        hdr_framebuffer_create_info.layers = 1;
        vkCreateFramebuffer ( device, &hdr_framebuffer_create_info, nullptr, &mVkHdrFramebuffer );

        // Tonemap framebuffers: one per swapchain image (colour only), drawn
        // with the tonemap render pass.
        mVkFramebuffers.resize ( mSwapchainImageCount );
        for ( uint32_t i = 0; i < mSwapchainImageCount; ++i )
        {
            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = mVkTonemapRenderPass;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments = &mVkSwapchainImageViews[i];
            framebuffer_create_info.width = width;
            framebuffer_create_info.height = height;
            framebuffer_create_info.layers = 1;
            vkCreateFramebuffer ( device, &framebuffer_create_info, nullptr, &mVkFramebuffers[i] );
        }

        // Tonemap descriptor set: the HDR colour image sampled by tonemap.frag
        // (set 0, binding 0). Mirrors the shadow-map combined image sampler; the
        // VK_SHADER_STAGE_ALL layout stays compatible with the reflected pipeline
        // layout. Recreated on resize because it points at the HDR image view.
        VkDescriptorSetLayoutCreateInfo tonemap_layout_create_info{};
        tonemap_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        tonemap_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding tonemap_layout_binding{};
        tonemap_layout_binding.binding = 0;
        tonemap_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        tonemap_layout_binding.descriptorCount = 1;
        tonemap_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        tonemap_layout_create_info.pBindings = &tonemap_layout_binding;
        mTonemapDescriptorPool = CreateDescriptorPool ( device, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}} );
        mTonemapDescriptorSet = CreateDescriptorSet ( device, mTonemapDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( tonemap_layout_create_info ) );
        VkDescriptorImageInfo tonemap_image_info{};
        tonemap_image_info.sampler = mVkHdrSampler;
        tonemap_image_info.imageView = mVkHdrColorImageView;
        tonemap_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet tonemap_write{};
        tonemap_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        tonemap_write.dstSet = mTonemapDescriptorSet;
        tonemap_write.dstBinding = 0;
        tonemap_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        tonemap_write.descriptorCount = 1;
        tonemap_write.pImageInfo = &tonemap_image_info;
        vkUpdateDescriptorSets ( device, 1, &tonemap_write, 0, nullptr );
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
            // Prefer a UNORM (non-sRGB) surface so the fragment shader's own sRGB
            // encode is the only gamma applied, keeping brightness consistent with
            // the OpenGL path (which does not enable GL_FRAMEBUFFER_SRGB). Fall
            // back to the driver's first advertised format.
            mVkSurfaceFormatKHR = surface_format_list[0];
            for ( const VkSurfaceFormatKHR& candidate : surface_format_list )
            {
                if ( ( candidate.format == VK_FORMAT_B8G8R8A8_UNORM ) ||
                     ( candidate.format == VK_FORMAT_R8G8B8A8_UNORM ) )
                {
                    mVkSurfaceFormatKHR = candidate;
                    break;
                }
            }
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
        // The scene renders linear HDR radiance here; a fullscreen tonemap pass
        // resolves it to the swapchain, so the colour target is an RGBA16F image
        // left in SHADER_READ_ONLY for that pass (not the swapchain any more).
        attachment_descriptions[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
        attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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

        std::array<VkSubpassDependency, 2> subpass_dependencies{};
        subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependencies[0].dstSubpass = 0;
        subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependencies[0].srcAccessMask = 0;
        subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        // Make the HDR colour writes available/visible to the tonemap pass's
        // fragment-shader sample of this image.
        subpass_dependencies[1].srcSubpass = 0;
        subpass_dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        subpass_dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        VkRenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = static_cast<uint32_t> ( attachment_descriptions.size() );
        render_pass_create_info.pAttachments = attachment_descriptions.data();
        render_pass_create_info.dependencyCount = static_cast<uint32_t> ( subpass_dependencies.size() );
        render_pass_create_info.pDependencies = subpass_dependencies.data();
        render_pass_create_info.subpassCount = static_cast<uint32_t> ( subpass_descriptions.size() );
        render_pass_create_info.pSubpasses = subpass_descriptions.data();
        vkCreateRenderPass ( mVulkanRenderer.GetDevice(), &render_pass_create_info, nullptr, &mVkRenderPass );

        // --- Tonemap resolve resources (extent-independent) ---------------------
        // Linear sampler for the HDR colour image (clamp; no depth comparison).
        VkSamplerCreateInfo hdr_sampler_create_info{};
        hdr_sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        hdr_sampler_create_info.magFilter = VK_FILTER_LINEAR;
        hdr_sampler_create_info.minFilter = VK_FILTER_LINEAR;
        hdr_sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        hdr_sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        hdr_sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        hdr_sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        hdr_sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        hdr_sampler_create_info.maxLod = 1.0f;
        vkCreateSampler ( mVulkanRenderer.GetDevice(), &hdr_sampler_create_info, nullptr, &mVkHdrSampler );

        // Tonemap render pass: one swapchain colour attachment, no depth. The
        // fullscreen pass overwrites every pixel (loadOp DONT_CARE) and leaves
        // the result ready to present.
        VkAttachmentDescription tonemap_attachment{};
        tonemap_attachment.format = mVkSurfaceFormatKHR.format;
        tonemap_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        tonemap_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        tonemap_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        tonemap_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        tonemap_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        tonemap_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        tonemap_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference tonemap_color_reference{};
        tonemap_color_reference.attachment = 0;
        tonemap_color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription tonemap_subpass{};
        tonemap_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        tonemap_subpass.colorAttachmentCount = 1;
        tonemap_subpass.pColorAttachments = &tonemap_color_reference;
        VkSubpassDependency tonemap_dependency{};
        tonemap_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        tonemap_dependency.dstSubpass = 0;
        tonemap_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        tonemap_dependency.srcAccessMask = 0;
        tonemap_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        tonemap_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkRenderPassCreateInfo tonemap_render_pass_create_info{};
        tonemap_render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        tonemap_render_pass_create_info.attachmentCount = 1;
        tonemap_render_pass_create_info.pAttachments = &tonemap_attachment;
        tonemap_render_pass_create_info.subpassCount = 1;
        tonemap_render_pass_create_info.pSubpasses = &tonemap_subpass;
        tonemap_render_pass_create_info.dependencyCount = 1;
        tonemap_render_pass_create_info.pDependencies = &tonemap_dependency;
        vkCreateRenderPass ( mVulkanRenderer.GetDevice(), &tonemap_render_pass_create_info, nullptr, &mVkTonemapRenderPass );
    }

    void VulkanWindow::FinalizeRenderPass()
    {
        if ( mVkRenderPass != VK_NULL_HANDLE )
        {
            vkDestroyRenderPass ( mVulkanRenderer.GetDevice(), mVkRenderPass, nullptr );
            mVkRenderPass = VK_NULL_HANDLE;
        }
        if ( mVkTonemapRenderPass != VK_NULL_HANDLE )
        {
            vkDestroyRenderPass ( mVulkanRenderer.GetDevice(), mVkTonemapRenderPass, nullptr );
            mVkTonemapRenderPass = VK_NULL_HANDLE;
        }
        if ( mVkHdrSampler != VK_NULL_HANDLE )
        {
            vkDestroySampler ( mVulkanRenderer.GetDevice(), mVkHdrSampler, nullptr );
            mVkHdrSampler = VK_NULL_HANDLE;
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

    void VulkanWindow::InitializeGlobals()
    {
        GpuGlobals empty{};
        mGlobals.Initialize (
            sizeof ( GpuGlobals ),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &empty );

        VkDescriptorSetLayoutCreateInfo globals_descriptor_set_layout_create_info{};
        globals_descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        globals_descriptor_set_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding globals_descriptor_set_layout_binding{};
        globals_descriptor_set_layout_binding.binding = 0;
        globals_descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        globals_descriptor_set_layout_binding.descriptorCount = 1;
        globals_descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        globals_descriptor_set_layout_binding.pImmutableSamplers = nullptr;
        globals_descriptor_set_layout_create_info.pBindings = &globals_descriptor_set_layout_binding;

        mGlobalsDescriptorPool = CreateDescriptorPool ( mVulkanRenderer.GetDevice(), {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}} );
        mGlobalsDescriptorSet = CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mGlobalsDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( globals_descriptor_set_layout_create_info ) );
        VkDescriptorBufferInfo descriptor_buffer_info{};
        descriptor_buffer_info.buffer = mGlobals.GetBuffer();
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range = mGlobals.GetSize();
        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.pNext = nullptr;
        write_descriptor_set.dstSet = mGlobalsDescriptorSet;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
        write_descriptor_set.pImageInfo = nullptr;
        write_descriptor_set.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), 1, &write_descriptor_set, 0, nullptr );
    }

    void VulkanWindow::FinalizeGlobals()
    {
        DestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mGlobalsDescriptorPool );
        mGlobals.Finalize();
    }

    void VulkanWindow::SetGlobals ( const GpuGlobals& aGlobals )
    {
        mGlobals.WriteMemory ( 0, sizeof ( GpuGlobals ), &aGlobals );
    }

    void VulkanWindow::InitializeShadowMap()
    {
        const VkDevice device = mVulkanRenderer.GetDevice();
        const bool has_stencil =
            ( mVkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT );

        // Local helper: create a device-local 2D image and bind fresh memory.
        auto create_image = [&] ( VkFormat format, VkImageUsageFlags usage,
                                  VkImage & image, VkDeviceMemory & memory )
        {
            VkImageCreateInfo image_create_info{};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.format = format;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.extent.width = SHADOW_MAP_RESOLUTION;
            image_create_info.extent.height = SHADOW_MAP_RESOLUTION;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.usage = usage;
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if ( VkResult result = vkCreateImage ( device, &image_create_info, nullptr, &image ) )
            {
                std::ostringstream stream;
                stream << "Shadow vkCreateImage failed: ( " << GetVulkanResultString ( result ) << " )";
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }
            VkMemoryRequirements memory_requirements;
            vkGetImageMemoryRequirements ( device, image, &memory_requirements );
            VkMemoryAllocateInfo memory_allocate_info{};
            memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memory_allocate_info.allocationSize = memory_requirements.size;
            memory_allocate_info.memoryTypeIndex =
                mVulkanRenderer.FindMemoryTypeIndex ( memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
            vkAllocateMemory ( device, &memory_allocate_info, nullptr, &memory );
            vkBindImageMemory ( device, image, memory, 0 );
        };

        // Sampleable depth target the shadow pass writes; uses the same depth
        // format as the main pass so the shadow render pass stays
        // attachment-compatible with pipelines created against the main pass.
        create_image ( mVkDepthStencilFormat,
                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                       mVkShadowDepthImage, mVkShadowDepthImageMemory );
        // Throwaway color attachment, never sampled, carried only for
        // render-pass compatibility with the main color+depth render pass.
        create_image ( mVkSurfaceFormatKHR.format,
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                       mVkShadowColorImage, mVkShadowColorImageMemory );

        // Depth view (depth aspect only so it can be sampled as sampler2DShadow).
        VkImageViewCreateInfo depth_view_create_info{};
        depth_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depth_view_create_info.image = mVkShadowDepthImage;
        depth_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depth_view_create_info.format = mVkDepthStencilFormat;
        depth_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depth_view_create_info.subresourceRange.levelCount = 1;
        depth_view_create_info.subresourceRange.layerCount = 1;
        vkCreateImageView ( device, &depth_view_create_info, nullptr, &mVkShadowDepthImageView );

        VkImageViewCreateInfo color_view_create_info{};
        color_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        color_view_create_info.image = mVkShadowColorImage;
        color_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        color_view_create_info.format = mVkSurfaceFormatKHR.format;
        color_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_view_create_info.subresourceRange.levelCount = 1;
        color_view_create_info.subresourceRange.layerCount = 1;
        vkCreateImageView ( device, &color_view_create_info, nullptr, &mVkShadowColorImageView );

        // Comparison sampler: hardware PCF, depth <= stored => lit; sampling
        // outside the map returns the white border (1.0) so it reads as lit.
        VkSamplerCreateInfo sampler_create_info{};
        sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.magFilter = VK_FILTER_LINEAR;
        sampler_create_info.minFilter = VK_FILTER_LINEAR;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        sampler_create_info.compareEnable = VK_TRUE;
        sampler_create_info.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        sampler_create_info.maxLod = 1.0f;
        vkCreateSampler ( device, &sampler_create_info, nullptr, &mVkShadowSampler );

        // Shadow render pass. Attachment 0 (color) is throwaway; attachment 1
        // (depth) is cleared, stored, and left in SHADER_READ_ONLY for sampling.
        std::array<VkAttachmentDescription, 2> attachment_descriptions{ {} };
        attachment_descriptions[0].format = mVkSurfaceFormatKHR.format;
        attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment_descriptions[1].format = mVkDepthStencilFormat;
        attachment_descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference color_attachment_reference{};
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depth_attachment_reference{};
        depth_attachment_reference.attachment = 1;
        depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.colorAttachmentCount = 1;
        subpass_description.pColorAttachments = &color_attachment_reference;
        subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

        // Render-pass compatibility (Vulkan spec, Render Pass Compatibility)
        // requires identical subpass dependencies, because the shadow depth
        // pipeline is created against the window's main render pass yet executed
        // here. Mirror the main pass's single dependency exactly; the depth
        // write -> shader read hazard is instead handled by an explicit barrier
        // in EndShadowPass.
        VkSubpassDependency subpass_dependency{};
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
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass_description;
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &subpass_dependency;
        vkCreateRenderPass ( device, &render_pass_create_info, nullptr, &mVkShadowRenderPass );

        std::array<VkImageView, 2> framebuffer_attachments{ { mVkShadowColorImageView, mVkShadowDepthImageView } };
        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = mVkShadowRenderPass;
        framebuffer_create_info.attachmentCount = static_cast<uint32_t> ( framebuffer_attachments.size() );
        framebuffer_create_info.pAttachments = framebuffer_attachments.data();
        framebuffer_create_info.width = SHADOW_MAP_RESOLUTION;
        framebuffer_create_info.height = SHADOW_MAP_RESOLUTION;
        framebuffer_create_info.layers = 1;
        vkCreateFramebuffer ( device, &framebuffer_create_info, nullptr, &mVkShadowFramebuffer );

        // ShadowParams UBO (light view-projection + bias/PCF/enable), shared by
        // the shadow depth vertex shader (set 8) and the shading fragment shader.
        const GpuShadowParams empty_shadow_params{};
        mShadowParams.Initialize (
            sizeof ( GpuShadowParams ),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &empty_shadow_params );

        VkDescriptorSetLayoutCreateInfo shadow_params_layout_create_info{};
        shadow_params_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        shadow_params_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding shadow_params_layout_binding{};
        shadow_params_layout_binding.binding = 0;
        shadow_params_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        shadow_params_layout_binding.descriptorCount = 1;
        shadow_params_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        shadow_params_layout_create_info.pBindings = &shadow_params_layout_binding;

        mShadowParamsDescriptorPool = CreateDescriptorPool ( device, {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}} );
        mShadowParamsDescriptorSet = CreateDescriptorSet ( device, mShadowParamsDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( shadow_params_layout_create_info ) );
        VkDescriptorBufferInfo shadow_params_buffer_info{};
        shadow_params_buffer_info.buffer = mShadowParams.GetBuffer();
        shadow_params_buffer_info.offset = 0;
        shadow_params_buffer_info.range = mShadowParams.GetSize();
        VkWriteDescriptorSet shadow_params_write{};
        shadow_params_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        shadow_params_write.dstSet = mShadowParamsDescriptorSet;
        shadow_params_write.dstBinding = 0;
        shadow_params_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        shadow_params_write.descriptorCount = 1;
        shadow_params_write.pBufferInfo = &shadow_params_buffer_info;
        vkUpdateDescriptorSets ( device, 1, &shadow_params_write, 0, nullptr );

        // ShadowMap combined image sampler (set 9), sampled by the shading pass.
        VkDescriptorSetLayoutCreateInfo shadow_map_layout_create_info{};
        shadow_map_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        shadow_map_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding shadow_map_layout_binding{};
        shadow_map_layout_binding.binding = 0;
        shadow_map_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        shadow_map_layout_binding.descriptorCount = 1;
        shadow_map_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        shadow_map_layout_create_info.pBindings = &shadow_map_layout_binding;

        mShadowMapDescriptorPool = CreateDescriptorPool ( device, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}} );
        mShadowMapDescriptorSet = CreateDescriptorSet ( device, mShadowMapDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( shadow_map_layout_create_info ) );
        VkDescriptorImageInfo shadow_map_image_info{};
        shadow_map_image_info.sampler = mVkShadowSampler;
        shadow_map_image_info.imageView = mVkShadowDepthImageView;
        shadow_map_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet shadow_map_write{};
        shadow_map_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        shadow_map_write.dstSet = mShadowMapDescriptorSet;
        shadow_map_write.dstBinding = 0;
        shadow_map_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        shadow_map_write.descriptorCount = 1;
        shadow_map_write.pImageInfo = &shadow_map_image_info;
        vkUpdateDescriptorSets ( device, 1, &shadow_map_write, 0, nullptr );

        // Transition the shadow depth image to the sampled layout once, so a
        // frame that draws no caster (ShadowParams.enabled == 0, shader skips
        // the sample) still presents a descriptor in a valid layout.
        VkCommandBuffer command_buffer = mVulkanRenderer.BeginSingleTimeCommands();
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = mVkShadowDepthImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | ( has_stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier ( command_buffer,
                               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &barrier );
        mVulkanRenderer.EndSingleTimeCommands ( command_buffer );
    }

    void VulkanWindow::InitializeSpotShadowMap()
    {
        const VkDevice device = mVulkanRenderer.GetDevice();
        const bool has_stencil =
            ( mVkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT );

        // Local helper: create a device-local image (optionally a 2D array) and
        // bind fresh memory.
        auto create_image = [&] ( VkFormat format, VkImageUsageFlags usage,
                                  uint32_t array_layers, VkImage & image, VkDeviceMemory & memory )
        {
            VkImageCreateInfo image_create_info{};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.format = format;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.extent.width = SPOT_SHADOW_MAP_RESOLUTION;
            image_create_info.extent.height = SPOT_SHADOW_MAP_RESOLUTION;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = array_layers;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.usage = usage;
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if ( VkResult result = vkCreateImage ( device, &image_create_info, nullptr, &image ) )
            {
                std::ostringstream stream;
                stream << "Spot shadow vkCreateImage failed: ( " << GetVulkanResultString ( result ) << " )";
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }
            VkMemoryRequirements memory_requirements;
            vkGetImageMemoryRequirements ( device, image, &memory_requirements );
            VkMemoryAllocateInfo memory_allocate_info{};
            memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memory_allocate_info.allocationSize = memory_requirements.size;
            memory_allocate_info.memoryTypeIndex =
                mVulkanRenderer.FindMemoryTypeIndex ( memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
            vkAllocateMemory ( device, &memory_allocate_info, nullptr, &memory );
            vkBindImageMemory ( device, image, memory, 0 );
        };

        // Sampleable depth array: one layer per caster, written by the spot
        // depth passes and sampled together as a sampler2DArrayShadow.
        create_image ( mVkDepthStencilFormat,
                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                       MAX_SPOT_SHADOW_CASTERS, mVkSpotShadowDepthImage, mVkSpotShadowDepthImageMemory );
        // One throwaway color image, shared by every layer's framebuffer, only
        // for render-pass compatibility with the shared shadow render pass.
        create_image ( mVkSurfaceFormatKHR.format,
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                       1, mVkSpotShadowColorImage, mVkSpotShadowColorImageMemory );

        // 2D-array view (all layers) for sampling.
        VkImageViewCreateInfo array_view_create_info{};
        array_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        array_view_create_info.image = mVkSpotShadowDepthImage;
        array_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        array_view_create_info.format = mVkDepthStencilFormat;
        array_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        array_view_create_info.subresourceRange.levelCount = 1;
        array_view_create_info.subresourceRange.layerCount = MAX_SPOT_SHADOW_CASTERS;
        vkCreateImageView ( device, &array_view_create_info, nullptr, &mVkSpotShadowDepthArrayView );

        VkImageViewCreateInfo color_view_create_info{};
        color_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        color_view_create_info.image = mVkSpotShadowColorImage;
        color_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        color_view_create_info.format = mVkSurfaceFormatKHR.format;
        color_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_view_create_info.subresourceRange.levelCount = 1;
        color_view_create_info.subresourceRange.layerCount = 1;
        vkCreateImageView ( device, &color_view_create_info, nullptr, &mVkSpotShadowColorImageView );

        // Per-layer single-layer depth views + framebuffers (reusing the shared
        // shadow render pass, which is compatible: same color/depth formats).
        for ( uint32_t slot = 0; slot < MAX_SPOT_SHADOW_CASTERS; ++slot )
        {
            VkImageViewCreateInfo layer_view_create_info{};
            layer_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            layer_view_create_info.image = mVkSpotShadowDepthImage;
            layer_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            layer_view_create_info.format = mVkDepthStencilFormat;
            layer_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            layer_view_create_info.subresourceRange.levelCount = 1;
            layer_view_create_info.subresourceRange.baseArrayLayer = slot;
            layer_view_create_info.subresourceRange.layerCount = 1;
            vkCreateImageView ( device, &layer_view_create_info, nullptr, &mVkSpotShadowDepthLayerViews[slot] );

            std::array<VkImageView, 2> framebuffer_attachments{ { mVkSpotShadowColorImageView, mVkSpotShadowDepthLayerViews[slot] } };
            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = mVkShadowRenderPass;
            framebuffer_create_info.attachmentCount = static_cast<uint32_t> ( framebuffer_attachments.size() );
            framebuffer_create_info.pAttachments = framebuffer_attachments.data();
            framebuffer_create_info.width = SPOT_SHADOW_MAP_RESOLUTION;
            framebuffer_create_info.height = SPOT_SHADOW_MAP_RESOLUTION;
            framebuffer_create_info.layers = 1;
            vkCreateFramebuffer ( device, &framebuffer_create_info, nullptr, &mVkSpotShadowFramebuffers[slot] );
        }

        // Spot ShadowParams UBO (all caster matrices + positions), sampled by
        // the shading fragment shader (set 10).
        const GpuSpotShadowParams empty_spot_params{};
        mSpotShadowParams.Initialize (
            sizeof ( GpuSpotShadowParams ),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &empty_spot_params );

        VkDescriptorSetLayoutCreateInfo spot_params_layout_create_info{};
        spot_params_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        spot_params_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding spot_params_layout_binding{};
        spot_params_layout_binding.binding = 0;
        spot_params_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        spot_params_layout_binding.descriptorCount = 1;
        spot_params_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        spot_params_layout_create_info.pBindings = &spot_params_layout_binding;

        mSpotShadowParamsDescriptorPool = CreateDescriptorPool ( device, {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}} );
        mSpotShadowParamsDescriptorSet = CreateDescriptorSet ( device, mSpotShadowParamsDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( spot_params_layout_create_info ) );
        VkDescriptorBufferInfo spot_params_buffer_info{};
        spot_params_buffer_info.buffer = mSpotShadowParams.GetBuffer();
        spot_params_buffer_info.offset = 0;
        spot_params_buffer_info.range = mSpotShadowParams.GetSize();
        VkWriteDescriptorSet spot_params_write{};
        spot_params_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        spot_params_write.dstSet = mSpotShadowParamsDescriptorSet;
        spot_params_write.dstBinding = 0;
        spot_params_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        spot_params_write.descriptorCount = 1;
        spot_params_write.pBufferInfo = &spot_params_buffer_info;
        vkUpdateDescriptorSets ( device, 1, &spot_params_write, 0, nullptr );

        // SpotShadowMap combined image sampler (set 11): the depth array sampled
        // with the same comparison sampler the directional map uses.
        VkDescriptorSetLayoutCreateInfo spot_map_layout_create_info{};
        spot_map_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        spot_map_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding spot_map_layout_binding{};
        spot_map_layout_binding.binding = 0;
        spot_map_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        spot_map_layout_binding.descriptorCount = 1;
        spot_map_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        spot_map_layout_create_info.pBindings = &spot_map_layout_binding;

        mSpotShadowMapDescriptorPool = CreateDescriptorPool ( device, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}} );
        mSpotShadowMapDescriptorSet = CreateDescriptorSet ( device, mSpotShadowMapDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( spot_map_layout_create_info ) );
        VkDescriptorImageInfo spot_map_image_info{};
        spot_map_image_info.sampler = mVkShadowSampler;
        spot_map_image_info.imageView = mVkSpotShadowDepthArrayView;
        spot_map_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet spot_map_write{};
        spot_map_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        spot_map_write.dstSet = mSpotShadowMapDescriptorSet;
        spot_map_write.dstBinding = 0;
        spot_map_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        spot_map_write.descriptorCount = 1;
        spot_map_write.pImageInfo = &spot_map_image_info;
        vkUpdateDescriptorSets ( device, 1, &spot_map_write, 0, nullptr );

        // Per-slot depth matrices: one UBO holding MAX_SPOT_SHADOW_CASTERS
        // aligned GpuShadowParams slots, with one descriptor set per slot bound
        // at its region. Each spot depth pass binds its slot's set at the depth
        // pipeline's ShadowParams slot, so every pass reads its own matrix from
        // a distinct buffer region (no single-buffer write hazard).
        const VkDeviceSize alignment =
            mVulkanRenderer.GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
        mSpotShadowDepthMatrixStride =
            ( ( sizeof ( GpuShadowParams ) - 1 ) | ( ( alignment > 0 ? alignment : 1 ) - 1 ) ) + 1;
        mSpotShadowDepthMatrices.Initialize (
            mSpotShadowDepthMatrixStride * MAX_SPOT_SHADOW_CASTERS,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            nullptr );

        VkDescriptorSetLayoutCreateInfo depth_matrix_layout_create_info{};
        depth_matrix_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        depth_matrix_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding depth_matrix_layout_binding{};
        depth_matrix_layout_binding.binding = 0;
        depth_matrix_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        depth_matrix_layout_binding.descriptorCount = 1;
        depth_matrix_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        depth_matrix_layout_create_info.pBindings = &depth_matrix_layout_binding;
        const VkDescriptorSetLayout depth_matrix_layout = mVulkanRenderer.GetDescriptorSetLayout ( depth_matrix_layout_create_info );

        // One pool entry per slot so maxSets covers all caster descriptor sets.
        std::vector<VkDescriptorPoolSize> depth_matrix_pool_sizes ( MAX_SPOT_SHADOW_CASTERS, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1} );
        mSpotShadowDepthMatricesDescriptorPool = CreateDescriptorPool ( device, depth_matrix_pool_sizes );
        for ( uint32_t slot = 0; slot < MAX_SPOT_SHADOW_CASTERS; ++slot )
        {
            mSpotShadowDepthMatricesDescriptorSets[slot] =
                CreateDescriptorSet ( device, mSpotShadowDepthMatricesDescriptorPool, depth_matrix_layout );
            VkDescriptorBufferInfo depth_matrix_buffer_info{};
            depth_matrix_buffer_info.buffer = mSpotShadowDepthMatrices.GetBuffer();
            depth_matrix_buffer_info.offset = slot * mSpotShadowDepthMatrixStride;
            depth_matrix_buffer_info.range = sizeof ( GpuShadowParams );
            VkWriteDescriptorSet depth_matrix_write{};
            depth_matrix_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            depth_matrix_write.dstSet = mSpotShadowDepthMatricesDescriptorSets[slot];
            depth_matrix_write.dstBinding = 0;
            depth_matrix_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            depth_matrix_write.descriptorCount = 1;
            depth_matrix_write.pBufferInfo = &depth_matrix_buffer_info;
            vkUpdateDescriptorSets ( device, 1, &depth_matrix_write, 0, nullptr );
        }

        // Transition every depth layer to the sampled layout once, so frames
        // with fewer (or no) casters still bind the array in a valid layout.
        VkCommandBuffer command_buffer = mVulkanRenderer.BeginSingleTimeCommands();
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = mVkSpotShadowDepthImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | ( has_stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = MAX_SPOT_SHADOW_CASTERS;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier ( command_buffer,
                               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &barrier );
        mVulkanRenderer.EndSingleTimeCommands ( command_buffer );
    }

    void VulkanWindow::FinalizeSpotShadowMap()
    {
        const VkDevice device = mVulkanRenderer.GetDevice();
        DestroyDescriptorPool ( device, mSpotShadowDepthMatricesDescriptorPool );
        DestroyDescriptorPool ( device, mSpotShadowMapDescriptorPool );
        DestroyDescriptorPool ( device, mSpotShadowParamsDescriptorPool );
        mSpotShadowDepthMatrices.Finalize();
        mSpotShadowParams.Finalize();
        for ( uint32_t slot = 0; slot < MAX_SPOT_SHADOW_CASTERS; ++slot )
        {
            if ( mVkSpotShadowFramebuffers[slot] != VK_NULL_HANDLE )
            {
                vkDestroyFramebuffer ( device, mVkSpotShadowFramebuffers[slot], nullptr );
            }
            if ( mVkSpotShadowDepthLayerViews[slot] != VK_NULL_HANDLE )
            {
                vkDestroyImageView ( device, mVkSpotShadowDepthLayerViews[slot], nullptr );
            }
        }
        if ( mVkSpotShadowColorImageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( device, mVkSpotShadowColorImageView, nullptr );
        }
        if ( mVkSpotShadowColorImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( device, mVkSpotShadowColorImage, nullptr );
        }
        if ( mVkSpotShadowColorImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( device, mVkSpotShadowColorImageMemory, nullptr );
        }
        if ( mVkSpotShadowDepthArrayView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( device, mVkSpotShadowDepthArrayView, nullptr );
        }
        if ( mVkSpotShadowDepthImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( device, mVkSpotShadowDepthImage, nullptr );
        }
        if ( mVkSpotShadowDepthImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( device, mVkSpotShadowDepthImageMemory, nullptr );
        }
    }

    void VulkanWindow::InitializePointShadowMap()
    {
        const VkDevice device = mVulkanRenderer.GetDevice();
        const bool has_stencil =
            ( mVkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT );

        auto create_image = [&] ( VkFormat format, VkImageUsageFlags usage,
                                  uint32_t array_layers, VkImageCreateFlags flags,
                                  VkImage & image, VkDeviceMemory & memory )
        {
            VkImageCreateInfo image_create_info{};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.flags = flags;
            image_create_info.format = format;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.extent.width = POINT_SHADOW_MAP_RESOLUTION;
            image_create_info.extent.height = POINT_SHADOW_MAP_RESOLUTION;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = array_layers;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.usage = usage;
            image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if ( VkResult result = vkCreateImage ( device, &image_create_info, nullptr, &image ) )
            {
                std::ostringstream stream;
                stream << "Point shadow vkCreateImage failed: ( " << GetVulkanResultString ( result ) << " )";
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }
            VkMemoryRequirements memory_requirements;
            vkGetImageMemoryRequirements ( device, image, &memory_requirements );
            VkMemoryAllocateInfo memory_allocate_info{};
            memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memory_allocate_info.allocationSize = memory_requirements.size;
            memory_allocate_info.memoryTypeIndex =
                mVulkanRenderer.FindMemoryTypeIndex ( memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
            vkAllocateMemory ( device, &memory_allocate_info, nullptr, &memory );
            vkBindImageMemory ( device, image, memory, 0 );
        };

        // The depth image is CUBE-COMPATIBLE so it can be sampled as a cube map
        // array (six faces per caster); the throwaway colour image is six-layered
        // to match the multiview view mask.
        create_image ( mVkDepthStencilFormat,
                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                       POINT_SHADOW_LAYERS, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                       mVkPointShadowDepthImage, mVkPointShadowDepthImageMemory );
        create_image ( mVkSurfaceFormatKHR.format,
                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                       POINT_SHADOW_FACES, 0,
                       mVkPointShadowColorImage, mVkPointShadowColorImageMemory );

        VkImageViewCreateInfo array_view_create_info{};
        array_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        array_view_create_info.image = mVkPointShadowDepthImage;
        array_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        array_view_create_info.format = mVkDepthStencilFormat;
        array_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        array_view_create_info.subresourceRange.levelCount = 1;
        array_view_create_info.subresourceRange.layerCount = POINT_SHADOW_LAYERS;
        vkCreateImageView ( device, &array_view_create_info, nullptr, &mVkPointShadowDepthArrayView );

        VkImageViewCreateInfo color_view_create_info{};
        color_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        color_view_create_info.image = mVkPointShadowColorImage;
        color_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        color_view_create_info.format = mVkSurfaceFormatKHR.format;
        color_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_view_create_info.subresourceRange.levelCount = 1;
        color_view_create_info.subresourceRange.layerCount = POINT_SHADOW_FACES;
        vkCreateImageView ( device, &color_view_create_info, nullptr, &mVkPointShadowColorImageView );

        // Multiview render pass: one view per cube face (a six-bit view mask), so
        // a single draw renders all six faces and the multiview vertex shader
        // selects each face's projection by gl_ViewIndex. The point depth
        // pipeline is created against THIS pass because the view mask is baked
        // into the pipeline. Attachments mirror the directional shadow pass.
        {
            std::array<VkAttachmentDescription, 2> attachments{ {} };
            attachments[0].format = mVkSurfaceFormatKHR.format;
            attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachments[1].format = mVkDepthStencilFormat;
            attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            VkAttachmentReference color_ref{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
            VkAttachmentReference depth_ref{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_ref;
            subpass.pDepthStencilAttachment = &depth_ref;
            const uint32_t view_mask = ( 1u << POINT_SHADOW_FACES ) - 1u; // 0b111111
            VkRenderPassMultiviewCreateInfo multiview_create_info{};
            multiview_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
            multiview_create_info.subpassCount = 1;
            multiview_create_info.pViewMasks = &view_mask;
            VkRenderPassCreateInfo render_pass_create_info{};
            render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_create_info.pNext = &multiview_create_info;
            render_pass_create_info.attachmentCount = static_cast<uint32_t> ( attachments.size() );
            render_pass_create_info.pAttachments = attachments.data();
            render_pass_create_info.subpassCount = 1;
            render_pass_create_info.pSubpasses = &subpass;
            vkCreateRenderPass ( device, &render_pass_create_info, nullptr, &mVkPointShadowRenderPass );
        }

        // One six-layer depth view + framebuffer per caster. Multiview renders to
        // all six of the caster's layers (baseArrayLayer = caster*6) in one pass;
        // the framebuffer layer count is 1, the views supply the six array layers.
        for ( uint32_t caster = 0; caster < MAX_POINT_SHADOW_CASTERS; ++caster )
        {
            VkImageViewCreateInfo caster_view_create_info{};
            caster_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            caster_view_create_info.image = mVkPointShadowDepthImage;
            caster_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            caster_view_create_info.format = mVkDepthStencilFormat;
            caster_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            caster_view_create_info.subresourceRange.levelCount = 1;
            caster_view_create_info.subresourceRange.baseArrayLayer = caster * POINT_SHADOW_FACES;
            caster_view_create_info.subresourceRange.layerCount = POINT_SHADOW_FACES;
            vkCreateImageView ( device, &caster_view_create_info, nullptr, &mVkPointShadowDepthCasterViews[caster] );

            std::array<VkImageView, 2> framebuffer_attachments{ { mVkPointShadowColorImageView, mVkPointShadowDepthCasterViews[caster] } };
            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = mVkPointShadowRenderPass;
            framebuffer_create_info.attachmentCount = static_cast<uint32_t> ( framebuffer_attachments.size() );
            framebuffer_create_info.pAttachments = framebuffer_attachments.data();
            framebuffer_create_info.width = POINT_SHADOW_MAP_RESOLUTION;
            framebuffer_create_info.height = POINT_SHADOW_MAP_RESOLUTION;
            framebuffer_create_info.layers = 1;
            vkCreateFramebuffer ( device, &framebuffer_create_info, nullptr, &mVkPointShadowFramebuffers[caster] );
        }

        // Point ShadowParams UBO (all caster matrices + positions), sampled by
        // the shading fragment shader (set 12).
        const GpuPointShadowParams empty_point_params{};
        mPointShadowParams.Initialize (
            sizeof ( GpuPointShadowParams ),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &empty_point_params );

        VkDescriptorSetLayoutCreateInfo point_params_layout_create_info{};
        point_params_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        point_params_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding point_params_layout_binding{};
        point_params_layout_binding.binding = 0;
        point_params_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        point_params_layout_binding.descriptorCount = 1;
        point_params_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        point_params_layout_create_info.pBindings = &point_params_layout_binding;

        mPointShadowParamsDescriptorPool = CreateDescriptorPool ( device, {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}} );
        mPointShadowParamsDescriptorSet = CreateDescriptorSet ( device, mPointShadowParamsDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( point_params_layout_create_info ) );
        VkDescriptorBufferInfo point_params_buffer_info{};
        point_params_buffer_info.buffer = mPointShadowParams.GetBuffer();
        point_params_buffer_info.offset = 0;
        point_params_buffer_info.range = mPointShadowParams.GetSize();
        VkWriteDescriptorSet point_params_write{};
        point_params_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        point_params_write.dstSet = mPointShadowParamsDescriptorSet;
        point_params_write.dstBinding = 0;
        point_params_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        point_params_write.descriptorCount = 1;
        point_params_write.pBufferInfo = &point_params_buffer_info;
        vkUpdateDescriptorSets ( device, 1, &point_params_write, 0, nullptr );

        // PointShadowMap combined image sampler (set 13).
        VkDescriptorSetLayoutCreateInfo point_map_layout_create_info{};
        point_map_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        point_map_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding point_map_layout_binding{};
        point_map_layout_binding.binding = 0;
        point_map_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        point_map_layout_binding.descriptorCount = 1;
        point_map_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        point_map_layout_create_info.pBindings = &point_map_layout_binding;

        mPointShadowMapDescriptorPool = CreateDescriptorPool ( device, {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}} );
        mPointShadowMapDescriptorSet = CreateDescriptorSet ( device, mPointShadowMapDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( point_map_layout_create_info ) );
        VkDescriptorImageInfo point_map_image_info{};
        point_map_image_info.sampler = mVkShadowSampler;
        point_map_image_info.imageView = mVkPointShadowDepthArrayView;
        point_map_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet point_map_write{};
        point_map_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        point_map_write.dstSet = mPointShadowMapDescriptorSet;
        point_map_write.dstBinding = 0;
        point_map_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        point_map_write.descriptorCount = 1;
        point_map_write.pImageInfo = &point_map_image_info;
        vkUpdateDescriptorSets ( device, 1, &point_map_write, 0, nullptr );

        // Per-caster depth params: one descriptor set per caster bound to its
        // aligned region of a shared UBO. Each holds all six face projections;
        // the multiview vertex shader indexes them by gl_ViewIndex.
        const VkDeviceSize alignment =
            mVulkanRenderer.GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
        mPointShadowDepthMatrixStride =
            ( ( sizeof ( GpuPointDepthParams ) - 1 ) | ( ( alignment > 0 ? alignment : 1 ) - 1 ) ) + 1;
        mPointShadowDepthMatrices.Initialize (
            mPointShadowDepthMatrixStride * MAX_POINT_SHADOW_CASTERS,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            nullptr );

        VkDescriptorSetLayoutCreateInfo depth_matrix_layout_create_info{};
        depth_matrix_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        depth_matrix_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding depth_matrix_layout_binding{};
        depth_matrix_layout_binding.binding = 0;
        depth_matrix_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        depth_matrix_layout_binding.descriptorCount = 1;
        depth_matrix_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        depth_matrix_layout_create_info.pBindings = &depth_matrix_layout_binding;
        const VkDescriptorSetLayout depth_matrix_layout = mVulkanRenderer.GetDescriptorSetLayout ( depth_matrix_layout_create_info );

        std::vector<VkDescriptorPoolSize> depth_matrix_pool_sizes ( MAX_POINT_SHADOW_CASTERS, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1} );
        mPointShadowDepthMatricesDescriptorPool = CreateDescriptorPool ( device, depth_matrix_pool_sizes );
        for ( uint32_t caster = 0; caster < MAX_POINT_SHADOW_CASTERS; ++caster )
        {
            mPointShadowDepthMatricesDescriptorSets[caster] =
                CreateDescriptorSet ( device, mPointShadowDepthMatricesDescriptorPool, depth_matrix_layout );
            VkDescriptorBufferInfo depth_matrix_buffer_info{};
            depth_matrix_buffer_info.buffer = mPointShadowDepthMatrices.GetBuffer();
            depth_matrix_buffer_info.offset = caster * mPointShadowDepthMatrixStride;
            depth_matrix_buffer_info.range = sizeof ( GpuPointDepthParams );
            VkWriteDescriptorSet depth_matrix_write{};
            depth_matrix_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            depth_matrix_write.dstSet = mPointShadowDepthMatricesDescriptorSets[caster];
            depth_matrix_write.dstBinding = 0;
            depth_matrix_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            depth_matrix_write.descriptorCount = 1;
            depth_matrix_write.pBufferInfo = &depth_matrix_buffer_info;
            vkUpdateDescriptorSets ( device, 1, &depth_matrix_write, 0, nullptr );
        }

        // Transition every depth layer to the sampled layout once.
        VkCommandBuffer command_buffer = mVulkanRenderer.BeginSingleTimeCommands();
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = mVkPointShadowDepthImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | ( has_stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = POINT_SHADOW_LAYERS;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier ( command_buffer,
                               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &barrier );
        mVulkanRenderer.EndSingleTimeCommands ( command_buffer );
    }

    void VulkanWindow::FinalizePointShadowMap()
    {
        const VkDevice device = mVulkanRenderer.GetDevice();
        DestroyDescriptorPool ( device, mPointShadowDepthMatricesDescriptorPool );
        DestroyDescriptorPool ( device, mPointShadowMapDescriptorPool );
        DestroyDescriptorPool ( device, mPointShadowParamsDescriptorPool );
        mPointShadowDepthMatrices.Finalize();
        mPointShadowParams.Finalize();
        for ( uint32_t caster = 0; caster < MAX_POINT_SHADOW_CASTERS; ++caster )
        {
            if ( mVkPointShadowFramebuffers[caster] != VK_NULL_HANDLE )
            {
                vkDestroyFramebuffer ( device, mVkPointShadowFramebuffers[caster], nullptr );
            }
            if ( mVkPointShadowDepthCasterViews[caster] != VK_NULL_HANDLE )
            {
                vkDestroyImageView ( device, mVkPointShadowDepthCasterViews[caster], nullptr );
            }
        }
        if ( mVkPointShadowRenderPass != VK_NULL_HANDLE )
        {
            vkDestroyRenderPass ( device, mVkPointShadowRenderPass, nullptr );
        }
        if ( mVkPointShadowColorImageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( device, mVkPointShadowColorImageView, nullptr );
        }
        if ( mVkPointShadowColorImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( device, mVkPointShadowColorImage, nullptr );
        }
        if ( mVkPointShadowColorImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( device, mVkPointShadowColorImageMemory, nullptr );
        }
        if ( mVkPointShadowDepthArrayView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( device, mVkPointShadowDepthArrayView, nullptr );
        }
        if ( mVkPointShadowDepthImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( device, mVkPointShadowDepthImage, nullptr );
        }
        if ( mVkPointShadowDepthImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( device, mVkPointShadowDepthImageMemory, nullptr );
        }
    }

    void VulkanWindow::FinalizeShadowMap()
    {
        const VkDevice device = mVulkanRenderer.GetDevice();
        DestroyDescriptorPool ( device, mShadowMapDescriptorPool );
        DestroyDescriptorPool ( device, mShadowParamsDescriptorPool );
        mShadowParams.Finalize();
        if ( mVkShadowFramebuffer != VK_NULL_HANDLE )
        {
            vkDestroyFramebuffer ( device, mVkShadowFramebuffer, nullptr );
        }
        if ( mVkShadowRenderPass != VK_NULL_HANDLE )
        {
            vkDestroyRenderPass ( device, mVkShadowRenderPass, nullptr );
        }
        if ( mVkShadowSampler != VK_NULL_HANDLE )
        {
            vkDestroySampler ( device, mVkShadowSampler, nullptr );
        }
        if ( mVkShadowColorImageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( device, mVkShadowColorImageView, nullptr );
        }
        if ( mVkShadowColorImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( device, mVkShadowColorImage, nullptr );
        }
        if ( mVkShadowColorImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( device, mVkShadowColorImageMemory, nullptr );
        }
        if ( mVkShadowDepthImageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( device, mVkShadowDepthImageView, nullptr );
        }
        if ( mVkShadowDepthImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( device, mVkShadowDepthImage, nullptr );
        }
        if ( mVkShadowDepthImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( device, mVkShadowDepthImageMemory, nullptr );
        }
    }

    void VulkanWindow::BeginShadowPass ( const Matrix4x4& aLightViewProjection )
    {
        // Lazily load the renderer-owned shadow depth pipeline, substituted for
        // the scene's draw pipelines during the shadow pass.
        if ( !mShadowDepthLoaded )
        {
            mShadowDepthPipeline.LoadFromFile ( "shaders/shadow_depth" );
            mShadowDepthLoaded = true;
        }
        // Upload this frame's light view-projection and shadow parameters.
        GpuShadowParams shadow_params{};
        shadow_params.light_view_projection = aLightViewProjection;
        shadow_params.params[0] = 1.0f / static_cast<float> ( SHADOW_MAP_RESOLUTION );
        shadow_params.params[1] = 0.0015f;
        shadow_params.params[2] = 1.0f;
        shadow_params.params[3] = 1.0f; // enabled
        mShadowParams.WriteMemory ( 0, sizeof ( GpuShadowParams ), &shadow_params );

        // The main render pass was opened by BeginRender for the depth
        // pre-pass; close it so the fixed-size shadow pass can run.
        vkCmdEndRenderPass ( mVkCommandBuffer );

        // Two clear values to satisfy the render pass's two attachments. The
        // throwaway color attachment (index 0) is DONT_CARE so its value is
        // ignored, but the array must still be large enough to index the depth
        // attachment (index 1), which is cleared to the far plane.
        std::array<VkClearValue, 2> clear_values{};
        clear_values[1].depthStencil.depth = 1.0f;
        clear_values[1].depthStencil.stencil = 0;
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = mVkShadowRenderPass;
        render_pass_begin_info.framebuffer = mVkShadowFramebuffer;
        render_pass_begin_info.renderArea.offset = { 0, 0 };
        render_pass_begin_info.renderArea.extent = { SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION };
        render_pass_begin_info.clearValueCount = static_cast<uint32_t> ( clear_values.size() );
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass ( mVkCommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );

        VkViewport shadow_viewport{ 0.0f, 0.0f, static_cast<float> ( SHADOW_MAP_RESOLUTION ), static_cast<float> ( SHADOW_MAP_RESOLUTION ), 0.0f, 1.0f };
        VkRect2D shadow_scissor{ { 0, 0 }, { SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION } };
        vkCmdSetViewport ( mVkCommandBuffer, 0, 1, &shadow_viewport );
        vkCmdSetScissor ( mVkCommandBuffer, 0, 1, &shadow_scissor );
        // Slope-scaled depth bias pushes caster depths away from the light so
        // surfaces do not shadow themselves (the single whole-scene shadow map
        // has coarse texels, producing large blocky acne otherwise). Mirrors the
        // OpenGL glPolygonOffset(2.5, 4.0): slope factor 2.5, constant 4.0.
        vkCmdSetDepthBias ( mVkCommandBuffer, 4.0f, 0.0f, 2.5f );
    }

    void VulkanWindow::EndShadowPass()
    {
        // Close the shadow pass; its final layout leaves the depth image
        // sampleable by the shading fragment shader.
        vkCmdEndRenderPass ( mVkCommandBuffer );
        // The shadow render pass mirrors the main pass's single colour-output
        // dependency for render-pass compatibility, so it does not synchronise
        // the depth write -> shader read hazard. Do it explicitly here, while no
        // render pass is active. The render pass already transitioned the image
        // to SHADER_READ_ONLY_OPTIMAL, so this is a pure execution/memory
        // barrier (old == new layout).
        const bool has_stencil =
            ( mVkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT );
        VkImageMemoryBarrier shadow_depth_barrier{};
        shadow_depth_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        shadow_depth_barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadow_depth_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadow_depth_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadow_depth_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shadow_depth_barrier.image = mVkShadowDepthImage;
        shadow_depth_barrier.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_DEPTH_BIT | ( has_stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
        shadow_depth_barrier.subresourceRange.levelCount = 1;
        shadow_depth_barrier.subresourceRange.layerCount = 1;
        shadow_depth_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        shadow_depth_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier ( mVkCommandBuffer,
                               VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &shadow_depth_barrier );
        // Restore the window viewport/scissor for the rest of the frame.
        vkCmdSetViewport ( mVkCommandBuffer, 0, 1, &mVkViewport );
        vkCmdSetScissor ( mVkCommandBuffer, 0, 1, &mVkScissor );
        // Clear the shadow-pass depth bias so the depth pre-pass and shading
        // draws that follow are not biased.
        vkCmdSetDepthBias ( mVkCommandBuffer, 0.0f, 0.0f, 0.0f );
        // Reopen the main render pass so the depth pre-pass can continue. No
        // geometry has been recorded into it yet, so the re-clear is harmless.
        BeginRenderPass();
    }

    void VulkanWindow::SetSpotShadowParams ( const GpuSpotShadowParams& aSpotShadowParams )
    {
        mSpotShadowParams.WriteMemory ( 0, sizeof ( GpuSpotShadowParams ), &aSpotShadowParams );
    }

    void VulkanWindow::BeginSpotShadowPass ( uint32_t aSlot, const Matrix4x4& aLightViewProjection )
    {
        if ( aSlot >= MAX_SPOT_SHADOW_CASTERS )
        {
            return;
        }
        // Lazily load the renderer-owned shadow depth pipeline (shared with the
        // directional pass).
        if ( !mShadowDepthLoaded )
        {
            mShadowDepthPipeline.LoadFromFile ( "shaders/shadow_depth" );
            mShadowDepthLoaded = true;
        }
        // Write this caster's matrix into its own slot region of the depth
        // matrices UBO; the matching descriptor set binds that region at the
        // depth pipeline's ShadowParams slot.
        GpuShadowParams depth_matrix{};
        depth_matrix.light_view_projection = aLightViewProjection;
        depth_matrix.params[3] = 1.0f; // enabled (unused by the depth vertex shader)
        mSpotShadowDepthMatrices.WriteMemory ( aSlot * mSpotShadowDepthMatrixStride,
                                               sizeof ( GpuShadowParams ), &depth_matrix );
        mInSpotShadowPass = true;
        mCurrentSpotShadowSlot = aSlot;

        // Close the main render pass opened by BeginRender so this fixed-size
        // spot shadow pass can run.
        vkCmdEndRenderPass ( mVkCommandBuffer );

        std::array<VkClearValue, 2> clear_values{};
        clear_values[1].depthStencil.depth = 1.0f;
        clear_values[1].depthStencil.stencil = 0;
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = mVkShadowRenderPass;
        render_pass_begin_info.framebuffer = mVkSpotShadowFramebuffers[aSlot];
        render_pass_begin_info.renderArea.offset = { 0, 0 };
        render_pass_begin_info.renderArea.extent = { SPOT_SHADOW_MAP_RESOLUTION, SPOT_SHADOW_MAP_RESOLUTION };
        render_pass_begin_info.clearValueCount = static_cast<uint32_t> ( clear_values.size() );
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass ( mVkCommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );

        VkViewport shadow_viewport{ 0.0f, 0.0f, static_cast<float> ( SPOT_SHADOW_MAP_RESOLUTION ), static_cast<float> ( SPOT_SHADOW_MAP_RESOLUTION ), 0.0f, 1.0f };
        VkRect2D shadow_scissor{ { 0, 0 }, { SPOT_SHADOW_MAP_RESOLUTION, SPOT_SHADOW_MAP_RESOLUTION } };
        vkCmdSetViewport ( mVkCommandBuffer, 0, 1, &shadow_viewport );
        vkCmdSetScissor ( mVkCommandBuffer, 0, 1, &shadow_scissor );
        vkCmdSetDepthBias ( mVkCommandBuffer, 4.0f, 0.0f, 2.5f );
    }

    void VulkanWindow::EndSpotShadowPass()
    {
        mInSpotShadowPass = false;
        // Close the spot shadow pass; its final layout leaves the rendered layer
        // sampleable by the shading fragment shader.
        vkCmdEndRenderPass ( mVkCommandBuffer );
        // Explicit depth-write -> shader-read barrier for the layer just
        // rendered (the shared render pass omits this dependency for
        // compatibility, as in EndShadowPass).
        const bool has_stencil =
            ( mVkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT );
        VkImageMemoryBarrier spot_depth_barrier{};
        spot_depth_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        spot_depth_barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        spot_depth_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        spot_depth_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        spot_depth_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        spot_depth_barrier.image = mVkSpotShadowDepthImage;
        spot_depth_barrier.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_DEPTH_BIT | ( has_stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
        spot_depth_barrier.subresourceRange.levelCount = 1;
        spot_depth_barrier.subresourceRange.baseArrayLayer = mCurrentSpotShadowSlot;
        spot_depth_barrier.subresourceRange.layerCount = 1;
        spot_depth_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        spot_depth_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier ( mVkCommandBuffer,
                               VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &spot_depth_barrier );
        vkCmdSetViewport ( mVkCommandBuffer, 0, 1, &mVkViewport );
        vkCmdSetScissor ( mVkCommandBuffer, 0, 1, &mVkScissor );
        vkCmdSetDepthBias ( mVkCommandBuffer, 0.0f, 0.0f, 0.0f );
        // Reopen the main render pass so the next pass (another spot, the
        // directional pass, or the depth pre-pass) can continue.
        BeginRenderPass();
    }

    void VulkanWindow::SetPointShadowParams ( const GpuPointShadowParams& aPointShadowParams )
    {
        mPointShadowParamsCpu = aPointShadowParams;
        mPointShadowParams.WriteMemory ( 0, sizeof ( GpuPointShadowParams ), &aPointShadowParams );
    }

    void VulkanWindow::BeginPointShadowPass ( uint32_t aCaster )
    {
        if ( aCaster >= MAX_POINT_SHADOW_CASTERS )
        {
            return;
        }
        if ( !mPointShadowDepthLoaded )
        {
            mPointShadowDepthPipeline.LoadFromFile ( "shaders/point_shadow_depth" );
            mPointShadowDepthLoaded = true;
        }
        // Pack this caster's six face projections + light position/radius into
        // the caster's slot of the depth-params UBO. The multiview vertex shader
        // indexes the matrices by gl_ViewIndex (one view per cube face); the
        // fragment shader writes normalized radial distance from the light.
        // face_params stays zero: on Vulkan the framebuffer is a per-caster
        // six-layer view, so no gl_Layer base offset is needed.
        GpuPointDepthParams depth_params{};
        for ( uint32_t face = 0; face < POINT_SHADOW_FACES; ++face )
        {
            depth_params.face_view_projection[face] =
                mPointShadowParamsCpu.point_light_view_projection[aCaster * POINT_SHADOW_FACES + face];
        }
        depth_params.light_position_radius = mPointShadowParamsCpu.caster_position_radius[aCaster];
        mPointShadowDepthMatrices.WriteMemory ( aCaster * mPointShadowDepthMatrixStride,
                                                sizeof ( GpuPointDepthParams ), &depth_params );
        mInPointShadowPass = true;
        mCurrentPointShadowCaster = aCaster;

        // Close the main render pass opened by BeginRender so the multiview cube
        // pass can run, then open the caster's six-layer multiview framebuffer.
        vkCmdEndRenderPass ( mVkCommandBuffer );

        std::array<VkClearValue, 2> clear_values{};
        clear_values[1].depthStencil.depth = 1.0f;
        clear_values[1].depthStencil.stencil = 0;
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = mVkPointShadowRenderPass;
        render_pass_begin_info.framebuffer = mVkPointShadowFramebuffers[aCaster];
        render_pass_begin_info.renderArea.offset = { 0, 0 };
        render_pass_begin_info.renderArea.extent = { POINT_SHADOW_MAP_RESOLUTION, POINT_SHADOW_MAP_RESOLUTION };
        render_pass_begin_info.clearValueCount = static_cast<uint32_t> ( clear_values.size() );
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass ( mVkCommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );

        VkViewport shadow_viewport{ 0.0f, 0.0f, static_cast<float> ( POINT_SHADOW_MAP_RESOLUTION ), static_cast<float> ( POINT_SHADOW_MAP_RESOLUTION ), 0.0f, 1.0f };
        VkRect2D shadow_scissor{ { 0, 0 }, { POINT_SHADOW_MAP_RESOLUTION, POINT_SHADOW_MAP_RESOLUTION } };
        vkCmdSetViewport ( mVkCommandBuffer, 0, 1, &shadow_viewport );
        vkCmdSetScissor ( mVkCommandBuffer, 0, 1, &shadow_scissor );
        vkCmdSetDepthBias ( mVkCommandBuffer, 4.0f, 0.0f, 2.5f );
    }

    void VulkanWindow::EndPointShadowPass()
    {
        mInPointShadowPass = false;
        vkCmdEndRenderPass ( mVkCommandBuffer );
        const bool has_stencil =
            ( mVkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
              mVkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT );
        VkImageMemoryBarrier point_depth_barrier{};
        point_depth_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        point_depth_barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        point_depth_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        point_depth_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        point_depth_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        point_depth_barrier.image = mVkPointShadowDepthImage;
        point_depth_barrier.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_DEPTH_BIT | ( has_stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
        point_depth_barrier.subresourceRange.levelCount = 1;
        // The multiview pass wrote all six of this caster's cube-face layers.
        point_depth_barrier.subresourceRange.baseArrayLayer = mCurrentPointShadowCaster * POINT_SHADOW_FACES;
        point_depth_barrier.subresourceRange.layerCount = POINT_SHADOW_FACES;
        point_depth_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        point_depth_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier ( mVkCommandBuffer,
                               VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &point_depth_barrier );
        vkCmdSetViewport ( mVkCommandBuffer, 0, 1, &mVkViewport );
        vkCmdSetScissor ( mVkCommandBuffer, 0, 1, &mVkScissor );
        vkCmdSetDepthBias ( mVkCommandBuffer, 0.0f, 0.0f, 0.0f );
        BeginRenderPass();
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
        InitializeGlobals();
        InitializeSurface();
        InitializeRenderPass();
        InitializeSwapchain();
        InitializeImageViews();
        InitializeDepthStencil();
        InitializeFrameBuffers();
        InitializeShadowMap();
        InitializeSpotShadowMap();
        InitializePointShadowMap();
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
        FinalizePointShadowMap();
        FinalizeSpotShadowMap();
        FinalizeShadowMap();
        FinalizeGlobals();
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
        // Depth bias is dynamic and enabled on every pipeline; default it to
        // zero for the frame so normal geometry is unbiased. Only the shadow
        // depth pass overrides this with a non-zero slope/constant bias.
        vkCmdSetDepthBias ( mVkCommandBuffer, 0.0f, 0.0f, 0.0f );
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
        render_pass_begin_info.framebuffer = mVkHdrFramebuffer;
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
                mClusterMarkPipeline.LoadFromFile ( "shaders/cluster_mark" );
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
        const uint32_t stage_count = aComputePipeline.GetComputeStageCount ( mVulkanRenderer.GetName() );
        for ( uint32_t stage = 1; stage < stage_count; ++stage )
        {
            Dispatch ( aComputePipeline, group_count, 1, 1, bindings, stage );
            Barrier();
        }
    }

    void VulkanWindow::EndRender()
    {
        vkCmdEndRenderPass ( mVkCommandBuffer );

        // Resolve the linear HDR scene target to the swapchain: a second render
        // pass runs the fullscreen tonemap pipeline (exposure + ACES tone map +
        // sRGB encode), sampling the HDR colour image the scene pass left in
        // SHADER_READ_ONLY. The fullscreen triangle needs no vertex buffer.
        if ( !mTonemapLoaded )
        {
            mTonemapPipeline.LoadFromFile ( "shaders/tonemap" );
            mTonemapLoaded = true;
        }
        const VulkanPipeline* tonemap_pipeline = mVulkanRenderer.GetVulkanPipeline ( mTonemapPipeline, mVkTonemapRenderPass );
        VkRenderPassBeginInfo tonemap_begin_info{};
        tonemap_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        tonemap_begin_info.renderPass = mVkTonemapRenderPass;
        tonemap_begin_info.framebuffer = mVkFramebuffers[mActiveImageIndex];
        tonemap_begin_info.renderArea = mVkScissor;
        tonemap_begin_info.clearValueCount = 0;
        tonemap_begin_info.pClearValues = nullptr;
        vkCmdBeginRenderPass ( mVkCommandBuffer, &tonemap_begin_info, VK_SUBPASS_CONTENTS_INLINE );
        vkCmdBindPipeline ( mVkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, tonemap_pipeline->GetVkPipeline() );
        vkCmdSetPrimitiveTopology ( mVkCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );
        // The tonemap pipeline inherits the engine-wide dynamic depth-bias
        // state; it draws no depth, so keep it at zero for this pass.
        vkCmdSetDepthBias ( mVkCommandBuffer, 0.0f, 0.0f, 0.0f );
        vkCmdBindDescriptorSets ( mVkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  tonemap_pipeline->GetPipelineLayout(), 0, 1, &mTonemapDescriptorSet, 0, nullptr );
        vkCmdDraw ( mVkCommandBuffer, 3, 1, 0, 0 );
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
        // A single object: hardware-instanced aInstanceCount times (e.g. the
        // editor grid draws one line matrix repeated across gl_InstanceIndex).
        RenderCommon ( { &aModelMatrix, 1 }, aMesh, aPipeline, aMaterial, aTopology,
                       aVertexStart, aVertexCount, aInstanceCount, aFirstInstance,
                       aSkinnedVertices, aRenderPass );
    }

#ifndef NDEBUG
    namespace
    {
        // Debug-only regression guard for the per-pass binding helpers below.
        // Every descriptor set a pipeline reflects must be one that some binder in
        // the active render pass handles. If a shader gains a new descriptor set
        // but the matching Bind*PassSets/caller sequence is not updated, that set
        // would silently go unbound and the GPU would fault at draw time (device
        // lost) -- exactly the class of bug that unifying the draw paths is meant
        // to prevent. Assert at the bind site so the omission is caught loudly.
        void AssertDescriptorSetsHandled ( const VulkanPipeline* aPipeline,
                                           std::initializer_list<uint32_t> aHandledHashes )
        {
            for ( const VulkanDescriptorSetInfo& descriptor_set : aPipeline->GetDescriptorSetInfos() )
            {
                assert ( std::find ( aHandledHashes.begin(), aHandledHashes.end(), descriptor_set.hash ) != aHandledHashes.end() &&
                         "Pipeline reflects a descriptor set that no binder handles for this render pass; "
                         "wire it into the matching Bind*PassSets helper (and this handled-set list)." );
            }
        }
    }
#endif

    void VulkanWindow::BindShadingPassSets ( const VulkanPipeline* aPipeline ) const
    {
#ifndef NDEBUG
        // Engine sets bound here, plus MATERIAL/SAMPLERS (VulkanMaterial::Bind) and
        // INSTANCE_MATRICES (BindObjectMatrices) bound by the caller after us.
        AssertDescriptorSetsHandled ( aPipeline,
        {
            Mesh::BindingLocations::MATRICES, Mesh::BindingLocations::LIGHTS,
            Mesh::BindingLocations::CLUSTER_PARAMS, Mesh::BindingLocations::GLOBALS,
            Mesh::BindingLocations::LIGHT_GRID, Mesh::BindingLocations::LIGHT_INDEX_LIST,
            Mesh::BindingLocations::SHADOW_PARAMS, Mesh::BindingLocations::SHADOW_MAP,
            Mesh::BindingLocations::SPOT_SHADOW_PARAMS, Mesh::BindingLocations::SPOT_SHADOW_MAP,
            Mesh::BindingLocations::POINT_SHADOW_PARAMS, Mesh::BindingLocations::POINT_SHADOW_MAP,
            Mesh::BindingLocations::MATERIAL, Mesh::BindingLocations::SAMPLERS,
            Mesh::BindingLocations::INSTANCE_MATRICES,
        } );
#endif
        // Bind an engine-owned descriptor set at its reflected set index, if the
        // pipeline declares it.
        auto bind = [&] ( uint32_t aBinding, VkDescriptorSet aSet )
        {
            if ( uint32_t set_index = aPipeline->GetDescriptorSetIndex ( aBinding ); set_index != std::numeric_limits<uint32_t>::max() )
            {
                vkCmdBindDescriptorSets ( GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          aPipeline->GetPipelineLayout(), set_index, 1, &aSet, 0, nullptr );
            }
        };
        // Clustered Forward+ light lists live in single-frame pool buffers bound
        // with a dynamic offset; skip when the light-cull stage produced none.
        auto bind_cluster_storage = [&] ( uint32_t aBinding, const BufferAccessor & aAccessor )
        {
            if ( aAccessor.GetMemoryPoolBuffer() == nullptr )
            {
                return;
            }
            if ( uint32_t set_index = aPipeline->GetDescriptorSetIndex ( aBinding ); set_index != std::numeric_limits<uint32_t>::max() )
            {
                const VulkanStorageMemoryPoolBuffer* memory_pool_buffer =
                    reinterpret_cast<const VulkanStorageMemoryPoolBuffer*> ( aAccessor.GetMemoryPoolBuffer() );
                uint32_t dynamic_offset = 0;
                vkCmdBindDescriptorSets ( GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          aPipeline->GetPipelineLayout(), set_index, 1,
                                          &memory_pool_buffer->GetDescriptorSet ( aAccessor.GetOffset() ), 1, &dynamic_offset );
            }
        };
        bind ( Mesh::BindingLocations::MATRICES, mMatricesDescriptorSet );
        bind ( Mesh::BindingLocations::LIGHTS, mLightsDescriptorSet );
        bind ( Mesh::BindingLocations::CLUSTER_PARAMS, mClusterParamsDescriptorSet );
        bind ( Mesh::BindingLocations::GLOBALS, mGlobalsDescriptorSet );
        bind_cluster_storage ( Mesh::BindingLocations::LIGHT_GRID, mFrameLightGrid );
        bind_cluster_storage ( Mesh::BindingLocations::LIGHT_INDEX_LIST, mFrameLightIndexList );
        bind ( Mesh::BindingLocations::SHADOW_PARAMS, mShadowParamsDescriptorSet );
        bind ( Mesh::BindingLocations::SHADOW_MAP, mShadowMapDescriptorSet );
        bind ( Mesh::BindingLocations::SPOT_SHADOW_PARAMS, mSpotShadowParamsDescriptorSet );
        bind ( Mesh::BindingLocations::SPOT_SHADOW_MAP, mSpotShadowMapDescriptorSet );
        bind ( Mesh::BindingLocations::POINT_SHADOW_PARAMS, mPointShadowParamsDescriptorSet );
        bind ( Mesh::BindingLocations::POINT_SHADOW_MAP, mPointShadowMapDescriptorSet );
    }

    void VulkanWindow::BindDepthPrePassSets ( const VulkanPipeline* aPipeline ) const
    {
#ifndef NDEBUG
        // Plus INSTANCE_MATRICES bound by the caller (BindObjectMatrices).
        AssertDescriptorSetsHandled ( aPipeline,
        {
            Mesh::BindingLocations::MATRICES, Mesh::BindingLocations::CLUSTER_PARAMS,
            Mesh::BindingLocations::CLUSTER_ACTIVE, Mesh::BindingLocations::INSTANCE_MATRICES,
        } );
#endif
        auto bind = [&] ( uint32_t aBinding, VkDescriptorSet aSet )
        {
            if ( uint32_t set_index = aPipeline->GetDescriptorSetIndex ( aBinding ); set_index != std::numeric_limits<uint32_t>::max() )
            {
                vkCmdBindDescriptorSets ( GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          aPipeline->GetPipelineLayout(), set_index, 1, &aSet, 0, nullptr );
            }
        };
        bind ( Mesh::BindingLocations::MATRICES, mMatricesDescriptorSet );
        bind ( Mesh::BindingLocations::CLUSTER_PARAMS, mClusterParamsDescriptorSet );
        if ( mFrameClusterActive.GetMemoryPoolBuffer() != nullptr )
        {
            if ( uint32_t active_set_index = aPipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::CLUSTER_ACTIVE ); active_set_index != std::numeric_limits<uint32_t>::max() )
            {
                const VulkanStorageMemoryPoolBuffer* memory_pool_buffer =
                    reinterpret_cast<const VulkanStorageMemoryPoolBuffer*> ( mFrameClusterActive.GetMemoryPoolBuffer() );
                uint32_t dynamic_offset = 0;
                vkCmdBindDescriptorSets ( GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          aPipeline->GetPipelineLayout(), active_set_index, 1,
                                          &memory_pool_buffer->GetDescriptorSet ( mFrameClusterActive.GetOffset() ), 1, &dynamic_offset );
            }
        }
    }

    void VulkanWindow::BindShadowPassSets ( const VulkanPipeline* aPipeline ) const
    {
#ifndef NDEBUG
        // Plus INSTANCE_MATRICES bound by the caller (BindObjectMatrices).
        AssertDescriptorSetsHandled ( aPipeline,
        {
            Mesh::BindingLocations::SHADOW_PARAMS, Mesh::BindingLocations::INSTANCE_MATRICES,
        } );
#endif
        if ( uint32_t shadow_params_set_index = aPipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::SHADOW_PARAMS ); shadow_params_set_index != std::numeric_limits<uint32_t>::max() )
        {
            // Point passes bind their caster's six-face matrix set, spot passes
            // their slot's set, the directional pass the directional set.
            const VkDescriptorSet shadow_depth_set = mInPointShadowPass
                ? mPointShadowDepthMatricesDescriptorSets[mCurrentPointShadowCaster]
                : mInSpotShadowPass
                ? mSpotShadowDepthMatricesDescriptorSets[mCurrentSpotShadowSlot]
                : mShadowParamsDescriptorSet;
            vkCmdBindDescriptorSets ( GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      aPipeline->GetPipelineLayout(), shadow_params_set_index, 1, &shadow_depth_set, 0, nullptr );
        }
    }

    void VulkanWindow::BindObjectMatrices ( const VulkanPipeline* aPipeline, std::span<const Matrix4x4> aMatrices ) const
    {
        uint32_t set_index = aPipeline->GetDescriptorSetIndex ( Mesh::BindingLocations::INSTANCE_MATRICES );
        if ( set_index == std::numeric_limits<uint32_t>::max() )
        {
            return;
        }
        const size_t size = aMatrices.size() * sizeof ( float ) * 16;
        // High-frequency per-draw allocation. Each allocation gets its own
        // descriptor set whose buffer range is exactly this allocation, bound
        // with a zero dynamic offset: a dynamic offset combined with a
        // VK_WHOLE_SIZE range is illegal (VUID-vkCmdBindDescriptorSets-
        // pDescriptorSets-06715). The descriptor pool grows on demand, so the
        // number of batched draws is not bounded by a fixed pool capacity.
        BufferAccessor object_matrices = mStorageMemoryPoolBuffer.Allocate ( size );
        object_matrices.WriteMemory ( 0, size, aMatrices.data() );
        const VulkanStorageMemoryPoolBuffer* memory_pool_buffer =
            reinterpret_cast<const VulkanStorageMemoryPoolBuffer*> ( object_matrices.GetMemoryPoolBuffer() );
        size_t offset = object_matrices.GetOffset();
        uint32_t dynamic_offset = 0;
        vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  aPipeline->GetPipelineLayout(),
                                  set_index,
                                  1,
                                  &memory_pool_buffer->GetDescriptorSet ( offset ), 1, &dynamic_offset );
    }

    void VulkanWindow::RenderCommon ( std::span<const Matrix4x4> aModelMatrices,
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
        if ( aModelMatrices.empty() )
        {
            return;
        }
        // Resolve the optional pre-skinned vertex buffer produced by the compute
        // skinning pre-pass. When present it is bound as the vertex input in
        // place of the mesh's rest-pose vertices (the index buffer still comes
        // from the mesh). Batched instancing is never skinned, so those callers
        // pass a null accessor.
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
        // Select the pipeline for this pass. The shadow and depth-pre passes
        // substitute renderer-owned pipelines (shadow-depth / cluster-mark) that
        // transform geometry by the light view-projection or record cluster
        // occupancy and ignore the item's own pipeline/material; the shading
        // pass uses the item's pipeline. Point shadow passes are drawn against
        // the multiview render pass the pipeline must be created against.
        const VulkanPipeline* pipeline = nullptr;
        switch ( aRenderPass )
        {
        case RenderPass::ShadowPass:
            pipeline = mVulkanRenderer.GetVulkanPipeline ( mInPointShadowPass ? mPointShadowDepthPipeline : mShadowDepthPipeline, mInPointShadowPass ? mVkPointShadowRenderPass : mVkShadowRenderPass );
            break;
        case RenderPass::DepthPrePass:
            pipeline = mVulkanRenderer.GetVulkanPipeline ( mClusterMarkPipeline );
            break;
        default:
            pipeline = mVulkanRenderer.GetVulkanPipeline ( aPipeline );
            break;
        }
        vkCmdBindPipeline ( mVkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline() );
        vkCmdSetPrimitiveTopology ( mVkCommandBuffer, TopologyMap.at ( aTopology ) );
        // Bind the engine-owned descriptor sets this pass needs. Shared with the
        // sibling draw path so the set list has a single definition.
        switch ( aRenderPass )
        {
        case RenderPass::ShadowPass:
            BindShadowPassSets ( pipeline );
            break;
        case RenderPass::DepthPrePass:
            BindDepthPrePassSets ( pipeline );
            break;
        default:
            BindShadingPassSets ( pipeline );
            break;
        }
        // Object transform: a single matrix can take the pipeline's push-constant
        // fast path; otherwise (multiple instances, or a pipeline without a model
        // push constant) every instance is driven from the per-object matrix
        // buffer (InstanceMatrices), indexed by gl_InstanceIndex.
        if ( const VkPushConstantRange& push_constant_model_matrix = pipeline->GetPushConstantModelMatrix() ; push_constant_model_matrix.size != 0 && aModelMatrices.size() == 1 )
        {
            vkCmdPushConstants ( mVkCommandBuffer,
                                 pipeline->GetPipelineLayout(),
                                 push_constant_model_matrix.stageFlags,
                                 push_constant_model_matrix.offset, push_constant_model_matrix.size,
                                 aModelMatrices[0].GetMatrix4x4() );
        }
        else
        {
            BindObjectMatrices ( pipeline, aModelMatrices );
        }
        // Material state applies only to the shading pass; the substituted
        // shadow/depth pipelines ignore it.
        if ( aRenderPass != RenderPass::ShadowPass && aRenderPass != RenderPass::DepthPrePass && aMaterial != nullptr )
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

    void VulkanWindow::RenderInstanced ( std::span<const Matrix4x4> aModelMatrices,
                                         const Mesh& aMesh,
                                         const Pipeline& aPipeline,
                                         const Material* aMaterial,
                                         Topology aTopology,
                                         uint32_t aVertexStart,
                                         uint32_t aVertexCount,
                                         RenderPass aRenderPass ) const
    {
        // One draw per matrix in the span: instance count is the span size and
        // each instance reads its own transform from InstanceMatrices. Batches
        // are never skinned, so no pre-skinned vertex buffer is forwarded.
        RenderCommon ( aModelMatrices, aMesh, aPipeline, aMaterial, aTopology,
                       aVertexStart, aVertexCount,
                       static_cast<uint32_t> ( aModelMatrices.size() ), 0, nullptr, aRenderPass );
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
        const VkDevice device = mVulkanRenderer.GetDevice();
        for ( auto& i : mVkFramebuffers )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyFramebuffer ( device, i, nullptr );
                i = VK_NULL_HANDLE;
            }
        }
        if ( mVkHdrFramebuffer != VK_NULL_HANDLE )
        {
            vkDestroyFramebuffer ( device, mVkHdrFramebuffer, nullptr );
            mVkHdrFramebuffer = VK_NULL_HANDLE;
        }
        if ( mTonemapDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( device, mTonemapDescriptorPool, nullptr );
            mTonemapDescriptorPool = VK_NULL_HANDLE;
            mTonemapDescriptorSet = VK_NULL_HANDLE;
        }
        if ( mVkHdrColorImageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( device, mVkHdrColorImageView, nullptr );
            mVkHdrColorImageView = VK_NULL_HANDLE;
        }
        if ( mVkHdrColorImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( device, mVkHdrColorImage, nullptr );
            mVkHdrColorImage = VK_NULL_HANDLE;
        }
        if ( mVkHdrColorImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( device, mVkHdrColorImageMemory, nullptr );
            mVkHdrColorImageMemory = VK_NULL_HANDLE;
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
