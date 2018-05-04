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
#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"
#include "VulkanMesh.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <array>
#include <utility>
#include "aeongames/Frustum.h"
#include "aeongames/ModelInstance.h"
#include "aeongames/AABB.h"
#include "aeongames/Scene.h"

namespace AeonGames
{
    VulkanWindow::VulkanWindow ( void* aWindowId, const std::shared_ptr<const VulkanRenderer>&  aVulkanRenderer ) :
        mWindowId ( aWindowId ), mVulkanRenderer ( aVulkanRenderer )
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

    VulkanWindow::~VulkanWindow()
    {
        Finalize();
    }

    void VulkanWindow::InitializeSurface()
    {
#if defined ( VK_USE_PLATFORM_WIN32_KHR )
        RECT rect;
        GetClientRect ( reinterpret_cast<HWND> ( mWindowId ), &rect );
        mVkViewport.x = 0.0f;
        mVkViewport.y = 0.0f;
        mVkViewport.width = static_cast<float> ( rect.right );
        mVkViewport.height = static_cast<float> ( rect.bottom );
        mVkViewport.minDepth = 0.0f;
        mVkViewport.maxDepth = 1.0f;

        mVkScissor.offset.x = 0;
        mVkScissor.offset.y = 0;
        mVkScissor.extent.width = rect.right;
        mVkScissor.extent.height = rect.bottom;

        VkWin32SurfaceCreateInfoKHR win32_surface_create_info_khr {};
        win32_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create_info_khr.hwnd = reinterpret_cast<HWND> ( mWindowId );
        win32_surface_create_info_khr.hinstance = reinterpret_cast<HINSTANCE> ( GetWindowLongPtr ( win32_surface_create_info_khr.hwnd, GWLP_HINSTANCE ) );
        if ( VkResult result = vkCreateWin32SurfaceKHR ( mVulkanRenderer->GetInstance(), &win32_surface_create_info_khr, nullptr, &mVkSurfaceKHR ) )
        {
            std::ostringstream stream;
            stream << "Call to vkCreateWin32SurfaceKHR failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
#elif defined( VK_USE_PLATFORM_XLIB_KHR )
        VkXlibSurfaceCreateInfoKHR xlib_surface_create_info_khr {};
        xlib_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        xlib_surface_create_info_khr.dpy = XOpenDisplay ( nullptr );
        xlib_surface_create_info_khr.window = reinterpret_cast<::Window> ( mWindowId );
        if ( VkResult result = vkCreateXlibSurfaceKHR ( mVulkanRenderer->GetInstance(), &xlib_surface_create_info_khr, nullptr, &mVkSurfaceKHR ) )
        {
            std::ostringstream stream;
            stream << "Call to vkCreateXlibSurfaceKHR failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
#endif
        VkBool32 wsi_supported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR ( mVulkanRenderer->GetPhysicalDevice(), mVulkanRenderer->GetQueueFamilyIndex(), mVkSurfaceKHR, &wsi_supported );
        if ( !wsi_supported )
        {
            std::ostringstream stream;
            stream << "WSI not supported.";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanWindow::InitializeSwapchain()
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( mVulkanRenderer->GetPhysicalDevice(), mVkSurfaceKHR, &mVkSurfaceCapabilitiesKHR );
        uint32_t surface_format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR ( mVulkanRenderer->GetPhysicalDevice(), mVkSurfaceKHR, &surface_format_count, nullptr );
        if ( surface_format_count == 0 )
        {
            std::ostringstream stream;
            stream << "Physical device reports no surface formats.";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkSurfaceFormatKHR surface_format_khr;
        std::vector<VkSurfaceFormatKHR> surface_format_list ( surface_format_count );
        vkGetPhysicalDeviceSurfaceFormatsKHR ( mVulkanRenderer->GetPhysicalDevice(), mVkSurfaceKHR, &surface_format_count, surface_format_list.data() );
        if ( surface_format_list[0].format == VK_FORMAT_UNDEFINED )
        {
            surface_format_khr = mVulkanRenderer->GetSurfaceFormatKHR();
        }
        else
        {
            surface_format_khr = surface_format_list[0];
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
        swapchain_create_info.imageFormat = surface_format_khr.format;
        swapchain_create_info.imageColorSpace = surface_format_khr.colorSpace;
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
        swapchain_create_info.oldSwapchain = mVkSwapchainKHR; // Used for Resising.
        {
            uint32_t present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR ( mVulkanRenderer->GetPhysicalDevice(), mVkSurfaceKHR, &present_mode_count, nullptr );
            std::vector<VkPresentModeKHR> present_mode_list ( present_mode_count );
            vkGetPhysicalDeviceSurfacePresentModesKHR ( mVulkanRenderer->GetPhysicalDevice(), mVkSurfaceKHR, &present_mode_count, present_mode_list.data() );
            for ( auto& i : present_mode_list )
            {
                if ( i == VK_PRESENT_MODE_MAILBOX_KHR )
                {
                    swapchain_create_info.presentMode = i;
                    break;
                }
            }
        }
        if ( VkResult result = vkCreateSwapchainKHR ( mVulkanRenderer->GetDevice(), &swapchain_create_info, nullptr, &mVkSwapchainKHR ) )
        {
            std::ostringstream stream;
            stream << "Call to vkCreateSwapchainKHR failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        if ( swapchain_create_info.oldSwapchain != VK_NULL_HANDLE )
        {
            vkDestroySwapchainKHR ( mVulkanRenderer->GetDevice(), swapchain_create_info.oldSwapchain, nullptr );
        }
    }

    void VulkanWindow::InitializeImageViews()
    {
        if ( VkResult result = vkGetSwapchainImagesKHR ( mVulkanRenderer->GetDevice(), mVkSwapchainKHR, &mSwapchainImageCount, nullptr ) )
        {
            std::ostringstream stream;
            stream << "Get swapchain image count failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        mVkSwapchainImages.resize ( mSwapchainImageCount );
        mVkSwapchainImageViews.resize ( mSwapchainImageCount );
        if ( VkResult result = vkGetSwapchainImagesKHR ( mVulkanRenderer->GetDevice(),
                               mVkSwapchainKHR,
                               &mSwapchainImageCount,
                               mVkSwapchainImages.data() ) )
        {
            std::ostringstream stream;
            stream << "Get swapchain images failed: ( " << GetVulkanResultString ( result ) << " )";
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
            image_view_create_info.format = mVulkanRenderer->GetSurfaceFormatKHR().format;
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            vkCreateImageView ( mVulkanRenderer->GetDevice(), &image_view_create_info, nullptr, &mVkSwapchainImageViews[i] );
        }
    }

    void VulkanWindow::InitializeDepthStencil()
    {
        mHasStencil =
            ( mVulkanRenderer->GetDepthStencilFormat() == VK_FORMAT_D32_SFLOAT_S8_UINT ||
              mVulkanRenderer->GetDepthStencilFormat() == VK_FORMAT_D24_UNORM_S8_UINT ||
              mVulkanRenderer->GetDepthStencilFormat() == VK_FORMAT_D16_UNORM_S8_UINT );
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.flags = 0;
        image_create_info.format = mVulkanRenderer->GetDepthStencilFormat();
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
        if ( VkResult result = vkCreateImage ( mVulkanRenderer->GetDevice(), &image_create_info, nullptr, &mVkDepthStencilImage ) )
        {
            std::ostringstream stream;
            stream << "Call to vkCreateImage failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements ( mVulkanRenderer->GetDevice(), mVkDepthStencilImage, &memory_requirements );

        auto required_bits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t memory_index = UINT32_MAX;
        for ( uint32_t i = 0; i < mVulkanRenderer->GetPhysicalDeviceMemoryProperties().memoryTypeCount; ++i )
        {
            if ( memory_requirements.memoryTypeBits & ( 1 << i ) )
            {
                if ( ( mVulkanRenderer->GetPhysicalDeviceMemoryProperties().memoryTypes[i].propertyFlags & required_bits ) == required_bits )
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
            throw std::runtime_error ( stream.str().c_str() );
        }
        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = memory_index;
        vkAllocateMemory ( mVulkanRenderer->GetDevice(), &memory_allocate_info, nullptr, &mVkDepthStencilImageMemory );
        vkBindImageMemory ( mVulkanRenderer->GetDevice(), mVkDepthStencilImage, mVkDepthStencilImageMemory, 0 );

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = mVkDepthStencilImage;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = mVulkanRenderer->GetDepthStencilFormat();
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | ( mHasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        vkCreateImageView ( mVulkanRenderer->GetDevice(), &image_view_create_info, nullptr, &mVkDepthStencilImageView );
    }

    void VulkanWindow::InitializeFrameBuffers()
    {
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
            framebuffer_create_info.renderPass = mVulkanRenderer->GetRenderPass();
            framebuffer_create_info.attachmentCount = static_cast<uint32_t> ( attachments.size() );
            framebuffer_create_info.pAttachments = attachments.data();
            framebuffer_create_info.width = mVkSurfaceCapabilitiesKHR.currentExtent.width;
            framebuffer_create_info.height = mVkSurfaceCapabilitiesKHR.currentExtent.height;
            framebuffer_create_info.layers = 1;
            vkCreateFramebuffer ( mVulkanRenderer->GetDevice(), &framebuffer_create_info, nullptr, &mVkFramebuffers[i] );
        }
    }

    const uint32_t& VulkanWindow::GetActiveImageIndex() const
    {
        return mActiveImageIndex;
    }

    const VkFramebuffer & VulkanWindow::GetActiveFrameBuffer() const
    {
        return mVkFramebuffers[mActiveImageIndex];
    }

    void VulkanWindow::AcquireNextImage()
    {
        if ( VkResult result = vkAcquireNextImageKHR (
                                   mVulkanRenderer->GetDevice(),
                                   mVkSwapchainKHR,
                                   UINT64_MAX, VK_NULL_HANDLE,
                                   mVulkanRenderer->GetFence(),
                                   const_cast<uint32_t*> ( &mActiveImageIndex ) ) )
        {
            std::cout << GetVulkanResultString ( result ) << std::endl;
        }
    }

    const VkViewport & VulkanWindow::GetViewport() const
    {
        return mVkViewport;
    }

    const VkRect2D & VulkanWindow::GetScissor() const
    {
        return mVkScissor;
    }

    void VulkanWindow::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        InitializeSurface();
        InitializeSwapchain();
        InitializeImageViews();
        InitializeDepthStencil();
        InitializeFrameBuffers();
    }

    void VulkanWindow::Finalize()
    {
        if ( VkResult result = vkQueueWaitIdle ( mVulkanRenderer->GetQueue() ) )
        {
            std::cerr << "vkQueueWaitIdle failed: " << GetVulkanResultString ( result );
        }
        if ( VkResult result = vkDeviceWaitIdle ( mVulkanRenderer->GetDevice() ) )
        {
            std::cerr << "vkDeviceWaitIdle failed: " << GetVulkanResultString ( result );
        }
        FinalizeFrameBuffers();
        FinalizeDepthStencil();
        FinalizeImageViews();
        FinalizeSwapchain();
        FinalizeSurface();
    }

    void VulkanWindow::ResizeViewport ( uint32_t aWidth, uint32_t aHeight )
    {
        if ( ( aWidth && aHeight ) &&
             ( mVkSurfaceCapabilitiesKHR.currentExtent.width != aWidth ||
               mVkSurfaceCapabilitiesKHR.currentExtent.height != aHeight ) )
        {
            if ( VkResult result = vkQueueWaitIdle ( mVulkanRenderer->GetQueue() ) )
            {
                std::ostringstream stream;
                stream << "vkQueueWaitIdle failed: " << GetVulkanResultString ( result );
                throw std::runtime_error ( stream.str().c_str() );
            }

            if ( VkResult result = vkDeviceWaitIdle ( mVulkanRenderer->GetDevice() ) )
            {
                std::ostringstream stream;
                stream << "vkDeviceWaitIdle failed: " << GetVulkanResultString ( result );
                throw std::runtime_error ( stream.str().c_str() );
            }

            mVkViewport.width = static_cast<float> ( aWidth );
            mVkViewport.height = static_cast<float> ( aHeight );
            mVkScissor.extent.width = aWidth;
            mVkScissor.extent.height = aHeight;

            FinalizeFrameBuffers();
            FinalizeDepthStencil();
            FinalizeImageViews();
            InitializeSwapchain();
            InitializeImageViews();
            InitializeDepthStencil();
            InitializeFrameBuffers();
        }
    }

    void VulkanWindow::BeginRender() const
    {
        if ( VkResult result = vkAcquireNextImageKHR (
                                   mVulkanRenderer->GetDevice(),
                                   mVkSwapchainKHR,
                                   UINT64_MAX, VK_NULL_HANDLE,
                                   mVulkanRenderer->GetFence(),
                                   const_cast<uint32_t*> ( &mActiveImageIndex ) ) )
        {
            std::cout << GetVulkanResultString ( result ) << std::endl;
        }

        if ( VkResult result = vkWaitForFences ( mVulkanRenderer->GetDevice(), 1,
                               &mVulkanRenderer->GetFence(),
                               VK_TRUE, UINT64_MAX ) )
        {
            std::cout << GetVulkanResultString ( result ) << std::endl;
        }
        if ( VkResult result = vkResetFences ( mVulkanRenderer->GetDevice(), 1,
                                               &mVulkanRenderer->GetFence() ) )
        {
            std::cout << GetVulkanResultString ( result ) << std::endl;
        }
        if ( VkResult result = vkQueueWaitIdle ( mVulkanRenderer->GetQueue() ) )
        {
            std::cout << GetVulkanResultString ( result ) << std::endl;
        }

        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if ( VkResult result = vkBeginCommandBuffer ( mVulkanRenderer->GetCommandBuffer(), &command_buffer_begin_info ) )
        {
            std::cout << GetVulkanResultString ( result ) << std::endl;
        }

        vkCmdSetViewport ( mVulkanRenderer->GetCommandBuffer(), 0, 1, &GetViewport() );
        vkCmdSetScissor ( mVulkanRenderer->GetCommandBuffer(), 0, 1, &GetScissor() );

        VkRect2D render_area{ { 0, 0 }, { GetWidth(), GetHeight() } };
        /* [1] is depth/stencil [0] is color.*/
        std::array<VkClearValue, 2> clear_values{ { { {{0}} }, { {{0}} } } };
        clear_values[0].color.float32[0] = 0.5f;
        clear_values[0].color.float32[1] = 0.5f;
        clear_values[0].color.float32[2] = 0.5f;
        clear_values[0].color.float32[3] = 0.0f;
        clear_values[1].depthStencil.depth = 1.0f;
        clear_values[1].depthStencil.stencil = 0;

        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = mVulkanRenderer->GetRenderPass();
        render_pass_begin_info.framebuffer = GetActiveFrameBuffer();
        render_pass_begin_info.renderArea = render_area;
        render_pass_begin_info.clearValueCount = static_cast<uint32_t> ( clear_values.size() );
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass ( mVulkanRenderer->GetCommandBuffer(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );
    }

    void VulkanWindow::EndRender() const
    {
        vkCmdEndRenderPass ( mVulkanRenderer->GetCommandBuffer() );
        if ( VkResult result = vkEndCommandBuffer ( mVulkanRenderer->GetCommandBuffer() ) )
        {
            std::cout << GetVulkanResultString ( result ) << std::endl;
        }
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = nullptr;
        submit_info.pWaitDstStageMask = nullptr;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &mVulkanRenderer->GetCommandBuffer();
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &mVulkanRenderer->GetSignalSemaphore();
        if ( VkResult result = vkQueueSubmit ( mVulkanRenderer->GetQueue(), 1, &submit_info, VK_NULL_HANDLE ) )
        {
            std::cout << GetVulkanResultString ( result ) << std::endl;
        }
        std::array<VkResult, 1> result_array{ { VkResult::VK_SUCCESS } };
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &mVulkanRenderer->GetSignalSemaphore();
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &GetSwapchain();
        present_info.pImageIndices = &GetActiveImageIndex();
        present_info.pResults = result_array.data();
        if ( VkResult result = vkQueuePresentKHR ( mVulkanRenderer->GetQueue(), &present_info ) )
        {
            std::cout << GetVulkanResultString ( result ) << std::endl;
        }
    }

    void VulkanWindow::Render ( const Scene* aScene ) const
    {
#if 0
        Matrix4x4 view_matrix { mViewTransform.GetInverted().GetMatrix() };
        Frustum frustum ( mProjectionMatrix * view_matrix );
        aScene->LoopTraverseDFSPreOrder ( [this, &frustum, &view_matrix] ( Node & aNode )
        {
            if ( frustum.Intersects ( aNode.GetGlobalAABB() ) )
            {
                mVulkanRenderer->Render ( aNode, mProjectionMatrix, view_matrix );
            }
        } );
#endif
    }

    void VulkanWindow::Render ( const Mesh& aMesh ) const
    {
        const Mesh::IRenderMesh* render_mesh = aMesh.GetRenderMesh();
        if ( render_mesh )
        {
            render_mesh->Render();
        }
        else
        {
            aMesh.SetRenderMesh ( std::make_unique<VulkanMesh> ( aMesh, mVulkanRenderer ) );
        }
    }

    const VkSwapchainKHR& VulkanWindow::GetSwapchain() const
    {
        return mVkSwapchainKHR;
    }

    uint32_t VulkanWindow::GetWidth() const
    {
        return mVkSurfaceCapabilitiesKHR.currentExtent.width;
    }

    uint32_t VulkanWindow::GetHeight() const
    {
        return mVkSurfaceCapabilitiesKHR.currentExtent.height;
    }

    void VulkanWindow::FinalizeSurface()
    {
        if ( mVkSurfaceKHR != VK_NULL_HANDLE )
        {
            vkDestroySurfaceKHR ( mVulkanRenderer->GetInstance(), mVkSurfaceKHR, nullptr );
        }
    }

    void VulkanWindow::FinalizeSwapchain()
    {
        if ( mVkSwapchainKHR != VK_NULL_HANDLE )
        {
            vkDestroySwapchainKHR ( mVulkanRenderer->GetDevice(), mVkSwapchainKHR, nullptr );
        }
    }

    void VulkanWindow::FinalizeImageViews()
    {
        for ( auto& i : mVkSwapchainImageViews )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyImageView ( mVulkanRenderer->GetDevice(), i, nullptr );
            }
        }
    }

    void VulkanWindow::FinalizeDepthStencil()
    {
        if ( mVkDepthStencilImageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( mVulkanRenderer->GetDevice(), mVkDepthStencilImageView, nullptr );
        }
        if ( mVkDepthStencilImageMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVulkanRenderer->GetDevice(), mVkDepthStencilImageMemory, nullptr );
        }

        if ( mVkDepthStencilImage != VK_NULL_HANDLE )
        {
            vkDestroyImage ( mVulkanRenderer->GetDevice(), mVkDepthStencilImage, nullptr );
        }
    }

    void VulkanWindow::FinalizeFrameBuffers()
    {
        for ( auto& i : mVkFramebuffers )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyFramebuffer ( mVulkanRenderer->GetDevice(), i, nullptr );
            }
        }
    }
}
