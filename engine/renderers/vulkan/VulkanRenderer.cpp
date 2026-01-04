/*
Copyright (C) 2016-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/LogLevel.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/MemoryPool.hpp"
#include "SPIR-V/CompilerLinker.h"

namespace AeonGames
{
    VulkanRenderer::VulkanRenderer ( void* aWindow )
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
            InitializeCommandPools();
            AttachWindow ( aWindow );
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
        mWindowStore.clear();
        mTextureStore.clear();
        mMaterialStore.clear();
        mPipelineStore.clear();
        mMeshStore.clear();
        for ( auto& i : mVkDescriptorSetLayouts )
        {
            vkDestroyDescriptorSetLayout ( mVkDevice, std::get<1> ( i ), nullptr );
        }
        mVkDescriptorSetLayouts.clear();
        FinalizeCommandPools();
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

    VkRenderPass VulkanRenderer::GetRenderPass() const
    {
        auto it = mWindowStore.begin();
        if ( it != mWindowStore.end() )
        {
            return it->second.GetRenderPass();
        }
        std::cout << LogLevel::Error << __FUNCTION__ << " No RenderPass found!" << std::endl;
        return VK_NULL_HANDLE;
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
                std::cout << LogLevel::Error << "vkGetInstanceProcAddr failed to load vkCreateDebugUtilsMessengerEXT" << std::endl;
                throw std::runtime_error ( "vkGetInstanceProcAddr failed to load vkCreateDebugUtilsMessengerEXT" );
            }
            if ( ( vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT> ( vkGetInstanceProcAddr ( mVkInstance, "vkDestroyDebugUtilsMessengerEXT" ) ) ) == nullptr )
            {
                std::cout << LogLevel::Error << "vkGetInstanceProcAddr failed to load vkDestroyDebugUtilsMessengerEXT" << std::endl;
                throw std::runtime_error ( "vkGetInstanceProcAddr failed to load vkDestroyDebugUtilsMessengerEXT" );
            }
            mFunctionsLoaded = true;
        }
    }


    void VulkanRenderer::SetupDebug()
    {
        mInstanceExtensionNames.emplace_back ( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
        mInstanceExtensionNames.emplace_back ( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
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
#elif defined (VK_USE_PLATFORM_METAL_EXT)
        mInstanceExtensionNames.push_back ( VK_EXT_METAL_SURFACE_EXTENSION_NAME );
        mInstanceExtensionNames.push_back ( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
        mInstanceExtensionNames.push_back ( VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME );
        mDeviceExtensionNames.push_back ( "VK_KHR_portability_subset" );
        mDeviceExtensionNames.push_back ( VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME );
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
        application_info.apiVersion = VK_API_VERSION_1_3;
        application_info.applicationVersion = VK_MAKE_VERSION ( 0, 1, 0 );
        application_info.pApplicationName = "AeonEngine Vulkan Renderer";

        // Validate that all requested layers are available
        {
            uint32_t available_layer_count;
            vkEnumerateInstanceLayerProperties ( &available_layer_count, nullptr );
            std::vector<VkLayerProperties> available_layers ( available_layer_count );
            vkEnumerateInstanceLayerProperties ( &available_layer_count, available_layers.data() );
            std::cout << LogLevel::Info << "VulkanRenderer Available Layers" << std::endl;
            for ( auto& i : available_layers )
            {
                std::cout << LogLevel::Info << "  - " << i.layerName << ": " << i.description << std::endl;
                {
                    uint32_t instance_extension_layer_count;
                    vkEnumerateInstanceExtensionProperties ( i.layerName, &instance_extension_layer_count, nullptr );
                    std::vector<VkExtensionProperties> instance_extension_layer_list ( instance_extension_layer_count );
                    vkEnumerateInstanceExtensionProperties ( i.layerName, &instance_extension_layer_count, instance_extension_layer_list.data() );
                    for ( auto& j : instance_extension_layer_list )
                    {
                        std::cout << LogLevel::Info << "    - " << j.extensionName << std::endl;
                    }
                }
            }

            for ( auto it = mInstanceLayerNames.begin(); it != mInstanceLayerNames.end(); )
            {
                bool layer_found = false;
                for ( const auto& available_layer : available_layers )
                {
                    if ( strcmp ( *it, available_layer.layerName ) == 0 )
                    {
                        layer_found = true;
                        break;
                    }
                }
                if ( !layer_found )
                {
                    std::cout << LogLevel::Warning << "Requested layer '" << *it
                              << "' is not available." << std::endl;
                    it = mInstanceLayerNames.erase ( it ); // erase returns an iterator to the next element
                }
                else
                {
                    ++it; // only increment if no erase occurred
                }
            }
        }

        // Validate that all requested extensions are available
        {
            // Get global instance extensions
            uint32_t available_extension_count;
            vkEnumerateInstanceExtensionProperties ( nullptr, &available_extension_count, nullptr );
            std::vector<VkExtensionProperties> available_extensions ( available_extension_count );
            vkEnumerateInstanceExtensionProperties ( nullptr, &available_extension_count, available_extensions.data() );

            // Also get extensions provided by layers
            std::vector<VkExtensionProperties> layer_extensions;
            for ( const char * layer_name : mInstanceLayerNames )
            {
                uint32_t layer_extension_count;
                vkEnumerateInstanceExtensionProperties ( layer_name, &layer_extension_count, nullptr );
                std::vector<VkExtensionProperties> layer_ext_props ( layer_extension_count );
                vkEnumerateInstanceExtensionProperties ( layer_name, &layer_extension_count, layer_ext_props.data() );
                layer_extensions.insert ( layer_extensions.end(), layer_ext_props.begin(), layer_ext_props.end() );
            }

            std::cout << LogLevel::Info << "VulkanRenderer Available Instance Extensions" << std::endl;
            for ( const auto& extension : available_extensions )
            {
                std::cout << LogLevel::Info << "  - " << extension.extensionName << " (version " << extension.specVersion << ")" << std::endl;
            }

            for ( auto it = mInstanceExtensionNames.begin(); it != mInstanceExtensionNames.end(); )
            {
                bool extension_found = false;

                // Check global extensions
                for ( const auto& available_extension : available_extensions )
                {
                    if ( strcmp ( *it, available_extension.extensionName ) == 0 )
                    {
                        extension_found = true;
                        break;
                    }
                }

                // If not found in global extensions, check layer extensions
                if ( !extension_found )
                {
                    for ( const auto& layer_extension : layer_extensions )
                    {
                        if ( strcmp ( *it, layer_extension.extensionName ) == 0 )
                        {
                            extension_found = true;
                            break;
                        }
                    }
                }

                if ( !extension_found )
                {
                    std::cout << LogLevel::Warning << "Requested extension '" << *it
                              << "' is not available." << std::endl;
                    it = mInstanceExtensionNames.erase ( it ); // erase returns an iterator to the next element
                }
                else
                {
                    ++it; // only increment if no erase occurred
                }
            }
        }

        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.enabledLayerCount = static_cast<uint32_t> ( mInstanceLayerNames.size() );
        instance_create_info.ppEnabledLayerNames = mInstanceLayerNames.data();
        instance_create_info.enabledExtensionCount = static_cast<uint32_t> ( mInstanceExtensionNames.size() );
        instance_create_info.ppEnabledExtensionNames = mInstanceExtensionNames.data();
#ifdef VK_USE_PLATFORM_METAL_EXT
        instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
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
            std::cout << LogLevel::Error << "mVkInstance is a nullptr." << std::endl;
            throw std::runtime_error ( "mVkInstance is a nullptr." );
        }
        {
            uint32_t physical_device_count;
            vkEnumeratePhysicalDevices ( mVkInstance, &physical_device_count, nullptr );

            if ( physical_device_count == 0 )
            {
                std::cout << LogLevel::Error << "No VulkanRenderer physical device found" << std::endl;
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
        {
            uint32_t family_properties_count;
            vkGetPhysicalDeviceQueueFamilyProperties ( mVkPhysicalDevice, &family_properties_count, nullptr );
            if ( family_properties_count == 0 )
            {
                std::cout << LogLevel::Error << "VulkanRenderer physical device has no queue family properties." << std::endl;
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
                std::cout << LogLevel::Error << "No graphics queue family found." << std::endl;
                throw std::runtime_error ( "No graphics queue family found." );
            }
        }

        {
            uint32_t extension_property_count;
            vkEnumerateDeviceExtensionProperties ( mVkPhysicalDevice, nullptr, &extension_property_count, nullptr );
            std::vector<VkExtensionProperties> extension_properties ( extension_property_count );
            vkEnumerateDeviceExtensionProperties ( mVkPhysicalDevice, nullptr, &extension_property_count, extension_properties.data() );
            std::cout << LogLevel::Info << "Vulkan Renderer Available Device Extensions" << std::endl;
            for ( auto& i : extension_properties )
            {
                std::cout << LogLevel::Info << "  - " << i.extensionName << " (version " << i.specVersion << ")" << std::endl;

                if ( !strcmp ( i.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME ) )
                {
                    mDeviceExtensionNames.emplace_back ( VK_EXT_DEBUG_MARKER_EXTENSION_NAME );
                }
            }
        }

        /// @todo Remove hardcoded Queue Info
        float queue_priorities[] {1.0f};
        VkDeviceQueueCreateInfo device_queue_create_info{};
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.pNext = nullptr;
        device_queue_create_info.queueCount = 1;
        device_queue_create_info.queueFamilyIndex = mQueueFamilyIndex;
        device_queue_create_info.pQueuePriorities = queue_priorities;

#ifdef VK_USE_PLATFORM_METAL_EXT
        // For some reason this causes a crash on AMD drivers on the RogAlly, so keep it MacOS only while I figure it out.
        VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT physical_device_primitive_topology_list_restart_features {};
        physical_device_primitive_topology_list_restart_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;
        physical_device_primitive_topology_list_restart_features.pNext = nullptr;
        physical_device_primitive_topology_list_restart_features.primitiveTopologyListRestart = VK_TRUE;
        physical_device_primitive_topology_list_restart_features.primitiveTopologyPatchListRestart = VK_TRUE;
#endif
        VkDeviceCreateInfo device_create_info {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
#ifdef VK_USE_PLATFORM_METAL_EXT
        // For some reason this causes a crash on AMD drivers on the RogAlly, so keep it MacOS only while I figure it out.
        device_create_info.pNext = &physical_device_primitive_topology_list_restart_features;
#else
        device_create_info.pNext = nullptr;
#endif
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &device_queue_create_info;
        device_create_info.enabledLayerCount = static_cast<uint32_t> ( mDeviceLayerNames.size() );
        device_create_info.ppEnabledLayerNames = mDeviceLayerNames.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t> ( mDeviceExtensionNames.size() );
        device_create_info.ppEnabledExtensionNames = mDeviceExtensionNames.data();
        device_create_info.pEnabledFeatures = nullptr;

        /// @todo Grab best device rather than first one
        if ( VkResult result = vkCreateDevice ( mVkPhysicalDevice, &device_create_info, nullptr, &mVkDevice ) )
        {
            std::ostringstream stream;
            stream << "Could not create VulkanRenderer device. error code: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
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
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanRenderer::InitializeFence()
    {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vkCreateFence ( mVkDevice, &fence_create_info, nullptr, &mVkFence );
    }

    void VulkanRenderer::InitializeCommandPools()
    {
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = mQueueFamilyIndex;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        vkCreateCommandPool ( mVkDevice, &command_pool_create_info, nullptr, &mVkSingleTimeCommandPool );
    }

    void VulkanRenderer::FinalizeCommandPools()
    {
        if ( mVkSingleTimeCommandPool != VK_NULL_HANDLE )
        {
            vkDestroyCommandPool ( mVkDevice, mVkSingleTimeCommandPool, nullptr );
            mVkSingleTimeCommandPool = VK_NULL_HANDLE;
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


    VkCommandBuffer VulkanRenderer::BeginSingleTimeCommands() const
    {
        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        VkCommandBuffer command_buffer{};
        VkCommandBufferBeginInfo beginInfo{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandPool = mVkSingleTimeCommandPool;
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
        vkFreeCommandBuffers ( mVkDevice, mVkSingleTimeCommandPool, 1, &aVkCommandBuffer );
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

    const VulkanMesh* VulkanRenderer::GetVulkanMesh ( const Mesh& aMesh )
    {
        auto it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        if ( it == mMeshStore.end() )
        {
            LoadMesh ( aMesh );
            it = mMeshStore.find ( aMesh.GetConsecutiveId() );
        }
        if ( it != mMeshStore.end() )
        {
            return &it->second;
        }
        return nullptr;
    }

    /*-----------------Pipeline-----------------------*/

    const VulkanPipeline* VulkanRenderer::GetVulkanPipeline ( const Pipeline& aPipeline )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it == mPipelineStore.end() )
        {
            LoadPipeline ( aPipeline );
            it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        }
        if ( it != mPipelineStore.end() )
        {
            return &it->second;
        }
        return nullptr;
    }

    const VulkanMaterial* VulkanRenderer::GetVulkanMaterial ( const Material& aMaterial )
    {
        auto it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        if ( it == mMaterialStore.end() )
        {
            LoadMaterial ( aMaterial );
            it = mMaterialStore.find ( aMaterial.GetConsecutiveId() );
        }

        if ( it != mMaterialStore.end() )
        {
            return &it->second;
        }
        return nullptr;
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

    const VkDescriptorSetLayout& VulkanRenderer::GetDescriptorSetLayout ( const VkDescriptorSetLayoutCreateInfo& aDescriptorSetLayoutCreateInfo ) const
    {
        uint32_t key
        {
            crc32i ( reinterpret_cast<const char*> ( &aDescriptorSetLayoutCreateInfo.bindingCount ), sizeof ( uint32_t ),
                     crc32i ( reinterpret_cast<const char*> ( &aDescriptorSetLayoutCreateInfo.flags ), sizeof ( VkDescriptorSetLayoutCreateFlags ),
                              crc32i ( reinterpret_cast<const char*> ( &aDescriptorSetLayoutCreateInfo.sType ), sizeof ( VkStructureType ) ) ) )
        };

        for ( uint32_t i = 0; i < aDescriptorSetLayoutCreateInfo.bindingCount; ++i )
        {
            key =
                crc32i ( reinterpret_cast<const char*> ( &aDescriptorSetLayoutCreateInfo.pBindings[i].stageFlags ), sizeof ( VkShaderStageFlags ),
                         crc32i ( reinterpret_cast<const char*> ( &aDescriptorSetLayoutCreateInfo.pBindings[i].descriptorCount ), sizeof ( uint32_t ),
                                  crc32i ( reinterpret_cast<const char*> ( &aDescriptorSetLayoutCreateInfo.pBindings[i].descriptorType ), sizeof ( VkDescriptorType ),
                                           crc32i ( reinterpret_cast<const char*> ( &aDescriptorSetLayoutCreateInfo.pBindings[i].binding ), sizeof ( uint32_t ), key ) ) ) );
        }

        auto lb = std::lower_bound ( mVkDescriptorSetLayouts.begin(), mVkDescriptorSetLayouts.end(), key,
                                     [] ( const std::tuple<size_t, VkDescriptorSetLayout>& a, size_t b )
        {
            return std::get<0> ( a ) < b;
        } );

        if ( lb != mVkDescriptorSetLayouts.end() && std::get<0> ( *lb ) == key )
        {
            return std::get<1> ( *lb );
        }
        VkDescriptorSetLayout descriptor_set_layout;
        if ( VkResult result = vkCreateDescriptorSetLayout ( mVkDevice, &aDescriptorSetLayoutCreateInfo, nullptr, &descriptor_set_layout ) )
        {
            std::ostringstream stream;
            stream << "DescriptorSet Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        lb = mVkDescriptorSetLayouts.insert ( lb, { {key}, {descriptor_set_layout} } );
        return std::get<1> ( *lb );
    }
#if 0
    const VkDescriptorSetLayout& VulkanRenderer::GetSamplerDescriptorSetLayout ( size_t aSamplerCount ) const
    {
        auto lb = std::lower_bound ( mVkDescriptorSetLayouts.begin(), mVkDescriptorSetLayouts.end(), aSamplerCount,
                                     [] ( const std::tuple<size_t, VkDescriptorSetLayout>&a, size_t b )
        {
            return std::get<0> ( a ) < b;
        } );
        if ( lb != mVkDescriptorSetLayouts.end() && std::get<0> ( *lb ) == aSamplerCount )
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
            lb = mVkDescriptorSetLayouts.insert ( lb, {{aSamplerCount}, {VK_NULL_HANDLE}} );
            if ( VkResult result = vkCreateDescriptorSetLayout ( mVkDevice, &descriptor_set_layout_create_info, nullptr,
                                   &std::get<1> ( *lb ) ) )
            {
                std::ostringstream stream;
                stream << "DescriptorSet Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
            return std::get<1> ( *lb );
        }
        std::cout << LogLevel::Error << "Sampler Count must be > 0" << std::endl;
        throw std::runtime_error ( "Sampler Count must be > 0" );
    }
#endif
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
            std::cout << LogLevel::Error << stream.str() << std::endl;
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

    void VulkanRenderer::AttachWindow ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it != mWindowStore.end() )
        {
            std::cout << LogLevel::Warning << " Window " << aWindowId << " Already Loaded at: " << __FUNCTION__ << std::endl;
            return;
        }
        mWindowStore.emplace ( aWindowId, VulkanWindow{*this, aWindowId} );
    }
    void VulkanRenderer::DetachWindow ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        mWindowStore.erase ( it );
    }

    void VulkanRenderer::SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetProjectionMatrix ( aMatrix );
    }

    void VulkanRenderer::SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetViewMatrix ( aMatrix );
    }

    void VulkanRenderer::ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.ResizeViewport ( aX, aY, aWidth, aHeight );
    }

    void VulkanRenderer::BeginRender ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginRender();
    }
    void VulkanRenderer::EndRender ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndRender();
    }
    void VulkanRenderer::Render ( void* aWindowId,
                                  const Matrix4x4& aModelMatrix,
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
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Render ( aModelMatrix, aMesh, aPipeline, aMaterial, aSkeleton, aTopology, aVertexStart, aVertexCount, aInstanceCount, aFirstInstance );
    }

    const Frustum& VulkanRenderer::GetFrustum ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            std::cout << LogLevel::Error << "Unknown Window Id." << std::endl;
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.GetFrustum();
    }

    BufferAccessor VulkanRenderer::AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            std::cout << LogLevel::Error << "Unknown Window Id." << std::endl;
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.AllocateSingleFrameUniformMemory ( aSize );
    }

    void VulkanRenderer::SetClearColor ( void* aWindowId, float R, float G, float B, float A )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        ///@todo finish this
        //it->second.SetClearColor ( R, G, B, A );
    }
}
