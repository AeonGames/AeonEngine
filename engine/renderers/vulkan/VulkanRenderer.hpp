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
#include <array>
#include "aeongames/Platform.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/MaterialSamplers.hpp"
#include "VulkanWindow.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanMesh.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanMaterial.hpp"
#include "VulkanTexture.hpp"
#include "VulkanMemoryPoolBuffer.hpp"

namespace AeonGames
{
    class Texture;
    class VulkanTexture;
    class VulkanWindow;
    class Scene;
    /** @brief Vulkan rendering backend implementing the Renderer interface. */
    class VulkanRenderer final : public Renderer
    {
    public:
        /// @brief Construct from a native window handle.
        VulkanRenderer ( void* aWindow );
        ~VulkanRenderer() final;
        /// @brief Registered renderer name ("Vulkan"); selects per-renderer
        ///        pipeline shader variants.
        std::string_view GetName() const final;
        /// @brief Get the Vulkan instance handle.
        const VkInstance& GetInstance() const;
        /// @brief Get the physical device handle.
        const VkPhysicalDevice& GetPhysicalDevice() const;
        /// @brief Get the logical device handle.
        const VkDevice& GetDevice() const;
        /// @brief Get the graphics queue handle.
        const VkQueue& GetQueue() const;
        /// @brief Get the physical device properties.
        const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const;
        /// @brief Get the physical device memory properties.
        const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const;
        /// @brief Get the descriptor-indexing (bindless) limits queried at
        ///        device creation, used to size the global bindless resource arrays.
        const VkPhysicalDeviceDescriptorIndexingProperties& GetDescriptorIndexingProperties() const;
        /// @brief Get or create a descriptor set layout matching the given create info.
        const VkDescriptorSetLayout& GetDescriptorSetLayout ( const VkDescriptorSetLayoutCreateInfo& aDescriptorSetLayoutCreateInfo ) const;
        //const VkDescriptorSetLayout& GetSamplerDescriptorSetLayout ( size_t aSamplerCount ) const;
        /// @brief Get the queue family index used for graphics operations.
        uint32_t GetQueueFamilyIndex() const;
        /// @brief Get a memory type index matching the requested property flags.
        uint32_t GetMemoryTypeIndex ( VkMemoryPropertyFlags aVkMemoryPropertyFlags ) const;
        /// @brief Find a memory type index matching both a type filter and property flags.
        uint32_t FindMemoryTypeIndex ( uint32_t typeFilter, VkMemoryPropertyFlags properties ) const;
        /// @brief Begin recording a single-use command buffer.
        VkCommandBuffer BeginSingleTimeCommands() const;
        /// @brief Submit and free a single-use command buffer.
        void EndSingleTimeCommands ( VkCommandBuffer commandBuffer ) const;
        void LoadMesh ( const Mesh& aMesh ) final;
        void UnloadMesh ( const Mesh& aMesh ) final;
        void LoadPipeline ( const Pipeline& aPipeline ) final;
        void UnloadPipeline ( const Pipeline& aPipeline ) final;
        void LoadMaterial ( const Material& aMaterial ) final;
        void UnloadMaterial ( const Material& aMaterial ) final;
        void LoadTexture ( const Texture& aTexture ) final;
        void UnloadTexture ( const Texture& aTexture ) final;
        /// @brief Get the descriptor image info for a loaded texture.
        const VkDescriptorImageInfo* GetTextureDescriptorImageInfo ( const Texture& aTexture ) const;
        /// @brief Get the descriptor image info for the fallback texture used by
        /// materials that lack a sampler required by their pipeline.
        const VkDescriptorImageInfo* GetDefaultTextureDescriptorImageInfo() const;
        /// @brief Get the descriptor image info for the canonical fallback
        /// texture of material sampler slot @p aSlot (see kMaterialSamplerSlots),
        /// bound when a material omits that sampler.
        const VkDescriptorImageInfo* GetMaterialSamplerFallbackDescriptorImageInfo ( size_t aSlot ) const;

        void AttachWindow ( void* aWindowId ) final;
        void DetachWindow ( void* aWindowId ) final;
        void SetClearColor ( void* aWindowId, float R, float G, float B, float A ) final;
        void SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void SetLights ( void* aWindowId, std::span<const GpuLight> aLights ) final;
        void SetGlobals ( void* aWindowId, const GpuGlobals& aGlobals ) final;
        void SetEnvironmentMap ( void* aWindowId, const Texture* aEnvironmentMap ) final;
        void ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) final;
        void BeginRender ( void* aWindowId, const Pipeline* aComputePipeline = nullptr ) final;
        void BeginFrame ( void* aWindowId ) final;
        void BeginRenderPass ( void* aWindowId ) final;
        void EndDepthPrePass ( void* aWindowId, const Pipeline* aComputePipeline ) final;
        void BeginShadowPass ( void* aWindowId, const Matrix4x4& aLightViewProjection ) final;
        void EndShadowPass ( void* aWindowId ) final;
        void SetSpotShadowParams ( void* aWindowId, const GpuSpotShadowParams& aSpotShadowParams ) final;
        void BeginSpotShadowPass ( void* aWindowId, uint32_t aSlot, const Matrix4x4& aLightViewProjection ) final;
        void EndSpotShadowPass ( void* aWindowId ) final;
        void SetPointShadowParams ( void* aWindowId, const GpuPointShadowParams& aPointShadowParams ) final;
        void BeginPointShadowPass ( void* aWindowId, uint32_t aCaster ) final;
        void EndPointShadowPass ( void* aWindowId ) final;
        void EndRender ( void* aWindowId ) final;
        void Finish ( void* aWindowId ) final;
        void Render ( void* aWindowId,
                      const Matrix4x4& aModelMatrix,
                      const Mesh& aMesh,
                      const Pipeline& aPipeline,
                      const Material* aMaterial = nullptr,
                      Topology aTopology = Topology::TRIANGLE_LIST,
                      uint32_t aVertexStart = 0,
                      uint32_t aVertexCount = 0xffffffff,
                      uint32_t aInstanceCount = 1,
                      uint32_t aFirstInstance = 0,
                      const BufferAccessor* aSkinnedVertices = nullptr,
                      RenderPass aRenderPass = RenderPass::Shading ) const final;
        void RenderInstanced ( void* aWindowId,
                               std::span<const Matrix4x4> aModelMatrices,
                               const Mesh& aMesh,
                               const Pipeline& aPipeline,
                               const Material* aMaterial = nullptr,
                               Topology aTopology = Topology::TRIANGLE_LIST,
                               uint32_t aVertexStart = 0,
                               uint32_t aVertexCount = 0xffffffff,
                               RenderPass aRenderPass = RenderPass::Shading ) final;
        void Dispatch ( void* aWindowId,
                        const Pipeline& aPipeline,
                        uint32_t aGroupCountX,
                        uint32_t aGroupCountY = 1,
                        uint32_t aGroupCountZ = 1,
                        std::span<const StorageBufferBinding> aStorageBuffers = {},
                        uint32_t aComputeStageIndex = 0 ) const final;
        void Skin ( void* aWindowId,
                    const Pipeline& aSkinningPipeline,
                    const Mesh& aMesh,
                    const BufferAccessor& aSkinningMatrices,
                    const BufferAccessor& aSkinnedVertices ) const final;
        void Barrier ( void* aWindowId ) const final;
        const Frustum& GetFrustum ( void* aWindowId ) const final;
        const Matrix4x4& GetProjectionMatrix ( void* aWindowId ) const final;
        const BufferAccessor* GetFrameLightGrid ( void* aWindowId ) const final;
        const BufferAccessor* GetFrameClusterActive ( void* aWindowId ) const final;
        BufferAccessor AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize ) final;
        BufferAccessor AllocateSingleFrameStorageMemory ( void* aWindowId, size_t aSize ) final;
        void RenderOverlay ( void* aWindowId, const GuiOverlay& aGuiOverlay ) final;
        /// @brief Get the common Vulkan render pass.
        VkRenderPass GetRenderPass() const;
        /// @brief Get the cached VulkanPipeline for a Pipeline resource.
        const VulkanPipeline* GetVulkanPipeline ( const Pipeline& aPipeline, VkRenderPass aRenderPass = VK_NULL_HANDLE );
        /// @brief Get the cached VulkanMaterial for a Material resource.
        const VulkanMaterial* GetVulkanMaterial ( const Material& aMaterial );
        /// @brief Get the cached VulkanMesh for a Mesh resource.
        const VulkanMesh* GetVulkanMesh ( const Mesh& aMesh );
#if defined (VK_USE_PLATFORM_XLIB_KHR)
        Display* GetDisplay() const;
#endif
        bool HasPrimitiveTopologyListRestart() const;
    private:
        void InitializeInstance();
        void InitializeDevice();
        void InitializeCommandPools();
        void InitializeDebug();
        void InitializeOverlay();
        void SetupLayersAndExtensions();
        void SetupDebug();
        void LoadFunctions();
        void FinalizeInstance();
        void FinalizeDevice();
        void FinalizeCommandPools();
        void FinalizeDebug();
        void FinalizeOverlay();
        void InitializeDescriptorSetLayout ( VkDescriptorSetLayout& aVkDescriptorSetLayout, VkDescriptorType aVkDescriptorType );
        void FinalizeDescriptorSetLayout ( VkDescriptorSetLayout& aVkDescriptorSetLayout );
        /// @brief Submit the scene's render queue for one pass, resolving the
        ///        target window once and merging sorted runs of identical-geometry
        ///        items into instanced draws.
        void SubmitRenderQueue ( void* aWindowId, const Scene& aScene, RenderPass aRenderPass ) final;
        /// @brief True when aWindowId names an attached window.
        bool IsValidWindow ( void* aWindowId ) const final;
#if defined (VK_USE_PLATFORM_XLIB_KHR)
        Display* mDisplay {XOpenDisplay ( nullptr ) };
#endif
        bool mValidate { true };
        VkInstance mVkInstance{ VK_NULL_HANDLE };
        VkDevice mVkDevice { VK_NULL_HANDLE};
        VkPhysicalDevice mVkPhysicalDevice{ VK_NULL_HANDLE };
        VkPhysicalDeviceProperties  mVkPhysicalDeviceProperties{};
        VkPhysicalDeviceMemoryProperties  mVkPhysicalDeviceMemoryProperties{};
        // Descriptor-indexing (bindless) limits queried at device creation and
        // used later to size the global bindless texture array. Populated in
        // InitializeDevice alongside the feature checks.
        VkPhysicalDeviceDescriptorIndexingProperties mVkDescriptorIndexingProperties{};
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
        // Scratch buffer holding one batch's contiguous model matrices, reused
        // across batches so SubmitRenderQueue only allocates when a batch grows
        // beyond any previously seen size.
        std::vector<Matrix4x4> mInstanceTransforms{};
        // Fallback textures for each canonical material sampler slot
        // (kMaterialSamplerSlots), bound when a material omits that sampler
        // (e.g. an untextured material drawn with a diffuse-map shader, or a
        // material with no normal map). Loaded once at construction from each
        // slot's fallback_path. Slot 0 ("textures/default.png") also serves as
        // the general fallback exposed by GetDefaultTextureDescriptorImageInfo.
        std::array<const Texture*, kMaterialSamplerSlots.size() > mMaterialSamplerFallbacks{};
        const Texture* mDefaultTexture{ nullptr };
        bool mHasPrimitiveTopologyListRestart{false};

        // Overlay resources
        VkPipeline mOverlayPipeline{ VK_NULL_HANDLE };
        VkPipelineLayout mOverlayPipelineLayout{ VK_NULL_HANDLE };
        VkDescriptorSetLayout mOverlayDescriptorSetLayout{ VK_NULL_HANDLE };
        VkSampler mOverlaySampler{ VK_NULL_HANDLE };
        VkBuffer mOverlayVertexBuffer{ VK_NULL_HANDLE };
        VkDeviceMemory mOverlayVertexBufferMemory{ VK_NULL_HANDLE };
        struct OverlayTextureCache
        {
            VkImage image{ VK_NULL_HANDLE };
            VkDeviceMemory memory{ VK_NULL_HANDLE };
            VkImageView imageView{ VK_NULL_HANDLE };
            VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
            VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
            VkBuffer stagingBuffer{ VK_NULL_HANDLE };
            VkDeviceMemory stagingMemory{ VK_NULL_HANDLE };
            uint32_t width{};
            uint32_t height{};
        };
        std::unordered_map<void*, OverlayTextureCache> mOverlayTextureCache{};
    };
}
#endif
