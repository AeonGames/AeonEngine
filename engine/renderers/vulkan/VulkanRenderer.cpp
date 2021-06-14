/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <array>
#include <algorithm>
#include <limits>
#include "VulkanRenderer.h"
#include "VulkanWindow.h"
#include "VulkanBuffer.h"
#include "VulkanUtilities.h"
#include "aeongames/LogLevel.h"
#include "aeongames/Mesh.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "aeongames/Texture.h"
#include "aeongames/Utilities.h"
#include "SPIR-V/CompilerLinker.h"

namespace AeonGames
{
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
            InitializeSemaphores();
            InitializeFence();
            InitializeRenderPass();
            InitializeCommandPool();
            InitializeDescriptorSetLayout ( mVkUniformBufferDescriptorSetLayout, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER );
            InitializeDescriptorSetLayout ( mVkUniformBufferDynamicDescriptorSetLayout, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC );
        }
        catch ( ... )
        {
            this->~VulkanRenderer();
            throw;
        }
    }

    VulkanRenderer::~VulkanRenderer()
    {
        vkQueueWaitIdle ( mVkQueue );

        for ( auto& i : mVkSamplerDescriptorSetLayouts )
        {
            vkDestroyDescriptorSetLayout ( mVkDevice, std::get<1> ( i ), nullptr );
        }
        mVkSamplerDescriptorSetLayouts.clear();

        FinalizeDescriptorSetLayout ( mVkUniformBufferDynamicDescriptorSetLayout );
        FinalizeDescriptorSetLayout ( mVkUniformBufferDescriptorSetLayout );
        FinalizeCommandPool();
        FinalizeRenderPass();
        FinalizeFence();
        FinalizeSemaphores();
        FinalizeDevice();
        FinalizeDebug();
        FinalizeInstance();
    }

    const VkDevice & VulkanRenderer::GetDevice() const
    {
        return mVkDevice;
    }

    const VkQueue & VulkanRenderer::GetQueue() const
    {
        return mVkQueue;
    }

    const VkFence & VulkanRenderer::GetFence() const
    {
        return mVkFence;
    }

    const VkInstance & VulkanRenderer::GetInstance() const
    {
        return mVkInstance;
    }

    const VkPhysicalDevice & VulkanRenderer::GetPhysicalDevice() const
    {
        return mVkPhysicalDevice;
    }

    const VkPhysicalDeviceProperties & VulkanRenderer::GetPhysicalDeviceProperties() const
    {
        return mVkPhysicalDeviceProperties;
    }

    const VkPhysicalDeviceMemoryProperties & VulkanRenderer::GetPhysicalDeviceMemoryProperties() const
    {
        return mVkPhysicalDeviceMemoryProperties;
    }

    const VkRenderPass & VulkanRenderer::GetRenderPass() const
    {
        return mVkRenderPass;
    }

    const VkFormat & VulkanRenderer::GetDepthStencilFormat() const
    {
        return mVkDepthStencilFormat;
    }

    const VkSurfaceFormatKHR & VulkanRenderer::GetSurfaceFormatKHR() const
    {
        return mVkSurfaceFormatKHR;
    }

    const VkCommandBuffer & VulkanRenderer::GetCommandBuffer() const
    {
        return mVkCommandBuffer;
    }

    const VkSemaphore & VulkanRenderer::GetSignalSemaphore() const
    {
        return mVkSignalSemaphore;
    }

    uint32_t VulkanRenderer::GetQueueFamilyIndex() const
    {
        return mQueueFamilyIndex;
    }

#if defined (VK_USE_PLATFORM_XLIB_KHR)
    Display* VulkanRenderer::GetDisplay() const
    {
        return mDisplay;
    }
#endif

    uint32_t VulkanRenderer::GetMemoryTypeIndex ( VkMemoryPropertyFlags aVkMemoryPropertyFlags ) const
    {
        for ( uint32_t i = 0; i < mVkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i )
        {
            if ( ( mVkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags &
                   ( aVkMemoryPropertyFlags ) ) == ( aVkMemoryPropertyFlags ) )
            {
                return i;
            }
        }
        return std::numeric_limits<uint32_t>::max();
    }

    uint32_t VulkanRenderer::FindMemoryTypeIndex ( uint32_t typeFilter, VkMemoryPropertyFlags properties ) const
    {
        for ( uint32_t i = 0; i < mVkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i )
        {
            if ( ( typeFilter & ( 1 << i ) ) && ( mVkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties ) == properties )
            {
                return i;
            }
        }
        return std::numeric_limits<uint32_t>::max();
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
        mInstanceExtensionNames.emplace_back ( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
        mDeviceLayerNames.emplace_back ( "VK_LAYER_LUNARG_standard_validation" );
    }

    void VulkanRenderer::InitializeDebug()
    {
        if ( VkResult result = vkCreateDebugReportCallbackEXT ( mVkInstance, &mDebugReportCallbackCreateInfo, nullptr, &mVkDebugReportCallbackEXT ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer debug report callback. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::SetupLayersAndExtensions()
    {
        mInstanceExtensionNames.push_back ( VK_KHR_SURFACE_EXTENSION_NAME );
#ifdef VK_USE_PLATFORM_WIN32_KHR
        mInstanceExtensionNames.push_back ( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
#elif defined (VK_USE_PLATFORM_XLIB_KHR)
        mInstanceExtensionNames.push_back ( VK_KHR_XLIB_SURFACE_EXTENSION_NAME );
#endif
        mDeviceExtensionNames.push_back ( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    }

    void VulkanRenderer::InitializeInstance()
    {
        VkInstanceCreateInfo instance_create_info {};
        VkApplicationInfo application_info {};

        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.apiVersion = 0;
        application_info.applicationVersion = VK_MAKE_VERSION ( 0, 1, 0 );
        application_info.pApplicationName = "AeonEngine Vulkan Renderer";

        {
            uint32_t device_layer_count;
            vkEnumerateInstanceLayerProperties ( &device_layer_count, nullptr );
            std::vector<VkLayerProperties> device_layer_list ( device_layer_count );
            vkEnumerateInstanceLayerProperties ( &device_layer_count, device_layer_list.data() );
            std::cout << "VulkanRenderer Instance Layers" << std::endl;
            for ( auto& i : device_layer_list )
            {
                std::cout << " " << i.layerName << ": " << i.description << std::endl;
                {
                    uint32_t instance_extension_layer_count;
                    vkEnumerateInstanceExtensionProperties ( i.layerName, &instance_extension_layer_count, nullptr );
                    std::vector<VkExtensionProperties> instance_extension_layer_list ( instance_extension_layer_count );
                    vkEnumerateInstanceExtensionProperties ( i.layerName, &instance_extension_layer_count, instance_extension_layer_list.data() );
                    for ( auto& j : instance_extension_layer_list )
                    {
                        std::cout << "\t" << j.extensionName << std::endl;
                    }
                }
            }
        }

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
            stream << "Could not create VulkanRenderer instance. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::InitializeDevice()
    {
        if ( !mVkInstance )
        {
            throw std::runtime_error ( "mVkInstance is a nullptr." );
        }
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
            std::cout << LogLevel::Debug << "minUniformBufferOffsetAlignment: " << mVkPhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment << std::endl;
            std::cout << LogLevel::Debug << "maxUniformBufferRange: " << mVkPhysicalDeviceProperties.limits.maxUniformBufferRange << std::endl;
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
            uint32_t extension_property_count;
            vkEnumerateDeviceExtensionProperties ( mVkPhysicalDevice, nullptr, &extension_property_count, nullptr );

            std::vector<VkExtensionProperties> extension_properties ( extension_property_count );
            vkEnumerateDeviceExtensionProperties ( mVkPhysicalDevice, nullptr, &extension_property_count, extension_properties.data() );
            std::cout << "Vulkan Extensions" << std::endl;
            for ( auto& i : extension_properties )
            {
                std::cout << " " << i.extensionName << "t|\t" << i.specVersion << std::endl;
                if ( !strcmp ( i.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME ) )
                {
                    mDeviceExtensionNames.emplace_back ( VK_EXT_DEBUG_MARKER_EXTENSION_NAME );
                }
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
            stream << "Could not create VulkanRenderer device. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        vkGetDeviceQueue ( mVkDevice, mQueueFamilyIndex, 0, &mVkQueue );
    }

    void VulkanRenderer::InitializeSemaphores()
    {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if ( VkResult result = vkCreateSemaphore ( mVkDevice, &semaphore_create_info, nullptr, &mVkSignalSemaphore ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer semaphore. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::InitializeFence()
    {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vkCreateFence ( mVkDevice, &fence_create_info, nullptr, &mVkFence );
    }

    void VulkanRenderer::InitializeRenderPass()
    {
        mVkSurfaceFormatKHR.format = VK_FORMAT_B8G8R8A8_UNORM;
        mVkSurfaceFormatKHR.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
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
            vkGetPhysicalDeviceFormatProperties ( mVkPhysicalDevice, format, &format_properties );
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
        vkCreateRenderPass ( mVkDevice, &render_pass_create_info, nullptr, &mVkRenderPass );
    }

    void VulkanRenderer::InitializeCommandPool()
    {
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.queueFamilyIndex = mQueueFamilyIndex;
        vkCreateCommandPool ( mVkDevice, &command_pool_create_info, nullptr, &mVkCommandPool );

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool = mVkCommandPool;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1;
        vkAllocateCommandBuffers ( mVkDevice, &command_buffer_allocate_info, &mVkCommandBuffer );
    }

    void VulkanRenderer::FinalizeCommandPool()
    {
        if ( mVkCommandPool != VK_NULL_HANDLE )
        {
            vkDestroyCommandPool ( mVkDevice, mVkCommandPool, nullptr );
            mVkCommandPool = VK_NULL_HANDLE;
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
            if ( VkResult result = vkDeviceWaitIdle ( mVkDevice ) )
            {
                std::cout << "vkDeviceWaitIdle returned " << GetVulkanResultString ( result ) << std::endl;
            }
            vkDestroyDevice ( mVkDevice, nullptr );
            mVkDevice = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeSemaphores()
    {
        if ( mVkSignalSemaphore != VK_NULL_HANDLE )
        {
            vkDestroySemaphore ( mVkDevice, mVkSignalSemaphore, nullptr );
            mVkSignalSemaphore = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeFence()
    {
        if ( mVkFence != VK_NULL_HANDLE )
        {
            vkDestroyFence ( mVkDevice, mVkFence, nullptr );
            mVkFence = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::FinalizeRenderPass()
    {
        if ( mVkRenderPass != VK_NULL_HANDLE )
        {
            vkDestroyRenderPass ( mVkDevice, mVkRenderPass, nullptr );
        }
    }

    VkCommandBuffer VulkanRenderer::BeginSingleTimeCommands() const
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        VkCommandBuffer command_buffer{};
        VkCommandBufferBeginInfo beginInfo{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandPool = mVkCommandPool;
        command_buffer_allocate_info.commandBufferCount = 1;
        vkAllocateCommandBuffers ( mVkDevice, &command_buffer_allocate_info, &command_buffer );
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer ( command_buffer, &beginInfo );
        return command_buffer;
    }

    void VulkanRenderer::EndSingleTimeCommands ( VkCommandBuffer aVkCommandBuffer ) const
    {
        vkEndCommandBuffer ( aVkCommandBuffer );
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &aVkCommandBuffer;
        vkQueueSubmit ( mVkQueue, 1, &submit_info, VK_NULL_HANDLE );
        vkQueueWaitIdle ( mVkQueue );
        vkFreeCommandBuffers ( mVkDevice, mVkCommandPool, 1, &aVkCommandBuffer );
    }

    std::unique_ptr<Window> VulkanRenderer::CreateWindowProxy ( void * aWindowId ) const
    {
        return std::make_unique<VulkanWindow> ( *this, aWindowId );
    }

    std::unique_ptr<Window> VulkanRenderer::CreateWindowInstance ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) const
    {
        return std::make_unique<VulkanWindow> ( *this, aX, aY, aWidth, aHeight, aFullScreen );
    }

    void VulkanRenderer::LoadMesh ( const Mesh& aMesh )
    {
        if ( aMesh.GetVertexCount() == 0 )
        {
            return;
        }
        auto it = mBufferStore.find ( aMesh.GetConsecutiveId() );
        if ( it != mBufferStore.end() )
        {
            return;
        }

        std::vector<uint8_t> vertex_buffer (  sizeof ( Vertex ) * aMesh.GetVertexCount() );
        auto* vertices = reinterpret_cast<Vertex*> ( vertex_buffer.data() );
        const auto* mesh_vertices = aMesh.GetVertexBuffer().data();
        uintptr_t offset = 0;
        for ( uint32_t j = 0; j < aMesh.GetVertexCount(); ++j )
        {
            if ( aMesh.GetVertexFlags() & Mesh::POSITION_BIT )
            {
                memcpy ( vertices[j].position, mesh_vertices + offset, sizeof ( Vertex::position ) );
                offset += sizeof ( Vertex::position );
            }

            if ( aMesh.GetVertexFlags() & Mesh::NORMAL_BIT )
            {
                memcpy ( vertices[j].normal, mesh_vertices + offset, sizeof ( Vertex::normal ) );
                offset += sizeof ( Vertex::normal );
            }

            if ( aMesh.GetVertexFlags() & Mesh::TANGENT_BIT )
            {
                memcpy ( vertices[j].tangent, mesh_vertices + offset, sizeof ( Vertex::tangent ) );
                offset += sizeof ( Vertex::tangent );
            }

            if ( aMesh.GetVertexFlags() & Mesh::BITANGENT_BIT )
            {
                memcpy ( vertices[j].bitangent, mesh_vertices + offset, sizeof ( Vertex::bitangent ) );
                offset += sizeof ( Vertex::bitangent );
            }

            if ( aMesh.GetVertexFlags() & Mesh::UV_BIT )
            {
                memcpy ( vertices[j].uv, mesh_vertices + offset, sizeof ( Vertex::uv ) );
                offset += sizeof ( Vertex::uv );
            }

            if ( aMesh.GetVertexFlags() & Mesh::WEIGHT_IDX_BIT )
            {
                memcpy ( vertices[j].weight_indices, mesh_vertices + offset, sizeof ( Vertex::weight_indices ) );
                offset += sizeof ( Vertex::weight_indices );
            }

            if ( aMesh.GetVertexFlags() & Mesh::WEIGHT_BIT )
            {
                memcpy ( vertices[j].weight_influences, mesh_vertices + offset, sizeof ( Vertex::weight_influences ) );
                offset += sizeof ( Vertex::weight_influences );
            }

            if ( aMesh.GetVertexFlags() & Mesh::COLOR_BIT )
            {
                memcpy ( vertices[j].color, mesh_vertices + offset, sizeof ( Vertex::color ) );
                offset += sizeof ( Vertex::color );
            }
        }

        if ( aMesh.GetIndexCount() )
        {
            const uint8_t* buffer{};
            size_t   buffer_size{};
            std::vector<uint8_t> index_buffer{};
            if ( aMesh.GetIndexSize() != 1 )
            {
                buffer = aMesh.GetIndexBuffer().data();
                buffer_size = aMesh.GetIndexBuffer().size();
            }
            else
            {
                /**@note upcast to 16 bit indices.
                   We need to expand 1 byte indices to 2 since they're not supported on Vulkan */
                index_buffer.resize ( aMesh.GetIndexBuffer().size() * 2 );
                uint8_t* index = index_buffer.data();
                for ( size_t j = 0; j < aMesh.GetIndexCount(); ++j )
                {
                    ( reinterpret_cast<uint16_t*> ( index ) [j] ) = aMesh.GetIndexBuffer() [j];
                }
                buffer = index_buffer.data();
                buffer_size = index_buffer.size();
            }
            mBufferStore.emplace ( aMesh.GetConsecutiveId(),
                                   std::vector<VulkanBuffer>
            {
                {*this, vertex_buffer.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer.data() },
                {*this, buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer}
            } );
        }
        else
        {
            mBufferStore.emplace ( aMesh.GetConsecutiveId(),
                                   std::vector<VulkanBuffer>
            {
                {*this, vertex_buffer.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer.data() }
            } );
        }
    }

    void VulkanRenderer::UnloadMesh ( const Mesh& aMesh )
    {
        auto it = mBufferStore.find ( aMesh.GetConsecutiveId() );
        if ( it != mBufferStore.end() )
        {
            mBufferStore.erase ( it );
        }
    }

    static VkIndexType GetIndexType ( const Mesh& aMesh )
    {
        switch ( aMesh.GetIndexSize() )
        {
        case 1:
        case 2:
            return VK_INDEX_TYPE_UINT16;
        case 4:
            return VK_INDEX_TYPE_UINT32;
        default:
            break;
        };
        throw std::runtime_error ( "Invalid Index Size." );
    }

    void VulkanRenderer::BindMesh ( const Mesh& aMesh ) const
    {
        auto it = mBufferStore.find ( aMesh.GetConsecutiveId() );
        if ( it == mBufferStore.end() )
        {
            return;
        }
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers ( GetCommandBuffer(), 0, 1, &it->second[0].GetBuffer(), &offset );
        if ( aMesh.GetIndexCount() )
        {
            vkCmdBindIndexBuffer ( GetCommandBuffer(),
                                   it->second[1].GetBuffer(), 0,
                                   GetIndexType ( aMesh ) );
        }
    }

    /*-----------------Pipeline-----------------------*/

    static std::string GetSamplersCode ( const Pipeline& aPipeline, uint32_t aSetNumber )
    {
        std::string samplers ( "//----SAMPLERS-START----\n" );
        {
            uint32_t sampler_binding = 0;
            for ( auto& i : aPipeline.GetSamplerDescriptors() )
            {
                samplers += "layout(set = " + std::to_string ( aSetNumber ) + ", binding = " + std::to_string ( sampler_binding ) + ", location =" + std::to_string ( sampler_binding ) + ") uniform sampler2D " + i + ";\n";
                ++sampler_binding;
            }
        }
        samplers.append ( "//----SAMPLERS-END----\n" );
        return samplers;
    }

    static std::string GetVertexShaderCode ( const Pipeline& aPipeline )
    {
        std::string vertex_shader{ "#version 450\n" };
        vertex_shader.append ( aPipeline.GetAttributes () );

        uint32_t set_count = 0;
        std::string transforms (
            "layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };\n"
            "layout(set = " + std::to_string ( set_count++ ) + ", binding = 0, std140) uniform Matrices{\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );

        vertex_shader.append ( transforms );

        if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
        {
            // Skeleton
            std::string skeleton (
                "layout(set = " + std::to_string ( set_count++ ) + ", binding = 0, std140) uniform Skeleton{\n"
                "mat4 skeleton[256];\n"
                "};\n"
            );
            vertex_shader.append ( skeleton );
        }

        // Properties
        vertex_shader.append (
            "layout(set = " + std::to_string ( set_count++ ) +
            ", binding = 0,std140) uniform Properties{\n" +
            aPipeline.GetProperties( ) + "};\n" );
        vertex_shader.append ( GetSamplersCode ( aPipeline, set_count++ ) );

        vertex_shader.append ( aPipeline.GetVertexShaderCode() );
        return vertex_shader;
    }

    static std::string GetFragmentShaderCode ( const Pipeline& aPipeline )
    {
        uint32_t set_count = 0;
        std::string fragment_shader{"#version 450\n"};
        std::string transforms (
            "layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };\n"
            "layout(set = " + std::to_string ( set_count++ ) + ", binding = 0, std140) uniform Matrices{\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );
        fragment_shader.append ( transforms );

        if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
        {
            // Skeleton
            std::string skeleton (
                "layout(set = " + std::to_string ( set_count++ ) + ", binding = 0, std140) uniform Skeleton{\n"
                "mat4 skeleton[256];\n"
                "};\n"
            );
            fragment_shader.append ( skeleton );
        }

        fragment_shader.append ( "layout(set = " + std::to_string ( set_count++ ) +
                                 ", binding = 0,std140) uniform Properties{\n" +
                                 aPipeline.GetProperties() + "};\n" );
        fragment_shader.append ( GetSamplersCode ( aPipeline, set_count++ ) );

        fragment_shader.append ( aPipeline.GetFragmentShaderCode() );
        return fragment_shader;
    }

    static uint32_t GetLocation ( AttributeBits aAttributeBit )
    {
        return ffs ( aAttributeBit );
    }

    static VkFormat GetFormat ( AttributeBits aAttributeBit )
    {
        return ( aAttributeBit & VertexUVBit ) ? VK_FORMAT_R32G32_SFLOAT :
               ( aAttributeBit & VertexWeightIdxBit ) ? VK_FORMAT_R8G8B8A8_UINT :
               ( aAttributeBit & VertexWeightBit ) ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R32G32B32_SFLOAT;
    }

    static uint32_t GetSize ( AttributeBits aAttributeBit )
    {
        switch ( GetFormat ( aAttributeBit ) )
        {
        case VK_FORMAT_R32G32_SFLOAT:
            return sizeof ( float ) * 2;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return sizeof ( float ) * 3;
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_UNORM:
            return sizeof ( uint8_t ) * 4;
        default:
            return 0;
        }
        return 0;
    }

    static uint32_t GetOffset ( AttributeBits aAttributeBit )
    {
        uint32_t offset = 0;
        for ( uint32_t i = 1; i != aAttributeBit; i = i << 1 )
        {
            offset += GetSize ( static_cast<AttributeBits> ( i ) );
        }
        return offset;
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

    void VulkanRenderer::BindPipeline ( const Pipeline& aPipeline, const Material* aMaterial, const BufferAccessor* aSkeletonBuffer ) const
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it == mPipelineStore.end() )
        {
            return;
        }
        mBoundPipeline = & ( it->second );
        vkCmdBindPipeline ( GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::get<VkPipeline> ( it->second ) );
#if 0
        vkCmdPushConstants ( GetCommandBuffer(),
                             std::get<VkPipelineLayout> ( it->second ),
                             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                             0, sizeof ( float ) * 16, aModelMatrix.GetMatrix4x4() );

        std::array<VkDescriptorSet, 4> descriptor_sets
        {
            mMatrices.GetUniformDescriptorSet(),
            ( aSkeleton != nullptr ) ? mMemoryPoolBuffer.GetDescriptorSet() : VK_NULL_HANDLE,
            reinterpret_cast<const VulkanMaterial*> ( aMaterial )->GetUniformDescriptorSet(),
            reinterpret_cast<const VulkanMaterial*> ( aMaterial )->GetSamplerDescriptorSet()
        };

        uint32_t descriptor_set_count = static_cast<uint32_t> ( std::remove ( descriptor_sets.begin(), descriptor_sets.end(), ( VkDescriptorSet ) VK_NULL_HANDLE ) - descriptor_sets.begin() );


        if ( descriptor_set_count )
        {
            uint32_t offset_count = ( aSkeleton != nullptr ) ? 1 : 0;
            uint32_t offset = ( offset_count != 0 ) ? static_cast<uint32_t> ( aSkeleton->GetOffset() ) : 0;
            vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      std::get<VkPipelineLayout> ( it->second ),
                                      0,
                                      descriptor_set_count,
                                      descriptor_sets.data(), offset_count, ( offset_count != 0 ) ? &offset : nullptr );
        }
#endif
    }

    void VulkanRenderer::LoadPipeline ( const Pipeline& aPipeline )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it != mPipelineStore.end() )
        {
            return;
        }

        std::array < VkShaderModule, ffs ( ~VK_SHADER_STAGE_ALL_GRAPHICS ) >
        shader_modules{ { VK_NULL_HANDLE } };

        std::string vertex_shader_code = GetVertexShaderCode ( aPipeline );
        std::string fragment_shader_code = GetFragmentShaderCode ( aPipeline );

        CompilerLinker compiler_linker;
        compiler_linker.AddShaderSource ( EShLanguage::EShLangVertex, vertex_shader_code.c_str() );
        compiler_linker.AddShaderSource ( EShLanguage::EShLangFragment, fragment_shader_code.c_str() );
        if ( CompilerLinker::FailCode result = compiler_linker.CompileAndLink() )
        {
            std::ostringstream stream;
            stream << vertex_shader_code << std::endl;
            stream << fragment_shader_code << std::endl;
            stream << ( ( result == CompilerLinker::EFailCompile ) ? "Compilation" : "Linking" ) <<
                   " Error:" << std::endl << compiler_linker.GetLog();
            std::cout << ( ( result == CompilerLinker::EFailCompile ) ? "Compilation" : "Linking" ) <<
                      " Error:" << std::endl << compiler_linker.GetLog();
            std::cout << fragment_shader_code << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        {
            VkShaderModuleCreateInfo shader_module_create_info{};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = compiler_linker.GetSpirV ( EShLanguage::EShLangVertex ).size() * sizeof ( uint32_t );
            shader_module_create_info.pCode = compiler_linker.GetSpirV ( EShLanguage::EShLangVertex ).data();

            if ( VkResult result = vkCreateShaderModule ( GetDevice(), &shader_module_create_info, nullptr, &shader_modules[ffs ( VK_SHADER_STAGE_VERTEX_BIT )] ) )
            {
                std::ostringstream stream;
                stream << "Shader module creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        {
            VkShaderModuleCreateInfo shader_module_create_info{};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = compiler_linker.GetSpirV ( EShLanguage::EShLangFragment ).size() * sizeof ( uint32_t );
            shader_module_create_info.pCode = compiler_linker.GetSpirV ( EShLanguage::EShLangFragment ).data();
            if ( VkResult result = vkCreateShaderModule ( GetDevice(), &shader_module_create_info, nullptr, &shader_modules[ffs ( VK_SHADER_STAGE_FRAGMENT_BIT )] ) )
            {
                std::ostringstream stream;
                stream << "Shader module creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }

        std::array<VkPipelineShaderStageCreateInfo, 2> pipeline_shader_stage_create_infos{ {} };
        pipeline_shader_stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_shader_stage_create_infos[0].pNext = nullptr;
        pipeline_shader_stage_create_infos[0].flags = 0;
        pipeline_shader_stage_create_infos[0].module = shader_modules[ffs ( VK_SHADER_STAGE_VERTEX_BIT )];
        pipeline_shader_stage_create_infos[0].pName = "main";
        pipeline_shader_stage_create_infos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stage_create_infos[0].pSpecializationInfo = nullptr;

        pipeline_shader_stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_shader_stage_create_infos[1].pNext = nullptr;
        pipeline_shader_stage_create_infos[1].flags = 0;
        pipeline_shader_stage_create_infos[1].module = shader_modules[ffs ( VK_SHADER_STAGE_FRAGMENT_BIT )];
        pipeline_shader_stage_create_infos[1].pName = "main";
        pipeline_shader_stage_create_infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipeline_shader_stage_create_infos[1].pSpecializationInfo = nullptr;

        std::array<VkVertexInputBindingDescription, 1> vertex_input_binding_descriptions { {} };
        vertex_input_binding_descriptions[0].binding = 0;
        vertex_input_binding_descriptions[0].stride = sizeof ( Vertex );
        vertex_input_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        uint32_t attributes = aPipeline.GetAttributeBitmap();
        std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions ( popcount ( attributes ) );
        for ( auto& i : vertex_input_attribute_descriptions )
        {
            uint32_t attribute_bit = ( 1 << ffs ( attributes ) );
            i.location = GetLocation ( static_cast<AttributeBits> ( attribute_bit ) );
            i.binding = 0;
            i.format = GetFormat ( static_cast<AttributeBits> ( attribute_bit ) );
            i.offset = GetOffset ( static_cast<AttributeBits> ( attribute_bit ) );
            attributes ^= attribute_bit;
        }

        VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info {};
        pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipeline_vertex_input_state_create_info.pNext = nullptr;
        pipeline_vertex_input_state_create_info.flags = 0;
        pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t> ( vertex_input_binding_descriptions.size() );
        pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = vertex_input_binding_descriptions.data();
        pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t> ( vertex_input_attribute_descriptions.size() );
        pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();

        VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
        pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipeline_input_assembly_state_create_info.pNext = nullptr;
        pipeline_input_assembly_state_create_info.flags = 0;
        pipeline_input_assembly_state_create_info.topology = TopologyMap.at ( aPipeline.GetTopology() );
        pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info {};
        pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipeline_viewport_state_create_info.pNext = nullptr;
        pipeline_viewport_state_create_info.flags = 0;
        pipeline_viewport_state_create_info.viewportCount = 1;
        pipeline_viewport_state_create_info.pViewports = nullptr;
        pipeline_viewport_state_create_info.scissorCount = 1;
        pipeline_viewport_state_create_info.pScissors = nullptr;

        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{};
        pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipeline_rasterization_state_create_info.pNext = nullptr;
        pipeline_rasterization_state_create_info.flags = 0;
        pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
        pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        pipeline_rasterization_state_create_info.depthBiasEnable = VK_TRUE;
        pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
        pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
        pipeline_rasterization_state_create_info.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{};
        pipeline_multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipeline_multisample_state_create_info.pNext = nullptr;
        pipeline_multisample_state_create_info.flags = 0;
        pipeline_multisample_state_create_info.sampleShadingEnable = VK_FALSE;
        pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipeline_multisample_state_create_info.minSampleShading = 0.0f;
        pipeline_multisample_state_create_info.pSampleMask = nullptr;
        pipeline_multisample_state_create_info.alphaToCoverageEnable = VK_TRUE;
        pipeline_multisample_state_create_info.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{};
        pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipeline_depth_stencil_state_create_info.pNext = nullptr;
        pipeline_depth_stencil_state_create_info.flags = 0;
        pipeline_depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
        pipeline_depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
        pipeline_depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
        pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
        pipeline_depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
        pipeline_depth_stencil_state_create_info.front = {};
        pipeline_depth_stencil_state_create_info.back = {};
        pipeline_depth_stencil_state_create_info.minDepthBounds = 0.0f;
        pipeline_depth_stencil_state_create_info.maxDepthBounds = 1.0f;

        std::array<VkPipelineColorBlendAttachmentState, 1> pipeline_color_blend_attachment_states{ {} };
        pipeline_color_blend_attachment_states[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pipeline_color_blend_attachment_states[0].blendEnable = VK_TRUE;
        pipeline_color_blend_attachment_states[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipeline_color_blend_attachment_states[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipeline_color_blend_attachment_states[0].colorBlendOp = VK_BLEND_OP_ADD;
        pipeline_color_blend_attachment_states[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pipeline_color_blend_attachment_states[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipeline_color_blend_attachment_states[0].alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{};
        pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipeline_color_blend_state_create_info.pNext = nullptr;
        pipeline_color_blend_state_create_info.flags = 0;
        pipeline_color_blend_state_create_info.logicOpEnable = VK_FALSE;
        pipeline_color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
        pipeline_color_blend_state_create_info.attachmentCount = static_cast<uint32_t> ( pipeline_color_blend_attachment_states.size() );
        pipeline_color_blend_state_create_info.pAttachments = pipeline_color_blend_attachment_states.data();
        memset ( pipeline_color_blend_state_create_info.blendConstants, 0, sizeof ( VkPipelineColorBlendStateCreateInfo::blendConstants ) );

        std::array<VkDynamicState, 2> dynamic_states
        {
            {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            }
        };
        VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};
        pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipeline_dynamic_state_create_info.pNext = nullptr;
        pipeline_dynamic_state_create_info.flags = 0;
        pipeline_dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t> ( dynamic_states.size() );
        pipeline_dynamic_state_create_info.pDynamicStates = dynamic_states.data();
        std::array<VkPushConstantRange, 1> push_constant_ranges {};
        push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; ///@todo determine ALL stage flags based on usage.
        push_constant_ranges[0].offset = 0;
        push_constant_ranges[0].size = sizeof ( float ) * 16; // the push constant will contain just the Model Matrix

        std::array<VkDescriptorSetLayout, 4> descriptor_set_layouts;

        uint32_t descriptor_set_layout_count = 0;

        // Matrix Descriptor Set Layout
        descriptor_set_layouts[descriptor_set_layout_count++] = GetUniformBufferDescriptorSetLayout();

        if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = GetUniformBufferDynamicDescriptorSetLayout();
        }
        if ( aPipeline.GetUniformDescriptors().size() )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = GetUniformBufferDescriptorSetLayout();
        }
        if ( aPipeline.GetSamplerDescriptors().size() )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = GetSamplerDescriptorSetLayout ( aPipeline.GetSamplerDescriptors().size() );
        }

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pNext = nullptr;
        pipeline_layout_create_info.setLayoutCount = descriptor_set_layout_count;
        pipeline_layout_create_info.pSetLayouts = descriptor_set_layout_count ? descriptor_set_layouts.data() : nullptr;
        pipeline_layout_create_info.pushConstantRangeCount = static_cast<uint32_t> ( push_constant_ranges.size() );
        pipeline_layout_create_info.pPushConstantRanges = push_constant_ranges.data();
        VkPipelineLayout pipeline_layout{ VK_NULL_HANDLE };
        if ( VkResult result = vkCreatePipelineLayout ( GetDevice(), &pipeline_layout_create_info, nullptr, &pipeline_layout ) )
        {
            std::ostringstream stream;
            stream << "Pipeline Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info {};
        graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphics_pipeline_create_info.pNext = nullptr;
        graphics_pipeline_create_info.flags = 0;
        graphics_pipeline_create_info.stageCount = static_cast<uint32_t> ( pipeline_shader_stage_create_infos.size() );
        graphics_pipeline_create_info.pStages = pipeline_shader_stage_create_infos.data();
        graphics_pipeline_create_info.pVertexInputState = &pipeline_vertex_input_state_create_info;
        graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_state_create_info;
        graphics_pipeline_create_info.pTessellationState = nullptr;
        graphics_pipeline_create_info.pViewportState = &pipeline_viewport_state_create_info;
        graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
        graphics_pipeline_create_info.pMultisampleState = &pipeline_multisample_state_create_info;
        graphics_pipeline_create_info.pDepthStencilState = &pipeline_depth_stencil_state_create_info;
        graphics_pipeline_create_info.pColorBlendState = &pipeline_color_blend_state_create_info;
        graphics_pipeline_create_info.pDynamicState = &pipeline_dynamic_state_create_info;
        graphics_pipeline_create_info.layout = pipeline_layout;
        graphics_pipeline_create_info.renderPass = GetRenderPass();
        graphics_pipeline_create_info.subpass = 0;
        graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        graphics_pipeline_create_info.basePipelineIndex = 0;

        VkPipeline pipeline{ VK_NULL_HANDLE };
        if ( VkResult result = vkCreateGraphicsPipelines ( GetDevice(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline ) )
        {
            std::ostringstream stream;
            stream << "Pipeline creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        for ( auto& i : shader_modules )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyShaderModule ( GetDevice(), i, nullptr );
                i = VK_NULL_HANDLE;
            }
        }
        mPipelineStore.emplace ( aPipeline.GetConsecutiveId(), std::tuple<VkPipelineLayout, VkPipeline> {pipeline_layout, pipeline} );
    }

    void VulkanRenderer::UnloadPipeline ( const Pipeline& aPipeline )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it == mPipelineStore.end() )
        {
            return;
        }
        if ( mBoundPipeline == &it->second )
        {
            mBoundPipeline = nullptr;
        }
        vkDestroyPipeline ( GetDevice(), std::get<VkPipeline> ( it->second ), nullptr );
        vkDestroyPipelineLayout ( GetDevice(), std::get<VkPipelineLayout> ( it->second ), nullptr );
        mPipelineStore.erase ( it );
    }

    /*---------------------Material---------------------*/
    void VulkanRenderer::LoadMaterial ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it != mMaterialStore.end() )
        {
            return;
        }

    }
    void VulkanRenderer::UnloadMaterial ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it == mMaterialStore.end() )
        {
            return;
        }
    }
    void VulkanRenderer::LoadTexture ( const Texture& aTexture )
    {

    }
    void VulkanRenderer::UnloadTexture ( const Texture& aTexture )
    {

    }

    const VkDescriptorSetLayout& VulkanRenderer::GetUniformBufferDescriptorSetLayout() const
    {
        return mVkUniformBufferDescriptorSetLayout;
    }

    const VkDescriptorSetLayout& VulkanRenderer::GetUniformBufferDynamicDescriptorSetLayout() const
    {
        return mVkUniformBufferDynamicDescriptorSetLayout;
    }

    const VkDescriptorSetLayout& VulkanRenderer::GetSamplerDescriptorSetLayout ( size_t aSamplerCount ) const
    {
        auto lb = std::lower_bound ( mVkSamplerDescriptorSetLayouts.begin(), mVkSamplerDescriptorSetLayouts.end(), aSamplerCount,
                                     [] ( const std::tuple<size_t, VkDescriptorSetLayout>&a, size_t b )
        {
            return std::get<0> ( a ) < b;
        } );
        if ( lb != mVkSamplerDescriptorSetLayouts.end() && std::get<0> ( *lb ) == aSamplerCount )
        {
            return std::get<1> ( *lb );
        }
        if ( aSamplerCount )
        {
            std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
            descriptor_set_layout_bindings.resize ( aSamplerCount );
            for ( uint32_t i = 0; i < aSamplerCount; ++i )
            {
                descriptor_set_layout_bindings[i].binding = i;
                descriptor_set_layout_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                // Descriptor Count is the count of elements in an array.
                descriptor_set_layout_bindings[i].descriptorCount = 1;
                descriptor_set_layout_bindings[i].stageFlags = VK_SHADER_STAGE_ALL;
                descriptor_set_layout_bindings[i].pImmutableSamplers = nullptr;
            }

            VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
            descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptor_set_layout_create_info.pNext = nullptr;
            descriptor_set_layout_create_info.flags = 0;
            descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t> ( descriptor_set_layout_bindings.size() );
            descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings.data();
            lb = mVkSamplerDescriptorSetLayouts.insert ( lb, {{aSamplerCount}, {VK_NULL_HANDLE}} );
            if ( VkResult result = vkCreateDescriptorSetLayout ( mVkDevice, &descriptor_set_layout_create_info, nullptr,
                                   &std::get<1> ( *lb ) ) )
            {
                std::ostringstream stream;
                stream << "DescriptorSet Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
            return std::get<1> ( *lb );
        }
        throw std::runtime_error ( "Sampler Count must be > 0" );
    }

    void VulkanRenderer::InitializeDescriptorSetLayout ( VkDescriptorSetLayout& aVkDescriptorSetLayout, VkDescriptorType aVkDescriptorType )
    {
        VkDescriptorSetLayoutBinding descriptor_set_layout_binding;
        descriptor_set_layout_binding.binding = 0;
        descriptor_set_layout_binding.descriptorType = aVkDescriptorType;
        /* We will bind just 1 UBO, descriptor count is the number of array elements, and we just use a single struct. */
        descriptor_set_layout_binding.descriptorCount = 1;
        descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        descriptor_set_layout_binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.pNext = nullptr;
        descriptor_set_layout_create_info.flags = 0;
        descriptor_set_layout_create_info.bindingCount = 1;
        descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
        if ( VkResult result = vkCreateDescriptorSetLayout ( mVkDevice, &descriptor_set_layout_create_info, nullptr, &aVkDescriptorSetLayout ) )
        {
            std::ostringstream stream;
            stream << "DescriptorSet Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::FinalizeDescriptorSetLayout ( VkDescriptorSetLayout& aVkDescriptorSetLayout )
    {
        if ( aVkDescriptorSetLayout != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorSetLayout ( mVkDevice, aVkDescriptorSetLayout, nullptr );
            aVkDescriptorSetLayout = VK_NULL_HANDLE;
        }
    }
}
