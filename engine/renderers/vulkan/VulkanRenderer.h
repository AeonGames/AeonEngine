/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Matrix4x4.h"
#include "aeongames/Transform.h"
#include "VulkanWindow.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanWindow;
    class VulkanModel;
    class VulkanRenderer : public Renderer
    {
    public:
        VulkanRenderer ( bool aValidate = true );
        ~VulkanRenderer() override;
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
        const VkCommandPool& GetCommandPool() const;
        const VkSemaphore& GetSignalSemaphore() const;
        const VkDescriptorSetLayout& GetMatrixDescriptorSetLayout() const;
        uint32_t GetQueueFamilyIndex() const;
        uint32_t GetMemoryTypeIndex ( VkMemoryPropertyFlags aVkMemoryPropertyFlags ) const;
        uint32_t FindMemoryTypeIndex ( uint32_t typeFilter, VkMemoryPropertyFlags properties ) const;
        VkCommandBuffer BeginSingleTimeCommands() const;
        void EndSingleTimeCommands ( VkCommandBuffer commandBuffer ) const;
        std::unique_ptr<Window> CreateWindowProxy ( void* aWindowId ) const final;

        std::unique_ptr<Mesh> CreateMesh() const final;
        std::unique_ptr<Pipeline> CreatePipeline() const final;
        std::unique_ptr<Material> CreateMaterial() const final;
        std::unique_ptr<Image> CreateImage() const final;
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
        void InitializeMatrixDescriptorSetLayout();
        void FinalizeMatrixDescriptorSetLayout();
        bool mValidate{ true };
        VkInstance mVkInstance{ VK_NULL_HANDLE };
        VkDevice mVkDevice { VK_NULL_HANDLE};
        VkPhysicalDevice mVkPhysicalDevice{ VK_NULL_HANDLE };
        VkPhysicalDeviceProperties  mVkPhysicalDeviceProperties{};
        VkPhysicalDeviceMemoryProperties  mVkPhysicalDeviceMemoryProperties{};
        VkDebugReportCallbackEXT mVkDebugReportCallbackEXT{ VK_NULL_HANDLE };
        VkCommandPool mVkCommandPool{ VK_NULL_HANDLE };
        VkCommandBuffer mVkCommandBuffer{ VK_NULL_HANDLE };
        VkQueue mVkQueue{ VK_NULL_HANDLE };
        VkSemaphore mVkSignalSemaphore{ VK_NULL_HANDLE };
        VkFence mVkFence{ VK_NULL_HANDLE };
        VkRenderPass mVkRenderPass{ VK_NULL_HANDLE };
        VkFormat mVkDepthStencilFormat{ VK_FORMAT_UNDEFINED };
        VkSurfaceFormatKHR mVkSurfaceFormatKHR{};
        VkDescriptorSetLayout mVkMatrixDescriptorSetLayout{ VK_NULL_HANDLE };
        VkDebugReportCallbackCreateInfoEXT mDebugReportCallbackCreateInfo {};
        uint32_t mQueueFamilyIndex{};
        std::vector<const char*> mInstanceLayerNames;
        std::vector<const char*> mInstanceExtensionNames;
        std::vector<const char*> mDeviceLayerNames;
        std::vector<const char*> mDeviceExtensionNames;
        // Instance Functions
        bool mFunctionsLoaded = false;
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT { VK_NULL_HANDLE };
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT { VK_NULL_HANDLE };
#if 0
        // Device Extension Functions
        PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTagEXT { VK_NULL_HANDLE };
        PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT { VK_NULL_HANDLE };
        PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBeginEXT { VK_NULL_HANDLE };
        PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEndEXT {VK_NULL_HANDLE };
        PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsertEXT { VK_NULL_HANDLE };
#endif
    };
}
#endif
