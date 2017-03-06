/*
Copyright (C) 2016,2017 Rodrigo Jose Hernandez Cordoba

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

/*  Reference and Credits:
    Vulkan SDK Demo code.
    https://www.youtube.com/playlist?list=PLUXvZMiAqNbK8jd7s52BIDtCbZnKNGp0P
    https://vulkan.lunarg.com/app/docs/v1.0.8.0/layers
    http://gpuopen.com/using-the-vulkan-validation-layers/?utm_source=silverpop&utm_medium=email&utm_campaign=25324445&utm_term=link-article2&utm_content=p-global-developer-hcnewsflash-april-2016%20%281%29:&spMailingID=25324445&spUserID=NzI5Mzc5ODY4NjQS1&spJobID=783815030&spReportId=NzgzODE1MDMwS0
*/

#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <stdexcept>
#include "aeongames/LogLevel.h"
#include "VulkanRenderer.h"
#include "math/3DMath.h"

namespace AeonGames
{
    static const char* GetVulkanRendererResultString ( VkResult aResult )
    {
        switch ( aResult )
        {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:
            return "VK_ERROR_INVALID_SHADER_NV";
        default:
            return "Unknown Result";
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback (
        VkFlags aFlags,
        VkDebugReportObjectTypeEXT aObjType,
        uint64_t aSrcObject,
        size_t aLocation,
        int32_t aCode,
        const char *aLayerPrefix,
        const char *aMsg,
        void *aUserData )
    {
        std::ostringstream stream;

        stream << "[ ";

        if ( aFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT )
        {
            stream << "INFO ";
        }
        if ( aFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT )
        {
            stream << "WARNING ";
        }
        if ( aFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT )
        {
            stream << "PERFORMANCE ";
        }
        if ( aFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT )
        {
            stream << "ERROR ";
#if _WIN32
            MessageBox ( nullptr, aMsg, aLayerPrefix, MB_ICONERROR | MB_OK );
#endif
        }
        if ( aFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT )
        {
            stream << "DEBUG ";
        }
        stream << "] ";
        stream << aLayerPrefix << ": " << aMsg << std::endl;

        std::cout << stream.str();

        return false;
    }

    VulkanRenderer::VulkanRenderer ( bool aValidate ) : mValidate ( aValidate )
    {
        try
        {
            SetupLayersAndExtensions();
            if ( mValidate )
            {
                SetupDebug();
            }
            InitializeInstance();
            if ( mValidate )
            {
                /* LoadFunctions currently only loads
                    Debug Functions. */
                LoadFunctions();
                InitializeDebug();
            }
            InitializeDevice();
            InitializeSemaphore();
        }
        catch ( ... )
        {
            FinalizeSemaphore();
            FinalizeDevice();
            FinalizeDebug();
            FinalizeInstance();
            throw;
        }
    }

    void VulkanRenderer::LoadFunctions()
    {
        assert ( mVkInstance && "mVkInstance is a nullptr." );
        if ( !mFunctionsLoaded && mVkInstance )
        {
            if ( ( vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT> ( vkGetInstanceProcAddr ( mVkInstance, "vkCreateDebugReportCallbackEXT" ) ) ) == nullptr )
            {
                throw std::runtime_error ( "vkGetInstanceProcAddr failed to load vkCreateDebugReportCallbackEXT" );
            }
            if ( ( vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT> ( vkGetInstanceProcAddr ( mVkInstance, "vkDestroyDebugReportCallbackEXT" ) ) ) == nullptr )
            {
                throw std::runtime_error ( "vkGetInstanceProcAddr failed to load vkDestroyDebugReportCallbackEXT" );
            }
            mFunctionsLoaded = true;
        }
    }


    void VulkanRenderer::SetupDebug()
    {
        mDebugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        mDebugReportCallbackCreateInfo.pfnCallback = DebugCallback;
        mDebugReportCallbackCreateInfo.flags =
            VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
            VK_DEBUG_REPORT_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_ERROR_BIT_EXT |
            VK_DEBUG_REPORT_DEBUG_BIT_EXT;

        mInstanceLayerNames.emplace_back ( "VK_LAYER_LUNARG_standard_validation" );

        mInstanceExtensionNames.emplace_back ( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

        mDeviceLayerNames.emplace_back ( "VK_LAYER_LUNARG_standard_validation" );
    }

    void VulkanRenderer::InitializeDebug()
    {
        if ( VkResult result = vkCreateDebugReportCallbackEXT ( mVkInstance, &mDebugReportCallbackCreateInfo, nullptr, &mVkDebugReportCallbackEXT ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer debug report callback. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::SetupLayersAndExtensions()
    {
        mInstanceExtensionNames.push_back ( VK_KHR_SURFACE_EXTENSION_NAME );
#ifdef VK_USE_PLATFORM_WIN32_KHR
        mInstanceExtensionNames.push_back ( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#endif
        mDeviceExtensionNames.push_back ( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    }

    void VulkanRenderer::InitializeInstance()
    {
        VkInstanceCreateInfo instance_create_info {};
        VkApplicationInfo application_info {};

        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.apiVersion = 0;//VK_API_VERSION_1_0;
        application_info.applicationVersion = VK_MAKE_VERSION ( 0, 1, 0 );
        application_info.pApplicationName = "AeonEngine VulkanRenderer Renderer";

        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.enabledLayerCount = static_cast<uint32_t> ( mInstanceLayerNames.size() );
        instance_create_info.ppEnabledLayerNames = mInstanceLayerNames.data();
        instance_create_info.enabledExtensionCount = static_cast<uint32_t> ( mInstanceExtensionNames.size() );
        instance_create_info.ppEnabledExtensionNames = mInstanceExtensionNames.data();
        if ( mValidate )
        {
            instance_create_info.pNext = &mDebugReportCallbackCreateInfo;
        }
        if ( VkResult result = vkCreateInstance ( &instance_create_info, nullptr, &mVkInstance ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer instance. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::InitializeDevice()
    {
        assert ( mVkInstance && "mVkInstance is a nullptr." );
        {
            uint32_t physical_device_count;
            vkEnumeratePhysicalDevices ( mVkInstance, &physical_device_count, nullptr );

            if ( physical_device_count == 0 )
            {
                throw std::runtime_error ( "No VulkanRenderer physical device found" );
            }

            std::vector<VkPhysicalDevice> physical_device_list ( physical_device_count );
            vkEnumeratePhysicalDevices ( mVkInstance, &physical_device_count, physical_device_list.data() );

            mVkPhysicalDevice = physical_device_list[0];
            vkGetPhysicalDeviceProperties ( mVkPhysicalDevice, &mVkPhysicalDeviceProperties );
            vkGetPhysicalDeviceMemoryProperties ( mVkPhysicalDevice, &mVkPhysicalDeviceMemoryProperties );
        }

        VkDeviceCreateInfo device_create_info{};
        VkDeviceQueueCreateInfo device_queue_create_info{};
        {
            uint32_t family_properties_count;
            vkGetPhysicalDeviceQueueFamilyProperties ( mVkPhysicalDevice, &family_properties_count, nullptr );
            if ( family_properties_count == 0 )
            {
                throw std::runtime_error ( "VulkanRenderer physical device has no queue family properties." );
            }
            std::vector<VkQueueFamilyProperties> family_properties_list ( family_properties_count );
            vkGetPhysicalDeviceQueueFamilyProperties ( mVkPhysicalDevice, &family_properties_count, family_properties_list.data() );
            bool graphics_queue_family_found = false;
            for ( auto family_property = family_properties_list.begin();
                  family_property != family_properties_list.end();
                  ++family_property )
            {
                if ( family_property->queueFlags & VK_QUEUE_GRAPHICS_BIT )
                {
                    graphics_queue_family_found = true;
                    mQueueFamilyIndex =
                        static_cast<uint32_t> ( family_properties_list.begin() - family_property );
                    break;
                }
            }
            if ( !graphics_queue_family_found )
            {
                throw std::runtime_error ( "No graphics queue family found." );
            }
        }

        {
            uint32_t instance_layer_count;
            vkEnumerateInstanceLayerProperties ( &instance_layer_count, nullptr );
            std::vector<VkLayerProperties> instance_layer_list ( instance_layer_count );
            vkEnumerateInstanceLayerProperties ( &instance_layer_count, instance_layer_list.data() );
            std::cout << "VulkanRenderer Instance Layers" << std::endl;
            for ( auto& i : instance_layer_list )
            {
                std::cout << " " << i.layerName << "\t|\t" << i.description << std::endl;
            }
        }

        {
            uint32_t device_layer_count;
            vkEnumerateDeviceLayerProperties ( mVkPhysicalDevice, &device_layer_count, nullptr );
            std::vector<VkLayerProperties> device_layer_list ( device_layer_count );
            vkEnumerateInstanceLayerProperties ( &device_layer_count, device_layer_list.data() );
            std::cout << "VulkanRenderer Device Layers" << std::endl;
            for ( auto& i : device_layer_list )
            {
                std::cout << " " << i.layerName << "\t|\t" << i.description << std::endl;
            }
        }

        /// @todo Remove hardcoded Queue Info
        float queue_priorities[] {1.0f};
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.queueCount = 1;
        device_queue_create_info.queueFamilyIndex = mQueueFamilyIndex;
        device_queue_create_info.pQueuePriorities = queue_priorities;

        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &device_queue_create_info;
        device_create_info.enabledLayerCount = static_cast<uint32_t> ( mDeviceLayerNames.size() );
        device_create_info.ppEnabledLayerNames = mDeviceLayerNames.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t> ( mDeviceExtensionNames.size() );
        device_create_info.ppEnabledExtensionNames = mDeviceExtensionNames.data();

        /// @todo Grab best device rather than first one
        if ( VkResult result = vkCreateDevice ( mVkPhysicalDevice, &device_create_info, nullptr, &mVkDevice ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer device. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        vkGetDeviceQueue ( mVkDevice, mQueueFamilyIndex, 0, &mVkQueue );
    }

    void VulkanRenderer::InitializeSemaphore()
    {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if ( VkResult result = vkCreateSemaphore ( mVkDevice, &semaphore_create_info, nullptr, &mVkSemaphore ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer semaphore. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::FinalizeDebug()
    {
        if ( mVkInstance && ( mVkDebugReportCallbackEXT != VK_NULL_HANDLE ) )
        {
            vkDestroyDebugReportCallbackEXT ( mVkInstance, mVkDebugReportCallbackEXT, nullptr );
            mVkDebugReportCallbackEXT = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeInstance()
    {
        if ( mVkInstance != VK_NULL_HANDLE )
        {
            vkDestroyInstance ( mVkInstance, nullptr );
            mVkInstance = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeDevice()
    {
        if ( mVkDevice != VK_NULL_HANDLE )
        {
            vkDestroyDevice ( mVkDevice, nullptr );
            mVkDevice = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeSemaphore()
    {
        if ( mVkSemaphore != VK_NULL_HANDLE )
        {
            vkDestroySemaphore ( mVkDevice, mVkSemaphore, nullptr );
            mVkSemaphore = VK_NULL_HANDLE;
        }
    }

    VulkanRenderer::~VulkanRenderer()
    {
        vkQueueWaitIdle ( mVkQueue );
        FinalizeSemaphore();
        FinalizeDevice();
        FinalizeDebug();
        FinalizeInstance();
    }

    void VulkanRenderer::BeginRender() const
    {
#if defined ( VK_USE_PLATFORM_WIN32_KHR )
        for ( auto& i : mWindowRegistry )
        {
            vkAcquireNextImageKHR ( mVkDevice, i.mVkSwapchainKHR, UINT64_MAX, VK_NULL_HANDLE, i.mVkFence, const_cast<uint32_t*> ( &i.mActiveImageIndex ) );
            vkWaitForFences ( mVkDevice, 1, &i.mVkFence, VK_TRUE, UINT64_MAX );
            vkResetFences ( mVkDevice, 1, &i.mVkFence );
            /** @todo This will probably break for more than one Window, revisit!. */
            vkQueueWaitIdle ( mVkQueue );
            VkCommandBufferBeginInfo command_buffer_begin_info{};
            command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer ( i.mVkCommandBuffer, &command_buffer_begin_info );

            VkRect2D render_area{ {0, 0}, i.mVkSurfaceCapabilitiesKHR.currentExtent };
            /**@todo These clear values work for now since we want all black,
            the proper fields should be set accordingly if a different color is needed.
            Also which of the color union member is filled depends on the surface format.*/
            std::array<VkClearValue, 2> clear_values{{{0}, {0}}};
            VkRenderPassBeginInfo render_pass_begin_info{};
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.renderPass = i.mVkRenderPass;
            render_pass_begin_info.framebuffer = i.mVkFramebuffers[i.mActiveImageIndex];
            render_pass_begin_info.renderArea = render_area;
            render_pass_begin_info.clearValueCount = static_cast<uint32_t> ( clear_values.size() );
            render_pass_begin_info.pClearValues = clear_values.data();
            vkCmdBeginRenderPass ( i.mVkCommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );
        }
#else
#endif
    }

    void VulkanRenderer::EndRender() const
    {
#if defined ( VK_USE_PLATFORM_WIN32_KHR )
        /** @todo This will probably break for more than one Window, revisit!. */
        for ( auto& i : mWindowRegistry )
        {
            vkCmdEndRenderPass ( i.mVkCommandBuffer );
            vkEndCommandBuffer ( i.mVkCommandBuffer );

            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.waitSemaphoreCount = 0;
            submit_info.pWaitSemaphores = nullptr;
            submit_info.pWaitDstStageMask = nullptr;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &i.mVkCommandBuffer;
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &mVkSemaphore;
            vkQueueSubmit ( mVkQueue, 1, &submit_info, VK_NULL_HANDLE );

            VkResult result = VkResult::VK_RESULT_MAX_ENUM;
            VkPresentInfoKHR present_info{};
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &mVkSemaphore;
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &i.mVkSwapchainKHR;
            present_info.pImageIndices = &i.mActiveImageIndex;
            present_info.pResults = &result;
            vkQueuePresentKHR ( mVkQueue, &present_info );
        }
#else
#endif
    }

    void VulkanRenderer::Render ( const std::shared_ptr<Model> aModel ) const
    {
    }

    bool VulkanRenderer::AllocateModelRenderData ( std::shared_ptr<Model> aModel )
    {
        return false;
    }

    bool VulkanRenderer::AddRenderingWindow ( uintptr_t aWindowId )
    {
        mWindowRegistry.emplace_back();
        mWindowRegistry.back().mWindowId = aWindowId;
#if defined ( VK_USE_PLATFORM_WIN32_KHR )
        VkWin32SurfaceCreateInfoKHR win32_surface_create_info_khr {};
        win32_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create_info_khr.hwnd = reinterpret_cast<HWND> ( mWindowRegistry.back().mWindowId );
        win32_surface_create_info_khr.hinstance = reinterpret_cast<HINSTANCE> ( GetWindowLongPtr ( win32_surface_create_info_khr.hwnd, GWLP_HINSTANCE ) );
        if ( VkResult result = vkCreateWin32SurfaceKHR ( mVkInstance, &win32_surface_create_info_khr, nullptr, &mWindowRegistry.back().mVkSurfaceKHR ) )
        {
            std::cout << LogLevel ( LogLevel::Level::Error ) << "Call to vkCreateWin32SurfaceKHR failed: ( " << GetVulkanRendererResultString ( result ) << " )";
            return false;
        }
        // From here on this is platform dependent code, move it later
        VkBool32 wsi_supported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR ( mVkPhysicalDevice, mQueueFamilyIndex, mWindowRegistry.back().mVkSurfaceKHR, &wsi_supported );
        if ( !wsi_supported )
        {
            assert ( 0 && "WSI not supported." );
            return false;
        }

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( mVkPhysicalDevice, mWindowRegistry.back().mVkSurfaceKHR, &mWindowRegistry.back().mVkSurfaceCapabilitiesKHR );

        uint32_t surface_format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR ( mVkPhysicalDevice, mWindowRegistry.back().mVkSurfaceKHR, &surface_format_count, nullptr );
        if ( surface_format_count == 0 )
        {
            assert ( 0 && "No surface formats." );
            return false;
        }
        std::vector<VkSurfaceFormatKHR> surface_format_list ( surface_format_count );
        vkGetPhysicalDeviceSurfaceFormatsKHR ( mVkPhysicalDevice, mWindowRegistry.back().mVkSurfaceKHR, &surface_format_count, surface_format_list.data() );
        if ( surface_format_list[0].format == VK_FORMAT_UNDEFINED )
        {
            mWindowRegistry.back().mVkSurfaceFormatKHR.format = VK_FORMAT_B8G8R8A8_UNORM;
            mWindowRegistry.back().mVkSurfaceFormatKHR.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }
        else
        {
            mWindowRegistry.back().mVkSurfaceFormatKHR = surface_format_list[0];
        }

        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        {
            uint32_t present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR ( mVkPhysicalDevice, mWindowRegistry.back().mVkSurfaceKHR, &present_mode_count, nullptr );
            std::vector<VkPresentModeKHR> present_mode_list ( present_mode_count );
            vkGetPhysicalDeviceSurfacePresentModesKHR ( mVkPhysicalDevice, mWindowRegistry.back().mVkSurfaceKHR, &present_mode_count, present_mode_list.data() );
            for ( auto& i : present_mode_list )
            {
                if ( i == VK_PRESENT_MODE_MAILBOX_KHR )
                {
                    present_mode = i;
                    break;
                }
            }
        }

        if ( mWindowRegistry.back().mSwapchainImageCount < mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.minImageCount )
        {
            mWindowRegistry.back().mSwapchainImageCount = mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.minImageCount;
        }
        if ( ( mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.maxImageCount > 0 ) &&
             ( mWindowRegistry.back().mSwapchainImageCount > mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.maxImageCount ) )
        {
            mWindowRegistry.back().mSwapchainImageCount = mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchain_create_info{};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = mWindowRegistry.back().mVkSurfaceKHR;
        swapchain_create_info.minImageCount = mWindowRegistry.back().mSwapchainImageCount;
        swapchain_create_info.imageFormat = mWindowRegistry.back().mVkSurfaceFormatKHR.format;
        swapchain_create_info.imageColorSpace = mWindowRegistry.back().mVkSurfaceFormatKHR.colorSpace;
        swapchain_create_info.imageExtent.width = mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.currentExtent.width;
        swapchain_create_info.imageExtent.height = mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.currentExtent.height;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = nullptr;
        swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = present_mode;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = VK_NULL_HANDLE; // Used for Resising later.

        vkCreateSwapchainKHR ( mVkDevice, &swapchain_create_info, nullptr, &mWindowRegistry.back().mVkSwapchainKHR );
        vkGetSwapchainImagesKHR ( mVkDevice, mWindowRegistry.back().mVkSwapchainKHR, &mWindowRegistry.back().mSwapchainImageCount, nullptr );
        mWindowRegistry.back().mVkSwapchainImages.resize ( mWindowRegistry.back().mSwapchainImageCount );
        mWindowRegistry.back().mVkSwapchainImageViews.resize ( mWindowRegistry.back().mSwapchainImageCount );
        vkGetSwapchainImagesKHR ( mVkDevice,
                                  mWindowRegistry.back().mVkSwapchainKHR,
                                  &mWindowRegistry.back().mSwapchainImageCount,
                                  mWindowRegistry.back().mVkSwapchainImages.data() );
        for ( uint32_t i = 0; i < mWindowRegistry.back().mSwapchainImageCount; ++i )
        {
            VkImageViewCreateInfo image_view_create_info{};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = mWindowRegistry.back().mVkSwapchainImages[i];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = mWindowRegistry.back().mVkSurfaceFormatKHR.format;
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            vkCreateImageView ( mVkDevice, &image_view_create_info, nullptr, &mWindowRegistry.back().mVkSwapchainImageViews[i] );
        }

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
                vkGetPhysicalDeviceFormatProperties ( mVkPhysicalDevice, format, &format_properties );
                if ( format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
                {
                    mWindowRegistry.back().mVkDepthStencilFormat = format;
                    break;
                }
            }

            assert ( ( std::find ( try_formats.begin(), try_formats.end(), mWindowRegistry.back().mVkDepthStencilFormat ) != try_formats.end() )
                     && "Unable to find a suitable depth stencil format" );
            mWindowRegistry.back().mHasStencil =
                ( mWindowRegistry.back().mVkDepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                  mWindowRegistry.back().mVkDepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
                  mWindowRegistry.back().mVkDepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT );
        }
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.flags = 0;
        image_create_info.format = mWindowRegistry.back().mVkDepthStencilFormat;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.currentExtent.width;
        image_create_info.extent.height = mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.currentExtent.height;
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
        vkCreateImage ( mVkDevice, &image_create_info, nullptr, &mWindowRegistry.back().mVkDepthStencilImage );

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements ( mVkDevice, mWindowRegistry.back().mVkDepthStencilImage, &memory_requirements );

        auto required_bits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t memory_index = UINT32_MAX;
        for ( uint32_t i = 0; i < mVkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i )
        {
            if ( memory_requirements.memoryTypeBits & ( 1 << i ) )
            {
                if ( ( mVkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & required_bits ) == required_bits )
                {
                    memory_index = i;
                    break;
                }
            }
        }
        assert ( memory_index != UINT32_MAX && "Could not find a suitable memory index." );
        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = memory_index;
        vkAllocateMemory ( mVkDevice, &memory_allocate_info, nullptr, &mWindowRegistry.back().mVkDepthStencilImageMemory );
        vkBindImageMemory ( mVkDevice, mWindowRegistry.back().mVkDepthStencilImage, mWindowRegistry.back().mVkDepthStencilImageMemory, 0 );

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = mWindowRegistry.back().mVkDepthStencilImage;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = mWindowRegistry.back().mVkDepthStencilFormat;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | ( mWindowRegistry.back().mHasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0 );
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        vkCreateImageView ( mVkDevice, &image_view_create_info, nullptr, &mWindowRegistry.back().mVkDepthStencilImageView );

        std::array<VkAttachmentDescription, 2> attachment_descriptions{};
        attachment_descriptions[0].flags = 0;
        attachment_descriptions[0].format = mWindowRegistry.back().mVkDepthStencilFormat;
        attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachment_descriptions[1].flags = 0;
        attachment_descriptions[1].format = mWindowRegistry.back().mVkSurfaceFormatKHR.format;
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
        vkCreateRenderPass ( mVkDevice, &render_pass_create_info, nullptr, &mWindowRegistry.back().mVkRenderPass );

        mWindowRegistry.back().mVkFramebuffers.resize ( mWindowRegistry.back().mSwapchainImageCount );
        for ( uint32_t i = 0; i < mWindowRegistry.back().mSwapchainImageCount; ++i )
        {
            std::array<VkImageView, 2> attachments
            {
                mWindowRegistry.back().mVkDepthStencilImageView,
                mWindowRegistry.back().mVkSwapchainImageViews[i]
            };
            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = mWindowRegistry.back().mVkRenderPass;
            framebuffer_create_info.attachmentCount = static_cast<uint32_t> ( attachments.size() );
            framebuffer_create_info.pAttachments = attachments.data();
            framebuffer_create_info.width = mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.currentExtent.width;
            framebuffer_create_info.height = mWindowRegistry.back().mVkSurfaceCapabilitiesKHR.currentExtent.height;
            framebuffer_create_info.layers = 1;
            vkCreateFramebuffer ( mVkDevice, &framebuffer_create_info, nullptr, &mWindowRegistry.back().mVkFramebuffers[i] );
        }

        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vkCreateFence ( mVkDevice, &fence_create_info, nullptr, &mWindowRegistry.back().mVkFence );

        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.queueFamilyIndex = mQueueFamilyIndex;
        vkCreateCommandPool ( mVkDevice, &command_pool_create_info, nullptr, &mWindowRegistry.back().mVkCommandPool );

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = mWindowRegistry.back().mVkCommandPool;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1;
        vkAllocateCommandBuffers ( mVkDevice, &command_buffer_allocate_info, &mWindowRegistry.back().mVkCommandBuffer );

        return true;
#elif defined( VK_USE_PLATFORM_XLIB_KHR )
        VkXlibSurfaceCreateInfoKHR xlib_surface_create_info_khr {};
        xlib_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        xlib_surface_create_info_khr.dpy = XOpenDisplay ( nullptr );
        xlib_surface_create_info_khr.window = aWindowId;
        if ( VkResult result = vkCreateXlibSurfaceKHR ( mVkInstance, &xlib_surface_create_info_khr, nullptr, &mWindowRegistry.back().mVkSurfaceKHR ) )
        {
            std::cout << LogLevel ( LogLevel::Level::Error ) << "Call to vkCreateXlibSurfaceKHR failed: ( " << GetVulkanRendererResultString ( result ) << " )";
            return false;
        }
        return true;
#endif
    }

    void VulkanRenderer::RemoveRenderingWindow ( uintptr_t aWindowId )
    {
        vkQueueWaitIdle ( mVkQueue );
        for ( auto& w : mWindowRegistry )
        {
#if defined ( VK_USE_PLATFORM_WIN32_KHR )
            if ( w.mVkCommandPool != VK_NULL_HANDLE )
            {
                vkDestroyCommandPool ( mVkDevice, w.mVkCommandPool, nullptr );
                w.mVkCommandPool = VK_NULL_HANDLE;
            }

            if ( w.mVkFence != VK_NULL_HANDLE )
            {
                vkDestroyFence ( mVkDevice, w.mVkFence, nullptr );
                w.mVkFence = VK_NULL_HANDLE;
            }

            for ( auto& i : w.mVkFramebuffers )
            {
                if ( i != VK_NULL_HANDLE )
                {
                    vkDestroyFramebuffer ( mVkDevice, i, nullptr );
                    i = VK_NULL_HANDLE;
                }
            }
            if ( w.mVkRenderPass != VK_NULL_HANDLE )
            {
                vkDestroyRenderPass ( mVkDevice, w.mVkRenderPass, nullptr );
                w.mVkRenderPass = VK_NULL_HANDLE;
            }
            if ( w.mVkDepthStencilImageView != VK_NULL_HANDLE )
            {
                vkDestroyImageView ( mVkDevice, w.mVkDepthStencilImageView, nullptr );
                w.mVkDepthStencilImageView = VK_NULL_HANDLE;
            }
            if ( w.mVkDepthStencilImageMemory != VK_NULL_HANDLE )
            {
                vkFreeMemory ( mVkDevice, w.mVkDepthStencilImageMemory, nullptr );
                w.mVkDepthStencilImageMemory = VK_NULL_HANDLE;
            }

            if ( w.mVkDepthStencilImage != VK_NULL_HANDLE )
            {
                vkDestroyImage ( mVkDevice, w.mVkDepthStencilImage, nullptr );
                w.mVkDepthStencilImage = VK_NULL_HANDLE;
            }
            for ( auto& i : w.mVkSwapchainImageViews )
            {
                if ( i != VK_NULL_HANDLE )
                {
                    vkDestroyImageView ( mVkDevice, i, nullptr );
                    i = VK_NULL_HANDLE;
                }
            }
            if ( w.mVkSwapchainKHR != VK_NULL_HANDLE )
            {
                vkDestroySwapchainKHR ( mVkDevice, w.mVkSwapchainKHR, nullptr );
                w.mVkSwapchainKHR = VK_NULL_HANDLE;
            }
            if ( w.mVkSurfaceKHR != VK_NULL_HANDLE )
            {
                vkDestroySurfaceKHR ( mVkInstance, w.mVkSurfaceKHR, nullptr );
                w.mVkSurfaceKHR = VK_NULL_HANDLE;
            }
#elif defined( VK_USE_PLATFORM_XLIB_KHR )
            if ( w.mVkSurfaceKHR != VK_NULL_HANDLE )
            {
                vkDestroySurfaceKHR ( mVkInstance, w.mVkSurfaceKHR, nullptr );
                w.mVkSurfaceKHR = VK_NULL_HANDLE;
            }
#endif
        }
    }

    void VulkanRenderer::Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const
    {
    }

    void VulkanRenderer::UpdateMatrices()
    {
        /** @todo Either publish this function or
        add arguments so just some matrices are
        updated based on which one changed.*/
        // Update mViewProjectionMatrix
        Multiply4x4Matrix ( mProjectionMatrix, mViewMatrix, mViewProjectionMatrix );
        // Update mModelViewMatrix
        Multiply4x4Matrix ( mViewMatrix, mModelMatrix, mModelViewMatrix );
        // Update mModelViewProjectionMatrix
        Multiply4x4Matrix ( mViewProjectionMatrix, mModelMatrix, mModelViewProjectionMatrix );
        /*  Calculate Normal Matrix
        Inverting a 3x3 matrix is cheaper than inverting a 4x4 matrix,
        so even if the shader alignment requires us to pad the 3x3 matrix into
        a 4x3 matrix we do these operations on a 3x3 basis.*/
        Extract3x3Matrix ( mModelViewMatrix, mNormalMatrix );
        Invert3x3Matrix ( mNormalMatrix, mNormalMatrix );
        Transpose3x3Matrix ( mNormalMatrix, mNormalMatrix );
        Convert3x3To4x3 ( mNormalMatrix, mNormalMatrix );
    }

    void VulkanRenderer::SetViewMatrix ( const float aMatrix[16] )
    {
        memcpy ( mViewMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }

    void VulkanRenderer::SetProjectionMatrix ( const float aMatrix[16] )
    {
        memcpy ( mProjectionMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }

    void VulkanRenderer::SetModelMatrix ( const float aMatrix[16] )
    {
        memcpy ( mModelMatrix, aMatrix, sizeof ( float ) * 16 );
        UpdateMatrices();
    }
}
