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
    https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_debug_utils.html
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
    VulkanRenderer::VulkanRenderer ( void* aWindow ) :
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
        mTextureStore.clear();
        mMaterialStore.clear();
        mPipelineStore.clear();
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

    void VulkanRenderer::LoadFunctions()
    {
        assert ( mVkInstance && "mVkInstance is a nullptr." );
        if ( !mFunctionsLoaded && mVkInstance )
        {

            if ( ( vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT> ( vkGetInstanceProcAddr ( mVkInstance, "vkCreateDebugUtilsMessengerEXT" ) ) ) == nullptr )
            {
                throw std::runtime_error ( "vkGetInstanceProcAddr failed to load vkCreateDebugUtilsMessengerEXT" );
            }
            if ( ( vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT> ( vkGetInstanceProcAddr ( mVkInstance, "vkDestroyDebugUtilsMessengerEXT" ) ) ) == nullptr )
            {
                throw std::runtime_error ( "vkGetInstanceProcAddr failed to load vkDestroyDebugUtilsMessengerEXT" );
            }
            mFunctionsLoaded = true;
        }
    }


    void VulkanRenderer::SetupDebug()
    {
        mInstanceExtensionNames.emplace_back ( VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME );
        mInstanceExtensionNames.emplace_back ( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
        mInstanceLayerNames.emplace_back ( "VK_LAYER_KHRONOS_validation" );
    }

    void VulkanRenderer::InitializeDebug()
    {
        VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info_ext
        {
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            nullptr,
            0,
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            DebugCallback,
            nullptr
        };

        if ( VkResult result = vkCreateDebugUtilsMessengerEXT ( mVkInstance, &debug_utils_messenger_create_info_ext, nullptr, &mVkDebugUtilsMessengerEXT ) )
        {
            std::ostringstream stream;
            stream << "Could not create Debug Utils Messenger. error code: ( " << GetVulkanResultString ( result ) << " )";
            std::string error_string = stream.str();
            std::cout << LogLevel::Error << error_string << std::endl;
            throw std::runtime_error ( error_string.c_str() );
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
            instance_create_info.pNext = &validation_features_ext;
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
        if ( mVkInstance && ( mVkDebugUtilsMessengerEXT != VK_NULL_HANDLE ) )
        {
            vkDestroyDebugUtilsMessengerEXT ( mVkInstance, mVkDebugUtilsMessengerEXT, nullptr );
            mVkDebugUtilsMessengerEXT = VK_NULL_HANDLE;
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

    void VulkanRenderer::BindMesh ( const Mesh& aMesh )
    {
        auto it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        if ( it == mMeshStore.end() )
        {
            LoadMesh ( aMesh );
            it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        }
        it->second.Bind();
    }

    /*-----------------Pipeline-----------------------*/
    void VulkanRenderer::BindPipeline ( const Pipeline& aPipeline )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it == mPipelineStore.end() )
        {
            LoadPipeline ( aPipeline );
            it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        }
        mBoundPipeline = &it->second;
        vkCmdBindPipeline ( GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, it->second.GetPipeline() );
        ///@todo Move Matrices binding to BeginRender
        vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  it->second.GetPipelineLayout(),
                                  MATRICES,
                                  1,
                                  &mMatricesDescriptorSet, 0, nullptr );
    }

    void VulkanRenderer::SetMaterial ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it == mMaterialStore.end() )
        {
            LoadMaterial ( aMaterial );
            it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        }
        it->second.Bind ( mBoundPipeline->GetPipelineLayout() );
    }

    void VulkanRenderer::SetSkeleton ( const BufferAccessor& aSkeletonBuffer ) const
    {
        uint32_t offset = static_cast<uint32_t> ( aSkeletonBuffer.GetOffset() );
        vkCmdBindDescriptorSets ( GetCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  mBoundPipeline->GetPipelineLayout(),
                                  SKELETON,
                                  1,
                                  &mMemoryPoolBuffer.GetDescriptorSet(), 1, &offset );
    }
    void VulkanRenderer::SetModelMatrix ( const Matrix4x4& aMatrix )
    {
        vkCmdPushConstants ( GetCommandBuffer(),
                             mBoundPipeline->GetPipelineLayout(),
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
        mPipelineStore.emplace ( aPipeline.GetConsecutiveId(), VulkanPipeline{*this, aPipeline} );
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
        mPipelineStore.erase ( it );
    }

    /*---------------------Material---------------------*/
    void VulkanRenderer::LoadMaterial ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it != mMaterialStore.end() )
        {
            std::cout << LogLevel::Warning << " Material " << aMaterial.GetConsecutiveId() << " already loaded." << std::endl;
            return;
        }
        // Preload linked textures
        for ( auto& i : aMaterial.GetSamplers() )
        {
            const Texture* texture = std::get<1> ( i ).Get<Texture>();
            if ( mTextureStore.find ( texture->GetConsecutiveId() ) == mTextureStore.end() )
            {
                LoadTexture ( *texture );
            }
        }
        mMaterialStore.emplace ( aMaterial.GetConsecutiveId(), VulkanMaterial{*this, aMaterial} );
    }

    void VulkanRenderer::UnloadMaterial ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it == mMaterialStore.end() )
        {
            return;
        }
        // Unload linked textures
        for ( auto& i : aMaterial.GetSamplers() )
        {
            const Texture* texture = std::get<1> ( i ).Get<Texture>();
            if ( mTextureStore.find ( texture->GetConsecutiveId() ) != mTextureStore.end() )
            {
                UnloadTexture ( *texture );
            }
        }
        mMaterialStore.erase ( it );
    }

    void VulkanRenderer::LoadTexture ( const Texture& aTexture )
    {
        auto it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        if ( it != mTextureStore.end() )
        {
            std::cout << LogLevel::Warning << " Texture " << aTexture.GetConsecutiveId() << " already loaded at: " << __FUNCTION__ << std::endl;
            return;
        }
        mTextureStore.emplace ( aTexture.GetConsecutiveId(), VulkanTexture{*this, aTexture} );
    }

    void VulkanRenderer::UnloadTexture ( const Texture& aTexture )
    {
        auto it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        if ( it == mTextureStore.end() )
        {
            return;
        }
        mTextureStore.erase ( it );
    }

    const VkDescriptorImageInfo* VulkanRenderer::GetTextureDescriptorImageInfo ( const Texture& aTexture ) const
    {
        auto it = mTextureStore.find ( aTexture.GetConsecutiveId() );
        if ( it == mTextureStore.end() )
        {
            std::ostringstream stream;
            stream << "Texture Not Found at: ( " << __FUNCTION__ << " )";
            std::string error_string = stream.str();
            std::cout << LogLevel::Error << error_string << std::endl;
            throw std::runtime_error ( error_string.c_str() );
        }
        return &it->second.GetDescriptorImageInfo();
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

    void VulkanRenderer::AttachWindow ( void* aWindowId )
    {

    }
    void VulkanRenderer::DetachWindow ( void* aWindowId )
    {

    }
}
