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
    private:
        void InitializeInstance();
        void InitializeDevice();
        void InitializeCommandPool();
        void InitializeDebug();
        void SetupDebug();
        void LoadFunctions();
        void FinalizeInstance();
        void FinalizeDevice();
        void FinalizeCommandPool();
        void FinalizeDebug();

        VkInstance mVkInstance = VK_NULL_HANDLE;
        VkDevice mVkDevice = VK_NULL_HANDLE;
        VkPhysicalDevice mVkPhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties  mVkPhysicalDeviceProperties {};
        VkDebugReportCallbackEXT mVkDebugReportCallbackEXT = VK_NULL_HANDLE;
        VkCommandPool mVkCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer mVkCommandBuffer = VK_NULL_HANDLE;
        VkQueue mVkQueue = VK_NULL_HANDLE;
        VkFence mVkFence = VK_NULL_HANDLE;
        VkSemaphore mVkSemaphore = VK_NULL_HANDLE;
        VkDebugReportCallbackCreateInfoEXT mDebugReportCallbackCreateInfo = {};
        VkViewport mVkViewport = {};
        uint32_t mQueueFamilyIndex = 0;
        std::vector<const char*> mInstanceLayerNames;
        std::vector<const char*> mInstanceExtensionNames;
        std::vector<const char*> mDeviceLayerNames;
        std::vector<const char*> mDeviceExtensionNames;
        // Vulkan Functions
        bool mFunctionsLoaded = false;
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

        // These members may change over time
        bool mValidate = true;
        bool mUseBreak = true;
        PFN_vkDebugReportMessageEXT mDebugReportMessage = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT mMsgCallback = VK_NULL_HANDLE;
    };
}
#endif
