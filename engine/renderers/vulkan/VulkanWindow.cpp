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
#include "VulkanWindow.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"
#include <sstream>
#include <algorithm>
#include <array>

namespace AeonGames
{
    VulkanWindow::VulkanWindow ( uintptr_t aWindowId, const VulkanRenderer* aVulkanRenderer ) :
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

    uintptr_t VulkanWindow::GetWindowId() const
    {
        return mWindowId;
    }

    void VulkanWindow::CreateSurface()
    {
#if defined ( VK_USE_PLATFORM_WIN32_KHR )
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
        xlib_surface_create_info_khr.window = mWindowId;
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
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( mVulkanRenderer->GetPhysicalDevice(), mVkSurfaceKHR, &mVkSurfaceCapabilitiesKHR );

        uint32_t surface_format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR ( mVulkanRenderer->GetPhysicalDevice(), mVkSurfaceKHR, &surface_format_count, nullptr );
        if ( surface_format_count == 0 )
        {
            std::ostringstream stream;
            stream << "Physical device reports no surface formats.";
            throw std::runtime_error ( stream.str().c_str() );
        }
        std::vector<VkSurfaceFormatKHR> surface_format_list ( surface_format_count );
        vkGetPhysicalDeviceSurfaceFormatsKHR ( mVulkanRenderer->GetPhysicalDevice(), mVkSurfaceKHR, &surface_format_count, surface_format_list.data() );
        if ( surface_format_list[0].format == VK_FORMAT_UNDEFINED )
        {
            mVkSurfaceFormatKHR.format = VK_FORMAT_B8G8R8A8_UNORM;
            mVkSurfaceFormatKHR.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }
        else
        {
            mVkSurfaceFormatKHR = surface_format_list[0];
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
    }

    void VulkanWindow::CreateSwapchain()
    {
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
            {
                std::ostringstream stream;
                stream << "Call to vkCreateSwapchainKHR failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        if ( swapchain_create_info.oldSwapchain != VK_NULL_HANDLE )
        {
            vkDestroySwapchainKHR ( mVulkanRenderer->GetDevice(), swapchain_create_info.oldSwapchain, nullptr );
        }
    }

    void VulkanWindow::CreateImageViews()
    {
        vkGetSwapchainImagesKHR ( mVulkanRenderer->GetDevice(), mVkSwapchainKHR, &mSwapchainImageCount, nullptr );
        mVkSwapchainImages.resize ( mSwapchainImageCount );
        mVkSwapchainImageViews.resize ( mSwapchainImageCount );
        vkGetSwapchainImagesKHR ( mVulkanRenderer->GetDevice(),
                                  mVkSwapchainKHR,
                                  &mSwapchainImageCount,
                                  mVkSwapchainImages.data() );
        for ( uint32_t i = 0; i < mSwapchainImageCount; ++i )
        {
            VkImageViewCreateInfo image_view_create_info{};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
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
            vkCreateImageView ( mVulkanRenderer->GetDevice(), &image_view_create_info, nullptr, &mVkSwapchainImageViews[i] );
        }
    }

    void VulkanWindow::CreateDepthStencil()
    {
        {
            std::array<VkFormat, 5> try_formats
            {
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D16_UNORM
            };
            for ( auto format : try_formats )
            {
                VkFormatProperties format_properties{};
                vkGetPhysicalDeviceFormatProperties ( mVulkanRenderer->GetPhysicalDevice(), format, &format_properties );
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
                throw std::runtime_error ( stream.str().c_str() );
            }
            mHasStencil =
                ( mVkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                  mVkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
                  mVkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT );
        }
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
        image_create_info.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
        image_create_info.pQueueFamilyIndices = nullptr;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkCreateImage ( mVulkanRenderer->GetDevice(), &image_create_info, nullptr, &mVkDepthStencilImage );

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
        vkCreateImageView ( mVulkanRenderer->GetDevice(), &image_view_create_info, nullptr, &mVkDepthStencilImageView );
    }

    void VulkanWindow::CreateRenderPass()
    {
        std::array<VkAttachmentDescription, 2> attachment_descriptions{};
        attachment_descriptions[0].flags = 0;
        attachment_descriptions[0].format = mVkDepthStencilFormat;
        attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachment_descriptions[1].flags = 0;
        attachment_descriptions[1].format = mVkSurfaceFormatKHR.format;
        attachment_descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference depth_stencil_attachment_reference{};
        depth_stencil_attachment_reference.attachment = 0;
        depth_stencil_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        std::array<VkAttachmentReference, 1> color_attachment_references{};
        color_attachment_references[0].attachment = 1;
        color_attachment_references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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


        VkRenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = static_cast<uint32_t> ( attachment_descriptions.size() );
        render_pass_create_info.pAttachments = attachment_descriptions.data();
        render_pass_create_info.dependencyCount = 0;
        render_pass_create_info.pDependencies = nullptr;
        render_pass_create_info.subpassCount = static_cast<uint32_t> ( subpass_descriptions.size() );
        render_pass_create_info.pSubpasses = subpass_descriptions.data();
        vkCreateRenderPass ( mVulkanRenderer->GetDevice(), &render_pass_create_info, nullptr, &mVkRenderPass );
    }

    void VulkanWindow::CreateFrameBuffers()
    {
        mVkFramebuffers.resize ( mSwapchainImageCount );
        for ( uint32_t i = 0; i < mSwapchainImageCount; ++i )
        {
            std::array<VkImageView, 2> attachments
            {
                mVkDepthStencilImageView,
                mVkSwapchainImageViews[i]
            };
            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = mVkRenderPass;
            framebuffer_create_info.attachmentCount = static_cast<uint32_t> ( attachments.size() );
            framebuffer_create_info.pAttachments = attachments.data();
            framebuffer_create_info.width = mVkSurfaceCapabilitiesKHR.currentExtent.width;
            framebuffer_create_info.height = mVkSurfaceCapabilitiesKHR.currentExtent.height;
            framebuffer_create_info.layers = 1;
            vkCreateFramebuffer ( mVulkanRenderer->GetDevice(), &framebuffer_create_info, nullptr, &mVkFramebuffers[i] );
        }
    }

    void VulkanWindow::CreateCommandPool()
    {
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.queueFamilyIndex = mVulkanRenderer->GetQueueFamilyIndex();
        vkCreateCommandPool ( mVulkanRenderer->GetDevice(), &command_pool_create_info, nullptr, &mVkCommandPool );

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = mVkCommandPool;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1;
        vkAllocateCommandBuffers ( mVulkanRenderer->GetDevice(), &command_buffer_allocate_info, &mVkCommandBuffer );
    }

    void VulkanWindow::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        CreateSurface();
        CreateSwapchain();
        CreateImageViews();
        CreateDepthStencil();
        CreateRenderPass();
        CreateFrameBuffers();
        CreateCommandPool();
    }

    void VulkanWindow::Finalize()
    {
        DestroyCommandPool();
        DestroyFrameBuffers();
        DestroyRenderPass();
        DestroyDepthStencil();
        DestroyImageViews();
        DestroySwapchain();
        DestroySurface();
    }

    void VulkanWindow::BeginRender() const
    {
        vkAcquireNextImageKHR (
            mVulkanRenderer->GetDevice(),
            mVkSwapchainKHR,
            UINT64_MAX, VK_NULL_HANDLE,
            mVulkanRenderer->GetFence(),
            const_cast<uint32_t*> ( &mActiveImageIndex ) );
        vkWaitForFences ( mVulkanRenderer->GetDevice(), 1,
                          &mVulkanRenderer->GetFence(),
                          VK_TRUE, UINT64_MAX );
        vkResetFences ( mVulkanRenderer->GetDevice(), 1,
                        &mVulkanRenderer->GetFence() );
        /** @todo This will probably break for more than one Window, revisit!. */
        vkQueueWaitIdle ( mVulkanRenderer->GetQueue() );
        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer ( mVkCommandBuffer, &command_buffer_begin_info );

        VkRect2D render_area{ { 0, 0 }, mVkSurfaceCapabilitiesKHR.currentExtent };
        /* [0] is depth/stencil [1] is color.*/
        std::array<VkClearValue, 2> clear_values{ { { 0 }, { 0 } } };
        clear_values[1].color.float32[0] = 0.5f;
        clear_values[1].color.float32[1] = 0.5f;
        clear_values[1].color.float32[2] = 0.5f;
        clear_values[1].color.float32[3] = 0.0f;
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = mVkRenderPass;
        render_pass_begin_info.framebuffer = mVkFramebuffers[mActiveImageIndex];
        render_pass_begin_info.renderArea = render_area;
        render_pass_begin_info.clearValueCount = static_cast<uint32_t> ( clear_values.size() );
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass ( mVkCommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );
    }

    void VulkanWindow::EndRender() const
    {
        vkCmdEndRenderPass ( mVkCommandBuffer );
        vkEndCommandBuffer ( mVkCommandBuffer );

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = nullptr;
        submit_info.pWaitDstStageMask = nullptr;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &mVkCommandBuffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &mVulkanRenderer->GetSemaphore();
        vkQueueSubmit ( mVulkanRenderer->GetQueue(), 1, &submit_info, VK_NULL_HANDLE );

        VkResult result = VkResult::VK_RESULT_MAX_ENUM;
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &mVulkanRenderer->GetSemaphore();
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &mVkSwapchainKHR;
        present_info.pImageIndices = &mActiveImageIndex;
        present_info.pResults = &result;
        vkQueuePresentKHR ( mVulkanRenderer->GetQueue(), &present_info );
    }

    void VulkanWindow::Resize ( uint32_t aWidth, uint32_t aHeight )
    {
    }

    void VulkanWindow::DestroySurface()
    {
        if ( mVkSurfaceKHR != VK_NULL_HANDLE )
        {
            vkDestroySurfaceKHR ( mVulkanRenderer->GetInstance(), mVkSurfaceKHR, nullptr );
        }
    }

    void VulkanWindow::DestroySwapchain()
    {
        if ( mVkSwapchainKHR != VK_NULL_HANDLE )
        {
            vkDestroySwapchainKHR ( mVulkanRenderer->GetDevice(), mVkSwapchainKHR, nullptr );
        }
    }

    void VulkanWindow::DestroyImageViews()
    {
        for ( auto& i : mVkSwapchainImageViews )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyImageView ( mVulkanRenderer->GetDevice(), i, nullptr );
            }
        }
    }

    void VulkanWindow::DestroyDepthStencil()
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

    void VulkanWindow::DestroyRenderPass()
    {
        if ( mVkRenderPass != VK_NULL_HANDLE )
        {
            vkDestroyRenderPass ( mVulkanRenderer->GetDevice(), mVkRenderPass, nullptr );
        }
    }

    void VulkanWindow::DestroyFrameBuffers()
    {
        for ( auto& i : mVkFramebuffers )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyFramebuffer ( mVulkanRenderer->GetDevice(), i, nullptr );
            }
        }
    }

    void VulkanWindow::DestroyCommandPool()
    {
        if ( mVkCommandPool != VK_NULL_HANDLE )
        {
            vkDestroyCommandPool ( mVulkanRenderer->GetDevice(), mVkCommandPool, nullptr );
        }
    }

}
