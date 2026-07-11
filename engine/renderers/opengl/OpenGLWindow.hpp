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
#include <array>
#include <mutex>
#include "aeongames/Platform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/Texture.hpp"
#include "aeongames/GpuLight.hpp"
#include "aeongames/GpuClusterParams.hpp"
#include "aeongames/GpuGlobals.hpp"
#include "aeongames/GpuShadowParams.hpp"
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
    /// @brief One candidate instance for GPU frustum culling; matches the
    ///        GpuCullInstance layout in cull.comp (112 bytes): model matrix,
    ///        local-space AABB (center + half-extents in .xyz) and packed draw
    ///        parameters (index count, first index, base vertex, material index).
    struct GpuCullInstance
    {
        Matrix4x4 mModel;
        float mCenter[4];
        float mRadii[4];
        uint32_t mDraw[4];
    };
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
        /// @brief Bind the off-screen shadow framebuffer and prepare the
        ///        directional shadow depth pass for the given light matrix.
        void BeginShadowPass ( const Matrix4x4& aLightViewProjection );
        /// @brief End the shadow depth pass and restore the main framebuffer
        ///        and viewport.
        void EndShadowPass();
        /// @brief Upload this frame's spot shadow casters into the spot
        ///        ShadowParams UBO sampled by the shading pass.
        void SetSpotShadowParams ( const GpuSpotShadowParams& aSpotShadowParams );
        /// @brief Bind the spot shadow framebuffer to one array layer and
        ///        prepare its depth pass for the given caster light matrix.
        void BeginSpotShadowPass ( uint32_t aSlot, const Matrix4x4& aLightViewProjection );
        /// @brief End the current spot shadow depth pass and restore the main
        ///        framebuffer and viewport.
        void EndSpotShadowPass();
        /// @brief Upload this frame's point shadow casters into the point
        ///        ShadowParams UBO sampled by the shading pass.
        void SetPointShadowParams ( const GpuPointShadowParams& aPointShadowParams );
        /// @brief Bind the point shadow framebuffer to one cube-face array layer
        ///        (caster*6 + face) and prepare its depth pass.
        void BeginPointShadowPass ( uint32_t aCaster );
        /// @brief End the current point shadow depth pass and restore the main
        ///        framebuffer and viewport.
        void EndPointShadowPass();
        /// @brief End the current frame and present.
        void EndRender();
        /// @brief Block until the GPU has finished every command issued for this
        ///        window (glFinish), so buffers can be safely read back.
        void Finish();
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
        /// @brief Issue one instanced draw for a batch of identical-geometry
        ///        nodes, uploading their model matrices to a transient
        ///        per-instance storage buffer indexed by gl_InstanceID.
        void RenderInstanced ( std::span<const Matrix4x4> aModelMatrices,
                               const Mesh& aMesh,
                               const Pipeline& aPipeline,
                               const Material* aMaterial = nullptr,
                               Topology aTopology = Topology::TRIANGLE_LIST,
                               uint32_t aVertexStart = 0,
                               uint32_t aVertexCount = 0xffffffff,
                               RenderPass aRenderPass = RenderPass::Shading );
        /// @brief Draw a super-batch of consecutive same-pipeline pooled meshes
        ///        with one glMultiDrawElementsIndirect. All transforms go into one
        ///        shared InstanceMatrices allocation and all material indices into
        ///        one InstanceMaterials allocation; one indirect command per
        ///        distinct mesh (consecutive equal meshes share a command with
        ///        instanceCount = run) selects its slice via base vertex / first
        ///        index / base instance. The GPU-driven merge (Vulkan parity).
        void RenderMultiBatch ( const Pipeline& aPipeline,
                                std::span<const Matrix4x4> aTransforms,
                                std::span<const Mesh* const> aMeshes,
                                std::span<const Material* const> aMaterials,
                                RenderPass aRenderPass ) const;
        /// @brief Dispatch the GPU frustum cull for one pooled shading batch:
        ///        the compute compacts the visible instances into a draw-command
        ///        list + count and parallel model / material arrays, recorded for
        ///        DrawCulledShadingBatches. Runs before the draws so the whole
        ///        pipeline group is culled first, then drawn.
        void CullShadingBatch ( const Pipeline& aShadingPipeline, const Mesh& aRepresentativeMesh,
                                std::span<const GpuCullInstance> aInstances );
        /// @brief Barrier making the cull compute's command / count / model /
        ///        material writes visible to the indirect draw and vertex stages.
        void BarrierComputeToIndirect() const;
        /// @brief Draw every culled shading batch with glMultiDrawElementsIndirect
        ///        Count, sourcing the surviving-draw count from the GPU.
        void DrawCulledShadingBatches();
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
        /// @brief Upload the per-frame scene-wide globals (ambient) to this window's Globals UBO.
        void SetGlobals ( const GpuGlobals& aGlobals );
        /// @brief Set the equirectangular HDR environment map drawn as a skybox.
        /// Uploads to a GL texture only when the source Texture changes.
        void SetEnvironmentMap ( const Texture* aEnvironmentMap );
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
        /// @brief Create the off-screen directional shadow map: a sampleable
        ///        depth texture, its framebuffer, and the ShadowParams UBO.
        void InitializeShadowMap();
        /// @brief Release the shadow map texture, framebuffer and UBO.
        void FinalizeShadowMap();
        /// @brief Create the off-screen spot shadow map: a sampleable depth
        ///        texture array (one layer per caster), its framebuffer, the
        ///        spot ShadowParams UBO and the per-pass depth matrix scratch UBO.
        void InitializeSpotShadowMap();
        /// @brief Release the spot shadow map array, framebuffer and UBOs.
        void FinalizeSpotShadowMap();
        /// @brief Create the off-screen point shadow map: a sampleable depth
        ///        texture array (six cube-face layers per caster), its
        ///        framebuffer, the point ShadowParams UBO and the per-pass depth
        ///        matrix scratch UBO.
        void InitializePointShadowMap();
        /// @brief Release the point shadow map array, framebuffer and UBOs.
        void FinalizePointShadowMap();
        /// @brief Upload per-object model matrices into a transient storage
        ///        buffer and bind it at INSTANCE_MATRICES. The uber-pipeline
        ///        vertex shaders index it by gl_InstanceID, so a single draw
        ///        covers the whole batch.
        void BindObjectMatrices ( std::span<const Matrix4x4> aMatrices ) const;
        /// @brief Upload @p aCount copies of a per-instance bindless material
        ///        index into a transient storage buffer and bind it at
        ///        INSTANCE_MATERIALS, parallel to BindObjectMatrices. The shading
        ///        vertex shader reads it by gl_InstanceID and forwards it to the
        ///        fragment shader, so a material-sorted batch shades correctly
        ///        (and a later indirect multi-draw can mix materials).
        void BindInstanceMaterials ( uint32_t aMaterialIndex, uint32_t aCount ) const;
        /// @brief Bind the engine-owned state for a Shading-pass draw (matrices,
        ///        lights, clustering, globals, shadow params + textures). Shared
        ///        by Render and RenderInstanced so the set has one definition.
        void BindShadingPassState() const;
        /// @brief Bind the engine-owned state for a depth pre-pass draw:
        ///        matrices, cluster params and the ClusterActive SSBO.
        void BindDepthPrePassState() const;
        /// @brief Bind the ShadowParams state for a shadow-depth draw (the active
        ///        point/spot caster's scratch UBO, or the directional buffer).
        void BindShadowPassState() const;
        /// @brief Shared implementation of Render and RenderInstanced: selects
        ///        the pass pipeline/state, uploads the object matrices, binds
        ///        material/mesh and issues one instanced draw. A single matrix
        ///        with aInstanceCount>1 hardware-instances that matrix (editor
        ///        grid); a multi-matrix span drives per-instance transforms.
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
        mutable OpenGLStorageMemoryPoolBuffer mStorageMemoryPoolBuffer;
        OpenGLBuffer mMatrices{};
        OpenGLBuffer mLights{};
        OpenGLBuffer mClusterParams{};
        OpenGLBuffer mGlobals{};
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
        // Renderer-owned fullscreen tone-map pipeline (loaded lazily) and its
        // empty VAO, used to resolve the off-screen linear HDR colour target to
        // the swapchain (exposure + ACES tone map + sRGB encode).
        Pipeline mTonemapPipeline{};
        bool mTonemapLoaded{false};
        GLuint mFullscreenVAO{0};
        // HDR environment: the scene's environment Texture (tracked so it is
        // uploaded only when it changes), the GL cube map it is resampled into
        // for the skybox, and the renderer-owned skybox pipeline drawn behind
        // the geometry.
        const Texture* mEnvironmentTexture{nullptr};
        GLuint mEnvironmentCubeTexture{0};
        Pipeline mSkyboxPipeline{};
        bool mSkyboxLoaded{false};
        // GGX-prefiltered specular IBL: a cube-map RGBA16F mip chain (built on
        // the CPU from the environment) sampled by the shading pass along the
        // reflection vector. A 1x1 fallback stands in when a scene has no
        // environment so the shader's textureSize gate keeps the flat look.
        GLuint mPrefilteredEnvTexture{0};
        // Off-screen directional shadow map: a sampleable depth texture and its
        // framebuffer, plus the ShadowParams UBO (light view-projection +
        // filtering params) and the renderer-owned depth-only pipeline that
        // substitutes the scene's draw pipelines during the shadow pass.
        GLuint mShadowDepthTexture{0};
        GLuint mShadowFrameBuffer{0};
        OpenGLBuffer mShadowParams{};
        Pipeline mShadowDepthPipeline{};
        bool mShadowDepthLoaded{false};
        // Off-screen spot shadow maps: a sampleable depth texture array with one
        // layer per caster, its framebuffer (a single layer attached per pass),
        // the spot ShadowParams UBO sampled by the shading pass, and a scratch
        // UBO holding the current pass's light matrix for the shared depth-only
        // pipeline (whose vertex shader reads the directional ShadowParams
        // block; the spot passes bind this scratch there instead). mInSpotShadowPass
        // selects which the shadow-pass draw branch binds as the depth matrix.
        GLuint mSpotShadowDepthTexture{0};
        GLuint mSpotShadowFrameBuffer{0};
        OpenGLBuffer mSpotShadowParams{};
        OpenGLBuffer mSpotShadowDepthScratch{};
        bool mInSpotShadowPass{false};
        // Off-screen point shadow maps: a sampleable depth texture array with
        // six cube-face layers per caster (layer = caster*6 + face), rendered
        // one face per pass, plus the point ShadowParams UBO sampled by the
        // shading pass and a per-pass depth matrix scratch UBO. mInPointShadowPass
        // routes the shadow-pass draw branch to the scratch matrix.
        GLuint mPointShadowDepthTexture{0};
        GLuint mPointShadowFrameBuffer{0};
        OpenGLBuffer mPointShadowParams{};
        OpenGLBuffer mPointShadowDepthScratch{};
        // Point passes use a dedicated depth pipeline that writes linear radial
        // distance from the light (point_shadow_depth) instead of projected
        // depth; lazily loaded on the first point pass.
        Pipeline mPointShadowDepthPipeline{};
        bool mPointShadowDepthLoaded{false};
        // CPU mirror of the point shadow params so a point pass can read its
        // caster's world position + radius, which the linear-distance depth
        // shader needs (passed through the depth scratch's params vec4).
        GpuPointShadowParams mPointShadowParamsCpu{};
        bool mInPointShadowPass{false};
        // True once BeginFrame() has begun this frame; makes BeginFrame()
        // idempotent so the app can run a pre-render-pass compute phase
        // (e.g. skinning) before BeginRender().
        bool mFrameBegun{false};
        // One GPU fence per frame in flight, inserted after a frame's draws and
        // waited on before that ring slot's persistent-mapped pool buffers are
        // reused kFramesInFlight frames later. Makes the storage/uniform ring
        // reuse explicitly correct instead of relying on SwapBuffers throttling
        // to keep the CPU less than kFramesInFlight frames ahead of the GPU.
        std::array<GLsync, kFramesInFlight> mFrameFences{};
        size_t mFrameIndex{0};
        // Drives ClusterParams.screen.w; enables active-cluster culling once
        // the mark stage has run this frame.
        bool mActiveCullEnabled{false};
        uint32_t mViewportWidth{0};
        uint32_t mViewportHeight{0};
        // Reused scratch holding one bindless material index per instance for
        // BindInstanceMaterials; grows once then amortises (no per-draw alloc).
        mutable std::vector<uint32_t> mInstanceMaterials{};
        // GL_ARB_multi_draw_indirect command layout (5 x GLuint = 20 bytes); one
        // per distinct mesh in a super-batch, consumed by glMultiDrawElementsIndirect.
        struct DrawElementsIndirectCommand
        {
            GLuint mCount;
            GLuint mInstanceCount;
            GLuint mFirstIndex;
            GLuint mBaseVertex;
            GLuint mBaseInstance;
        };
        // Reused scratch of indirect commands for RenderMultiBatch (amortised).
        mutable std::vector<DrawElementsIndirectCommand> mIndirectCommands{};
        // Renderer-owned GPU frustum-cull compute pipeline, loaded lazily the
        // first time a pooled shading batch is culled.
        Pipeline mCullPipeline{};
        bool mCullLoaded{false};
        // One pooled shading batch whose draw commands were generated on the GPU
        // by the cull compute; drawn with glMultiDrawElementsIndirectCount.
        struct CulledShadingBatch
        {
            const Pipeline* mPipeline;
            const Mesh* mRepresentativeMesh;
            BufferAccessor mCommands;
            BufferAccessor mCount;
            BufferAccessor mModels;
            BufferAccessor mMaterials;
            uint32_t mMaxDraws;
        };
        std::vector<CulledShadingBatch> mCulledShadingBatches{};
    };
}
#endif
