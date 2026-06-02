/*
Copyright (C) 2017-2022,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLWINDOW_HPP
#define AEONGAMES_OPENGLWINDOW_HPP

#include <cstdint>
#include <span>
#include <vector>
#include <mutex>
#include "aeongames/Platform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/GpuLight.hpp"
#include "aeongames/GpuClusterParams.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/BufferAccessor.hpp"
#include "OpenGLFunctions.hpp"
#include "OpenGLBuffer.hpp"
#include "OpenGLFrameBuffer.hpp"
#include "OpenGLMemoryPoolBuffer.hpp"
#include "OpenGLStorageMemoryPoolBuffer.hpp"

namespace AeonGames
{
    class Buffer;
    class OpenGLRenderer;
    /** @brief OpenGL per-window rendering context and state. */
    class OpenGLWindow
    {
    public:
#if defined(_WIN32)
        OpenGLWindow ( OpenGLRenderer& aOpenGLRenderer, HWND aWindowId );
#elif defined(__unix__)
        OpenGLWindow ( OpenGLRenderer& aOpenGLRenderer, Display* aDisplay, ::Window aWindowId );
#endif
        /// @brief Move constructor.
        OpenGLWindow ( OpenGLWindow&& aOpenGLWindow );
        OpenGLWindow ( const OpenGLWindow& aOpenGLWindow ) = delete;
        OpenGLWindow& operator= ( const OpenGLWindow& aOpenGLWindow ) = delete;
        OpenGLWindow& operator= ( OpenGLWindow&& aOpenGLWindow ) = delete;
        ~OpenGLWindow();
        /// @brief Get the native window identifier.
        void* GetWindowId() const;
        /// @brief BeginFrame, optionally dispatch the per-frame compute
        ///        pipeline's ordered stages, then BeginRenderPass.
        void BeginRender ( const Pipeline* aComputePipeline = nullptr );
        /// @brief Begin the frame: make the context current and bind the framebuffer.
        void BeginFrame();
        /// @brief Begin the render pass: clear buffers and enable depth testing.
        void BeginRenderPass();
        /// @brief End the depth pre-pass, dispatch light culling and begin the
        ///        main color pass.
        void EndDepthPrePass ( const Pipeline* aComputePipeline );
        /// @brief End the current frame and present.
        void EndRender();
        void Render (   const Matrix4x4& aModelMatrix,
                        const Mesh& aMesh,
                        const Pipeline& aPipeline,
                        const Material* aMaterial = nullptr,
                        Topology aTopology = Topology::TRIANGLE_LIST,
                        uint32_t aVertexStart = 0,
                        uint32_t aVertexCount = 0xffffffff,
                        uint32_t aInstanceCount = 1,
                        uint32_t aFirstInstance = 0,
                        const BufferAccessor* aSkinnedVertices = nullptr ) const;
        /** @brief Dispatch the compute stage of a pipeline.
         *  @param aPipeline Pipeline whose compute stage to dispatch.
         *  @param aGroupCountX Number of workgroups in X.
         *  @param aGroupCountY Number of workgroups in Y.
         *  @param aGroupCountZ Number of workgroups in Z.
         *  @param aStorageBuffers Storage buffers (SSBOs) to bind for this dispatch.
         *  @param aComputeStageIndex Ordered compute stage to dispatch.
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
        /// @brief Allocate transient uniform memory for the current frame.
        BufferAccessor AllocateSingleFrameUniformMemory ( size_t aSize );
        /// @brief Allocate transient storage (SSBO) memory for the current frame.
        BufferAccessor AllocateSingleFrameStorageMemory ( size_t aSize );
        /// @brief Write pixel data to the overlay texture.
        void WriteOverlayPixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Texture::Format aFormat, Texture::Type aType, const uint8_t* aPixels );
        /// @brief Set the projection matrix for this window.
        void SetProjectionMatrix ( const Matrix4x4& aMatrix );
        /// @brief Set the view matrix for this window.
        void SetViewMatrix ( const Matrix4x4& aMatrix );
        /// @brief Upload the per-frame light list to this window's Lights UBO.
        void SetLights ( std::span<const GpuLight> aLights );
        /// @brief Set the clear color for this window.
        void SetClearColor ( float R, float G, float B, float A );
        /// @brief Get the current projection matrix.
        const Matrix4x4 & GetProjectionMatrix() const;
        /// @brief Get the current view matrix.
        const Matrix4x4 & GetViewMatrix() const;
        /// @brief Get the view frustum derived from the current matrices.
        const Frustum & GetFrustum() const;
        /// @brief Get the current frame's per-cluster light grid (offset/count)
        ///        SSBO. For test/debug introspection of the clustering result.
        const BufferAccessor & GetFrameLightGrid() const;
        /// @brief Get the current frame's per-cluster active-flag SSBO produced
        ///        by the depth pre-pass mark stage. For test/debug introspection.
        const BufferAccessor & GetFrameClusterActive() const;
        /// @brief Resize the rendering viewport.
        void ResizeViewport ( int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight );
    private:
        void Initialize();
        void SwapBuffers();
        /// @brief Recompute and upload the ClusterParams UBO from the current
        ///        (render-space) projection matrix and viewport.
        void UpdateClusterParams();
        /// @brief Dispatch compute stage 0 (cluster build) and allocate the
        ///        per-frame clustering SSBOs. Runs before the depth pre-pass.
        void DispatchClusterBuild ( const Pipeline& aComputePipeline );
        /// @brief Dispatch the remaining compute stages (light culling) after
        ///        the depth pre-pass has flagged the active clusters.
        void DispatchLightCull ( const Pipeline& aComputePipeline );
        OpenGLRenderer& mOpenGLRenderer;
#if defined(_WIN32)
        HWND mWindowId {};
        HDC mDeviceContext{};
#elif defined(__unix__)
        Display* mDisplay {};
        ::Window mWindowId{None};
#endif
        Frustum mFrustum {};
        Matrix4x4 mProjectionMatrix{};
        Matrix4x4 mViewMatrix{};
        //OpenGLTexture mOverlay{Texture::Format::RGBA, Texture::Type::UNSIGNED_INT_8_8_8_8_REV};
        OpenGLFrameBuffer mFrameBuffer{};
        OpenGLMemoryPoolBuffer mMemoryPoolBuffer;
        OpenGLStorageMemoryPoolBuffer mStorageMemoryPoolBuffer;
        OpenGLBuffer mMatrices{};
        OpenGLBuffer mLights{};
        OpenGLBuffer mClusterParams{};
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
        // True once BeginFrame() has begun this frame; makes BeginFrame()
        // idempotent so the app can run a pre-render-pass compute phase
        // (e.g. skinning) before BeginRender().
        bool mFrameBegun{false};
        // Drives ClusterParams.screen.w; enables active-cluster culling once
        // the mark stage has run this frame.
        bool mActiveCullEnabled{false};
        uint32_t mViewportWidth{0};
        uint32_t mViewportHeight{0};
    };
}
#endif
