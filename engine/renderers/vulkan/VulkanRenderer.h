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
#include <unordered_map>
#include "aeongames/Platform.h"
#include "aeongames/Renderer.h"
#include "aeongames/Memory.h"
#include "VulkanWindow.h"

namespace AeonGames
{
    class VulkanWindow;
    class VulkanModel;
    class VulkanRenderer : public Renderer, public std::enable_shared_from_this<VulkanRenderer>
    {
    public:
        VulkanRenderer ( bool aValidate = true );
        ~VulkanRenderer() override;
        void Render ( const std::shared_ptr<const ModelInstance>& aModelInstance ) const final;
        void LoadModel ( const std::shared_ptr<const Model>& aModel ) final;
        void UnloadModel ( const std::shared_ptr<const Model>& aModel ) final;
        void LoadScene ( const std::shared_ptr<const Scene>& aScene ) final;
        void UnloadScene ( const std::shared_ptr<const Scene>& aScene ) final;
        std::unique_ptr<Window> CreateWindowProxy ( void* aWindowId ) const final;
        void SetViewTransform ( const Transform aTransform ) final;
        void SetProjectionMatrix ( const Matrix4x4& aMatrix ) final;
        //void SetModelMatrix ( const float aMatrix[16] ) final;
        const VkInstance& GetInstance() const;
        const VkPhysicalDevice& GetPhysicalDevice() const;
        const VkDevice& GetDevice() const;
        const VkQueue& GetQueue() const;
        const VkFence& GetFence() const;
        const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const;
        const VkRenderPass& GetRenderPass() const;
        const VkFormat& GetDepthStencilFormat() const;
        const VkSurfaceFormatKHR& GetSurfaceFormatKHR() const;
        const VkCommandBuffer& GetCommandBuffer() const;
        const VkSemaphore& GetSignalSemaphore() const;
        uint32_t GetQueueFamilyIndex() const;
        uint32_t GetMemoryTypeIndex ( VkMemoryPropertyFlags aVkMemoryPropertyFlags ) const;
        uint32_t FindMemoryTypeIndex ( uint32_t typeFilter, VkMemoryPropertyFlags properties ) const;
        VkCommandBuffer BeginSingleTimeCommands() const;
        void EndSingleTimeCommands ( VkCommandBuffer commandBuffer ) const;
        struct Transforms
        {
            float mProjectionMatrix[16];
            float mViewMatrix[16];
            float mModelMatrix[16];
        };
        const Transforms& GetTransforms() const;
    private:
        void InitializeInstance();
        void InitializeDevice();
        void InitializeSemaphores();
        void InitializeFence();
        void InitializeRenderPass();
        void InitializeCommandPool();
        void InitializeDebug();
        void SetupLayersAndExtensions();
        void SetupDebug();
        void LoadFunctions();
        void FinalizeInstance();
        void FinalizeDevice();
        void FinalizeSemaphores();
        void FinalizeFence();
        void FinalizeRenderPass();
        void FinalizeCommandPool();
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
        VkSemaphore mVkSignalSemaphore = VK_NULL_HANDLE;
        VkFence mVkFence = VK_NULL_HANDLE;
        VkRenderPass mVkRenderPass = VK_NULL_HANDLE;
        VkFormat mVkDepthStencilFormat = VK_FORMAT_UNDEFINED;
        VkSurfaceFormatKHR mVkSurfaceFormatKHR{};
        VkDebugReportCallbackCreateInfoEXT mDebugReportCallbackCreateInfo = {};
        uint32_t mQueueFamilyIndex = 0;
        std::vector<const char*> mInstanceLayerNames;
        std::vector<const char*> mInstanceExtensionNames;
        std::vector<const char*> mDeviceLayerNames;
        std::vector<const char*> mDeviceExtensionNames;
        std::unordered_map<std::string, std::unique_ptr<VulkanModel>> mModelLibrary;
        // Instance Functions
        bool mFunctionsLoaded = false;
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = VK_NULL_HANDLE;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = VK_NULL_HANDLE;
        // Device Extension Functions
        PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTagEXT = VK_NULL_HANDLE;
        PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT = VK_NULL_HANDLE;
        PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBeginEXT = VK_NULL_HANDLE;
        PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEndEXT = VK_NULL_HANDLE;
        PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsertEXT = VK_NULL_HANDLE;
        Transforms mTransforms =
        {
            // mProjectionMatrix
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            },
            // mViewMatrix
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            },
        };
    };
}
#endif
