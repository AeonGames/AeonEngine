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
#include "aeongames/Memory.h"
#include "VulkanWindow.h"

namespace AeonGames
{
    class VulkanWindow;
    class VulkanModel;
    class VulkanRenderer : public Renderer
    {
    public:
        VulkanRenderer ( bool aValidate = true );
        ~VulkanRenderer() override;
        void BeginRender ( uintptr_t aWindowId ) const final;
        void EndRender ( uintptr_t aWindowId ) const final;
        void Render ( const std::shared_ptr<Model> aModel ) const final;
        bool AllocateModelRenderData ( std::shared_ptr<Model> aModel ) final;
        bool AddRenderingWindow ( uintptr_t aWindowId ) final;
        void RemoveRenderingWindow ( uintptr_t aWindowId ) final;
        void Resize ( uintptr_t aWindowId, uint32_t aWidth, uint32_t aHeight ) final;
        void SetViewMatrix ( const float aMatrix[16] ) final;
        void SetProjectionMatrix ( const float aMatrix[16] ) final;
        void SetModelMatrix ( const float aMatrix[16] ) final;
        const VkInstance& GetInstance() const;
        const VkPhysicalDevice& GetPhysicalDevice() const;
        const VkDevice& GetDevice() const;
        const VkQueue& GetQueue() const;
        const VkSemaphore& GetSemaphore() const;
        const VkFence& GetFence() const;
        const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const;
        const VkRenderPass& GetRenderPass() const;
        const VkFormat& GetDepthStencilFormat() const;
        const VkSurfaceFormatKHR& GetSurfaceFormatKHR() const;
        const VkBuffer& GetMatricesUniformBuffer() const;
        const VkCommandBuffer& GetCommandBuffer() const;
        const VkDescriptorSetLayout& GetDescriptorSetLayout() const;
        const VkDescriptorSet& GetDescriptorSet() const;
        uint32_t GetQueueFamilyIndex() const;
    private:
        void InitializeInstance();
        void InitializeDevice();
        void InitializeSemaphore();
        void InitializeFence();
        void InitializeRenderPass();
        void InitializeCommandPool();
        void InitializeDescriptorPool();
        void InitializeDebug();
        void InitializeMatricesUniform();
        void SetupLayersAndExtensions();
        void SetupDebug();
        void LoadFunctions();
        void FinalizeInstance();
        void FinalizeDevice();
        void FinalizeSemaphore();
        void FinalizeFence();
        void FinalizeRenderPass();
        void FinalizeCommandPool();
        void FinalizeDescriptorPool();
        void FinalizeDebug();
        void FinalizeMatricesUniform();

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
        VkFence mVkFence = VK_NULL_HANDLE;
        VkRenderPass mVkRenderPass = VK_NULL_HANDLE;
        VkFormat mVkDepthStencilFormat = VK_FORMAT_UNDEFINED;
        VkBuffer mMatricesUniformBuffer = VK_NULL_HANDLE;
        VkDeviceMemory mMatricesUniformMemory = VK_NULL_HANDLE;
        VkSurfaceFormatKHR mVkSurfaceFormatKHR{};
        VkDescriptorSetLayout mVkDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool mVkDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet mVkDescriptorSet = VK_NULL_HANDLE;
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

        /** @todo From here on, these members are the same as the OpenGL renderer...
            shall we create a common class? */
        void UpdateMatrices();

        struct Matrices
        {
            float mViewMatrix[16];
            float mProjectionMatrix[16];
            float mModelMatrix[16];
            // Cache Matrices
            float mViewProjectionMatrix[16];
            float mModelViewMatrix[16];
            float mModelViewProjectionMatrix[16];
            float mNormalMatrix[12];
        };
        Matrices mMatrices =
        {
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            },
            // mProjectionMatrix
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            },
            // mModelMatrix
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            },
            // mViewProjectionMatrix
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            },
            // mModelViewMatrix
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            },
            // mModelViewProjectionMatrix
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            },
            /*  mNormalMatrix With Padding,
            this should really be a 3x3 matrix,
            but std140 packing requires 16 byte alignment
            and a mat3 is escentially a vec3[3]*/
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0
            }
        };
        std::vector<std::unique_ptr<VulkanWindow>> mWindowRegistry;
        std::unordered_map <
        std::shared_ptr<Model>,
            std::shared_ptr<VulkanModel >>
            mModelMap;
    };
}
#endif
