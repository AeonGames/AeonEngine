/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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

#ifdef _WIN32
#include <Windows.h>
#endif
#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>
#include "Vulkan.h"

namespace AeonGames
{
    /*
    * Return 1 (true) if all layer names specified in check_names
    * can be found in given layer properties.
    */
    static bool CheckLayers ( std::vector<const char*>& aNames,
                              uint32_t layer_count,
                              VkLayerProperties *layers )
    {
        for ( auto & i : aNames )
        {
            bool found = false;
            for ( uint32_t j = 0; j < layer_count; j++ )
            {
                if ( !strcmp ( i, layers[j].layerName ) )
                {
                    found = true;
                    break;
                }
            }
            if ( !found )
            {
                fprintf ( stderr, "Cannot find layer: %s\n", i );
                return false;
            }
        }
        return true;
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
        }
        if ( aFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT )
        {
            stream << "DEBUG ";
        }
        stream << "] ";
        stream << aLayerPrefix << ": " << aMsg << std::endl;

        std::cout << stream.str();

#if _WIN32
        if ( aFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT )
        {
            MessageBox ( nullptr, aMsg, aLayerPrefix, MB_ICONERROR | MB_OK );
        }
#endif

        return false;
    }

    Vulkan::Vulkan ( bool aValidate ) :
        mValidate ( aValidate )
    {
        SetupDebug();
        if ( !InitializeInstance() )
        {
            throw InstanceInitializationFailed{};
        }

        if ( !LoadFunctions() )
        {
            throw InstanceInitializationFailed{};
        }

        if ( !InitializeDebug() )
        {
            throw InstanceInitializationFailed{};
        }

        if ( !InitializeDevice() )
        {
            throw DeviceInitializationFailed{};
        }
    }

    Vulkan::~Vulkan()
    {
        FinalizeDevice();
        FinalizeDebug();
        FinalizeInstance();
    }

    bool Vulkan::LoadFunctions()
    {
        assert ( mVkInstance && "mVkInstance is a nullptr." );
        if ( !mFunctionsLoaded )
        {
            if ( ( vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT> ( vkGetInstanceProcAddr ( mVkInstance, "vkCreateDebugReportCallbackEXT" ) ) ) == nullptr )
            {
                return false;
            }
            if ( ( vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT> ( vkGetInstanceProcAddr ( mVkInstance, "vkDestroyDebugReportCallbackEXT" ) ) ) == nullptr )
            {
                return false;
            }
            mFunctionsLoaded = true;
        }
        return true;
    }


    void Vulkan::SetupDebug()
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

    bool Vulkan::InitializeDebug()
    {
        assert ( mVkInstance && "mVkInstance is a nullptr." );
        VkResult result;

        if ( ( result = vkCreateDebugReportCallbackEXT ( mVkInstance, &mDebugReportCallbackCreateInfo, nullptr, &mVkDebugReportCallbackEXT ) ) != VK_SUCCESS )
        {
            return false;
        }
        return true;
    }

    void Vulkan::FinalizeDebug()
    {
        assert ( mVkInstance && "mVkInstance is a nullptr." );
        vkDestroyDebugReportCallbackEXT ( mVkInstance, mVkDebugReportCallbackEXT, nullptr );
        mVkDebugReportCallbackEXT = nullptr;
    }

    bool Vulkan::InitializeInstance()
    {
        VkResult result;
        VkInstanceCreateInfo instance_create_info {};
        VkApplicationInfo application_info {};

        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.apiVersion = VK_API_VERSION_1_0;
        application_info.applicationVersion = VK_MAKE_VERSION ( 0, 1, 0 );
        application_info.pApplicationName = "AeonEngine Vulkan Renderer";

        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.enabledLayerCount = static_cast<uint32_t> ( mInstanceLayerNames.size() );
        instance_create_info.ppEnabledLayerNames = mInstanceLayerNames.data();
        instance_create_info.enabledExtensionCount = static_cast<uint32_t> ( mInstanceExtensionNames.size() );
        instance_create_info.ppEnabledExtensionNames = mInstanceExtensionNames.data();
        instance_create_info.pNext = &mDebugReportCallbackCreateInfo;

        if ( ( result = vkCreateInstance ( &instance_create_info, nullptr, &mVkInstance ) ) != VK_SUCCESS )
        {
            return false;
        }
        return true;
    }

    void Vulkan::FinalizeInstance()
    {
        vkDestroyInstance ( mVkInstance, nullptr );
        mVkInstance = nullptr;
    }

    bool Vulkan::InitializeDevice()
    {
        assert ( mVkInstance && "mVkInstance is a nullptr." );
        if ( !mVkInstance )
        {
            return false;
        }

        {
            uint32_t physical_device_count;
            vkEnumeratePhysicalDevices ( mVkInstance, &physical_device_count, nullptr );

            if ( physical_device_count == 0 )
            {
                return false;
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
                return false;
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
                    device_queue_create_info.queueFamilyIndex =
                        static_cast<uint32_t> ( family_properties_list.begin() - family_property );
                    break;
                }
            }
            if ( !graphics_queue_family_found )
            {
                return false;
            }
        }

        {
            uint32_t instance_layer_count;
            vkEnumerateInstanceLayerProperties ( &instance_layer_count, nullptr );
            std::vector<VkLayerProperties> instance_layer_list ( instance_layer_count );
            vkEnumerateInstanceLayerProperties ( &instance_layer_count, instance_layer_list.data() );
            std::cout << "Vulkan Instance Layers" << std::endl;
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
            std::cout << "Vulkan Device Layers" << std::endl;
            for ( auto& i : device_layer_list )
            {
                std::cout << " " << i.layerName << "\t|\t" << i.description << std::endl;
            }
        }

        /// @todo Remove hardcoded Queue Info
        float queue_priorities[] {1.0f};
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.queueCount = 1;
        // Note that device_queue_create_info.queueFamilyIndex is set above.
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
            return false;
        }
        return true;
    }

    void Vulkan::FinalizeDevice()
    {
        vkDestroyDevice ( mVkDevice, nullptr );
        mVkDevice = nullptr;
    }
}
