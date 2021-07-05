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
    enum  BindingLocations : uint32_t
    {
        MATRICES = 0,
        MATERIAL,
        SAMPLERS,
        SKELETON,
    };

    VulkanRenderer::VulkanRenderer ( bool aValidate ) : mValidate ( aValidate ),
        mMatrices ( *this ), mMemoryPoolBuffer ( *this )
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

            mMatricesDescriptorPool = CreateDescriptorPool ( mVkDevice, {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}} );
            mMatricesDescriptorSet = CreateDescriptorSet ( mVkDevice, mMatricesDescriptorPool, mVkUniformBufferDescriptorSetLayout );
            mMatrices.Initialize (
                sizeof ( float ) * 16 * 2,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

            VkDescriptorBufferInfo descriptor_buffer_info = { mMatrices.GetBuffer(), 0, mMatrices.GetSize() };
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
            vkUpdateDescriptorSets ( mVkDevice, 1, &write_descriptor_set, 0, nullptr );

            mMemoryPoolBuffer.Initialize ( 64_kb ); // @todo this should be 16_kb for mobile
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
        mMeshStore.clear();
        for ( auto& i : mVkSamplerDescriptorSetLayouts )
        {
            vkDestroyDescriptorSetLayout ( mVkDevice, std::get<1> ( i ), nullptr );
        }
        mVkSamplerDescriptorSetLayouts.clear();

        DestroyDescriptorPool ( mVkDevice, mMatricesDescriptorPool );
        mMemoryPoolBuffer.Finalize();
        mMatrices.Finalize();
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
        mInstanceExtensionNames.emplace_back ( VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME );
        mInstanceExtensionNames.emplace_back ( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
        mInstanceLayerNames.emplace_back ( "VK_LAYER_KHRONOS_validation" );
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

        std::array<VkValidationFeatureEnableEXT, 2> validation_feature_enable_exts
        {
            VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT
        };
        VkValidationFeaturesEXT validation_features_ext
        {
            VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
            nullptr,
            static_cast<uint32_t> ( validation_feature_enable_exts.size() ),
            validation_feature_enable_exts.data(),
            0,
            nullptr
        };

        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.enabledLayerCount = static_cast<uint32_t> ( mInstanceLayerNames.size() );
        instance_create_info.ppEnabledLayerNames = mInstanceLayerNames.data();
        instance_create_info.enabledExtensionCount = static_cast<uint32_t> ( mInstanceExtensionNames.size() );
        instance_create_info.ppEnabledExtensionNames = mInstanceExtensionNames.data();
        if ( mValidate )
        {
            mDebugReportCallbackCreateInfo.pNext = &validation_features_ext;
            instance_create_info.pNext = &mDebugReportCallbackCreateInfo;
        }
        if ( VkResult result = vkCreateInstance ( &instance_create_info, nullptr, &mVkInstance ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer instance. error code: ( " << GetVulkanResultString ( result ) << " )";
            std::string error_string = stream.str();
            std::cout << LogLevel::Error << error_string << std::endl;
            throw std::runtime_error ( error_string.c_str() );
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
#if 0
        uint32_t surface_format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR ( mVkPhysicalDevice, mVkSurfaceKHR, &surface_format_count, nullptr );
        if ( surface_format_count == 0 )
        {
            std::ostringstream stream;
            stream << "Physical device reports no surface formats.";
            throw std::runtime_error ( stream.str().c_str() );
        }

        std::vector<VkSurfaceFormatKHR> surface_format_list ( surface_format_count );
        vkGetPhysicalDeviceSurfaceFormatsKHR ( mVkPhysicalDevice, mVkSurfaceKHR, &surface_format_count, surface_format_list.data() );
        if ( surface_format_list[0].format == VK_FORMAT_UNDEFINED )
        {
#endif
            mVkSurfaceFormatKHR.format = VK_FORMAT_B8G8R8A8_UNORM;
            mVkSurfaceFormatKHR.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
#if 0
        }
        else
        {
            mVkSurfaceFormatKHR = surface_format_list[0];
        }
#endif
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
        return std::make_unique<VulkanWindow> ( *const_cast<VulkanRenderer*> ( this ), aWindowId );
    }

    std::unique_ptr<Window> VulkanRenderer::CreateWindowInstance ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) const
    {
        return std::make_unique<VulkanWindow> ( *const_cast<VulkanRenderer*> ( this ), aX, aY, aWidth, aHeight, aFullScreen );
    }

    void VulkanRenderer::LoadMesh ( const Mesh& aMesh )
    {
        if ( aMesh.GetVertexCount() == 0 )
        {
            return;
        }
        auto it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        if ( it != mMeshStore.end() )
        {
            std::cout << LogLevel::Warning << " Mesh " << aMesh.GetConsecutiveId() << " Already Loaded at: " << __FUNCTION__ << std::endl;
            return;
        }
        mMeshStore.emplace ( aMesh.GetConsecutiveId(), VulkanMesh{*this, aMesh} );
    }

    void VulkanRenderer::UnloadMesh ( const Mesh& aMesh )
    {
        auto it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        if ( it != mMeshStore.end() )
        {
            mMeshStore.erase ( it );
        }
    }

    void VulkanRenderer::BindMesh ( const Mesh& aMesh ) const
    {
        auto it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        if ( it == mMeshStore.end() )
        {
            return;
        }
        it->second.Bind();
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

        std::string transforms (
            "layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };\n"
            "layout(set = " + std::to_string ( MATRICES ) + ", binding = 0, std140) uniform Matrices{\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );

        vertex_shader.append ( transforms );

        // Properties
        vertex_shader.append (
            "layout(set = " + std::to_string ( MATERIAL ) +
            ", binding = 0,std140) uniform Properties{\n" +
            aPipeline.GetProperties( ) + "};\n" );

        vertex_shader.append ( GetSamplersCode ( aPipeline, SAMPLERS ) );

        if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
        {
            // Skeleton
            std::string skeleton (
                "layout(set = " + std::to_string ( SKELETON ) + ", binding = 0, std140) uniform Skeleton{\n"
                "mat4 skeleton[256];\n"
                "};\n"
            );
            vertex_shader.append ( skeleton );
        }

        vertex_shader.append ( aPipeline.GetVertexShaderCode() );
        return vertex_shader;
    }

    static std::string GetFragmentShaderCode ( const Pipeline& aPipeline )
    {
        std::string fragment_shader{"#version 450\n"};
        std::string transforms (
            "layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };\n"
            "layout(set = " + std::to_string ( MATRICES ) + ", binding = 0, std140) uniform Matrices{\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );
        fragment_shader.append ( transforms );

        fragment_shader.append ( "layout(set = " + std::to_string ( MATERIAL ) +
                                 ", binding = 0,std140) uniform Properties{\n" +
                                 aPipeline.GetProperties() + "};\n" );
        fragment_shader.append ( GetSamplersCode ( aPipeline, SAMPLERS ) );

        if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
        {
            // Skeleton
            std::string skeleton (
                "layout(set = " + std::to_string ( SKELETON ) + ", binding = 0, std140) uniform Skeleton{\n"
                "mat4 skeleton[256];\n"
                "};\n"
            );
            fragment_shader.append ( skeleton );
        }

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

    void VulkanRenderer::BindPipeline ( const Pipeline& aPipeline ) const
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it == mPipelineStore.end() )
        {
            std::cout << LogLevel::Warning << " Pipeline " << aPipeline.GetConsecutiveId() << " NOT found at: " << __FUNCTION__ << std::endl;
            return;
        }
        mBoundPipeline = & ( it->second );
        vkCmdBindPipeline ( GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, std::get<VkPipeline> ( it->second ) );
        ///@todo Move Matrices binding to BeginRender
        vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  std::get<VkPipelineLayout> ( it->second ),
                                  MATRICES,
                                  1,
                                  &mMatricesDescriptorSet, 0, nullptr );
    }

    void VulkanRenderer::SetMaterial ( const Material& aMaterial ) const
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it == mMaterialStore.end() )
        {
            std::cout << LogLevel::Warning << " Material " << aMaterial.GetConsecutiveId() << " NOT found at: " << __FUNCTION__ << std::endl;
            return;
        }

        std::array<VkDescriptorSet, 2> descriptor_sets
        {
            std::get<1> ( it->second ),
            std::get<2> ( it->second )
        };

        uint32_t descriptor_set_count = static_cast<uint32_t> ( std::remove ( descriptor_sets.begin(), descriptor_sets.end(), ( VkDescriptorSet ) VK_NULL_HANDLE ) - descriptor_sets.begin() );
        vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  std::get<VkPipelineLayout> ( *mBoundPipeline ),
                                  MATERIAL,
                                  descriptor_set_count,
                                  descriptor_sets.data(), 0, nullptr );
    }
    void VulkanRenderer::SetSkeleton ( const BufferAccessor& aSkeletonBuffer ) const
    {
        uint32_t offset = static_cast<uint32_t> ( aSkeletonBuffer.GetOffset() );
        vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  std::get<VkPipelineLayout> ( *mBoundPipeline ),
                                  SKELETON,
                                  1,
                                  &mMemoryPoolBuffer.GetDescriptorSet(), 1, &offset );
    }
    void VulkanRenderer::SetModelMatrix ( const Matrix4x4& aMatrix )
    {
        vkCmdPushConstants ( GetCommandBuffer(),
                             std::get<VkPipelineLayout> ( *mBoundPipeline ),
                             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                             0, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );

    }

    void VulkanRenderer::SetProjectionMatrix ( const Matrix4x4& aMatrix )
    {
        mMatrices.WriteMemory ( 0, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );
    }

    void VulkanRenderer::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mMatrices.WriteMemory ( sizeof ( float ) * 16, sizeof ( float ) * 16, aMatrix.GetMatrix4x4() );
    }

    void VulkanRenderer::LoadPipeline ( const Pipeline& aPipeline )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it != mPipelineStore.end() )
        {
            std::cout << LogLevel::Warning << " Pipeline " << aPipeline.GetConsecutiveId() << " Already Loaded at: " << __FUNCTION__ << std::endl;
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

        if ( aPipeline.GetUniformDescriptors().size() )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = GetUniformBufferDescriptorSetLayout();
        }
        if ( aPipeline.GetSamplerDescriptors().size() )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = GetSamplerDescriptorSetLayout ( aPipeline.GetSamplerDescriptors().size() );
        }
        if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = GetUniformBufferDynamicDescriptorSetLayout();
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
        VkDescriptorPool descriptor_pool{VK_NULL_HANDLE};
        VkDescriptorSet uniform_descriptor_set{VK_NULL_HANDLE};
        VkDescriptorSet sampler_descriptor_set{VK_NULL_HANDLE};

        // Initialize DescriptorPool
        {
            std::array<VkDescriptorPoolSize, 2> descriptor_pool_sizes{};
            uint32_t descriptor_pool_size_count = 0;

            if ( aMaterial.GetUniformBuffer().size() )
            {
                descriptor_pool_sizes[descriptor_pool_size_count].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptor_pool_sizes[descriptor_pool_size_count++].descriptorCount = 1;
            }
            if ( aMaterial.GetSamplers().size() )
            {
                descriptor_pool_sizes[descriptor_pool_size_count].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_pool_sizes[descriptor_pool_size_count++].descriptorCount = static_cast<uint32_t> ( aMaterial.GetSamplers().size() );
            }
            if ( descriptor_pool_size_count )
            {
                VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
                descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                descriptor_pool_create_info.pNext = nullptr;
                descriptor_pool_create_info.flags = 0;
                descriptor_pool_create_info.maxSets = descriptor_pool_size_count;
                descriptor_pool_create_info.poolSizeCount = descriptor_pool_size_count;
                descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
                if ( VkResult result = vkCreateDescriptorPool ( GetDevice(), &descriptor_pool_create_info, nullptr, &descriptor_pool ) )
                {
                    std::ostringstream stream;
                    stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }
            }
        }

        // Initialize Descriptor Sets
        {
            if ( aMaterial.GetUniformBuffer().size() )
            {
                VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
                descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                descriptor_set_allocate_info.descriptorPool = descriptor_pool;
                descriptor_set_allocate_info.descriptorSetCount = 1;
                descriptor_set_allocate_info.pSetLayouts = &GetUniformBufferDescriptorSetLayout();
                if ( VkResult result = vkAllocateDescriptorSets ( GetDevice(), &descriptor_set_allocate_info, &uniform_descriptor_set ) )
                {
                    std::ostringstream stream;
                    stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }
            }
            if ( aMaterial.GetSamplers().size() )
            {
                VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
                descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                descriptor_set_allocate_info.descriptorPool = descriptor_pool;
                descriptor_set_allocate_info.descriptorSetCount = 1;
                descriptor_set_allocate_info.pSetLayouts = &GetSamplerDescriptorSetLayout ( aMaterial.GetSamplers().size() );
                if ( VkResult result = vkAllocateDescriptorSets ( GetDevice(), &descriptor_set_allocate_info, &sampler_descriptor_set ) )
                {
                    std::ostringstream stream;
                    stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
                    throw std::runtime_error ( stream.str().c_str() );
                }
            }
            /*-----------------------------------------------------------------*/

            auto material = mMaterialStore.emplace (
                                aMaterial.GetConsecutiveId(),
                                std::tuple<VkDescriptorPool, VkDescriptorSet, VkDescriptorSet, std::unique_ptr<VulkanBuffer>>
            {
                descriptor_pool,
                uniform_descriptor_set,
                sampler_descriptor_set,
                ( uniform_descriptor_set == VK_NULL_HANDLE ) ? nullptr : std::make_unique<VulkanBuffer> ( *this )
            } );

            if ( !material.second )
            {
                std::ostringstream stream;
                stream << "Emplace failed at: ( " << __FUNCTION__ << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }

            std::vector<VkWriteDescriptorSet> write_descriptor_sets{};
            write_descriptor_sets.reserve ( ( aMaterial.GetUniformBuffer().size() ? 1 : 0 ) + aMaterial.GetSamplers().size() );

            if ( aMaterial.GetUniformBuffer().size() )
            {
                std::get<std::unique_ptr<VulkanBuffer>> ( material.first->second )->Initialize (
                        aMaterial.GetUniformBuffer().size(),
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        aMaterial.GetUniformBuffer().data() );
                VkDescriptorBufferInfo descriptor_buffer_info
                {
                    std::get<std::unique_ptr<VulkanBuffer>> ( material.first->second )->GetBuffer(),
                                                         0, aMaterial.GetUniformBuffer().size()
                };
                write_descriptor_sets.emplace_back();
                auto& write_descriptor_set = write_descriptor_sets.back();
                write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor_set.pNext = nullptr;
                write_descriptor_set.dstSet = uniform_descriptor_set;
                write_descriptor_set.dstBinding = 0;
                write_descriptor_set.dstArrayElement = 0;
                write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write_descriptor_set.descriptorCount = 1;
                write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
                write_descriptor_set.pImageInfo = nullptr;
                write_descriptor_set.pTexelBufferView = nullptr;
            }
            for ( uint32_t i = 0; i < aMaterial.GetSamplers().size(); ++i )
            {
                write_descriptor_sets.emplace_back();
                auto& write_descriptor_set = write_descriptor_sets.back();
                write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor_set.pNext = nullptr;
                // Note that the descriptor set does not change, we are setting multiple bindings on a single descriptor set.
                write_descriptor_set.dstSet = sampler_descriptor_set;
                write_descriptor_set.dstBinding = i;
                write_descriptor_set.dstArrayElement = 0;
                write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write_descriptor_set.descriptorCount = 1;
                write_descriptor_set.pBufferInfo = nullptr;
                write_descriptor_set.pImageInfo = &std::get<VkDescriptorImageInfo> ( mTextureStore.at ( std::get<1> ( aMaterial.GetSamplers() [i] ).Get<Texture>()->GetConsecutiveId() ) );
                write_descriptor_set.pTexelBufferView = nullptr;
            }
            vkUpdateDescriptorSets ( GetDevice(), static_cast<uint32_t> ( write_descriptor_sets.size() ), write_descriptor_sets.data(), 0, nullptr );
        }
    }

    void VulkanRenderer::UnloadMaterial ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it == mMaterialStore.end() )
        {
            return;
        }
        // Finalize Descriptor Pool
        if ( std::get<VkDescriptorPool> ( it->second ) != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( GetDevice(), std::get<VkDescriptorPool> ( it->second ), nullptr );
        }
        if ( std::get<std::unique_ptr<VulkanBuffer>> ( it->second ) != nullptr )
        {
            std::get<std::unique_ptr<VulkanBuffer>> ( it->second )->Finalize();
        }
        mMaterialStore.erase ( it );
    }

    void VulkanRenderer::LoadTexture ( const Texture& aTexture )
    {
        auto it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        if ( it != mTextureStore.end() )
        {
            std::cout << LogLevel::Warning << " Texture " << aTexture.GetConsecutiveId() << " Already Loaded at: " << __FUNCTION__ << std::endl;
            return;
        }

        VkImage image{VK_NULL_HANDLE};
        VkDeviceMemory device_memory{VK_NULL_HANDLE};
        // InitializeTexture ( mFormat, mType );
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.pNext = nullptr;
        image_create_info.flags = 0;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM; // This field contains both format and type, calculate rather than hardcode
        VkFormatProperties format_properties{};
        vkGetPhysicalDeviceFormatProperties ( GetPhysicalDevice(), image_create_info.format, &format_properties );
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
        if ( VkResult result = vkCreateImage ( GetDevice(), &image_create_info, nullptr, &image ) )
        {
            std::ostringstream stream;
            stream << "Image creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements memory_requirements{};
        vkGetImageMemoryRequirements ( GetDevice(), image, &memory_requirements );
        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.pNext = nullptr;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = FindMemoryTypeIndex ( memory_requirements.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        if ( memory_allocate_info.memoryTypeIndex == std::numeric_limits<uint32_t>::max() )
        {
            throw std::runtime_error ( "Unable to find a suitable memory type index" );
        }
        if ( VkResult result = vkAllocateMemory ( GetDevice(), &memory_allocate_info, nullptr, &device_memory ) )
        {
            std::ostringstream stream;
            stream << "Image Memory Allocation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        if ( VkResult result = vkBindImageMemory ( GetDevice(), image, device_memory, 0 ) )
        {
            std::ostringstream stream;
            stream << "Bind Image Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        ///InitializeTextureView();

        auto texture = mTextureStore.emplace ( aTexture.GetConsecutiveId(),
                                               std::tuple<VkImage, VkDeviceMemory, VkDescriptorImageInfo> {image, device_memory, {}} );
        if ( !texture.second )
        {
            std::ostringstream stream;
            stream << "Emplace failed at: ( " << __FUNCTION__ << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        std::get<VkDescriptorImageInfo> ( texture.first->second ).imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        if ( VkResult result = vkCreateImageView ( GetDevice(), &image_view_create_info, nullptr, &std::get<VkDescriptorImageInfo> ( texture.first->second ).imageView ) )
        {
            std::ostringstream stream;
            stream << "Create Image View failed: ( " << GetVulkanResultString ( result ) << " )";
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
        if ( VkResult result = vkCreateSampler ( GetDevice(), &sampler_create_info, nullptr, &std::get<VkDescriptorImageInfo> ( texture.first->second ).sampler ) )
        {
            std::ostringstream stream;
            stream << "Sampler creation failed: ( " << GetVulkanResultString ( result ) << " )";
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

        if ( VkResult result = vkCreateBuffer ( GetDevice(), &buffer_create_info, nullptr, &image_buffer ) )
        {
            std::ostringstream stream;
            stream << "Create Buffer failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        memset ( &memory_requirements, 0, sizeof ( VkMemoryRequirements ) );
        vkGetBufferMemoryRequirements ( GetDevice(), image_buffer, &memory_requirements );
        memset ( &memory_allocate_info, 0, sizeof ( VkMemoryAllocateInfo ) );
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = FindMemoryTypeIndex ( memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

        if ( VkResult result = vkAllocateMemory ( GetDevice(), &memory_allocate_info, nullptr, &image_buffer_memory ) )
        {
            std::ostringstream stream;
            stream << "Allocate Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        if ( VkResult result = vkBindBufferMemory ( GetDevice(), image_buffer, image_buffer_memory, 0 ) )
        {
            std::ostringstream stream;
            stream << "Bind Buffer Memory failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        void* image_memory = nullptr;
        if ( VkResult result = vkMapMemory ( GetDevice(), image_buffer_memory, 0, VK_WHOLE_SIZE, 0, &image_memory ) )
        {
            std::ostringstream stream;
            stream << "Map Memory failed: ( " << GetVulkanResultString ( result ) << " )";
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
        vkUnmapMemory ( GetDevice(), image_buffer_memory );

        //--------------------------------------------------------
        // Transition Image Layout
        VkCommandBuffer command_buffer = BeginSingleTimeCommands();
        VkImageMemoryBarrier image_memory_barrier{};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_memory_barrier.image = image;
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
        vkCmdCopyBufferToImage ( command_buffer, image_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy );

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
        EndSingleTimeCommands ( command_buffer );
        vkDestroyBuffer ( GetDevice(), image_buffer, nullptr );
        vkFreeMemory ( GetDevice(), image_buffer_memory, nullptr );
    }

    void VulkanRenderer::UnloadTexture ( const Texture& aTexture )
    {
        auto it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        if ( it == mTextureStore.end() )
        {
            return;
        }

        if ( std::get<VkDescriptorImageInfo> ( it->second ).sampler != VK_NULL_HANDLE )
        {
            vkDestroySampler ( GetDevice(), std::get<VkDescriptorImageInfo> ( it->second ).sampler, nullptr );
        }
        if ( std::get<VkDescriptorImageInfo> ( it->second ).imageView != VK_NULL_HANDLE )
        {
            vkDestroyImageView ( GetDevice(), std::get<VkDescriptorImageInfo> ( it->second ).imageView, nullptr );
        }
        if ( std::get<VkImage> ( it->second ) != VK_NULL_HANDLE )
        {
            vkDestroyImage ( GetDevice(), std::get<VkImage> ( it->second ), nullptr );
        }
        if ( std::get<VkDeviceMemory> ( it->second ) != VK_NULL_HANDLE )
        {
            vkFreeMemory ( GetDevice(), std::get<VkDeviceMemory> ( it->second ), nullptr );
        }
        mTextureStore.erase ( it );
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

    BufferAccessor VulkanRenderer::AllocateSingleFrameUniformMemory ( size_t aSize )
    {
        return mMemoryPoolBuffer.Allocate ( aSize );
    }
    void VulkanRenderer::ResetMemoryPoolBuffer()
    {
        mMemoryPoolBuffer.Reset();
    }
}
