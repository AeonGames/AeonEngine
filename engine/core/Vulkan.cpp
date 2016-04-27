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


/*  Reference:
    Vulkan SDK Demo code.
    https://vulkan.lunarg.com/app/docs/v1.0.8.0/layers
    http://gpuopen.com/using-the-vulkan-validation-layers/?utm_source=silverpop&utm_medium=email&utm_campaign=25324445&utm_term=link-article2&utm_content=p-global-developer-hcnewsflash-april-2016%20%281%29:&spMailingID=25324445&spUserID=NzI5Mzc5ODY4NjQS1&spJobID=783815030&spReportId=NzgzODE1MDMwS0
*/

#include <cstring>
#include <cassert>
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
    dbgFunc ( VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
              uint64_t srcObject, size_t location, int32_t msgCode,
              const char *pLayerPrefix, const char *pMsg, void *pUserData )
    {
        char *message = new char[ ( strlen ( pMsg ) + 100 )];

        assert ( message );

        if ( msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT )
        {
            sprintf ( message, "ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode,
                      pMsg );
        }
        else if ( msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT )
        {
            // We know that we're submitting queues without fences, ignore this
            // warning
            if ( strstr ( pMsg,
                          "vkQueueSubmit parameter, VkFence fence, is null pointer" ) )
            {
                delete[] message;
                return false;
            }
            sprintf ( message, "WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode,
                      pMsg );
        }
        else
        {
            delete[] message;
            return false;
        }

#ifdef _WIN32
        MessageBox ( NULL, message, "Alert", MB_OK );
#else
        printf ( "%s\n", message );
        fflush ( stdout );
#endif
        delete[] message;

        /*
        * false indicates that layer should not bail-out of an
        * API call that had validation failures. This may mean that the
        * app dies inside the driver due to invalid parameter(s).
        * That's what would happen without validation layers, so we'll
        * keep that behavior here.
        */
        return false;
    }


    VKAPI_ATTR VkBool32 VKAPI_CALL
    BreakCallback ( VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
                    uint64_t srcObject, size_t location, int32_t msgCode,
                    const char *pLayerPrefix, const char *pMsg,
                    void *pUserData )
    {
#ifndef WIN32
        raise ( SIGTRAP );
#else
        DebugBreak();
#endif

        return false;
    }

    Vulkan::Vulkan ( bool aValidate ) :
        mValidate ( aValidate ) /*,
        mEnabledExtensionCount ( 0 ),
        mEnabledLayerCount ( 0 )*/
    {
        VkResult err;
        uint32_t instance_extension_count = 0;
        uint32_t instance_layer_count = 0;
        //uint32_t device_validation_layer_count = 0;

        std::vector<const char *> instance_validation_layers_alt1
        {
            "VK_LAYER_LUNARG_standard_validation"
        };

        std::vector<const char *> instance_validation_layers_alt2
        {
            "VK_LAYER_GOOGLE_threading",     "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_device_limits", "VK_LAYER_LUNARG_object_tracker",
            "VK_LAYER_LUNARG_image",         "VK_LAYER_LUNARG_core_validation",
            "VK_LAYER_LUNARG_swapchain",     "VK_LAYER_GOOGLE_unique_objects"
        };

        /* Look for validation layers */
        bool validation_found = false;
        if ( mValidate )
        {
            err = vkEnumerateInstanceLayerProperties ( &instance_layer_count, nullptr );
            assert ( !err );

            //instance_validation_layers = &instance_validation_layers_alt1[0];
            if ( instance_layer_count > 0 )
            {
                VkLayerProperties *instance_layers =
                    new VkLayerProperties[instance_layer_count];
                err = vkEnumerateInstanceLayerProperties ( &instance_layer_count,
                        instance_layers );
                assert ( !err );

                validation_found = CheckLayers ( instance_validation_layers_alt1, instance_layer_count,
                                                 instance_layers );

                if ( validation_found )
                {
                    //mEnabledLayerCount = static_cast<uint32_t>(instance_validation_layers_alt1.size());
                    mDeviceValidationLayers = instance_validation_layers_alt1;
                    //device_validation_layer_count = static_cast<uint32_t>(mDeviceValidationLayers.size());
                }
                else
                {
                    // use alternative set of validation layers
                    //mEnabledLayerCount = static_cast<uint32_t>(instance_validation_layers_alt2.size());
                    validation_found = CheckLayers (
                                           instance_validation_layers_alt2,
                                           instance_layer_count,
                                           instance_layers );
                    //device_validation_layer_count =
                    //    static_cast<uint32_t>(instance_validation_layers_alt2.size());
                    mDeviceValidationLayers = instance_validation_layers_alt2;
                }
                delete [] instance_layers;
            }

            if ( !validation_found )
            {
                printf ( "%s: vkEnumerateInstanceLayerProperties failed to find"
                         "required validation layer.\n\n"
                         "Please look at the Getting Started guide for additional "
                         "information.\n",
                         "vkCreateInstance Failure" );
            }
        }

        /* Look for instance extensions */
        VkBool32 surfaceExtFound = 0;
        VkBool32 platformSurfaceExtFound = 0;

        err = vkEnumerateInstanceExtensionProperties (
                  nullptr, &instance_extension_count, nullptr );
        assert ( !err );

        if ( instance_extension_count > 0 )
        {
            VkExtensionProperties *instance_extensions =
                new VkExtensionProperties[instance_extension_count];
            err = vkEnumerateInstanceExtensionProperties (
                      nullptr, &instance_extension_count, instance_extensions );
            assert ( !err );
            for ( uint32_t i = 0; i < instance_extension_count; i++ )
            {
                if ( !strcmp ( VK_KHR_SURFACE_EXTENSION_NAME,
                               instance_extensions[i].extensionName ) )
                {
                    surfaceExtFound = 1;
                    mExtensionNames.emplace_back ( VK_KHR_SURFACE_EXTENSION_NAME );
                }
#ifdef _WIN32
                if ( !strcmp ( VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
                               instance_extensions[i].extensionName ) )
                {
                    platformSurfaceExtFound = 1;
                    mExtensionNames.emplace_back (
                        VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
                }
#else  // _WIN32
                if ( !strcmp ( VK_KHR_XCB_SURFACE_EXTENSION_NAME,
                               instance_extensions[i].extensionName ) )
                {
                    platformSurfaceExtFound = 1;
                    mExtensionNames.emplace_back ( VK_KHR_XCB_SURFACE_EXTENSION_NAME );
                }
#endif // _WIN32
                if ( !strcmp ( VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                               instance_extensions[i].extensionName ) )
                {
                    if ( mValidate )
                    {
                        mExtensionNames.emplace_back ( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
                    }
                }
            }
            delete[] instance_extensions;
        }

        if ( !surfaceExtFound )
        {
            printf ( "%s: vkEnumerateInstanceExtensionProperties failed to find "
                     "the " VK_KHR_SURFACE_EXTENSION_NAME
                     " extension.\n\nDo you have a compatible "
                     "Vulkan installable client driver (ICD) installed?\nPlease "
                     "look at the Getting Started guide for additional "
                     "information.\n",
                     "vkCreateInstance Failure" );
        }
        if ( !platformSurfaceExtFound )
        {
#ifdef _WIN32
            printf ( "%s: vkEnumerateInstanceExtensionProperties failed to find "
                     "the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME
                     " extension.\n\nDo you have a compatible "
                     "Vulkan installable client driver (ICD) installed?\nPlease "
                     "look at the Getting Started guide for additional "
                     "information.\n",
                     "vkCreateInstance Failure" );
#else  // _WIN32
            printf ( "vkEnumerateInstanceExtensionProperties failed to find "
                     "the " VK_KHR_XCB_SURFACE_EXTENSION_NAME
                     " extension.\n\nDo you have a compatible "
                     "Vulkan installable client driver (ICD) installed?\nPlease "
                     "look at the Getting Started guide for additional "
                     "information.\n",
                     "vkCreateInstance Failure" );
#endif // _WIN32
        }

        VkApplicationInfo app;

        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pNext = nullptr;
        app.pApplicationName = "AeonEngine";
        app.applicationVersion = 0;
        app.pEngineName = "AeonEngine";
        app.engineVersion = 0;
        app.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo inst_info;
        inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        inst_info.pNext = nullptr;
        inst_info.pApplicationInfo = &app;
        inst_info.enabledLayerCount = static_cast<uint32_t> ( mDeviceValidationLayers.size() );
        inst_info.ppEnabledLayerNames = ( const char *const * ) &mDeviceValidationLayers[0];
        inst_info.enabledExtensionCount = static_cast<uint32_t> ( mExtensionNames.size() );
        inst_info.ppEnabledExtensionNames = ( const char *const * ) &mExtensionNames[0];

        /*
        * This is info for a temp callback to use during CreateInstance.
        * After the instance is created, we use the instance-based
        * function to register the final callback.
        */
        VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
        if ( mValidate )
        {
            dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            dbgCreateInfo.pNext = nullptr;
            dbgCreateInfo.pfnCallback = mUseBreak ? BreakCallback : dbgFunc;
            dbgCreateInfo.pUserData = nullptr;
            dbgCreateInfo.flags =
                VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            inst_info.pNext = &dbgCreateInfo;
        }

        uint32_t gpu_count;

        err = vkCreateInstance ( &inst_info, nullptr, &mVkInstance );
        if ( err == VK_ERROR_INCOMPATIBLE_DRIVER )
        {
            printf ( "%s: Cannot find a compatible Vulkan installable client driver "
                     "(ICD).\n\nPlease look at the Getting Started guide for "
                     "additional information.\n",
                     "vkCreateInstance Failure" );
        }
        else if ( err == VK_ERROR_EXTENSION_NOT_PRESENT )
        {
            printf ( "%s: Cannot find a specified extension library"
                     ".\nMake sure your layers path is set appropriately.\n",
                     "vkCreateInstance Failure" );
        }
        else if ( err )
        {
            printf ( "%s: vkCreateInstance failed.\n\nDo you have a compatible Vulkan "
                     "installable client driver (ICD) installed?\nPlease look at "
                     "the Getting Started guide for additional information.\n",
                     "vkCreateInstance Failure" );
        }

        /* Make initial call to query gpu_count, then second call for gpu info*/
        err = vkEnumeratePhysicalDevices ( mVkInstance, &gpu_count, NULL );
        assert ( !err && gpu_count > 0 );

        if ( gpu_count > 0 )
        {
            VkPhysicalDevice *physical_devices = new VkPhysicalDevice[gpu_count];
            err = vkEnumeratePhysicalDevices ( mVkInstance, &gpu_count, physical_devices );
            assert ( !err );
            /* For cube demo we just grab the first physical device */
            mVkPhysicalDevice = physical_devices[0];
            delete[] physical_devices;
        }
        else
        {
            printf ( "%s: vkEnumeratePhysicalDevices reported zero accessible devices.\n\n"
                     "Do you have a compatible Vulkan installable client driver (ICD) "
                     "installed?\nPlease look at the Getting Started guide for "
                     "additional information.\n",
                     "vkEnumeratePhysicalDevices Failure" );
        }

        /* Look for validation layers */
        validation_found = 0;
        uint32_t device_layer_count = 0;
        err =
            vkEnumerateDeviceLayerProperties ( mVkPhysicalDevice, &device_layer_count, nullptr );
        assert ( !err );

        if ( device_layer_count > 0 )
        {
            VkLayerProperties *device_layers =
                new VkLayerProperties[device_layer_count];
            err = vkEnumerateDeviceLayerProperties ( mVkPhysicalDevice, &device_layer_count,
                    device_layers );
            assert ( !err );

            if ( mValidate )
            {
                validation_found = CheckLayers ( mDeviceValidationLayers,
                                                 device_layer_count,
                                                 device_layers );
            }

            delete[] device_layers;
        }

        if ( mValidate && !validation_found )
        {
            printf ( "%s: vkEnumerateDeviceLayerProperties failed to find"
                     "a required validation layer.\n\n"
                     "Please look at the Getting Started guide for additional "
                     "information.\n",
                     "vkCreateDevice Failure" );
        }

        /* Look for device extensions */
        uint32_t device_extension_count = 0;
        VkBool32 swapchainExtFound = 0;
        // Why does it drop extension names here?
        mExtensionNames.clear();

        err = vkEnumerateDeviceExtensionProperties ( mVkPhysicalDevice, nullptr,
                &device_extension_count, nullptr );
        assert ( !err );

        if ( device_extension_count > 0 )
        {
            VkExtensionProperties *device_extensions =
                new VkExtensionProperties[device_extension_count];
            err = vkEnumerateDeviceExtensionProperties (
                      mVkPhysicalDevice, nullptr, &device_extension_count, device_extensions );
            assert ( !err );

            for ( uint32_t i = 0; i < device_extension_count; i++ )
            {
                if ( !strcmp ( VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                               device_extensions[i].extensionName ) )
                {
                    swapchainExtFound = 1;
                    mExtensionNames.emplace_back ( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
                }
            }
            delete [] device_extensions;
        }

        if ( !swapchainExtFound )
        {
            printf ( "%s: vkEnumerateDeviceExtensionProperties failed to find "
                     "the " VK_KHR_SWAPCHAIN_EXTENSION_NAME
                     " extension.\n\nDo you have a compatible "
                     "Vulkan installable client driver (ICD) installed?\nPlease "
                     "look at the Getting Started guide for additional "
                     "information.\n",
                     "vkCreateInstance Failure" );
        }

        if ( mValidate )
        {
            mCreateDebugReportCallback =
                ( PFN_vkCreateDebugReportCallbackEXT ) vkGetInstanceProcAddr (
                    mVkInstance, "vkCreateDebugReportCallbackEXT" );
            mDestroyDebugReportCallback =
                ( PFN_vkDestroyDebugReportCallbackEXT ) vkGetInstanceProcAddr (
                    mVkInstance, "vkDestroyDebugReportCallbackEXT" );
            if ( !mCreateDebugReportCallback )
            {
                printf (
                    "%s: GetProcAddr: Unable to find vkCreateDebugReportCallbackEXT\n",
                    "vkGetProcAddr Failure" );
            }
            if ( !mDestroyDebugReportCallback )
            {
                printf (
                    "%s: GetProcAddr: Unable to find vkDestroyDebugReportCallbackEXT\n",
                    "vkGetProcAddr Failure" );
            }
            mDebugReportMessage =
                ( PFN_vkDebugReportMessageEXT ) vkGetInstanceProcAddr (
                    mVkInstance, "vkDebugReportMessageEXT" );
            if ( !mDebugReportMessage )
            {
                printf ( "%s: GetProcAddr: Unable to find vkDebugReportMessageEXT\n",
                         "vkGetProcAddr Failure" );
            }

            VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
            PFN_vkDebugReportCallbackEXT callback;
            callback = mUseBreak ? BreakCallback : dbgFunc;
            dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            dbgCreateInfo.pNext = nullptr;
            dbgCreateInfo.pfnCallback = callback;
            dbgCreateInfo.pUserData = nullptr;
            dbgCreateInfo.flags =
                VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            err = mCreateDebugReportCallback ( mVkInstance, &dbgCreateInfo, nullptr,
                                               &mMsgCallback );
            switch ( err )
            {
            case VK_SUCCESS:
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                printf ( "%s: CreateDebugReportCallback: out of host memory\n",
                         "CreateDebugReportCallback Failure" );
                break;
            default:
                printf ( "%s: CreateDebugReportCallback: unknown failure\n",
                         "CreateDebugReportCallback Failure" );
                break;
            }
        }
        vkGetPhysicalDeviceProperties ( mVkPhysicalDevice, &mVkPhysicalDeviceProperties );

        /* Call with NULL data to get count */
        uint32_t queue_count;
        vkGetPhysicalDeviceQueueFamilyProperties ( mVkPhysicalDevice, &queue_count, nullptr );
        assert ( queue_count >= 1 );

        VkQueueFamilyProperties *queue_props = new VkQueueFamilyProperties [queue_count];
        vkGetPhysicalDeviceQueueFamilyProperties ( mVkPhysicalDevice, &queue_count, queue_props );
        // Find a queue that supports gfx
        uint32_t gfx_queue_idx = 0;
        for ( gfx_queue_idx = 0; gfx_queue_idx < queue_count;
              gfx_queue_idx++ )
        {
            if ( queue_props[gfx_queue_idx].queueFlags & VK_QUEUE_GRAPHICS_BIT )
            {
                break;
            }
        }
        assert ( gfx_queue_idx < queue_count );

        // Query fine-grained feature support for this device.
        //  If app has specific feature requirements it should check supported
        //  features based on this query
        VkPhysicalDeviceFeatures physDevFeatures;
        vkGetPhysicalDeviceFeatures ( mVkPhysicalDevice, &physDevFeatures );

#if 0
        GET_INSTANCE_PROC_ADDR ( mVkInstance, GetPhysicalDeviceSurfaceSupportKHR );
        GET_INSTANCE_PROC_ADDR ( mVkInstance, GetPhysicalDeviceSurfaceCapabilitiesKHR );
        GET_INSTANCE_PROC_ADDR ( mVkInstance, GetPhysicalDeviceSurfaceFormatsKHR );
        GET_INSTANCE_PROC_ADDR ( mVkInstance, GetPhysicalDeviceSurfacePresentModesKHR );
        GET_INSTANCE_PROC_ADDR ( mVkInstance, GetSwapchainImagesKHR );
#endif
    }

    Vulkan::~Vulkan()
    {
    }
}