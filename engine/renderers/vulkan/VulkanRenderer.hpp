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
#include <vector>
#include "aeongames/Platform.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/MaterialSamplers.hpp"
#include "aeongames/GpuMaterial.hpp"
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
        /// @brief Register a texture's descriptor image info into the global
        ///        bindless combined-image-sampler array; returns its slot index.
        ///        Called by VulkanTexture once its descriptor is complete.
        uint32_t RegisterBindlessTexture ( const VkDescriptorImageInfo& aVkDescriptorImageInfo ) const;
        /// @brief Release a global bindless texture slot for reuse
        ///        (no-op for UINT32_MAX).
        void UnregisterBindlessTexture ( uint32_t aSlot ) const;
        /// @brief Slot of a loaded texture in the global bindless array.
        uint32_t GetTextureBindlessSlot ( const Texture& aTexture ) const;
        /// @brief Bindless slot of the canonical fallback texture for material
        ///        sampler slot @p aSlot, bound when a material omits that sampler.
        uint32_t GetMaterialSamplerFallbackBindlessSlot ( size_t aSlot ) const;
        /// @brief Write a material record into the global material storage buffer
        ///        and return its index (fetched per draw to read the factors and
        ///        texture slots). Called by VulkanMaterial at load.
        uint32_t RegisterBindlessMaterial ( const GpuMaterial& aGpuMaterial ) const;
        /// @brief Release a global material-buffer index for reuse.
        void UnregisterBindlessMaterial ( uint32_t aIndex ) const;
        /// @brief GPU device address of the global material storage buffer,
        ///        pushed per draw so the fragment shader reads material records
        ///        as a buffer_reference (BDA) rather than a descriptor.
        VkDeviceAddress GetMaterialStorageBufferDeviceAddress() const;
        /// @brief Result of registering a mesh into the shared geometry pool:
        ///        the base vertex (in stride units) and first index (uint32
        ///        units) to hand to the draw call.
        struct GeometryAllocation
        {
            uint32_t mBaseVertex;
            uint32_t mFirstIndex;
        };
        /// @brief Upload a static (non-skinned) mesh's vertices and uint32
        ///        indices into the shared per-stride vertex pool and the shared
        ///        index pool, returning the base vertex (stride units) and first
        ///        index (uint32 units) for the draw. Foundation for GPU-driven
        ///        indirect drawing: many meshes share one bindable buffer pair.
        GeometryAllocation RegisterMeshGeometry ( uint32_t aStride, const void* aVertexData,
                VkDeviceSize aVertexBytes, const uint32_t* aIndexData, uint32_t aIndexCount ) const;
        /// @brief The shared vertex pool buffer for a given vertex stride, bound
        ///        as vertex input for pooled static meshes of that stride.
        const VkBuffer& GetGeometryVertexBuffer ( uint32_t aStride ) const;
        /// @brief The shared uint32 index pool buffer, bound for pooled meshes.
        const VkBuffer& GetGeometryIndexBuffer() const;
        /// @brief The global bindless descriptor set (combined-image-sampler
        ///        array), bound once per frame by the shading passes.
        VkDescriptorSet GetBindlessDescriptorSet() const;
        /// @brief Layout of the global bindless descriptor set, for pipeline layouts.
        VkDescriptorSetLayout GetBindlessDescriptorSetLayout() const;

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
        /// @brief Create the global bindless combined-image-sampler array
        ///        (descriptor set, pool and layout). Called after the device.
        void InitializeBindless();
        /// @brief Destroy the global bindless descriptor set/pool/layout.
        void FinalizeBindless();
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
        // Global bindless combined-image-sampler array (descriptor set 0 of the
        // shading pipelines): every VulkanTexture registers a slot here at load,
        // so materials reference textures by index instead of owning per-material
        // sampler descriptor sets. Created in InitializeBindless after the device;
        // the free-list + high-water mark hand out and recycle array slots.
        VkDescriptorPool mVkBindlessDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout mVkBindlessDescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSet mVkBindlessDescriptorSet{ VK_NULL_HANDLE };
        uint32_t mBindlessTextureCapacity{ 0 };
        mutable uint32_t mBindlessTextureHighWater{ 0 };
        mutable std::vector<uint32_t> mBindlessTextureFreeSlots{};
        // Global material storage buffer (descriptor set 0, binding 1): one
        // GpuMaterial record per loaded material, indexed per draw by a material
        // index. Host-visible + coherent, written when materials load; the
        // free-list + high-water mark hand out and recycle record indices.
        VulkanBuffer mMaterialStorageBuffer;
        uint32_t mBindlessMaterialCapacity{ 0 };
        mutable uint32_t mBindlessMaterialHighWater{ 0 };
        mutable std::vector<uint32_t> mBindlessMaterialFreeSlots{};
        // Shared geometry pool for static (non-skinned) meshes: one growable
        // DEVICE_LOCAL vertex buffer per vertex stride plus one growable uint32
        // index buffer. Meshes register their geometry once at load and are drawn
        // from these shared buffers via base-vertex/first-index offsets -- the
        // foundation for GPU-driven indirect draws (many meshes, one bindable
        // buffer pair). Skinned meshes keep their own buffers because their
        // vertices are re-posed each frame into a separate compute-written buffer.
        struct GeometryArena
        {
            std::unique_ptr<VulkanBuffer> mBuffer{};
            VkDeviceSize mUsed{ 0 };
            VkDeviceSize mCapacity{ 0 };
        };
        mutable std::unordered_map<uint32_t, GeometryArena> mGeometryVertexArenas{};
        mutable GeometryArena mGeometryIndexArena{};
        // Arena buffers replaced by a larger one during growth. They may still be
        // bound to an in-flight command buffer (meshes load lazily while a frame
        // records), so freeing them immediately would invalidate that command
        // buffer. They are kept alive here -- their data was copied into the new
        // buffer -- and released together at shutdown. Growth is geometric and
        // stops once the scene's meshes are resident, so this stays bounded.
        mutable std::vector<std::unique_ptr<VulkanBuffer>> mRetiredGeometryBuffers{};
        /// @brief Ensure an arena buffer holds @p aRequired bytes, reallocating
        ///        and GPU-copying the existing contents when it must grow. Only
        ///        called at mesh-load time (never mid-frame), so no descriptor or
        ///        bind references the arena while it is reallocated.
        void EnsureArenaCapacity ( GeometryArena& aArena, VkDeviceSize aRequired,
                                   VkBufferUsageFlags aUsage ) const;
        /// @brief Destroy the shared geometry pool buffers.
        void FinalizeGeometry();
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
