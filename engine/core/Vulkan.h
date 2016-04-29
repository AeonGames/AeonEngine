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

#ifndef AEONGAMES_VULKAN_H
#define AEONGAMES_VULKAN_H

#include <vulkan/vulkan.h>
#include <exception>
#include <vector>

namespace AeonGames
{
    class Vulkan
    {
    public:
        Vulkan ( bool aValidate = true );
        ~Vulkan();
        struct InstanceInitializationFailed : public std::exception
        {
            const char * what() const throw ()
            {
                return "Vulkan Instance Initialization Failed";
            }
        };
        struct DeviceInitializationFailed : public std::exception
        {
            const char * what() const throw ()
            {
                return "Vulkan Device Initialization Failed";
            }
        };
    private:
        bool InitializeInstance();
        void FinalizeInstance();
        bool InitializeDevice();
        void FinalizeDevice();
        VkInstance mVkInstance = nullptr;
        // These members may change over time
        bool mValidate = true;
        bool mUseBreak = true;
        std::vector<const char*> mDeviceValidationLayers;
        std::vector<const char*> mExtensionNames;
        VkPhysicalDevice mVkPhysicalDevice = nullptr;
        VkPhysicalDeviceProperties mVkPhysicalDeviceProperties = {0};
        PFN_vkCreateDebugReportCallbackEXT mCreateDebugReportCallback = nullptr;
        PFN_vkDestroyDebugReportCallbackEXT mDestroyDebugReportCallback = nullptr;
        PFN_vkDebugReportMessageEXT mDebugReportMessage = nullptr;
        VkDebugReportCallbackEXT mMsgCallback = nullptr;
    };
}
#endif
