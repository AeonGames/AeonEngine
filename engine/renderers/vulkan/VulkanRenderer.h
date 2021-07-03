/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#include <memory>
#include <unordered_map>
#include <tuple>
#include "aeongames/Platform.h"
#include "aeongames/Renderer.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/Transform.h"
#include "VulkanWindow.h"
#include "VulkanBuffer.h"
#include "VulkanMesh.h"

namespace AeonGames
{
    class VulkanTexture;
    class VulkanWindow;
    class VulkanRenderer final : public Renderer
    {
    public:
        VulkanRenderer ( bool aValidate = true );
        ~VulkanRenderer() final;
        const VkInstance& GetInstance() const;
        const VkPhysicalDevice& GetPhysicalDevice() const;
        const VkDevice& GetDevice() const;
        const VkQueue& GetQueue() const;
        const VkFence& GetFence() const;
        const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const;
        const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const;
        const VkRenderPass& GetRenderPass() const;
        const VkFormat& GetDepthStencilFormat() const;
        const VkSurfaceFormatKHR& GetSurfaceFormatKHR() const;
        const VkCommandBuffer& GetCommandBuffer() const;
        const VkSemaphore& GetSignalSemaphore() const;
        const VkDescriptorSetLayout& GetUniformBufferDescriptorSetLayout() const;
        const VkDescriptorSetLayout& GetUniformBufferDynamicDescriptorSetLayout() const;
        const VkDescriptorSetLayout& GetSamplerDescriptorSetLayout ( size_t aSamplerCount ) const;
        uint32_t GetQueueFamilyIndex() const;
        uint32_t GetMemoryTypeIndex ( VkMemoryPropertyFlags aVkMemoryPropertyFlags ) const;
        uint32_t FindMemoryTypeIndex ( uint32_t typeFilter, VkMemoryPropertyFlags properties ) const;
        VkCommandBuffer BeginSingleTimeCommands() const;
        void EndSingleTimeCommands ( VkCommandBuffer commandBuffer ) const;
        std::unique_ptr<Window> CreateWindowProxy ( void* aWindowId ) const final;
        std::unique_ptr<Window> CreateWindowInstance ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen ) const final;
        void LoadMesh ( const Mesh& aMesh ) final;
        void UnloadMesh ( const Mesh& aMesh ) final;
        void BindMesh ( const Mesh& aMesh ) const final;
        void BindPipeline ( const Pipeline& aPipeline ) const final;
        void SetMaterial ( const Material& aMaterial ) const final;
        void SetSkeleton ( const BufferAccessor& aSkeletonBuffer ) const final;
        void SetModelMatrix ( const Matrix4x4& aMatrix ) final;
        void SetProjectionMatrix ( const Matrix4x4& aMatrix ) final;
        void SetViewMatrix ( const Matrix4x4& aMatrix ) final;
        void LoadPipeline ( const Pipeline& aPipeline ) final;
        void UnloadPipeline ( const Pipeline& aPipeline ) final;
        void LoadMaterial ( const Material& aMaterial ) final;
        void UnloadMaterial ( const Material& aMaterial ) final;
        void LoadTexture ( const Texture& aTexture ) final;
        void UnloadTexture ( const Texture& aTexture ) final;
        BufferAccessor AllocateSingleFrameUniformMemory ( size_t aSize );
        void ResetMemoryPoolBuffer();
#if defined (VK_USE_PLATFORM_XLIB_KHR)
        Display* GetDisplay() const;
#endif
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
        void InitializeDescriptorSetLayout ( VkDescriptorSetLayout& aVkDescriptorSetLayout, VkDescriptorType aVkDescriptorType );
        void FinalizeDescriptorSetLayout ( VkDescriptorSetLayout& aVkDescriptorSetLayout );

#if defined (VK_USE_PLATFORM_XLIB_KHR)
        Display* mDisplay {XOpenDisplay ( nullptr ) };
#endif

        VkDescriptorPool mMatricesDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mMatricesDescriptorSet{VK_NULL_HANDLE};
        bool mValidate { true };
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
        VkDescriptorSetLayout mVkUniformBufferDescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSetLayout mVkUniformBufferDynamicDescriptorSetLayout{ VK_NULL_HANDLE };
        mutable std::vector<std::tuple<size_t, VkDescriptorSetLayout>> mVkSamplerDescriptorSetLayouts{};
        mutable const std::tuple<VkPipelineLayout, VkPipeline>* mBoundPipeline{nullptr};
        VkDebugReportCallbackCreateInfoEXT mDebugReportCallbackCreateInfo {};
        uint32_t mQueueFamilyIndex{};
        std::vector<const char*> mInstanceLayerNames{};
        std::vector<const char*> mInstanceExtensionNames{};
        std::vector<const char*> mDeviceLayerNames{};
        std::vector<const char*> mDeviceExtensionNames{};
        // Instance Functions
        bool mFunctionsLoaded = false;
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT { VK_NULL_HANDLE };
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT { VK_NULL_HANDLE };
        std::unordered_map<size_t, VulkanMesh> mMeshStore{};
        std::unordered_map<size_t, std::tuple<VkPipelineLayout, VkPipeline>> mPipelineStore{};
        std::unordered_map<size_t, std::tuple<VkDescriptorPool, VkDescriptorSet, VkDescriptorSet, std::unique_ptr<VulkanBuffer>>> mMaterialStore{};
        std::unordered_map<size_t, std::tuple<VkImage, VkDeviceMemory, VkDescriptorImageInfo>> mTextureStore{};
#if 0
        // Device Extension Functions
        PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTagEXT { VK_NULL_HANDLE };
        PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT { VK_NULL_HANDLE };
        PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBeginEXT { VK_NULL_HANDLE };
        PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEndEXT {VK_NULL_HANDLE };
        PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsertEXT { VK_NULL_HANDLE };
#endif
        VulkanBuffer mMatrices;
        VulkanMemoryPoolBuffer mMemoryPoolBuffer;
    };
}
#endif
