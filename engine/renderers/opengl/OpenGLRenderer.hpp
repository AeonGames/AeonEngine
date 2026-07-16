/*
Copyright (C) 2016-2022,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLRENDERER_HPP
#define AEONGAMES_OPENGLRENDERER_HPP

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>
#include "aeongames/Renderer.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/GpuMaterial.hpp"
#include "OpenGLFunctions.hpp"
#include "OpenGLBuffer.hpp"
#include "OpenGLMesh.hpp"
#include "OpenGLPipeline.hpp"
#include "OpenGLMaterial.hpp"
#include "OpenGLTexture.hpp"
#include "OpenGLWindow.hpp"
#ifdef Status
#undef Status
#endif
#include "aeongames/ProtoBufClasses.hpp"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class UniformBuffer;
    class OpenGLMesh;
    class OpenGLModel;
    class Scene;
    /** @brief OpenGL rendering backend implementing the Renderer interface. */
    class OpenGLRenderer : public Renderer
    {
    public:
        /// @brief Construct from a native window handle.
        OpenGLRenderer ( void* aWindow );
        ~OpenGLRenderer();
        /// @brief Registered renderer name ("OpenGL"); selects per-renderer
        ///        pipeline shader variants.
        std::string_view GetName() const final;
        void LoadMesh ( const Mesh& aMesh ) final;
        void UnloadMesh ( const Mesh& aMesh ) final;
        /// @brief Get the platform-specific OpenGL rendering context.
        void* GetContext() const;
        /// @brief Get the shared vertex array object.
        GLuint GetVertexArrayObject() const;
        /// @brief Get the overlay shader program identifier.
        GLuint GetOverlayProgram() const;
        /// @brief Get the overlay screen-quad buffer.
        GLuint GetOverlayQuad() const;

        /// @brief Bind a mesh for subsequent draw calls.
        void BindMesh ( const Mesh& aMesh, GLuint aSkinnedVertexBufferId = 0, size_t aSkinnedVertexOffset = 0, size_t aSkinnedVertexStride = 0 );
        /// @brief Bind a pipeline (shader program) for rendering. When
        ///        @p aInstanced is true the pipeline's INSTANCED program variant
        ///        is selected if available.
        void BindPipeline ( const Pipeline& aPipeline );
        /// @brief Bind a pipeline's compute program for the given ordered
        ///        compute stage for subsequent dispatch.
        void BindComputePipeline ( const Pipeline& aPipeline, uint32_t aComputeStageIndex );
        /// @brief Set the active material for rendering.
        void SetMaterial ( const Material& aMaterial );
        /// @brief Bind the global bindless material SSBO to the current pipeline's
        ///        Bindless block. Used by RenderMultiBatch, whose per-instance
        ///        material index comes from InstanceMaterials (no per-material
        ///        Bind runs to bind it).
        void BindMaterialStorageBuffer() const;

        /// @brief Bind a storage buffer (SSBO) to the current pipeline's
        ///        storage block identified by the CRC32 of its GLSL block name.
        void BindStorageBuffer ( uint32_t aBinding, const BufferAccessor& aBuffer ) const;
        /// @brief Bind a raw GL buffer object to the current pipeline's storage
        ///        block identified by the CRC32 of its GLSL block name. Used to
        ///        expose a mesh's own vertex buffer as a compute SSBO source.
        void BindStorageBufferId ( uint32_t aBinding, GLuint aBufferId, size_t aOffset, size_t aSize ) const;
        /// @brief Resolve a mesh to its loaded OpenGL wrapper, loading it on demand.
        const OpenGLMesh* GetOpenGLMesh ( const Mesh& aMesh );
        /// @brief Bind the matrices uniform buffer for the current draw.
        void SetMatrices ( const OpenGLBuffer& aMatricesBuffer ) const;
        /// @brief Bind the per-frame lights uniform buffer for the current draw.
        void SetLights ( const OpenGLBuffer& aLightsBuffer ) const;
        /// @brief Bind the clustered-shading params uniform buffer for the current draw/dispatch.
        void SetClusterParams ( const OpenGLBuffer& aClusterParamsBuffer ) const;
        /// @brief Bind the per-frame scene-wide globals (ambient) uniform buffer for the current draw.
        void SetGlobals ( const OpenGLBuffer& aGlobalsBuffer ) const;
        /// @brief Bind the directional shadow params uniform buffer for the current draw.
        void SetShadowParams ( const OpenGLBuffer& aShadowParamsBuffer ) const;
        /// @brief Bind the spot shadow params uniform buffer for the current draw.
        void SetSpotShadowParams ( const OpenGLBuffer& aSpotShadowParamsBuffer ) const;
        /// @brief Bind the point shadow params uniform buffer for the current draw.
        void SetPointShadowParams ( const OpenGLBuffer& aPointShadowParamsBuffer ) const;
        void LoadPipeline ( const Pipeline& aPipeline ) final;
        void UnloadPipeline ( const Pipeline& aPipeline ) final;
        void LoadMaterial ( const Material& aMaterial ) final;
        void UnloadMaterial ( const Material& aMaterial ) final;
        void LoadTexture ( const Texture& aTexture ) final;
        void UnloadTexture ( const Texture& aTexture ) final;
        /// @brief Get the OpenGL texture identifier for a loaded texture.
        GLuint GetTextureId ( const Texture& aTexture );
        /// @brief Get the resident GL_ARB_bindless_texture handle for a loaded
        ///        texture, loading it on demand. Zero when the bindless path is
        ///        unavailable.
        GLuint64 GetTextureHandle ( const Texture& aTexture );
        /// @brief True when the GL_ARB_bindless_texture path is active; gates the
        ///        resident-handle / global material SSBO rendering path.
        bool HasBindlessTexture() const;
        /// @brief Write a material record into the global bindless material SSBO
        ///        and return its index (selected per draw by the MaterialIndex
        ///        uniform to pick the record).
        uint32_t RegisterBindlessMaterial ( const GpuMaterial& aMaterial );
        /// @brief Release a bindless material index for reuse.
        void UnregisterBindlessMaterial ( uint32_t aIndex );
        /// @brief Get the global bindless material storage buffer object.
        GLuint GetMaterialStorageBufferId() const;
        /// @brief Resolve a material to its global bindless material record index,
        ///        loading it on demand. The draw path writes this per-instance
        ///        into the InstanceMaterials buffer so the shading shader selects
        ///        the record (parallel to the per-instance model matrices).
        uint32_t GetMaterialBindlessIndex ( const Material& aMaterial );        /// @brief Result of registering a mesh into the shared geometry pool:
        ///        the base vertex (in stride units) and first index (uint32
        ///        units) handed to the draw call.
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
                GLsizeiptr aVertexBytes, const uint32_t* aIndexData, uint32_t aIndexCount ) const;
        /// @brief The shared vertex pool buffer object for a given vertex stride,
        ///        bound as the vertex array source for pooled static meshes.
        GLuint GetGeometryVertexBufferId ( uint32_t aStride ) const;
        /// @brief The shared uint32 index pool buffer object, bound for pooled meshes.
        GLuint GetGeometryIndexBufferId() const;

        void AttachWindow ( void* aWindowId ) final;
        void DetachWindow ( void* aWindowId ) final;
        void SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) final;
        void SetLights ( void* aWindowId, std::span<const GpuLight> aLights ) final;
        void SetGlobals ( void* aWindowId, const GpuGlobals& aGlobals ) final;
        void SetEnvironmentMap ( void* aWindowId, const Texture* aEnvironmentMap ) final;
        void SetClearColor ( void* aWindowId, float R, float G, float B, float A ) final;
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
        void RecordGpuTimestamp ( void* aWindowId, uint32_t aSlot ) final;
        bool ReadGpuTimestamps ( void* aWindowId, std::array<uint64_t, kGpuTimestampMarks>& aTimestampsNs ) final;
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
#if defined(_WIN32)
        bool MakeCurrent ( HDC aDeviceContext = nullptr );
#elif defined(__unix__)
        Display* GetDisplay() const;
        bool MakeCurrent ( ::Window aWindow = None );
#endif
    protected:
        /// @brief Initialize overlay shader program and quad buffer.
        void InitializeOverlay();
        /// @brief Release overlay shader program and quad buffer.
        void FinalizeOverlay();
        /// @brief Allocate the global bindless material storage buffer when the
        ///        GL_ARB_bindless_texture path is available.
        void InitializeBindlessMaterials();
        /// @brief Probe and log the GL version and the extensions the bindless /
        ///        GPU-driven path depends on; records the capability flags below.
        void LogCapabilities();
        /// @brief Submit the scene's render queue for a single pass, resolving
        ///        the target window once and issuing instanced draws on it.
        void SubmitRenderQueue ( void* aWindowId, const Scene& aScene, RenderPass aRenderPass ) final;
        /// @brief True when aWindowId names an attached window.
        bool IsValidWindow ( void* aWindowId ) const final;
#if defined(_WIN32)
        HWND mWindowId {};
        HDC mDeviceContext{};
        HGLRC mOpenGLContext{};
#elif defined(__unix__)
        GLXContext mOpenGLContext {None};
#endif
        /// General VAO
        GLuint mVertexArrayObject{};
        /// @name Bindless / GPU-driven capability flags, probed at construction
        ///       by LogCapabilities() and used to gate the bindless / indirect paths.
        ///@{
        bool mHasBindlessTexture{false};
        bool mHasIndirectParameters{false};
        bool mHasComputeShader{false};
        ///@}
        /// @name Global bindless material storage
        /// Renderer-owned SSBO of GpuMaterial records, bound once per bindless
        /// draw; each material writes its record here at load time and keeps the
        /// returned index, which the per-draw MaterialIndex uniform selects.
        ///@{
        OpenGLBuffer mMaterialStorageBuffer{};
        uint32_t mBindlessMaterialCapacity{};
        uint32_t mBindlessMaterialHighWater{};
        std::vector<uint32_t> mBindlessMaterialFreeSlots{};
        ///@}
        // Shared geometry pool for static (non-skinned) meshes: one growable
        // vertex buffer per vertex stride plus one growable uint32 index buffer.
        // Meshes register their geometry once at load and are drawn from these
        // shared buffers via base-vertex/first-index offsets -- the foundation
        // for GPU-driven indirect draws (many meshes, one bindable buffer pair).
        // Skinned meshes keep their own buffers because their vertices are
        // re-posed each frame into a separate compute-written buffer.
        struct GeometryArena
        {
            std::unique_ptr<OpenGLBuffer> mBuffer{};
            GLsizeiptr mUsed{ 0 };
            GLsizeiptr mCapacity{ 0 };
        };
        mutable std::unordered_map<uint32_t, GeometryArena> mGeometryVertexArenas{};
        mutable GeometryArena mGeometryIndexArena{};
        // Arena buffers replaced by a larger one during growth. Meshes load
        // lazily while a frame records, so an old buffer may still be bound to
        // the shared VAO; deleting it immediately could disturb that in-flight
        // draw. They are kept alive here -- their data was copied into the new
        // buffer -- and released together at shutdown. Growth is geometric and
        // stops once the scene's meshes are resident, so this stays bounded.
        mutable std::vector<std::unique_ptr<OpenGLBuffer>> mRetiredGeometryBuffers{};
        /// @brief Ensure an arena buffer holds @p aRequired bytes, reallocating
        ///        and GPU-copying the existing contents when it must grow. Only
        ///        called at mesh-load time.
        void EnsureArenaCapacity ( GeometryArena& aArena, GLsizeiptr aRequired ) const;
        /// @brief Destroy the shared geometry pool buffers.
        void FinalizeGeometry();
        /** \addtogroup OverlayFunctionality Overlay functionality.
         * Both the shader program and the buffer describing the window/screen quad are
         * pretty much constant and usable without modifications by any Opengl window,
         * so it makes sense that they reside inside the renderer object from which the windows
         * are created.
         *
         @{*/
        /// Raw overlay shader program.
        GLuint mOverlayProgram{};
        /// Overlay quadrilateral.
        OpenGLBuffer mOverlayQuad{};
        /// Per-window cached overlay textures, recreated only when size changes.
        struct OverlayTextureCache
        {
            GLuint texture{};
            uint32_t width{};
            uint32_t height{};
        };
        std::unordered_map<void*, OverlayTextureCache> mOverlayTextureCache{};
        /**@}*/
        OpenGLPipeline* mCurrentPipeline{nullptr}; ///< Currently bound pipeline.
        std::unordered_map<size_t, OpenGLPipeline> mPipelineStore{}; ///< Loaded pipeline cache.
        std::unordered_map<size_t, OpenGLMaterial> mMaterialStore{}; ///< Loaded material cache.
        std::unordered_map<size_t, OpenGLMesh> mMeshStore{}; ///< Loaded mesh cache.
        std::unordered_map<size_t, OpenGLTexture> mTextureStore{}; ///< Loaded texture cache.
        std::unordered_map<void*, OpenGLWindow> mWindowStore{}; ///< Attached window map.
        /// Reused scratch for gathering a batch's transforms for instanced draws.
        std::vector<Matrix4x4> mInstanceTransforms{};
        /// Reused scratch for gathering a super-batch's meshes and materials
        /// (consecutive same-pipeline pooled items) for one indirect multi-draw.
        std::vector<const Mesh*> mSuperBatchMeshes{};
        std::vector<const Material*> mSuperBatchMaterials{};
        /// Reused scratch of per-instance cull inputs for the GPU frustum cull
        /// (shading pass); one GpuCullInstance per pooled candidate.
        std::vector<GpuCullInstance> mCullInstances{};
    private:
        static std::atomic<size_t> mRendererCount;
#if defined(__unix__)
        static Display* mDisplay;
#endif
    };
}
#endif
