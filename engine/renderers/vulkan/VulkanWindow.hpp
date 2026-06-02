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
#include <vulkan/vulkan.h>
#include "aeongames/GpuLight.hpp"
#include "aeongames/GpuClusterParams.hpp"
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
        /** @brief Render a mesh with the given pipeline, material, and transform.
         *  @param aModelMatrix Model-to-world transform matrix.
         *  @param aMesh Mesh geometry to render.
         *  @param aPipeline Shader pipeline to use.
         *  @param aMaterial Optional material, may be nullptr.
         *  @param aSkeleton Optional skeleton buffer accessor for skinning.
         *  @param aTopology Primitive topology.
         *  @param aVertexStart First vertex index.
         *  @param aVertexCount Number of vertices to draw.
         *  @param aInstanceCount Number of instances.
         *  @param aFirstInstance First instance index.
         */
        void Render (   const Matrix4x4& aModelMatrix,
                        const Mesh& aMesh,
                        const Pipeline& aPipeline,
                        const Material* aMaterial = nullptr,
                        const BufferAccessor* aSkeleton = nullptr,
                        Topology aTopology = Topology::TRIANGLE_LIST,
                        uint32_t aVertexStart = 0,
                        uint32_t aVertexCount = 0xffffffff,
                        uint32_t aInstanceCount = 1,
                        uint32_t aFirstInstance = 0 ) const;
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
        /// @brief Insert a memory barrier making SSBO writes visible to subsequent shader reads.
        void Barrier() const;
        /// @brief Set the projection matrix for this window.
        void SetProjectionMatrix ( const Matrix4x4& aMatrix );
        /// @brief Set the view matrix for this window.
        void SetViewMatrix ( const Matrix4x4& aMatrix );
        /// @brief Upload per-frame light list to the Lights UBO.
        void SetLights ( std::span<const GpuLight> aLights );
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
        void FinalizeSurface();
        void FinalizeSwapchain();
        void FinalizeImageViews();
        void FinalizeDepthStencil();
        void FinalizeRenderPass();
        void FinalizeFrameBuffers();
        void FinalizeCommandBuffer();
        void FinalizeMatrices();
        void FinalizeLights();
        void FinalizeClusterParams();
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

        VulkanRenderer& mVulkanRenderer;
        void* mWindowId{};
        Frustum mFrustum{};
        VulkanMemoryPoolBuffer mMemoryPoolBuffer;
        VulkanStorageMemoryPoolBuffer mStorageMemoryPoolBuffer;
        Matrix4x4 mProjectionMatrix{};
        Matrix4x4 mViewMatrix{};
        VulkanBuffer mMatrices;
        VulkanBuffer mLights;
        VulkanBuffer mClusterParams;
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
        // True while recording the depth pre-pass: Render substitutes the
        // marking pipeline instead of the scene's draw pipelines.
        bool mDepthPrePassActive{false};
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
        VkSemaphore mVkAcquireSemaphore{VK_NULL_HANDLE};
        VkFence mVkFence{ VK_NULL_HANDLE };

        ::std::vector<VkImage> mVkSwapchainImages{};
        ::std::vector<VkImageView> mVkSwapchainImageViews{};
        ::std::vector<VkFramebuffer> mVkFramebuffers{};
        ::std::vector<VkSemaphore> mVkSubmitSemaphores{};
    };
}
#endif
