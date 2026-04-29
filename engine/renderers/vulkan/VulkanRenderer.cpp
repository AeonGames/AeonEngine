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
#include "VulkanRenderer.hpp"
#include "VulkanWindow.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanUtilities.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/GuiOverlay.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/MemoryPool.hpp"
#include "SPIR-V/CompilerLinker.hpp"

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
            InitializeCommandPools();
            AttachWindow ( aWindow );
            InitializeOverlay();
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
        FinalizeOverlay();
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

    bool VulkanRenderer::HasPrimitiveTopologyListRestart() const
    {
        return mHasPrimitiveTopologyListRestart;
    }

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
        mDeviceExtensionNames.push_back ( VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME );
#elif defined (VK_USE_PLATFORM_METAL_EXT)
        mInstanceExtensionNames.push_back ( VK_EXT_METAL_SURFACE_EXTENSION_NAME );
        mInstanceExtensionNames.push_back ( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
        mInstanceExtensionNames.push_back ( VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME );
        mDeviceExtensionNames.push_back ( "VK_KHR_portability_subset" );
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
                        static_cast<uint32_t> ( family_property - family_properties_list.begin() );
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
        // Query whether the device actually supports primitive topology list restart.
        VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT supported_restart_features {};
        supported_restart_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;
        supported_restart_features.pNext = nullptr;
        VkPhysicalDeviceFeatures2 physical_device_features2 {};
        physical_device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physical_device_features2.pNext = &supported_restart_features;
        vkGetPhysicalDeviceFeatures2 ( mVkPhysicalDevice, &physical_device_features2 );

        VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT physical_device_primitive_topology_list_restart_features {};
        physical_device_primitive_topology_list_restart_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT;
        physical_device_primitive_topology_list_restart_features.pNext = nullptr;
        physical_device_primitive_topology_list_restart_features.primitiveTopologyListRestart = supported_restart_features.primitiveTopologyListRestart;
        physical_device_primitive_topology_list_restart_features.primitiveTopologyPatchListRestart = supported_restart_features.primitiveTopologyPatchListRestart;
        bool use_primitive_topology_list_restart = ( supported_restart_features.primitiveTopologyListRestart == VK_TRUE ) ||
            ( supported_restart_features.primitiveTopologyPatchListRestart == VK_TRUE );
        mHasPrimitiveTopologyListRestart = ( supported_restart_features.primitiveTopologyListRestart == VK_TRUE );
#endif
        VkDeviceCreateInfo device_create_info {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
#ifdef VK_USE_PLATFORM_METAL_EXT
        device_create_info.pNext = use_primitive_topology_list_restart ? &physical_device_primitive_topology_list_restart_features : nullptr;
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
                std::cout << LogLevel::Error << "vkDeviceWaitIdle returned " << GetVulkanResultString ( result ) << std::endl;
            }
            vkDestroyDevice ( mVkDevice, nullptr );
            mVkDevice = VK_NULL_HANDLE;
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

    static const char overlay_vertex_shader_code[] =
        R"(#version 450
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

layout (location = 0) out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
}
)";

    static const char overlay_fragment_shader_code[] =
        R"(#version 450
layout (location = 0) in vec2 TexCoords;

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform sampler2D OverlayTexture;

void main()
{
    FragColor = texture(OverlayTexture, TexCoords);
}
)";

    static const float overlay_vertices[] =
    {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    void VulkanRenderer::InitializeOverlay()
    {
        // Compile shaders to SPIR-V
        CompilerLinker compiler_linker;
        compiler_linker.AddShaderSource ( EShLangVertex, overlay_vertex_shader_code );
        compiler_linker.AddShaderSource ( EShLangFragment, overlay_fragment_shader_code );
        if ( CompilerLinker::FailCode result = compiler_linker.CompileAndLink() )
        {
            std::ostringstream stream;
            stream << "Overlay shader " << ( ( result == CompilerLinker::EFailCompile ) ? "compilation" : "linking" )
                   << " failed:" << std::endl << compiler_linker.GetLog();
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        const auto& vert_spirv = compiler_linker.GetSpirV ( EShLangVertex );
        const auto& frag_spirv = compiler_linker.GetSpirV ( EShLangFragment );

        VkShaderModule vert_module{ VK_NULL_HANDLE };
        VkShaderModule frag_module{ VK_NULL_HANDLE };

        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize = vert_spirv.size() * sizeof ( uint32_t );
        shader_module_create_info.pCode = vert_spirv.data();
        if ( VkResult result = vkCreateShaderModule ( mVkDevice, &shader_module_create_info, nullptr, &vert_module ) )
        {
            std::ostringstream stream;
            stream << "Overlay vertex shader module creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        shader_module_create_info.codeSize = frag_spirv.size() * sizeof ( uint32_t );
        shader_module_create_info.pCode = frag_spirv.data();
        if ( VkResult result = vkCreateShaderModule ( mVkDevice, &shader_module_create_info, nullptr, &frag_module ) )
        {
            vkDestroyShaderModule ( mVkDevice, vert_module, nullptr );
            std::ostringstream stream;
            stream << "Overlay fragment shader module creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        // Create sampler
        VkSamplerCreateInfo sampler_create_info{};
        sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.magFilter = VK_FILTER_NEAREST;
        sampler_create_info.minFilter = VK_FILTER_NEAREST;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_create_info.maxLod = 1.0f;
        sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        if ( VkResult result = vkCreateSampler ( mVkDevice, &sampler_create_info, nullptr, &mOverlaySampler ) )
        {
            vkDestroyShaderModule ( mVkDevice, vert_module, nullptr );
            vkDestroyShaderModule ( mVkDevice, frag_module, nullptr );
            std::ostringstream stream;
            stream << "Overlay sampler creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        // Create descriptor set layout (one combined image sampler)
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.bindingCount = 1;
        descriptor_set_layout_create_info.pBindings = &binding;
        if ( VkResult result = vkCreateDescriptorSetLayout ( mVkDevice, &descriptor_set_layout_create_info, nullptr, &mOverlayDescriptorSetLayout ) )
        {
            vkDestroyShaderModule ( mVkDevice, vert_module, nullptr );
            vkDestroyShaderModule ( mVkDevice, frag_module, nullptr );
            std::ostringstream stream;
            stream << "Overlay descriptor set layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        // Create pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pSetLayouts = &mOverlayDescriptorSetLayout;
        if ( VkResult result = vkCreatePipelineLayout ( mVkDevice, &pipeline_layout_create_info, nullptr, &mOverlayPipelineLayout ) )
        {
            vkDestroyShaderModule ( mVkDevice, vert_module, nullptr );
            vkDestroyShaderModule ( mVkDevice, frag_module, nullptr );
            std::ostringstream stream;
            stream << "Overlay pipeline layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        // Shader stages
        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages{};
        shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stages[0].module = vert_module;
        shader_stages[0].pName = "main";
        shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stages[1].module = frag_module;
        shader_stages[1].pName = "main";

        // Vertex input: 2 floats position + 2 floats texcoord
        VkVertexInputBindingDescription vertex_binding{};
        vertex_binding.binding = 0;
        vertex_binding.stride = 4 * sizeof ( float );
        vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::array<VkVertexInputAttributeDescription, 2> vertex_attributes{};
        vertex_attributes[0].location = 0;
        vertex_attributes[0].binding = 0;
        vertex_attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
        vertex_attributes[0].offset = 0;
        vertex_attributes[1].location = 1;
        vertex_attributes[1].binding = 0;
        vertex_attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
        vertex_attributes[1].offset = 2 * sizeof ( float );

        VkPipelineVertexInputStateCreateInfo vertex_input_state{};
        vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state.vertexBindingDescriptionCount = 1;
        vertex_input_state.pVertexBindingDescriptions = &vertex_binding;
        vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t> ( vertex_attributes.size() );
        vertex_input_state.pVertexAttributeDescriptions = vertex_attributes.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterization{};
        rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization.cullMode = VK_CULL_MODE_NONE;
        rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample{};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable = VK_FALSE;
        depth_stencil.depthWriteEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState blend_attachment{};
        blend_attachment.blendEnable = VK_TRUE;
        blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
        blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo color_blend{};
        color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend.attachmentCount = 1;
        color_blend.pAttachments = &blend_attachment;

        std::array<VkDynamicState, 2> dynamic_states{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = static_cast<uint32_t> ( dynamic_states.size() );
        dynamic_state.pDynamicStates = dynamic_states.data();

        VkGraphicsPipelineCreateInfo pipeline_create_info{};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.stageCount = static_cast<uint32_t> ( shader_stages.size() );
        pipeline_create_info.pStages = shader_stages.data();
        pipeline_create_info.pVertexInputState = &vertex_input_state;
        pipeline_create_info.pInputAssemblyState = &input_assembly;
        pipeline_create_info.pViewportState = &viewport_state;
        pipeline_create_info.pRasterizationState = &rasterization;
        pipeline_create_info.pMultisampleState = &multisample;
        pipeline_create_info.pDepthStencilState = &depth_stencil;
        pipeline_create_info.pColorBlendState = &color_blend;
        pipeline_create_info.pDynamicState = &dynamic_state;
        pipeline_create_info.layout = mOverlayPipelineLayout;
        pipeline_create_info.renderPass = GetRenderPass();
        pipeline_create_info.subpass = 0;

        if ( VkResult result = vkCreateGraphicsPipelines ( mVkDevice, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &mOverlayPipeline ) )
        {
            vkDestroyShaderModule ( mVkDevice, vert_module, nullptr );
            vkDestroyShaderModule ( mVkDevice, frag_module, nullptr );
            std::ostringstream stream;
            stream << "Overlay pipeline creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        vkDestroyShaderModule ( mVkDevice, vert_module, nullptr );
        vkDestroyShaderModule ( mVkDevice, frag_module, nullptr );

        // Create vertex buffer
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = sizeof ( overlay_vertices );
        buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if ( VkResult result = vkCreateBuffer ( mVkDevice, &buffer_create_info, nullptr, &mOverlayVertexBuffer ) )
        {
            std::ostringstream stream;
            stream << "Overlay vertex buffer creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkMemoryRequirements mem_reqs{};
        vkGetBufferMemoryRequirements ( mVkDevice, mOverlayVertexBuffer, &mem_reqs );
        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_reqs.size;
        alloc_info.memoryTypeIndex = FindMemoryTypeIndex ( mem_reqs.memoryTypeBits,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
        if ( VkResult result = vkAllocateMemory ( mVkDevice, &alloc_info, nullptr, &mOverlayVertexBufferMemory ) )
        {
            std::ostringstream stream;
            stream << "Overlay vertex buffer memory allocation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }
        vkBindBufferMemory ( mVkDevice, mOverlayVertexBuffer, mOverlayVertexBufferMemory, 0 );

        void* mapped = nullptr;
        vkMapMemory ( mVkDevice, mOverlayVertexBufferMemory, 0, sizeof ( overlay_vertices ), 0, &mapped );
        memcpy ( mapped, overlay_vertices, sizeof ( overlay_vertices ) );
        vkUnmapMemory ( mVkDevice, mOverlayVertexBufferMemory );
    }

    void VulkanRenderer::FinalizeOverlay()
    {
        if ( mVkDevice == VK_NULL_HANDLE )
        {
            return;
        }
        for ( auto& [window_id, cache] : mOverlayTextureCache )
        {
            ( void ) window_id;
            if ( cache.stagingBuffer != VK_NULL_HANDLE )
            {
                vkDestroyBuffer ( mVkDevice, cache.stagingBuffer, nullptr );
            }
            if ( cache.stagingMemory != VK_NULL_HANDLE )
            {
                vkFreeMemory ( mVkDevice, cache.stagingMemory, nullptr );
            }
            if ( cache.descriptorPool != VK_NULL_HANDLE )
            {
                vkDestroyDescriptorPool ( mVkDevice, cache.descriptorPool, nullptr );
            }
            if ( cache.imageView != VK_NULL_HANDLE )
            {
                vkDestroyImageView ( mVkDevice, cache.imageView, nullptr );
            }
            if ( cache.image != VK_NULL_HANDLE )
            {
                vkDestroyImage ( mVkDevice, cache.image, nullptr );
            }
            if ( cache.memory != VK_NULL_HANDLE )
            {
                vkFreeMemory ( mVkDevice, cache.memory, nullptr );
            }
        }
        mOverlayTextureCache.clear();
        if ( mOverlayVertexBuffer != VK_NULL_HANDLE )
        {
            vkDestroyBuffer ( mVkDevice, mOverlayVertexBuffer, nullptr );
            mOverlayVertexBuffer = VK_NULL_HANDLE;
        }
        if ( mOverlayVertexBufferMemory != VK_NULL_HANDLE )
        {
            vkFreeMemory ( mVkDevice, mOverlayVertexBufferMemory, nullptr );
            mOverlayVertexBufferMemory = VK_NULL_HANDLE;
        }
        if ( mOverlayPipeline != VK_NULL_HANDLE )
        {
            vkDestroyPipeline ( mVkDevice, mOverlayPipeline, nullptr );
            mOverlayPipeline = VK_NULL_HANDLE;
        }
        if ( mOverlayPipelineLayout != VK_NULL_HANDLE )
        {
            vkDestroyPipelineLayout ( mVkDevice, mOverlayPipelineLayout, nullptr );
            mOverlayPipelineLayout = VK_NULL_HANDLE;
        }
        if ( mOverlayDescriptorSetLayout != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorSetLayout ( mVkDevice, mOverlayDescriptorSetLayout, nullptr );
            mOverlayDescriptorSetLayout = VK_NULL_HANDLE;
        }
        if ( mOverlaySampler != VK_NULL_HANDLE )
        {
            vkDestroySampler ( mVkDevice, mOverlaySampler, nullptr );
            mOverlaySampler = VK_NULL_HANDLE;
        }
    }

    void VulkanRenderer::RenderOverlay ( void* aWindowId, const GuiOverlay& aGuiOverlay )
    {
        const uint8_t* pixels = aGuiOverlay.GetPixels();
        const uint32_t width = aGuiOverlay.GetWidth();
        const uint32_t height = aGuiOverlay.GetHeight();
        if ( !pixels || !width || !height )
        {
            return;
        }

        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }

        auto& cache = mOverlayTextureCache[aWindowId];
        const VkDeviceSize image_size = static_cast<VkDeviceSize> ( width ) * height * 4;

        // Recreate texture resources if size changed
        if ( cache.width != width || cache.height != height )
        {
            vkQueueWaitIdle ( mVkQueue );

            // Clean up old resources
            if ( cache.stagingBuffer != VK_NULL_HANDLE )
            {
                vkDestroyBuffer ( mVkDevice, cache.stagingBuffer, nullptr );
                cache.stagingBuffer = VK_NULL_HANDLE;
            }
            if ( cache.stagingMemory != VK_NULL_HANDLE )
            {
                vkFreeMemory ( mVkDevice, cache.stagingMemory, nullptr );
                cache.stagingMemory = VK_NULL_HANDLE;
            }
            if ( cache.descriptorPool != VK_NULL_HANDLE )
            {
                vkDestroyDescriptorPool ( mVkDevice, cache.descriptorPool, nullptr );
                cache.descriptorPool = VK_NULL_HANDLE;
                cache.descriptorSet = VK_NULL_HANDLE;
            }
            if ( cache.imageView != VK_NULL_HANDLE )
            {
                vkDestroyImageView ( mVkDevice, cache.imageView, nullptr );
                cache.imageView = VK_NULL_HANDLE;
            }
            if ( cache.image != VK_NULL_HANDLE )
            {
                vkDestroyImage ( mVkDevice, cache.image, nullptr );
                cache.image = VK_NULL_HANDLE;
            }
            if ( cache.memory != VK_NULL_HANDLE )
            {
                vkFreeMemory ( mVkDevice, cache.memory, nullptr );
                cache.memory = VK_NULL_HANDLE;
            }

            // Create staging buffer
            VkBufferCreateInfo staging_buffer_info{};
            staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            staging_buffer_info.size = image_size;
            staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            staging_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            vkCreateBuffer ( mVkDevice, &staging_buffer_info, nullptr, &cache.stagingBuffer );

            VkMemoryRequirements staging_mem_reqs{};
            vkGetBufferMemoryRequirements ( mVkDevice, cache.stagingBuffer, &staging_mem_reqs );
            VkMemoryAllocateInfo staging_alloc_info{};
            staging_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            staging_alloc_info.allocationSize = staging_mem_reqs.size;
            staging_alloc_info.memoryTypeIndex = FindMemoryTypeIndex ( staging_mem_reqs.memoryTypeBits,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
            vkAllocateMemory ( mVkDevice, &staging_alloc_info, nullptr, &cache.stagingMemory );
            vkBindBufferMemory ( mVkDevice, cache.stagingBuffer, cache.stagingMemory, 0 );

            // Create image
            VkImageCreateInfo image_info{};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.imageType = VK_IMAGE_TYPE_2D;
            image_info.format = VK_FORMAT_B8G8R8A8_UNORM;
            image_info.extent = { width, height, 1 };
            image_info.mipLevels = 1;
            image_info.arrayLayers = 1;
            image_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            vkCreateImage ( mVkDevice, &image_info, nullptr, &cache.image );

            VkMemoryRequirements img_mem_reqs{};
            vkGetImageMemoryRequirements ( mVkDevice, cache.image, &img_mem_reqs );
            VkMemoryAllocateInfo img_alloc_info{};
            img_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            img_alloc_info.allocationSize = img_mem_reqs.size;
            img_alloc_info.memoryTypeIndex = FindMemoryTypeIndex ( img_mem_reqs.memoryTypeBits,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
            vkAllocateMemory ( mVkDevice, &img_alloc_info, nullptr, &cache.memory );
            vkBindImageMemory ( mVkDevice, cache.image, cache.memory, 0 );

            // Create image view
            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = cache.image;
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = VK_FORMAT_B8G8R8A8_UNORM;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.layerCount = 1;
            vkCreateImageView ( mVkDevice, &view_info, nullptr, &cache.imageView );

            // Create descriptor pool and set
            VkDescriptorPoolSize pool_size{};
            pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            pool_size.descriptorCount = 1;
            VkDescriptorPoolCreateInfo pool_info{};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.maxSets = 1;
            pool_info.poolSizeCount = 1;
            pool_info.pPoolSizes = &pool_size;
            vkCreateDescriptorPool ( mVkDevice, &pool_info, nullptr, &cache.descriptorPool );

            VkDescriptorSetAllocateInfo set_alloc_info{};
            set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            set_alloc_info.descriptorPool = cache.descriptorPool;
            set_alloc_info.descriptorSetCount = 1;
            set_alloc_info.pSetLayouts = &mOverlayDescriptorSetLayout;
            vkAllocateDescriptorSets ( mVkDevice, &set_alloc_info, &cache.descriptorSet );

            // Update descriptor set
            VkDescriptorImageInfo descriptor_image_info{};
            descriptor_image_info.sampler = mOverlaySampler;
            descriptor_image_info.imageView = cache.imageView;
            descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet write_set{};
            write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_set.dstSet = cache.descriptorSet;
            write_set.dstBinding = 0;
            write_set.descriptorCount = 1;
            write_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_set.pImageInfo = &descriptor_image_info;
            vkUpdateDescriptorSets ( mVkDevice, 1, &write_set, 0, nullptr );

            cache.width = width;
            cache.height = height;
        }

        // Upload pixels to staging buffer
        void* mapped = nullptr;
        vkMapMemory ( mVkDevice, cache.stagingMemory, 0, image_size, 0, &mapped );
        memcpy ( mapped, pixels, image_size );
        vkUnmapMemory ( mVkDevice, cache.stagingMemory );

        // Copy staging buffer to image via single-time commands
        VkCommandBuffer cmd = BeginSingleTimeCommands();

        // Transition image to transfer dst
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = cache.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier ( cmd,
                               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &barrier );

        // Copy buffer to image
        VkBufferImageCopy region{};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = { width, height, 1 };
        vkCmdCopyBufferToImage ( cmd, cache.stagingBuffer, cache.image,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

        // Transition image to shader read
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier ( cmd,
                               VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &barrier );

        EndSingleTimeCommands ( cmd );

        // Draw the overlay quad within the current render pass
        VkCommandBuffer render_cmd = it->second.GetCommandBuffer();
        vkCmdBindPipeline ( render_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mOverlayPipeline );
        vkCmdBindDescriptorSets ( render_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  mOverlayPipelineLayout, 0, 1, &cache.descriptorSet, 0, nullptr );
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers ( render_cmd, 0, 1, &mOverlayVertexBuffer, &offset );
        vkCmdDraw ( render_cmd, 4, 1, 0, 0 );
    }
}
