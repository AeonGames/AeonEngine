/*
Copyright (C) 2017-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANWINDOW_HPP
#define AEONGAMES_VULKANWINDOW_HPP

#include <cstdint>
#include <span>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include "aeongames/GpuLight.hpp"
#include "aeongames/GpuClusterParams.hpp"
#include "aeongames/GpuGlobals.hpp"
#include "aeongames/GpuShadowParams.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/BufferAccessor.hpp"
#include "VulkanMemoryPoolBuffer.hpp"
#include "VulkanStorageMemoryPoolBuffer.hpp"
#include "VulkanPipeline.hpp"

namespace AeonGames
{
    class Mesh;
    class Material;
    class Pipeline;
    class BufferAccessor;
    class VulkanRenderer;
    /** @brief Vulkan per-window swapchain, render pass, and rendering state. */
    class VulkanWindow
    {
    public:
        /// @brief Construct from a renderer and native window handle.
        VulkanWindow ( VulkanRenderer& aVulkanRenderer, void* aWindowId );
        ~VulkanWindow();
        /// @brief Move constructor.
        VulkanWindow ( VulkanWindow&& aVulkanWindow );
        VulkanWindow ( const VulkanWindow& aVulkanWindow ) = delete;
        VulkanWindow& operator= ( const VulkanWindow& aVulkanWindow ) = delete;
        VulkanWindow& operator= ( VulkanWindow&& aVulkanWindow ) = delete;

        /// @brief Begin the frame: acquire the swapchain image and begin the
        ///        command buffer. Compute dispatches may be recorded after this
        ///        and before BeginRenderPass.
        void BeginFrame();
        /// @brief Begin the main render pass. Must be called after BeginFrame.
        void BeginRenderPass();
        /// @brief BeginFrame, optionally dispatch the per-frame compute
        ///        pipeline's ordered stages, then BeginRenderPass.
        void BeginRender ( const Pipeline* aComputePipeline = nullptr );
        /// @brief End the current frame, submit commands, and present.
        void EndRender();
        /// @brief End the depth pre-pass: end its render pass, make the mark
        ///        stage's ClusterActive writes visible, dispatch the remaining
        ///        (light-cull) compute stages, then begin the main color pass.
        void EndDepthPrePass ( const Pipeline* aComputePipeline );
        /// @brief Begin the directional-light shadow depth pass. Closes the
        ///        currently-open main render pass, uploads the light
        ///        view-projection to the ShadowParams UBO, and opens the
        ///        fixed-size shadow render pass with the shadow framebuffer
        ///        bound. Caller submits the render queue with
        ///        RenderPass::ShadowPass, then calls EndShadowPass.
        void BeginShadowPass ( const Matrix4x4& aLightViewProjection );
        /// @brief End the shadow depth pass: close the shadow render pass
        ///        (transitioning the shadow map to a sampleable layout),
        ///        restore the window viewport/scissor, and reopen the main
        ///        render pass for the depth pre-pass to continue.
        void EndShadowPass();
        /// @brief Upload this frame's spot shadow casters into the spot
        ///        ShadowParams UBO sampled by the shading pass.
        void SetSpotShadowParams ( const GpuSpotShadowParams& aSpotShadowParams );
        /// @brief Begin a spot-light shadow depth pass rendering into one array
        ///        layer of the spot shadow map. Closes the open main render
        ///        pass and opens the spot shadow render pass for the given slot.
        void BeginSpotShadowPass ( uint32_t aSlot, const Matrix4x4& aLightViewProjection );
        /// @brief End the current spot shadow depth pass: close its render pass
        ///        (leaving that layer sampleable), restore the window
        ///        viewport/scissor, and reopen the main render pass.
        void EndSpotShadowPass();
        /// @brief Upload this frame's point shadow casters into the point
        ///        ShadowParams UBO sampled by the shading pass.
        void SetPointShadowParams ( const GpuPointShadowParams& aPointShadowParams );
        /// @brief Begin a point-light shadow depth pass rendering into one
        ///        cube-face layer (aCaster*6 + aFace) of the point shadow map.
        void BeginPointShadowPass ( uint32_t aCaster );
        /// @brief End the current point shadow depth pass: close its render pass,
        ///        restore the window viewport/scissor, and reopen the main pass.
        void EndPointShadowPass();
        void Render (   const Matrix4x4& aModelMatrix,
                        const Mesh& aMesh,
                        const Pipeline& aPipeline,
                        const Material* aMaterial = nullptr,
                        Topology aTopology = Topology::TRIANGLE_LIST,
                        uint32_t aVertexStart = 0,
                        uint32_t aVertexCount = 0xffffffff,
                        uint32_t aInstanceCount = 1,
                        uint32_t aFirstInstance = 0,
                        const BufferAccessor* aSkinnedVertices = nullptr,
                        RenderPass aRenderPass = RenderPass::Shading ) const;
        /** @brief Render a batch of identical-geometry nodes in one instanced
         *         draw, each positioned by its own model matrix.
         *  @param aModelMatrices Contiguous per-instance model matrices.
         *  @param aMesh Mesh shared by every instance.
         *  @param aPipeline Pipeline shared by every instance.
         *  @param aMaterial Optional material, may be nullptr.
         *  @param aTopology Primitive topology.
         *  @param aVertexStart First vertex index.
         *  @param aVertexCount Number of vertices to draw.
         *  @param aRenderPass Pass this draw feeds.
         */
        void RenderInstanced ( std::span<const Matrix4x4> aModelMatrices,
                               const Mesh& aMesh,
                               const Pipeline& aPipeline,
                               const Material* aMaterial = nullptr,
                               Topology aTopology = Topology::TRIANGLE_LIST,
                               uint32_t aVertexStart = 0,
                               uint32_t aVertexCount = 0xffffffff,
                               RenderPass aRenderPass = RenderPass::Shading ) const;
        /** @brief Dispatch the compute stage of a pipeline.
         *  @param aPipeline Pipeline whose compute stage to dispatch.
         *  @param aGroupCountX Number of workgroups in X.
         *  @param aGroupCountY Number of workgroups in Y.
         *  @param aGroupCountZ Number of workgroups in Z.
         *  @param aStorageBuffers Storage buffers (SSBOs) to bind for this dispatch.
         *  @param aComputeStageIndex Ordered compute stage of the pipeline to dispatch.
         */
        void Dispatch ( const Pipeline& aPipeline,
                        uint32_t aGroupCountX,
                        uint32_t aGroupCountY = 1,
                        uint32_t aGroupCountZ = 1,
                        std::span<const StorageBufferBinding> aStorageBuffers = {},
                        uint32_t aComputeStageIndex = 0 ) const;
        /// @brief Dispatch the compute skinning pre-pass for a single skinned mesh.
        void Skin ( const Pipeline& aSkinningPipeline,
                    const Mesh& aMesh,
                    const BufferAccessor& aSkinningMatrices,
                    const BufferAccessor& aSkinnedVertices ) const;
        /// @brief Insert a memory barrier making SSBO writes visible to subsequent shader reads.
        void Barrier() const;
        /// @brief Set the projection matrix for this window.
        void SetProjectionMatrix ( const Matrix4x4& aMatrix );
        /// @brief Set the view matrix for this window.
        void SetViewMatrix ( const Matrix4x4& aMatrix );
        /// @brief Upload per-frame light list to the Lights UBO.
        void SetLights ( std::span<const GpuLight> aLights );
        /// @brief Upload per-frame scene-wide globals (ambient) to the Globals UBO.
        void SetGlobals ( const GpuGlobals& aGlobals );
        /// @brief Set the equirectangular HDR environment map drawn as a skybox;
        /// uploads to a device image only when the source Texture changes.
        void SetEnvironmentMap ( const Texture* aEnvironmentMap );
        /// @brief Get the current projection matrix.
        const Matrix4x4& GetProjectionMatrix() const;
        /// @brief Get the current view matrix.
        const Matrix4x4& GetViewMatrix() const;
        /// @brief Get the view frustum derived from the current matrices.
        const Frustum& GetFrustum() const;
        /// @brief Get the current frame's per-cluster light grid (offset/count)
        ///        SSBO. For test/debug introspection of the clustering result.
        const BufferAccessor& GetFrameLightGrid() const;
        /// @brief Get the current frame's per-cluster active-flag SSBO produced
        ///        by the depth pre-pass mark stage. For test/debug introspection.
        const BufferAccessor& GetFrameClusterActive() const;
        /// @brief Resize the rendering viewport.
        void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight );
        /// @brief Allocate transient uniform memory for the current frame.
        BufferAccessor AllocateSingleFrameUniformMemory ( size_t aSize );
        /// @brief Allocate transient storage (SSBO) memory for the current frame.
        BufferAccessor AllocateSingleFrameStorageMemory ( size_t aSize );
        /// @brief Get the Vulkan render pass for this window.
        VkRenderPass GetRenderPass() const;
        /// @brief Get the current command buffer being recorded.
        VkCommandBuffer GetCommandBuffer() const;
    private:
        void Initialize();
        void Finalize();
        void InitializeSurface();
        void InitializeSwapchain();
        void InitializeImageViews();
        void InitializeDepthStencil();
        void InitializeRenderPass();
        void InitializeFrameBuffers();
        void InitializeCommandBuffer();
        void InitializeMatrices();
        void InitializeLights();
        void InitializeClusterParams();
        void InitializeGlobals();
        void InitializeShadowMap();
        /// @brief Create the GGX-prefiltered specular environment: a mipped
        ///        RGBA32F equirect image (1x1 dummy until a scene provides an
        ///        environment), its sampler and a combined-image-sampler
        ///        descriptor set bound at PREFILTERED_ENVIRONMENT during shading.
        void InitializePrefilteredEnvironment();
        /// @brief (Re)build the prefiltered specular image from @p aEnvironmentMap
        ///        (a nullptr yields the 1x1 dummy), then repoint the persistent
        ///        descriptor set at the new image. Waits for device idle.
        void UpdatePrefilteredEnvironmentImage ( const Texture* aEnvironmentMap );
        /// @brief Create the spot shadow map: a sampleable depth texture array
        ///        (one layer per caster), per-layer framebuffers reusing the
        ///        directional shadow render pass, the spot ShadowParams UBO and
        ///        the per-slot depth matrix UBO + descriptor sets.
        void InitializeSpotShadowMap();
        /// @brief Release the spot shadow map array, framebuffers and buffers.
        void FinalizeSpotShadowMap();
        /// @brief Create the point shadow map: a depth texture array with six
        ///        cube-face layers per caster, per-layer framebuffers reusing the
        ///        directional shadow render pass, the point ShadowParams UBO and
        ///        the per-layer depth matrix UBO + descriptor sets.
        void InitializePointShadowMap();
        /// @brief Release the point shadow map array, framebuffers and buffers.
        void FinalizePointShadowMap();
        void FinalizeSurface();
        void FinalizeSwapchain();
        void FinalizeImageViews();
        void FinalizeDepthStencil();
        void FinalizeRenderPass();
        void FinalizeFrameBuffers();
        void FinalizeEnvironmentMap();
        /// @brief Release the prefiltered specular environment image, view,
        ///        sampler and descriptor set/pool.
        void FinalizePrefilteredEnvironment();
        void FinalizeCommandBuffer();
        void FinalizeMatrices();
        void FinalizeLights();
        void FinalizeClusterParams();
        void FinalizeGlobals();
        void FinalizeShadowMap();
        /// @brief Recompute and upload the ClusterParams UBO from the current
        ///        projection matrix and viewport. Cheap; called on projection
        ///        or viewport change.
        void UpdateClusterParams();
        /// @brief Dispatch compute stage 0 (cluster build) and allocate the
        ///        per-frame clustering SSBOs. Runs before the depth pre-pass.
        void DispatchClusterBuild ( const Pipeline& aComputePipeline );
        /// @brief Dispatch the remaining compute stages (light culling) after
        ///        the depth pre-pass has flagged the active clusters.
        void DispatchLightCull ( const Pipeline& aComputePipeline );
        /// @brief Upload per-object model matrices into a transient storage
        ///        buffer and bind it at INSTANCE_MATRICES for the given
        ///        pipeline. The uber-pipeline vertex shaders index it by
        ///        gl_InstanceIndex, so a single draw covers the whole batch.
        void BindObjectMatrices ( const VulkanPipeline* aPipeline, std::span<const Matrix4x4> aMatrices ) const;
        /// @brief Bind the engine-owned descriptor sets for a Shading-pass draw
        ///        (matrices, lights, clustering, globals, shadows). Excludes the
        ///        per-draw material and object-matrix sets. Shared by Render and
        ///        RenderInstanced so the set list has a single definition.
        void BindShadingPassSets ( const VulkanPipeline* aPipeline ) const;
        /// @brief Bind the engine-owned descriptor sets for a depth pre-pass
        ///        (cluster-mark) draw: matrices, cluster params, ClusterActive.
        void BindDepthPrePassSets ( const VulkanPipeline* aPipeline ) const;
        /// @brief Bind the ShadowParams descriptor set for a shadow-depth draw
        ///        (directional, or the active spot/point caster's matrix set).
        void BindShadowPassSets ( const VulkanPipeline* aPipeline ) const;
        /// @brief Shared implementation of Render and RenderInstanced: selects
        ///        the pass pipeline, binds its engine descriptor sets, drives the
        ///        object transform (push-constant fast path for a single matrix,
        ///        else the per-object matrix buffer), binds material/mesh and
        ///        issues one (possibly instanced) draw. A single matrix span with
        ///        aInstanceCount>1 hardware-instances that matrix (editor grid);
        ///        a multi-matrix span drives per-instance transforms.
        void RenderCommon ( std::span<const Matrix4x4> aModelMatrices,
                            const Mesh& aMesh,
                            const Pipeline& aPipeline,
                            const Material* aMaterial,
                            Topology aTopology,
                            uint32_t aVertexStart,
                            uint32_t aVertexCount,
                            uint32_t aInstanceCount,
                            uint32_t aFirstInstance,
                            const BufferAccessor* aSkinnedVertices,
                            RenderPass aRenderPass ) const;

        VulkanRenderer& mVulkanRenderer;
        void* mWindowId{};
        Frustum mFrustum{};
        VulkanMemoryPoolBuffer mMemoryPoolBuffer;
        mutable VulkanStorageMemoryPoolBuffer mStorageMemoryPoolBuffer;
        Matrix4x4 mProjectionMatrix{};
        Matrix4x4 mViewMatrix{};
        VulkanBuffer mMatrices;
        VulkanBuffer mLights;
        VulkanBuffer mClusterParams;
        VulkanBuffer mGlobals;
        VulkanBuffer mShadowParams;
        // Spot shadow params (all caster matrices + positions) sampled by the
        // shading pass, and the per-slot depth matrices read by the spot depth
        // passes (one aligned GpuShadowParams slot per caster).
        VulkanBuffer mSpotShadowParams;
        VulkanBuffer mSpotShadowDepthMatrices;
        // Point shadow params (six-per-caster matrices + caster positions/radii)
        // sampled by the shading pass, and the per-layer depth matrices read by
        // the point depth passes (one aligned GpuShadowParams slot per cube face
        // per caster).
        VulkanBuffer mPointShadowParams;
        VulkanBuffer mPointShadowDepthMatrices;
        // Scratch buffer holding the frustum-culled subset of this frame's
        // lights; reused across frames so capacity is not reallocated.
        std::vector<GpuLight> mVisibleLights{};
        BufferAccessor mFrameLightGrid{};
        BufferAccessor mFrameLightIndexList{};
        BufferAccessor mFrameClusterAABBs{};
        BufferAccessor mFrameLightIndexCounter{};
        BufferAccessor mFrameClusterActive{};
        // Renderer-owned depth pre-pass marking pipeline (Phase R2), loaded
        // lazily the first frame clustering runs.
        Pipeline mClusterMarkPipeline{};
        bool mClusterMarkLoaded{false};
        // Renderer-owned directional-shadow depth pipeline, loaded lazily the
        // first frame a shadow caster is present.
        Pipeline mShadowDepthPipeline{};
        bool mShadowDepthLoaded{false};
        // True once BeginFrame() has acquired the swapchain image and begun the
        // command buffer this frame; makes BeginFrame() idempotent so the app
        // can run a pre-render-pass compute phase before BeginRender().
        bool mFrameBegun{false};
        // Drives ClusterParams.screen.w; enables active-cluster culling once
        // the mark stage has run this frame.
        bool mActiveCullEnabled{false};
        VkFormat mVkDepthStencilFormat{ VK_FORMAT_UNDEFINED };
        VkSurfaceFormatKHR mVkSurfaceFormatKHR{};
        VkRenderPass mVkRenderPass{ VK_NULL_HANDLE };
        VkSurfaceKHR mVkSurfaceKHR{ VK_NULL_HANDLE };
        VkSurfaceCapabilitiesKHR mVkSurfaceCapabilitiesKHR {};
        uint32_t mSwapchainImageCount{ 2 };
        VkSwapchainKHR mVkSwapchainKHR{ VK_NULL_HANDLE };
        VkImage mVkDepthStencilImage{ VK_NULL_HANDLE };
        VkDeviceMemory mVkDepthStencilImageMemory{ VK_NULL_HANDLE };
        VkImageView mVkDepthStencilImageView { VK_NULL_HANDLE};
        // Depth-aspect view + non-comparison sampler used by the SSR composite to
        // sample the scene depth (the attachment view above may carry stencil and
        // cannot be sampled directly).
        VkImageView mVkDepthSampleImageView { VK_NULL_HANDLE };
        VkSampler mVkDepthSampler{ VK_NULL_HANDLE };
        VkCommandPool mVkCommandPool{ VK_NULL_HANDLE };
        VkCommandBuffer mVkCommandBuffer{ VK_NULL_HANDLE };
        bool mHasStencil{ false };
        uint32_t mActiveImageIndex{ UINT32_MAX };
        VkViewport mVkViewport{0, 0, 1, 1, 0, 1};
        VkRect2D mVkScissor{};

        VkDescriptorPool mMatricesDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mMatricesDescriptorSet{VK_NULL_HANDLE};
        VkDescriptorPool mLightsDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mLightsDescriptorSet{VK_NULL_HANDLE};
        VkDescriptorPool mClusterParamsDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mClusterParamsDescriptorSet{VK_NULL_HANDLE};
        VkDescriptorPool mGlobalsDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mGlobalsDescriptorSet{VK_NULL_HANDLE};
        // Directional shadow map: a fixed-size depth target the shadow pass
        // writes and the shading pass samples. A throwaway color attachment is
        // carried so the shadow render pass stays attachment-compatible with
        // the window's main render pass (pipelines are created against the
        // latter). See render-pass compatibility notes.
        VkImage mVkShadowDepthImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkShadowDepthImageMemory{VK_NULL_HANDLE};
        VkImageView mVkShadowDepthImageView{VK_NULL_HANDLE};
        VkImage mVkShadowColorImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkShadowColorImageMemory{VK_NULL_HANDLE};
        VkImageView mVkShadowColorImageView{VK_NULL_HANDLE};
        VkSampler mVkShadowSampler{VK_NULL_HANDLE};
        VkRenderPass mVkShadowRenderPass{VK_NULL_HANDLE};
        VkFramebuffer mVkShadowFramebuffer{VK_NULL_HANDLE};
        VkDescriptorPool mShadowParamsDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mShadowParamsDescriptorSet{VK_NULL_HANDLE};
        VkDescriptorPool mShadowMapDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mShadowMapDescriptorSet{VK_NULL_HANDLE};
        // Spot shadow maps: a depth texture ARRAY with one layer per caster,
        // sampled as a sampler2DArrayShadow by the shading pass. Each layer is
        // rendered through its own single-layer image view and framebuffer
        // (reusing the directional shadow render pass, which is size-agnostic).
        // A throwaway color image is shared across the framebuffers for
        // render-pass compatibility. The per-slot light matrices live in one UBO
        // with aligned slots; one descriptor set per slot binds its slot region
        // at the depth pipeline's ShadowParams slot, so each spot depth pass
        // reads its own matrix with no single-buffer write hazard.
        VkImage mVkSpotShadowDepthImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkSpotShadowDepthImageMemory{VK_NULL_HANDLE};
        VkImageView mVkSpotShadowDepthArrayView{VK_NULL_HANDLE};
        std::array<VkImageView, MAX_SPOT_SHADOW_CASTERS> mVkSpotShadowDepthLayerViews{};
        VkImage mVkSpotShadowColorImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkSpotShadowColorImageMemory{VK_NULL_HANDLE};
        VkImageView mVkSpotShadowColorImageView{VK_NULL_HANDLE};
        std::array<VkFramebuffer, MAX_SPOT_SHADOW_CASTERS> mVkSpotShadowFramebuffers{};
        VkDescriptorPool mSpotShadowParamsDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mSpotShadowParamsDescriptorSet{VK_NULL_HANDLE};
        VkDescriptorPool mSpotShadowMapDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mSpotShadowMapDescriptorSet{VK_NULL_HANDLE};
        VkDescriptorPool mSpotShadowDepthMatricesDescriptorPool{VK_NULL_HANDLE};
        std::array<VkDescriptorSet, MAX_SPOT_SHADOW_CASTERS> mSpotShadowDepthMatricesDescriptorSets{};
        VkDeviceSize mSpotShadowDepthMatrixStride{0};
        bool mInSpotShadowPass{false};
        uint32_t mCurrentSpotShadowSlot{0};
        // Point shadow maps: a cube-map-array depth target with six faces per
        // caster (layers caster*6 .. caster*6+5). All six faces of a caster are
        // rendered in ONE pass via Vulkan multiview: a dedicated render pass with
        // a six-bit view mask plus a per-caster six-layer framebuffer drives the
        // multiview vertex shader (gl_ViewIndex per cube face). One depth-params
        // descriptor set per caster feeds that caster's six face matrices.
        static constexpr uint32_t POINT_SHADOW_LAYERS = POINT_SHADOW_FACES * MAX_POINT_SHADOW_CASTERS;
        VkRenderPass mVkPointShadowRenderPass{VK_NULL_HANDLE};
        VkImage mVkPointShadowDepthImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkPointShadowDepthImageMemory{VK_NULL_HANDLE};
        VkImageView mVkPointShadowDepthArrayView{VK_NULL_HANDLE};
        std::array<VkImageView, MAX_POINT_SHADOW_CASTERS> mVkPointShadowDepthCasterViews{};
        VkImage mVkPointShadowColorImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkPointShadowColorImageMemory{VK_NULL_HANDLE};
        VkImageView mVkPointShadowColorImageView{VK_NULL_HANDLE};
        std::array<VkFramebuffer, MAX_POINT_SHADOW_CASTERS> mVkPointShadowFramebuffers{};
        VkDescriptorPool mPointShadowParamsDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mPointShadowParamsDescriptorSet{VK_NULL_HANDLE};
        VkDescriptorPool mPointShadowMapDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mPointShadowMapDescriptorSet{VK_NULL_HANDLE};
        VkDescriptorPool mPointShadowDepthMatricesDescriptorPool{VK_NULL_HANDLE};
        std::array<VkDescriptorSet, MAX_POINT_SHADOW_CASTERS> mPointShadowDepthMatricesDescriptorSets{};
        VkDeviceSize mPointShadowDepthMatrixStride{0};
        // Point passes use a dedicated depth pipeline that writes linear radial
        // distance from the light (point_shadow_depth) instead of projected
        // depth; lazily loaded on the first point pass. On Vulkan it is the
        // multiview variant (no geometry shader) and must be created against the
        // multiview point shadow render pass.
        Pipeline mPointShadowDepthPipeline{};
        bool mPointShadowDepthLoaded{false};
        // CPU mirror of the point shadow params so a point pass can read its
        // caster's world position + radius for the linear-distance depth shader.
        GpuPointShadowParams mPointShadowParamsCpu{};
        bool mInPointShadowPass{false};
        uint32_t mCurrentPointShadowCaster{0};
        VkSemaphore mVkAcquireSemaphore{VK_NULL_HANDLE};
        VkFence mVkFence{ VK_NULL_HANDLE };

        ::std::vector<VkImage> mVkSwapchainImages{};
        ::std::vector<VkImageView> mVkSwapchainImageViews{};
        // Repurposed as the per-swapchain-image tonemap framebuffers (colour
        // only): the scene renders into the HDR image below and the fullscreen
        // tonemap pass resolves it into these swapchain images.
        ::std::vector<VkFramebuffer> mVkFramebuffers{};
        // HDR offscreen target the scene renders linear radiance into (RGBA16F),
        // sampled by the fullscreen tonemap pass. mVkTonemapRenderPass + sampler
        // are extent-independent (created in InitializeRenderPass); the image,
        // framebuffer and descriptor set are extent-dependent (recreated on
        // resize in Initialize/FinalizeFrameBuffers).
        VkImage mVkHdrColorImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkHdrColorImageMemory{VK_NULL_HANDLE};
        VkImageView mVkHdrColorImageView{VK_NULL_HANDLE};
        // Deferred-specular G-buffer, rendered alongside the HDR colour as extra
        // colour attachments of the main pass and sampled by the composite in
        // the tonemap pass: view-space normal + roughness, and specular weight.
        // Both RGBA16F, extent-dependent (recreated on resize).
        VkImage mVkGNormalRoughImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkGNormalRoughImageMemory{VK_NULL_HANDLE};
        VkImageView mVkGNormalRoughImageView{VK_NULL_HANDLE};
        VkImage mVkGSpecWeightImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkGSpecWeightImageMemory{VK_NULL_HANDLE};
        VkImageView mVkGSpecWeightImageView{VK_NULL_HANDLE};
        VkFramebuffer mVkHdrFramebuffer{VK_NULL_HANDLE};
        VkSampler mVkHdrSampler{VK_NULL_HANDLE};
        VkRenderPass mVkTonemapRenderPass{VK_NULL_HANDLE};
        VkDescriptorPool mTonemapDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mTonemapDescriptorSet{VK_NULL_HANDLE};
        Pipeline mTonemapPipeline{};
        bool mTonemapLoaded{false};
        // Equirectangular HDR environment (skybox). The source Texture is tracked
        // so the device image is uploaded only when it changes; the image is
        // RGBA32F (RGB expanded, alpha unused) sampled by the skybox pass through
        // its own combined-image-sampler descriptor set.
        const Texture* mEnvironmentTexture{nullptr};
        VkImage mVkEnvImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkEnvImageMemory{VK_NULL_HANDLE};
        VkImageView mVkEnvImageView{VK_NULL_HANDLE};
        VkSampler mVkEnvSampler{VK_NULL_HANDLE};
        VkDescriptorPool mEnvDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mEnvDescriptorSet{VK_NULL_HANDLE};
        Pipeline mSkyboxPipeline{};
        bool mSkyboxLoaded{false};
        // GGX-prefiltered specular IBL: a mipped RGBA32F equirect image (built on
        // the CPU from the environment; a 1x1 dummy stands in until a scene sets
        // one) sampled by the shading pass along the reflection vector at a
        // roughness-selected LOD, bound at PREFILTERED_ENVIRONMENT.
        VkImage mVkPrefilteredEnvImage{VK_NULL_HANDLE};
        VkDeviceMemory mVkPrefilteredEnvImageMemory{VK_NULL_HANDLE};
        VkImageView mVkPrefilteredEnvImageView{VK_NULL_HANDLE};
        VkSampler mVkPrefilteredEnvSampler{VK_NULL_HANDLE};
        VkDescriptorPool mPrefilteredEnvDescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSet mPrefilteredEnvDescriptorSet{VK_NULL_HANDLE};
        ::std::vector<VkSemaphore> mVkSubmitSemaphores{};
    };
}
#endif
