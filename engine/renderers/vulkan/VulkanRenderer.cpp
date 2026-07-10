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
#include "aeongames/Scene.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/ResourceId.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/MemoryPool.hpp"
#include "SPIR-V/CompilerLinker.hpp"

namespace AeonGames
{
    VulkanRenderer::VulkanRenderer ( void* aWindow ) :
        mMaterialStorageBuffer { *this }
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
            InitializeBindless();
            InitializeCommandPools();
            AttachWindow ( aWindow );
            InitializeOverlay();
            // Load the fallback textures used for material sampler slots a
            // material omits (see kMaterialSamplerSlots). Requires the device,
            // command pools and queue to already be initialized for the upload.
            {
                for ( size_t i = 0; i < kMaterialSamplerSlots.size(); ++i )
                {
                    ResourceId fallback{ "Texture"_crc32, kMaterialSamplerSlots[i].fallback_path };
                    mMaterialSamplerFallbacks[i] = fallback.Get<Texture>();
                    LoadTexture ( *mMaterialSamplerFallbacks[i] );
                }
                mDefaultTexture = mMaterialSamplerFallbacks[0];
            }
        }
        catch ( ... )
        {
            this->~VulkanRenderer();
            throw;
        }
    }

    VulkanRenderer::~VulkanRenderer()
    {
        // The destructor may run on a partially-constructed renderer when the
        // constructor's exception handler unwinds (e.g. no compatible Vulkan
        // driver). Guard against null handles so cleanup never calls Vulkan
        // entry points with invalid arguments, which would abort the process.
        if ( mVkQueue != VK_NULL_HANDLE )
        {
            vkQueueWaitIdle ( mVkQueue );
        }
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
        FinalizeBindless();
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

    std::string_view VulkanRenderer::GetName() const
    {
        return "Vulkan";
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

    const VkPhysicalDeviceDescriptorIndexingProperties & VulkanRenderer::GetDescriptorIndexingProperties() const
    {
        return mVkDescriptorIndexingProperties;
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
            // On failure the loader may still have written a non-null (but
            // unusable) handle into mVkInstance. Per the Vulkan spec, a failed
            // vkCreateInstance must not be paired with vkDestroyInstance, so
            // reset the handle to VK_NULL_HANDLE here; otherwise the destructor
            // would call vkDestroyInstance on a garbage pointer and crash.
            mVkInstance = VK_NULL_HANDLE;
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

            // Prefer a discrete GPU (e.g. the NVIDIA half of a hybrid-graphics
            // laptop) over an integrated one; fall back to the first enumerated
            // device when none reports itself as discrete.
            mVkPhysicalDevice = physical_device_list[0];
            for ( const VkPhysicalDevice& candidate : physical_device_list )
            {
                VkPhysicalDeviceProperties candidate_properties;
                vkGetPhysicalDeviceProperties ( candidate, &candidate_properties );
                if ( candidate_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
                {
                    mVkPhysicalDevice = candidate;
                    break;
                }
            }
            vkGetPhysicalDeviceProperties ( mVkPhysicalDevice, &mVkPhysicalDeviceProperties );
            std::cout << LogLevel::Info << "Vulkan physical device: " << mVkPhysicalDeviceProperties.deviceName << std::endl;
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
        // Query and require the descriptor-indexing (bindless), buffer device
        // address, indirect-draw-count and draw-parameter features the bindless
        // / GPU-driven renderer path depends on. All are core in the requested
        // Vulkan 1.2+, so no extension is needed; we still verify the physical
        // device actually supports them and fail fast with a precise message.
        VkPhysicalDeviceVulkan11Features supported_vulkan11_features{};
        supported_vulkan11_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        VkPhysicalDeviceVulkan12Features supported_vulkan12_features{};
        supported_vulkan12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        supported_vulkan11_features.pNext = &supported_vulkan12_features;
        VkPhysicalDeviceFeatures2 supported_features2{};
        supported_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        supported_features2.pNext = &supported_vulkan11_features;
        vkGetPhysicalDeviceFeatures2 ( mVkPhysicalDevice, &supported_features2 );

        struct RequiredFeature
        {
            VkBool32 supported;
            const char* name;
        };
        const RequiredFeature required_features[]
        {
            { supported_features2.features.multiDrawIndirect, "multiDrawIndirect" },
            { supported_vulkan11_features.shaderDrawParameters, "shaderDrawParameters" },
            { supported_vulkan12_features.descriptorIndexing, "descriptorIndexing" },
            { supported_vulkan12_features.shaderSampledImageArrayNonUniformIndexing, "shaderSampledImageArrayNonUniformIndexing" },
            { supported_vulkan12_features.runtimeDescriptorArray, "runtimeDescriptorArray" },
            { supported_vulkan12_features.descriptorBindingPartiallyBound, "descriptorBindingPartiallyBound" },
            { supported_vulkan12_features.descriptorBindingVariableDescriptorCount, "descriptorBindingVariableDescriptorCount" },
            { supported_vulkan12_features.descriptorBindingSampledImageUpdateAfterBind, "descriptorBindingSampledImageUpdateAfterBind" },
            { supported_vulkan12_features.bufferDeviceAddress, "bufferDeviceAddress" },
            { supported_vulkan12_features.drawIndirectCount, "drawIndirectCount" },
        };
        {
            std::ostringstream missing;
            for ( const RequiredFeature& feature : required_features )
            {
                if ( feature.supported != VK_TRUE )
                {
                    missing << ( missing.tellp() > 0 ? ", " : "" ) << feature.name;
                }
            }
            if ( missing.tellp() > 0 )
            {
                std::ostringstream stream;
                stream << "VulkanRenderer physical device is missing required features: " << missing.str();
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }
        }

        // Descriptor-indexing limits (max bindless array sizes) stored for the
        // global bindless resource arrays created in a later phase.
        mVkDescriptorIndexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
        VkPhysicalDeviceProperties2 physical_device_properties2{};
        physical_device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        physical_device_properties2.pNext = &mVkDescriptorIndexingProperties;
        vkGetPhysicalDeviceProperties2 ( mVkPhysicalDevice, &physical_device_properties2 );
        std::cout << LogLevel::Info << "VulkanRenderer bindless/GPU-driven features supported: descriptorIndexing, "
                                       "bufferDeviceAddress, drawIndirectCount, multiDrawIndirect, shaderDrawParameters." << std::endl;
        std::cout << LogLevel::Debug << "maxDescriptorSetUpdateAfterBindSampledImages: "
                  << mVkDescriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSampledImages << std::endl;
        std::cout << LogLevel::Debug << "maxPerStageDescriptorUpdateAfterBindSampledImages: "
                  << mVkDescriptorIndexingProperties.maxPerStageDescriptorUpdateAfterBindSampledImages << std::endl;

        VkDeviceCreateInfo device_create_info {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        // Multiview renders all six point-shadow cube faces in a single pass
        // (one view per cube face); enable it so the point shadow depth pipeline
        // can replicate geometry across the six views in hardware.
        VkPhysicalDeviceVulkan11Features vulkan11_features {};
        vulkan11_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        vulkan11_features.multiview = VK_TRUE;
        // shaderDrawParameters exposes gl_DrawID / gl_BaseInstance for the
        // GPU-driven indirect path.
        vulkan11_features.shaderDrawParameters = VK_TRUE;
        // Descriptor indexing (bindless), buffer device address and indirect
        // draw count drive the bindless / GPU-driven renderer path; all verified
        // supported above before this point.
        VkPhysicalDeviceVulkan12Features vulkan12_features {};
        vulkan12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vulkan12_features.descriptorIndexing = VK_TRUE;
        vulkan12_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        vulkan12_features.runtimeDescriptorArray = VK_TRUE;
        vulkan12_features.descriptorBindingPartiallyBound = VK_TRUE;
        vulkan12_features.descriptorBindingVariableDescriptorCount = VK_TRUE;
        vulkan12_features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        vulkan12_features.bufferDeviceAddress = VK_TRUE;
        vulkan12_features.drawIndirectCount = VK_TRUE;
#ifdef VK_USE_PLATFORM_METAL_EXT
        vulkan12_features.pNext = use_primitive_topology_list_restart ? &physical_device_primitive_topology_list_restart_features : nullptr;
#endif
        vulkan11_features.pNext = &vulkan12_features;
        device_create_info.pNext = &vulkan11_features;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &device_queue_create_info;
        device_create_info.enabledLayerCount = static_cast<uint32_t> ( mDeviceLayerNames.size() );
        device_create_info.ppEnabledLayerNames = mDeviceLayerNames.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t> ( mDeviceExtensionNames.size() );
        device_create_info.ppEnabledExtensionNames = mDeviceExtensionNames.data();
        // The cluster_mark fragment shader writes the per-cluster active-flag
        // storage buffer during the depth pre-pass; that requires the
        // fragmentStoresAndAtomics device feature. imageCubeArray lets the point
        // shadow depth array be sampled as a cube map array. independentBlend
        // lets the main pass's colour attachment keep alpha blending while its
        // deferred-specular G-buffer attachments overwrite (no blend).
        VkPhysicalDeviceFeatures enabled_features {};
        enabled_features.fragmentStoresAndAtomics = VK_TRUE;
        enabled_features.imageCubeArray = VK_TRUE;
        enabled_features.independentBlend = VK_TRUE;
        // multiDrawIndirect backs the GPU-driven multi-draw path.
        enabled_features.multiDrawIndirect = VK_TRUE;
        device_create_info.pEnabledFeatures = &enabled_features;

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

    const VulkanPipeline* VulkanRenderer::GetVulkanPipeline ( const Pipeline& aPipeline, VkRenderPass aRenderPass )
    {
        auto it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
        if ( it == mPipelineStore.end() )
        {
            if ( aRenderPass != VK_NULL_HANDLE )
            {
                // The pipeline must be created against this (e.g. multiview)
                // render pass; cached on first use against it.
                it = mPipelineStore.emplace ( aPipeline.GetConsecutiveId(), VulkanPipeline{*this, aPipeline, aRenderPass} ).first;
            }
            else
            {
                LoadPipeline ( aPipeline );
                it = mPipelineStore.find ( aPipeline.GetConsecutiveId() );
            }
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

    const VkDescriptorImageInfo* VulkanRenderer::GetDefaultTextureDescriptorImageInfo() const
    {
        return GetTextureDescriptorImageInfo ( *mDefaultTexture );
    }

    const VkDescriptorImageInfo* VulkanRenderer::GetMaterialSamplerFallbackDescriptorImageInfo ( size_t aSlot ) const
    {
        return GetTextureDescriptorImageInfo ( *mMaterialSamplerFallbacks[aSlot] );
    }

    void VulkanRenderer::InitializeBindless()
    {
        // Cap the global combined-image-sampler array at a generous size clamped
        // to the device's update-after-bind limits (a combined image sampler
        // consumes one sampled-image and one sampler descriptor).
        constexpr uint32_t kBindlessTextureHardCap = 16384;
        uint32_t capacity = kBindlessTextureHardCap;
        if ( mVkDescriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSampledImages < capacity )
        {
            capacity = mVkDescriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSampledImages;
        }
        if ( mVkDescriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSamplers < capacity )
        {
            capacity = mVkDescriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSamplers;
        }
        if ( capacity == 0 )
        {
            throw std::runtime_error ( "Device reports no update-after-bind sampled images; bindless textures unsupported." );
        }
        mBindlessTextureCapacity = capacity;
        mBindlessMaterialCapacity = 4096;

        // Global material storage buffer: one GpuMaterial record per material,
        // fetched per draw by a material index. Host-visible + coherent so the
        // records can be written directly when materials load.
        mMaterialStorageBuffer.Initialize (
            static_cast<VkDeviceSize> ( mBindlessMaterialCapacity ) * sizeof ( GpuMaterial ),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            nullptr );

        // Descriptor set 0: binding 0 = the combined-image-sampler array
        // (bindless textures; update-after-bind + partially bound so slots are
        // written lazily as textures load), binding 1 = the material storage
        // buffer (written once here, so it is not update-after-bind -- storage
        // buffer update-after-bind is not among the enabled features).
        std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = mBindlessTextureCapacity;
        bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
        std::array<VkDescriptorBindingFlags, 2> binding_flags
        {
            {
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                0
            }
        };
        VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{};
        binding_flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        binding_flags_info.bindingCount = static_cast<uint32_t> ( binding_flags.size() );
        binding_flags_info.pBindingFlags = binding_flags.data();
        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.pNext = &binding_flags_info;
        layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        layout_info.bindingCount = static_cast<uint32_t> ( bindings.size() );
        layout_info.pBindings = bindings.data();
        if ( VkResult result = vkCreateDescriptorSetLayout ( mVkDevice, &layout_info, nullptr, &mVkBindlessDescriptorSetLayout ) )
        {
            std::ostringstream stream;
            stream << "Bindless descriptor set layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        std::array<VkDescriptorPoolSize, 2> pool_sizes
        {
            {
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, mBindlessTextureCapacity },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
            }
        };
        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = static_cast<uint32_t> ( pool_sizes.size() );
        pool_info.pPoolSizes = pool_sizes.data();
        if ( VkResult result = vkCreateDescriptorPool ( mVkDevice, &pool_info, nullptr, &mVkBindlessDescriptorPool ) )
        {
            std::ostringstream stream;
            stream << "Bindless descriptor pool creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = mVkBindlessDescriptorPool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &mVkBindlessDescriptorSetLayout;
        if ( VkResult result = vkAllocateDescriptorSets ( mVkDevice, &alloc_info, &mVkBindlessDescriptorSet ) )
        {
            std::ostringstream stream;
            stream << "Bindless descriptor set allocation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        // Point binding 1 at the material storage buffer (written once; the
        // records inside it are updated as materials load).
        VkDescriptorBufferInfo material_buffer_info{ mMaterialStorageBuffer.GetBuffer(), 0, VK_WHOLE_SIZE };
        VkWriteDescriptorSet material_write{};
        material_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        material_write.dstSet = mVkBindlessDescriptorSet;
        material_write.dstBinding = 1;
        material_write.dstArrayElement = 0;
        material_write.descriptorCount = 1;
        material_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        material_write.pBufferInfo = &material_buffer_info;
        vkUpdateDescriptorSets ( mVkDevice, 1, &material_write, 0, nullptr );

        std::cout << LogLevel::Debug << "Bindless texture array capacity: " << mBindlessTextureCapacity
                  << ", material records: " << mBindlessMaterialCapacity << std::endl;
    }

    void VulkanRenderer::FinalizeBindless()
    {
        if ( mVkBindlessDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVkDevice, mVkBindlessDescriptorPool, nullptr );
            mVkBindlessDescriptorPool = VK_NULL_HANDLE;
            mVkBindlessDescriptorSet = VK_NULL_HANDLE;
        }
        if ( mVkBindlessDescriptorSetLayout != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorSetLayout ( mVkDevice, mVkBindlessDescriptorSetLayout, nullptr );
            mVkBindlessDescriptorSetLayout = VK_NULL_HANDLE;
        }
        mMaterialStorageBuffer.Finalize();
    }

    uint32_t VulkanRenderer::RegisterBindlessTexture ( const VkDescriptorImageInfo& aVkDescriptorImageInfo ) const
    {
        uint32_t slot;
        if ( !mBindlessTextureFreeSlots.empty() )
        {
            slot = mBindlessTextureFreeSlots.back();
            mBindlessTextureFreeSlots.pop_back();
        }
        else if ( mBindlessTextureHighWater < mBindlessTextureCapacity )
        {
            slot = mBindlessTextureHighWater++;
        }
        else
        {
            throw std::runtime_error ( "Global bindless texture array is full." );
        }
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = mVkBindlessDescriptorSet;
        write.dstBinding = 0;
        write.dstArrayElement = slot;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &aVkDescriptorImageInfo;
        vkUpdateDescriptorSets ( mVkDevice, 1, &write, 0, nullptr );
        return slot;
    }

    void VulkanRenderer::UnregisterBindlessTexture ( uint32_t aSlot ) const
    {
        if ( aSlot != UINT32_MAX )
        {
            mBindlessTextureFreeSlots.push_back ( aSlot );
        }
    }

    uint32_t VulkanRenderer::GetTextureBindlessSlot ( const Texture& aTexture ) const
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
        return it->second.GetBindlessSlot();
    }

    VkDescriptorSet VulkanRenderer::GetBindlessDescriptorSet() const
    {
        return mVkBindlessDescriptorSet;
    }

    VkDescriptorSetLayout VulkanRenderer::GetBindlessDescriptorSetLayout() const
    {
        return mVkBindlessDescriptorSetLayout;
    }

    uint32_t VulkanRenderer::GetMaterialSamplerFallbackBindlessSlot ( size_t aSlot ) const
    {
        return GetTextureBindlessSlot ( *mMaterialSamplerFallbacks[aSlot] );
    }

    uint32_t VulkanRenderer::RegisterBindlessMaterial ( const GpuMaterial& aGpuMaterial ) const
    {
        uint32_t index;
        if ( !mBindlessMaterialFreeSlots.empty() )
        {
            index = mBindlessMaterialFreeSlots.back();
            mBindlessMaterialFreeSlots.pop_back();
        }
        else if ( mBindlessMaterialHighWater < mBindlessMaterialCapacity )
        {
            index = mBindlessMaterialHighWater++;
        }
        else
        {
            throw std::runtime_error ( "Global material storage buffer is full." );
        }
        mMaterialStorageBuffer.WriteMemory ( index * sizeof ( GpuMaterial ), sizeof ( GpuMaterial ), &aGpuMaterial );
        return index;
    }

    void VulkanRenderer::UnregisterBindlessMaterial ( uint32_t aIndex ) const
    {
        if ( aIndex != UINT32_MAX )
        {
            mBindlessMaterialFreeSlots.push_back ( aIndex );
        }
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

    void VulkanRenderer::SetLights ( void* aWindowId, std::span<const GpuLight> aLights )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetLights ( FilterLightsByType ( aLights ) );
    }

    void VulkanRenderer::SetGlobals ( void* aWindowId, const GpuGlobals& aGlobals )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetGlobals ( aGlobals );
    }

    void VulkanRenderer::SetEnvironmentMap ( void* aWindowId, const Texture* aEnvironmentMap )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetEnvironmentMap ( aEnvironmentMap );
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

    void VulkanRenderer::BeginRender ( void* aWindowId, const Pipeline* aComputePipeline )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginRender ( aComputePipeline );
    }
    void VulkanRenderer::BeginFrame ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginFrame();
    }
    void VulkanRenderer::BeginRenderPass ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginRenderPass();
    }
    void VulkanRenderer::EndDepthPrePass ( void* aWindowId, const Pipeline* aComputePipeline )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndDepthPrePass ( aComputePipeline );
    }
    void VulkanRenderer::BeginShadowPass ( void* aWindowId, const Matrix4x4& aLightViewProjection )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginShadowPass ( aLightViewProjection );
    }
    void VulkanRenderer::EndShadowPass ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndShadowPass();
    }
    void VulkanRenderer::SetSpotShadowParams ( void* aWindowId, const GpuSpotShadowParams& aSpotShadowParams )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetSpotShadowParams ( aSpotShadowParams );
    }
    void VulkanRenderer::BeginSpotShadowPass ( void* aWindowId, uint32_t aSlot, const Matrix4x4& aLightViewProjection )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginSpotShadowPass ( aSlot, aLightViewProjection );
    }
    void VulkanRenderer::EndSpotShadowPass ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndSpotShadowPass();
    }
    void VulkanRenderer::SetPointShadowParams ( void* aWindowId, const GpuPointShadowParams& aPointShadowParams )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.SetPointShadowParams ( aPointShadowParams );
    }
    void VulkanRenderer::BeginPointShadowPass ( void* aWindowId, uint32_t aCaster )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.BeginPointShadowPass ( aCaster );
    }
    void VulkanRenderer::EndPointShadowPass ( void* aWindowId )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.EndPointShadowPass();
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
    void VulkanRenderer::Finish ( void* aWindowId )
    {
        // Block until every submission is complete so the caller can safely map
        // GPU-written buffers or capture the surface. With multiple frames in
        // flight BeginFrame only waits on its own slot's fence, so this explicit
        // drain is what guarantees a correct CPU read-back.
        if ( mWindowStore.find ( aWindowId ) == mWindowStore.end() )
        {
            return;
        }
        if ( VkResult result = vkDeviceWaitIdle ( mVkDevice ) )
        {
            std::cout << LogLevel::Error << "vkDeviceWaitIdle failed in Finish: " << GetVulkanResultString ( result ) << std::endl;
        }
    }
    void VulkanRenderer::SubmitRenderQueue ( void* aWindowId, const Scene& aScene, RenderPass aRenderPass )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        VulkanWindow& aWindow = it->second;
        aScene.ForEachRenderBatch ( [this, &aWindow, aRenderPass] ( std::span<const RenderItem> aBatch )
        {
            const RenderItem& head = aBatch.front();
            if ( aBatch.size() == 1 )
            {
                aWindow.Render (
                    head.mTransform,
                    *head.mMesh,
                    *head.mPipeline,
                    head.mMaterial,
                    Topology::TRIANGLE_LIST,
                    0,
                    0xffffffff,
                    1,
                    0,
                    head.mSkinnedVertices,
                    aRenderPass );
                return;
            }
            // Gather the batch's transforms contiguously for one instanced draw.
            // mInstanceTransforms is reused so this only allocates when a batch
            // grows beyond any previously seen size.
            mInstanceTransforms.clear();
            for ( const RenderItem& item : aBatch )
            {
                mInstanceTransforms.push_back ( item.mTransform );
            }
            aWindow.RenderInstanced (
                mInstanceTransforms,
                *head.mMesh,
                *head.mPipeline,
                head.mMaterial,
                Topology::TRIANGLE_LIST,
                0,
                0xffffffff,
                aRenderPass );
        } );
    }
    bool VulkanRenderer::IsValidWindow ( void* aWindowId ) const
    {
        return mWindowStore.find ( aWindowId ) != mWindowStore.end();
    }
    void VulkanRenderer::Render ( void* aWindowId,
                                  const Matrix4x4& aModelMatrix,
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
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Render ( aModelMatrix, aMesh, aPipeline, aMaterial, aTopology, aVertexStart, aVertexCount, aInstanceCount, aFirstInstance, aSkinnedVertices, aRenderPass );
    }

    void VulkanRenderer::RenderInstanced ( void* aWindowId,
                                           std::span<const Matrix4x4> aModelMatrices,
                                           const Mesh& aMesh,
                                           const Pipeline& aPipeline,
                                           const Material* aMaterial,
                                           Topology aTopology,
                                           uint32_t aVertexStart,
                                           uint32_t aVertexCount,
                                           RenderPass aRenderPass )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.RenderInstanced ( aModelMatrices, aMesh, aPipeline, aMaterial, aTopology, aVertexStart, aVertexCount, aRenderPass );
    }

    void VulkanRenderer::Dispatch ( void* aWindowId,
                                    const Pipeline& aPipeline,
                                    uint32_t aGroupCountX,
                                    uint32_t aGroupCountY,
                                    uint32_t aGroupCountZ,
                                    std::span<const StorageBufferBinding> aStorageBuffers,
                                    uint32_t aComputeStageIndex ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Dispatch ( aPipeline, aGroupCountX, aGroupCountY, aGroupCountZ, aStorageBuffers, aComputeStageIndex );
    }

    void VulkanRenderer::Skin ( void* aWindowId,
                                const Pipeline& aSkinningPipeline,
                                const Mesh& aMesh,
                                const BufferAccessor& aSkinningMatrices,
                                const BufferAccessor& aSkinnedVertices ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Skin ( aSkinningPipeline, aMesh, aSkinningMatrices, aSkinnedVertices );
    }

    void VulkanRenderer::Barrier ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return;
        }
        it->second.Barrier();
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

    const Matrix4x4& VulkanRenderer::GetProjectionMatrix ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            std::cout << LogLevel::Error << "Unknown Window Id." << std::endl;
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.GetProjectionMatrix();
    }

    const BufferAccessor* VulkanRenderer::GetFrameLightGrid ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return nullptr;
        }
        return &it->second.GetFrameLightGrid();
    }

    const BufferAccessor* VulkanRenderer::GetFrameClusterActive ( void* aWindowId ) const
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            return nullptr;
        }
        return &it->second.GetFrameClusterActive();
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

    BufferAccessor VulkanRenderer::AllocateSingleFrameStorageMemory ( void* aWindowId, size_t aSize )
    {
        auto it = mWindowStore.find ( aWindowId );
        if ( it == mWindowStore.end() )
        {
            std::cout << LogLevel::Error << "Unknown Window Id." << std::endl;
            throw std::runtime_error ( "Unknown Window Id." );
        }
        return it->second.AllocateSingleFrameStorageMemory ( aSize );
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

        // The overlay is composited in the window's main render pass, whose
        // subpass has one HDR colour attachment plus the deferred-specular
        // G-buffer attachments (3 colour attachments total). Vulkan requires a
        // graphics pipeline's colour blend state to declare exactly as many
        // attachments as the subpass it targets
        // (VUID-VkGraphicsPipelineCreateInfo-renderPass-07609). The overlay is a
        // 2D composite over the HDR colour only: attachment 0 alpha-blends the
        // UI, while the G-buffer attachments are masked off (colorWriteMask 0)
        // so the overlay never overwrites the normals/specular a later pass reads.
        constexpr uint32_t main_pass_color_attachment_count = 3;
        std::array<VkPipelineColorBlendAttachmentState, main_pass_color_attachment_count> blend_attachments{};
        blend_attachments[0].blendEnable = VK_TRUE;
        blend_attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blend_attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend_attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
        blend_attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend_attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend_attachments[0].alphaBlendOp = VK_BLEND_OP_ADD;
        blend_attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        for ( uint32_t a = 1; a < main_pass_color_attachment_count; ++a )
        {
            blend_attachments[a].blendEnable = VK_FALSE;
            blend_attachments[a].colorWriteMask = 0;
        }

        VkPipelineColorBlendStateCreateInfo color_blend{};
        color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend.attachmentCount = static_cast<uint32_t> ( blend_attachments.size() );
        color_blend.pAttachments = blend_attachments.data();

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
