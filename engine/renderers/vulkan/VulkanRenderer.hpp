/*
Copyright (C) 2016-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_VULKANRENDERER_HPP
#define AEONGAMES_VULKANRENDERER_HPP

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <memory>
#include <tuple>
#include "aeongames/Platform.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Transform.hpp"
#include "VulkanWindow.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanMesh.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanMaterial.hpp"
#include "VulkanTexture.hpp"
#include "VulkanMemoryPoolBuffer.hpp"

namespace AeonGames
{
    class VulkanTexture;
    class VulkanWindow;
    class VulkanRenderer final : public Renderer
    {
    public:
        VulkanRenderer ( void* aWindow );
        ~VulkanRenderer() final;
        const VkInstance& GetInstance() const;
        const VkPhysicalDevice& GetPhysicalDevice() const;
        const VkDevice& GetDevice() const;
        const VkQueue& GetQueue() const;
        const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const;
        const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const;
        const VkDescriptorSetLayout& GetDescriptorSetLayout ( const VkDescriptorSetLayoutCreateInfo& aDescriptorSetLayoutCreateInfo ) const;
        //const VkDescriptorSetLayout& GetSamplerDescriptorSetLayout ( size_t aSamplerCount ) const;
        uint32_t GetQueueFamilyIndex() const;
        uint32_t GetMemoryTypeIndex ( VkMemoryPropertyFlags aVkMemoryPropertyFlags ) const;
        uint32_t FindMemoryTypeIndex ( uint32_t typeFilter, VkMemoryPropertyFlags properties ) const;
        VkCommandBuffer BeginSingleTimeCommands() const;
        void EndSingleTimeCommands ( VkCommandBuffer commandBuffer ) const;
        void LoadMesh ( const Mesh& aMesh ) final;
        void UnloadMesh ( const Mesh& aMesh ) final;
        void LoadPipeline ( const Pipeline& aPipeline ) final;
        void UnloadPipeline ( const Pipeline& aPipeline ) final;
        void LoadMaterial ( const Material& aMaterial ) final;
        void UnloadMaterial ( const Material& aMaterial ) final;
        void LoadTexture ( const Texture& aTexture ) final;
        void UnloadTexture ( const Texture& aTexture ) final;
        const VkDescriptorImageInfo* GetTextureDescriptorImageInfo ( const Texture& aTexture ) const;

        void AttachWindow ( void* aWindowId ) final;
        void DetachWindow ( void* aWindowId ) final;
        void SetClearColor ( void* aWindowId, float R, float G, float B, float A ) final;
        void SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) final;
        void BeginRender ( void* aWindowId ) final;
        void EndRender ( void* aWindowId ) final;
        void Render ( void* aWindowId,
                      const Matrix4x4& aModelMatrix,
                      const Mesh& aMesh,
                      const Pipeline& aPipeline,
                      const Material* aMaterial = nullptr,
                      const BufferAccessor* aSkeleton = nullptr,
                      Topology aTopology = Topology::TRIANGLE_LIST,
                      uint32_t aVertexStart = 0,
                      uint32_t aVertexCount = 0xffffffff,
                      uint32_t aInstanceCount = 1,
                      uint32_t aFirstInstance = 0 ) const final;
        const Frustum& GetFrustum ( void* aWindowId ) const final;
        BufferAccessor AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize ) final;
        VkRenderPass GetRenderPass() const;
        const VulkanPipeline* GetVulkanPipeline ( const Pipeline& aPipeline );
        const VulkanMaterial* GetVulkanMaterial ( const Material& aMaterial );
        const VulkanMesh* GetVulkanMesh ( const Mesh& aMesh );
#if defined (VK_USE_PLATFORM_XLIB_KHR)
        Display* GetDisplay() const;
#endif
    private:
        void InitializeInstance();
        void InitializeDevice();
        void InitializeCommandPools();
        void InitializeDebug();
        void SetupLayersAndExtensions();
        void SetupDebug();
        void LoadFunctions();
        void FinalizeInstance();
        void FinalizeDevice();
        void FinalizeCommandPools();
        void FinalizeDebug();
        void InitializeDescriptorSetLayout ( VkDescriptorSetLayout& aVkDescriptorSetLayout, VkDescriptorType aVkDescriptorType );
        void FinalizeDescriptorSetLayout ( VkDescriptorSetLayout& aVkDescriptorSetLayout );
#if defined (VK_USE_PLATFORM_XLIB_KHR)
        Display* mDisplay {XOpenDisplay ( nullptr ) };
#endif
        bool mValidate { true };
        VkInstance mVkInstance{ VK_NULL_HANDLE };
        VkDevice mVkDevice { VK_NULL_HANDLE};
        VkPhysicalDevice mVkPhysicalDevice{ VK_NULL_HANDLE };
        VkPhysicalDeviceProperties  mVkPhysicalDeviceProperties{};
        VkPhysicalDeviceMemoryProperties  mVkPhysicalDeviceMemoryProperties{};
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{nullptr};
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT{nullptr};
        VkDebugUtilsMessengerEXT mVkDebugUtilsMessengerEXT{VK_NULL_HANDLE};
        VkCommandPool mVkSingleTimeCommandPool{ VK_NULL_HANDLE };
        VkQueue mVkQueue{ VK_NULL_HANDLE };
        mutable std::vector<std::tuple<size_t, VkDescriptorSetLayout >> mVkDescriptorSetLayouts{};
        mutable const VulkanPipeline* mBoundPipeline{nullptr};
        uint32_t mQueueFamilyIndex{};
        std::vector<const char*> mInstanceLayerNames{};
        std::vector<const char*> mInstanceExtensionNames{};
        std::vector<const char*> mDeviceLayerNames{};
        std::vector<const char*> mDeviceExtensionNames{};
        // Instance Functions
        bool mFunctionsLoaded = false;
        std::unordered_map<size_t, VulkanMesh> mMeshStore{};
        std::unordered_map<size_t, VulkanPipeline> mPipelineStore{};
        std::unordered_map<size_t, VulkanMaterial> mMaterialStore{};
        std::unordered_map<size_t, VulkanTexture> mTextureStore{};
        std::unordered_map<void*, VulkanWindow> mWindowStore{};
    };
}
#endif
