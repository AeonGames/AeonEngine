/*
Copyright 2016,2017 Rodrigo Jose Hernandez Cordoba

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
            InitializeCommandPool();
        }
        catch ( ... )
        {
            FinalizeCommandPool();
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
        VkResult result;
        if ( !mVkInstance && ( result = vkCreateDebugReportCallbackEXT ( mVkInstance, &mDebugReportCallbackCreateInfo, nullptr, &mVkDebugReportCallbackEXT ) ) != VK_SUCCESS )
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
        VkResult result;
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

        if ( ( result = vkCreateInstance ( &instance_create_info, nullptr, &mVkInstance ) ) != VK_SUCCESS )
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
        VkResult result;
        if ( ( result = vkCreateDevice ( mVkPhysicalDevice, &device_create_info, nullptr, &mVkDevice ) ) != VK_SUCCESS )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer device. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        vkGetDeviceQueue ( mVkDevice, mQueueFamilyIndex, 0, &mVkQueue );
    }

    void VulkanRenderer::InitializeCommandPool()
    {
        assert ( mVkDevice != VK_NULL_HANDLE );
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = mQueueFamilyIndex;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VkResult result;
        if ( ( result = vkCreateCommandPool ( mVkDevice, &command_pool_create_info, nullptr, &mVkCommandPool ) ) != VK_SUCCESS )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer command pool. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = mVkCommandPool;
        command_buffer_allocate_info.commandBufferCount = 1;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if ( ( result = vkAllocateCommandBuffers ( mVkDevice, &command_buffer_allocate_info, &mVkCommandBuffer ) ) != VK_SUCCESS )
        {
            std::ostringstream stream;
            stream << "Could not allocate VulkanRenderer command buffers. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if ( ( result = vkBeginCommandBuffer ( mVkCommandBuffer, &command_buffer_begin_info ) ) != VK_SUCCESS )
        {
            std::ostringstream stream;
            stream << "vkBeginCommandBuffer call failed. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        vkCmdPipelineBarrier ( mVkCommandBuffer,
                               VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                               VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                               0,
                               0, nullptr,
                               0, nullptr,
                               0, nullptr );

        if ( ( result = vkEndCommandBuffer ( mVkCommandBuffer ) ) != VK_SUCCESS )
        {
            std::ostringstream stream;
            stream << "vkEndCommandBuffer call failed. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkFenceCreateInfo fence_create_info {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        if ( ( result = vkCreateFence ( mVkDevice, &fence_create_info, nullptr, &mVkFence ) ) != VK_SUCCESS )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer fence. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if ( ( result = vkCreateSemaphore ( mVkDevice, &semaphore_create_info, nullptr, &mVkSemaphore ) ) != VK_SUCCESS )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer semaphore. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &mVkCommandBuffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &mVkSemaphore;

        if ( ( result = vkQueueSubmit ( mVkQueue, 1, &submit_info, mVkFence ) ) != VK_SUCCESS )
        {
            std::ostringstream stream;
            stream << "Could not submit VulkanRenderer queue. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
#if 0
        if ( ( result = vkWaitForFences ( mVkDevice, 1, &mVkFence, VK_TRUE, UINT64_MAX ) ) != VK_SUCCESS )
        {
            return false;
        }
#else
        if ( ( result = vkQueueWaitIdle ( mVkQueue ) ) != VK_SUCCESS )
        {
            std::ostringstream stream;
            stream << "Call to vkQueueWaitIdle failed. error code: ( " << GetVulkanRendererResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
#endif
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

    void VulkanRenderer::FinalizeCommandPool()
    {
        if ( mVkSemaphore != VK_NULL_HANDLE )
        {
            vkDestroySemaphore ( mVkDevice, mVkSemaphore, nullptr );
            mVkSemaphore = VK_NULL_HANDLE;
        }
        if ( mVkFence != VK_NULL_HANDLE )
        {
            vkDestroyFence ( mVkDevice, mVkFence, nullptr );
            mVkFence = VK_NULL_HANDLE;
        }
        if ( mVkCommandPool != VK_NULL_HANDLE )
        {
            vkDestroyCommandPool ( mVkDevice, mVkCommandPool, nullptr );
            mVkCommandPool = VK_NULL_HANDLE;
        }
    }

    VulkanRenderer::~VulkanRenderer()
    {
        FinalizeCommandPool();
        FinalizeDevice();
        FinalizeDebug();
        FinalizeInstance();
    }

    void VulkanRenderer::BeginRender() const
    {
    }

    void VulkanRenderer::EndRender() const
    {
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
        VkResult result;
#if defined ( VK_USE_PLATFORM_WIN32_KHR )
        VkWin32SurfaceCreateInfoKHR win32_surface_create_info_khr {};
        win32_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create_info_khr.hwnd = reinterpret_cast<HWND> ( mWindowRegistry.back().mWindowId );
        win32_surface_create_info_khr.hinstance = reinterpret_cast<HINSTANCE> ( GetWindowLongPtr ( win32_surface_create_info_khr.hwnd, GWLP_HINSTANCE ) );
        if ( ( result = vkCreateWin32SurfaceKHR ( mVkInstance, &win32_surface_create_info_khr, nullptr, &mWindowRegistry.back().mVkSurfaceKHR ) ) != VK_SUCCESS )
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
        return true;
#elif defined( VK_USE_PLATFORM_XLIB_KHR )
        VkXlibSurfaceCreateInfoKHR xlib_surface_create_info_khr {};
        xlib_surface_create_info_khr.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        xlib_surface_create_info_khr.dpy = XOpenDisplay ( nullptr );
        xlib_surface_create_info_khr.window = aWindowId;
        if ( ( result = vkCreateXlibSurfaceKHR ( mVkInstance, &xlib_surface_create_info_khr, nullptr, &mWindowRegistry.back().mVkSurfaceKHR ) ) != VK_SUCCESS )
        {
            std::cout << LogLevel ( LogLevel::Level::Error ) << "Call to vkCreateXlibSurfaceKHR failed: ( " << GetVulkanRendererResultString ( result ) << " )";
            return false;
        }
        return true;
#endif
    }

    void VulkanRenderer::RemoveRenderingWindow ( uintptr_t aWindowId )
    {
        for ( auto& w : mWindowRegistry )
        {
#if defined ( VK_USE_PLATFORM_WIN32_KHR )
            for ( uint32_t i = 0; i < w.mSwapchainImageCount; ++i )
            {
                if ( w.mVkSwapchainImageViews[i] != VK_NULL_HANDLE )
                {
                    vkDestroyImageView ( mVkDevice, w.mVkSwapchainImageViews[i], nullptr );
                    w.mVkSwapchainImageViews[i] = VK_NULL_HANDLE;
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
