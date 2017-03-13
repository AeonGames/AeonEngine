/*
Copyright (C) 2016,2017 Rodrigo Jose Hernandez Cordoba

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
#include <unordered_map>
#include "aeongames/Platform.h"
#include "aeongames/Renderer.h"

namespace AeonGames
{
    class VulkanRenderer : public Renderer
    {
    public:
        VulkanRenderer ( bool aValidate = true );
        ~VulkanRenderer() override;
        void BeginRender() const final;
        void EndRender() const final;
        void Render ( const std::shared_ptr<Model> aModel ) const final;
        bool AllocateModelRenderData ( std::shared_ptr<Model> aModel ) final;
        bool AddRenderingWindow ( uintptr_t aWindowId ) final;
        void RemoveRenderingWindow ( uintptr_t aWindowId ) final;
        void Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) const final;
        void SetViewMatrix ( const float aMatrix[16] ) final;
        void SetProjectionMatrix ( const float aMatrix[16] ) final;
        void SetModelMatrix ( const float aMatrix[16] ) final;
        const VkDevice& GetDevice() const;
        const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const;
    private:
        void InitializeInstance();
        void InitializeDevice();
        void InitializeSemaphore();
        void InitializeDebug();
        void SetupLayersAndExtensions();
        void SetupDebug();
        void LoadFunctions();
        void FinalizeInstance();
        void FinalizeDevice();
        void FinalizeSemaphore();
        void FinalizeDebug();

        bool mValidate = true;
        VkInstance mVkInstance = VK_NULL_HANDLE;
        VkDevice mVkDevice = VK_NULL_HANDLE;
        VkPhysicalDevice mVkPhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties  mVkPhysicalDeviceProperties{};
        VkPhysicalDeviceMemoryProperties  mVkPhysicalDeviceMemoryProperties{};
        VkDebugReportCallbackEXT mVkDebugReportCallbackEXT = VK_NULL_HANDLE;
        VkCommandPool mVkCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer mVkCommandBuffer = VK_NULL_HANDLE;
        VkQueue mVkQueue = VK_NULL_HANDLE;
        VkSemaphore mVkSemaphore = VK_NULL_HANDLE;
        VkDebugReportCallbackCreateInfoEXT mDebugReportCallbackCreateInfo = {};
        uint32_t mQueueFamilyIndex = 0;
        std::vector<const char*> mInstanceLayerNames;
        std::vector<const char*> mInstanceExtensionNames;
        std::vector<const char*> mDeviceLayerNames;
        std::vector<const char*> mDeviceExtensionNames;
        // Vulkan Functions
        bool mFunctionsLoaded = false;
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;

        /** @todo From here on, these members are the same as the Vulkan renderer...
            shall we create a common class? */
        void UpdateMatrices();
        float mMatrices[ ( 16 * 6 ) + ( 12 * 1 )] =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mProjectionMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mModelMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mViewProjectionMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mModelViewMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // mModelViewProjectionMatrix
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            /*  mNormalMatrix With Padding,
            this should really be a 3x3 matrix,
            but std140 packing requires 16 byte alignment
            and a mat3 is escentially a vec3[3]*/
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
        };

        float* mViewMatrix = mMatrices + ( 16 * 0 );
        float* mProjectionMatrix = mMatrices + ( 16 * 1 );
        float* mModelMatrix = mMatrices + ( 16 * 2 );
        // Cache Matrices
        float* mViewProjectionMatrix = mMatrices + ( 16 * 3 );
        float* mModelViewMatrix = mMatrices + ( 16 * 4 );
        float* mModelViewProjectionMatrix = mMatrices + ( 16 * 5 );
        float* mNormalMatrix = mMatrices + ( 16 * 6 );

        struct WindowData
        {
            /**@todo Reorder fields for better memory alignment.*/
            uintptr_t mWindowId = 0;
            VkSurfaceKHR mVkSurfaceKHR = VK_NULL_HANDLE;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            VkSurfaceCapabilitiesKHR mVkSurfaceCapabilitiesKHR {};
            VkSurfaceFormatKHR mVkSurfaceFormatKHR {};
            uint32_t mSwapchainImageCount = 2;
            VkSwapchainKHR mVkSwapchainKHR = VK_NULL_HANDLE;
            VkViewport mVkViewport = {};
            std::vector<VkImage> mVkSwapchainImages;
            std::vector<VkImageView> mVkSwapchainImageViews;
            VkImage mVkDepthStencilImage = VK_NULL_HANDLE;
            VkDeviceMemory mVkDepthStencilImageMemory = VK_NULL_HANDLE;
            VkImageView mVkDepthStencilImageView = VK_NULL_HANDLE;
            VkFormat mVkDepthStencilFormat = VK_FORMAT_UNDEFINED;
            bool mHasStencil = false;
            VkRenderPass mVkRenderPass = VK_NULL_HANDLE;
            std::vector<VkFramebuffer> mVkFramebuffers;
            uint32_t mActiveImageIndex = UINT32_MAX;
            VkFence mVkFence = VK_NULL_HANDLE;
            /* Not sure if the following should be here */
            VkCommandPool mVkCommandPool = VK_NULL_HANDLE;
            VkCommandBuffer mVkCommandBuffer = VK_NULL_HANDLE;
#endif
        };

        std::vector<WindowData> mWindowRegistry;
#if 0
        std::unordered_map <
        std::shared_ptr<Model>,
            std::shared_ptr<VulkanModel >>
            mModelMap;
#endif

    };
}
#endif
