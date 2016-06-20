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

#ifndef AEONGAMES_VULKANRENDERER_H
#define AEONGAMES_VULKANRENDERER_H

#include <vulkan/vulkan.h>
#include <exception>
#include <vector>
#include "aeongames/Renderer.h"

namespace AeonGames
{
    class VulkanRenderer : public Renderer
    {
    public:
        VulkanRenderer ( bool aValidate = true );
        ~VulkanRenderer();
        void BeginRender() const override final;
        void EndRender() const override final;
        void Render ( const std::shared_ptr<Mesh>& aMesh, const std::shared_ptr<Program>& aProgram ) const override final;
        std::shared_ptr<Mesh> GetMesh ( const std::string& aFilename ) const override final;
#if 0
#if _WIN32
        bool InitializeRenderingWindow ( HINSTANCE aInstance, HWND aHwnd ) override final;
#else
        bool InitializeRenderingWindow ( Display* aDisplay, Window aWindow ) override final;
#endif
        void FinalizeRenderingWindow() override final;
#endif
    private:
        void InitializeInstance();
        void InitializeDevice();
        void InitializeCommandPool();
        void InitializeDebug();
        void SetupLayersAndExtensions();
        void SetupDebug();
        void LoadFunctions();
        void FinalizeInstance();
        void FinalizeDevice();
        void FinalizeCommandPool();
        void FinalizeDebug();

        bool mValidate = true;
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
        VkSurfaceKHR mVkSurfaceKHR = VK_NULL_HANDLE;
        VkSurfaceCapabilitiesKHR mVkSurfaceCapabilitiesKHR {};
        VkSurfaceFormatKHR mVkSurfaceFormatKHR {};
        VkSwapchainKHR mVkSwapchainKHR = VK_NULL_HANDLE;
        uint32_t mSwapchainImageCount = 2;
        std::vector<VkImage> mVkSwapchainImages;
        std::vector<VkImageView> mVkSwapchainImageViews;
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
    };
}
#endif
