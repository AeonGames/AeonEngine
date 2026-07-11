/*
Copyright (C) 2016-2022,2024-2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_RENDERER_H
#define AEONGAMES_RENDERER_H

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <functional>
#include "aeongames/Platform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Vector4.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/RenderItem.hpp"
#include "aeongames/GpuLight.hpp"
#include "aeongames/GpuShadowParams.hpp"
#include "aeongames/GpuGlobals.hpp"

namespace AeonGames
{
    class Frustum;
    class StringId;
    class Buffer;
    class Texture;
    class Mesh;
    class GuiOverlay;
    class Pipeline;
    class Material;
    class Window;
    class Scene;
    class BufferAccessor;
    /** A storage buffer (SSBO) binding for a compute dispatch.
     *  Maps a shader storage block — identified by the CRC32 of its GLSL block
     *  name (e.g. "Output"_crc32 or a Mesh::BindingLocations value) — to a
     *  single-frame storage allocation obtained from
     *  Renderer::AllocateSingleFrameStorageMemory. */
    struct StorageBufferBinding
    {
        uint32_t mBinding;             /**< CRC32 of the GLSL storage block name. */
        const BufferAccessor* mBuffer; /**< Storage allocation to bind. */
    };
    /** Tunable parameters for the renderer's debug-geometry step (see
     *  Renderer::SetDebugRendering). Defaults reproduce a neutral editor-style
     *  view: a 1-unit ground grid with every tenth line emphasized, colored
     *  origin axes, and per-node AABB and octree-cell wireframes. */
    struct DebugRenderSettings
    {
        bool mDrawGrid{true};        /**< Draw the analytic infinite ground grid. */
        bool mDrawNodeAABBs{true};   /**< Draw per-node world-space AABB wireframes. */
        bool mDrawOctree{true};      /**< Draw the scene octree cell wireframes. */
        bool mDrawCameraFrustums{true}; /**< Draw a wireframe frustum for each camera node. */
        float mGridCellSize{1.0f};       /**< World units between minor grid lines. */
        float mGridMajorInterval{10.0f}; /**< Minor lines between emphasized major lines. */
        float mGridFadeDistance{100.0f}; /**< Distance from the camera at which the grid fully fades. */
        Vector4 mGridColor{0.4f, 0.4f, 0.4f, 1.0f};       /**< Minor grid line color. */
        Vector4 mGridMajorColor{0.65f, 0.65f, 0.65f, 1.0f}; /**< Major grid line color. */
        Vector4 mAxisXColor{0.85f, 0.2f, 0.2f, 1.0f};     /**< Color of the world X axis line. */
        Vector4 mAxisYColor{0.2f, 0.75f, 0.2f, 1.0f};     /**< Color of the world Y axis line. */
        Vector4 mNodeAABBColor{0.2f, 0.85f, 0.3f, 1.0f};  /**< Per-node AABB wireframe color (green). */
        Vector4 mOctreeColor{0.3f, 0.5f, 0.95f, 1.0f};    /**< Octree cell wireframe color (blue). */
        Vector4 mCameraFrustumColor{0.95f, 0.8f, 0.2f, 1.0f}; /**< Camera frustum wireframe color (yellow). */
    };
    /** Abstract base class for rendering backends.
     *
     * Defines the interface for loading and unloading GPU resources, managing
     * window surfaces, and issuing draw calls. Concrete implementations provide
     * API-specific logic (e.g., Vulkan, OpenGL).
     */
    class Renderer
    {
    public:
        /** Virtual destructor. */
        DLL virtual ~Renderer() = 0;
        ///@name Renderer specific resource functions
        ///@{
        /** Loads mesh data into GPU memory.
         * @param aMesh The mesh to load.
         */
        virtual void LoadMesh ( const Mesh& aMesh ) = 0;
        /** Unloads mesh data from GPU memory.
         * @param aMesh The mesh to unload.
         */
        virtual void UnloadMesh ( const Mesh& aMesh ) = 0;
        /** Loads a rendering pipeline (shaders and state) into the renderer.
         * @param aPipeline The pipeline to load.
         */
        virtual void LoadPipeline ( const Pipeline& aPipeline ) = 0;
        /** Unloads a rendering pipeline from the renderer.
         * @param aPipeline The pipeline to unload.
         */
        virtual void UnloadPipeline ( const Pipeline& aPipeline ) = 0;
        /** Loads material data into the renderer.
         * @param aMaterial The material to load.
         */
        virtual void LoadMaterial ( const Material& aMaterial ) = 0;
        /** Unloads material data from the renderer.
         * @param aMaterial The material to unload.
         */
        virtual void UnloadMaterial ( const Material& aMaterial ) = 0;
        /** Loads a texture into GPU memory.
         * @param aTexture The texture to load.
         */
        virtual void LoadTexture ( const Texture& aTexture ) = 0;
        /** Unloads a texture from GPU memory.
         * @param aTexture The texture to unload.
         */
        virtual void UnloadTexture ( const Texture& aTexture ) = 0;
        ///@}

        ///@name Window surface related functions
        ///@{
        /**
         * Attach a Window as a rendering surface.
         * @param aWindowId Platform depended window handle.
        */
        virtual void AttachWindow ( void* aWindowId ) = 0;
        /**
         * Detach a Window as a rendering surface.
         * @param aWindowId Platform depended window handle.
        */
        virtual void DetachWindow ( void* aWindowId ) = 0;
        /** Sets the projection matrix for a specific window surface.
         * @param aWindowId Platform depended window handle.
         * @param aMatrix The projection matrix.
        */
        virtual void SetProjectionMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) = 0;
        /** Sets the view matrix for a specific window surface.
         * @param aWindowId Platform depended window handle.
         * @param aMatrix The view matrix.
        */
        virtual void SetViewMatrix ( void* aWindowId, const Matrix4x4& aMatrix ) = 0;
        /** Uploads the per-frame lights for a specific window surface.
         *  The data is copied into the window's @c Lights uniform buffer so the
         *  caller's span does not need to outlive this call. Lights past
         *  @ref MAX_LIGHTS_PER_FRAME are silently dropped.
         *  @param aWindowId Platform depended window handle.
         *  @param aLights Span of lights collected during this frame's
         *                 @c Scene::Update (see @ref Scene::GetFrameLights).
        */
        virtual void SetLights ( void* aWindowId, std::span<const GpuLight> aLights ) = 0;
        /** Uploads the per-frame scene-wide shading globals (currently the
         *  ambient fill) into the window's @c Globals uniform buffer. Called
         *  once per frame like @ref SetLights. Virtual with an empty default so
         *  a backend that has not implemented it keeps the buffer's initial
         *  value, which reproduces the former constant ambient.
         *  @param aWindowId Platform depended window handle.
         *  @param aGlobals The scene-wide globals (ambient color/intensity) to
         *                  upload.
         */
        virtual void SetGlobals ( void* aWindowId, const GpuGlobals& aGlobals )
        {
            ( void ) aWindowId;
            ( void ) aGlobals;
        }
        /** Sets the scene environment map (an equirectangular HDR) for a window,
         *  drawn as a skybox and used for image-based lighting. Virtual with an
         *  empty default so a backend without skybox support is a no-op. The
         *  backend uploads the texture only when it changes, so this is cheap to
         *  call every frame; pass nullptr to clear it.
         *  @param aWindowId Platform depended window handle.
         *  @param aEnvironmentMap Equirectangular HDR texture, or nullptr.
         */
        virtual void SetEnvironmentMap ( void* aWindowId, const Texture* aEnvironmentMap )
        {
            ( void ) aWindowId;
            ( void ) aEnvironmentMap;
        }
        /** Sets the color to be used to clear the window background.
         * @param aWindowId Platform depended window handle.
         * @param R Red component of the clear color.
         * @param G Green component of the clear color.
         * @param B Blue component of the clear color.
         * @param A Alpha component of the clear color.
        */
        virtual void SetClearColor ( void* aWindowId, float R, float G, float B, float A ) = 0;
        /** Resizes the specific window surface's viewport.
         * @param aWindowId Platform dependent window handle.
         * @param aX X coordinate of the viewport.
         * @param aY Y coordinate of the viewport.
         * @param aWidth Width of the viewport.
         * @param aHeight Height of the viewport.
         */
        virtual void ResizeViewport ( void* aWindowId, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight ) = 0;
        /** Begins a render pass for the given window surface.
         * @param aWindowId Platform dependent window handle.
         * @param aComputePipeline Optional pipeline whose ordered compute stages
         *        are dispatched once per frame (e.g. light clustering) before the
         *        render pass begins. When nullptr no per-frame compute is run.
         */
        virtual void BeginRender ( void* aWindowId, const Pipeline* aComputePipeline = nullptr ) = 0;
        /** Begins the frame for the given window surface: acquires resources and
         * starts command recording, but does not begin the render pass. Compute
         * Dispatch and Barrier calls must be recorded between BeginFrame and
         * BeginRenderPass.
         * @param aWindowId Platform dependent window handle.
         */
        virtual void BeginFrame ( void* aWindowId ) = 0;
        /** Begins the main render pass for the given window surface. Must be
         * called after BeginFrame.
         * @param aWindowId Platform dependent window handle.
         */
        virtual void BeginRenderPass ( void* aWindowId ) = 0;
        /** Begins the directional shadow-depth pass for the given window surface.
         * Uploads the shadow-casting light's world-to-clip matrix into the
         * window's ShadowParams uniform block (marking shadows enabled for this
         * frame) and begins the depth-only shadow render pass that renders into
         * the window's shadow map. Geometry submitted with
         * RenderPass::ShadowPass between this call and EndShadowPass is drawn
         * from the light's point of view. Recorded after BeginFrame and before
         * the main color render pass; only called when the scene has both a
         * lighting pipeline and a directional shadow caster.
         * @param aWindowId Platform dependent window handle.
         * @param aLightViewProjection World-space to light clip-space matrix used
         *        to render and later sample the shadow map.
         */
        virtual void BeginShadowPass ( void* aWindowId, const Matrix4x4& aLightViewProjection ) = 0;
        /** Ends the directional shadow-depth pass and transitions the shadow map
         * so it can be sampled as a depth-comparison texture by the shading
         * pass. Must be paired with a preceding BeginShadowPass.
         * @param aWindowId Platform dependent window handle.
         */
        virtual void EndShadowPass ( void* aWindowId ) = 0;
        /** Uploads the per-frame spot shadow casters into the window's
         * SpotShadowParams uniform block so the shading pass can sample each
         * spot light's shadow map layer. Called once per frame before the spot
         * shadow depth passes, regardless of caster count (a count of zero
         * leaves every spot light unshadowed). Virtual with an empty default so
         * a backend that has not yet implemented spot shadows simply ignores it.
         * @param aWindowId Platform dependent window handle.
         * @param aSpotShadowParams The per-caster light view-projections, caster
         *        positions and filtering params to upload.
         */
        virtual void SetSpotShadowParams ( void* aWindowId, const GpuSpotShadowParams& aSpotShadowParams )
        {
            ( void ) aWindowId;
            ( void ) aSpotShadowParams;
        }
        /** Begins a spot shadow-depth pass that renders scene depth from one
         * spot light's point of view into the given layer of the window's spot
         * shadow map array. Geometry submitted with RenderPass::ShadowPass
         * between this call and EndSpotShadowPass is drawn from the light's point
         * of view. Mirrors BeginShadowPass but targets an array layer and a
         * perspective light frustum. Virtual with an empty default so a backend
         * without spot shadow support ignores it.
         * @param aWindowId Platform dependent window handle.
         * @param aSlot Spot shadow map array layer to render into
         *        (0 <= aSlot < MAX_SPOT_SHADOW_CASTERS).
         * @param aLightViewProjection World-space to light clip-space matrix for
         *        this caster, used to render and later sample its shadow layer.
         */
        virtual void BeginSpotShadowPass ( void* aWindowId, uint32_t aSlot, const Matrix4x4& aLightViewProjection )
        {
            ( void ) aWindowId;
            ( void ) aSlot;
            ( void ) aLightViewProjection;
        }
        /** Ends the current spot shadow-depth pass and transitions that layer of
         * the spot shadow map array so it can be sampled by the shading pass.
         * Must be paired with a preceding BeginSpotShadowPass. Virtual with an
         * empty default so a backend without spot shadow support ignores it.
         * @param aWindowId Platform dependent window handle.
         */
        virtual void EndSpotShadowPass ( void* aWindowId )
        {
            ( void ) aWindowId;
        }
        /** Uploads the per-frame point shadow casters into the window's
         * PointShadowParams uniform block so the shading pass can sample each
         * point light's cube-face shadow layers. Called once per frame before
         * the point shadow depth passes. Virtual with an empty default so a
         * backend without point shadow support ignores it.
         * @param aWindowId Platform dependent window handle.
         * @param aPointShadowParams The per-caster six-face light view-projections,
         *        caster positions/radii and filtering params to upload.
         */
        virtual void SetPointShadowParams ( void* aWindowId, const GpuPointShadowParams& aPointShadowParams )
        {
            ( void ) aWindowId;
            ( void ) aPointShadowParams;
        }
        /** Begins a point shadow-depth pass that renders the depth of all six
         * cube faces of one point light into that caster's six cube-map-array
         * layers in a single draw (Vulkan multiview or an OpenGL geometry
         * shader). Geometry submitted with RenderPass::ShadowPass between this
         * call and EndPointShadowPass is replicated to all six faces. Virtual
         * with an empty default so a backend without point shadow support
         * ignores it.
         * @param aWindowId Platform dependent window handle.
         * @param aCaster Point shadow caster index (0 <= aCaster < MAX_POINT_SHADOW_CASTERS).
         */
        virtual void BeginPointShadowPass ( void* aWindowId, uint32_t aCaster )
        {
            ( void ) aWindowId;
            ( void ) aCaster;
        }
        /** Ends the current point shadow-depth pass and transitions that layer
         * of the point shadow map array so it can be sampled by the shading
         * pass. Must be paired with a preceding BeginPointShadowPass. Virtual
         * with an empty default so a backend without point shadow support
         * ignores it.
         * @param aWindowId Platform dependent window handle.
         */
        virtual void EndPointShadowPass ( void* aWindowId )
        {
            ( void ) aWindowId;
        }
        /** Ends the depth pre-pass mark render pass, dispatches the remaining
         * clustering compute stages (light culling, which now gates on the
         * clusters the mark pass flagged as active), then begins the main color
         * render pass. Only called when BeginRender was given a lighting
         * pipeline; the application records the marking geometry traversal
         * between BeginRender and this call, and the shading traversal after it.
         * @param aWindowId Platform dependent window handle.
         * @param aComputePipeline The same lighting pipeline passed to
         *        BeginRender, whose post-mark stages are dispatched here.
         */
        virtual void EndDepthPrePass ( void* aWindowId, const Pipeline* aComputePipeline ) = 0;
        /** Ends the current render pass for the given window surface.
         * @param aWindowId Platform dependent window handle.
         */
        virtual void EndRender ( void* aWindowId ) = 0;
        /** Blocks until all GPU work previously submitted for the given window
         * surface has completed. Provided so callers can safely read back
         * GPU-written buffers, capture the framebuffer, or tear resources down
         * while frames are pipelined: with multiple frames in flight BeginFrame
         * no longer implies the previous frame finished. Backends implement it
         * with a full GPU drain (vkDeviceWaitIdle / glFinish).
         * @param aWindowId Platform dependent window handle.
         */
        virtual void Finish ( void* aWindowId ) = 0;
        /** Issues a draw call for a mesh with the given pipeline and optional material.
         * @param aWindowId Platform dependent window handle.
         * @param aModelMatrix Model transformation matrix.
         * @param aMesh Mesh to render.
         * @param aPipeline Pipeline (shaders/state) to use.
         * @param aMaterial Optional material to bind.
         * @param aTopology Primitive topology (default: TRIANGLE_LIST).
         * @param aVertexStart First vertex index.
         * @param aVertexCount Number of vertices to draw (default: all).
         * @param aInstanceCount Number of instances to draw.
         * @param aFirstInstance Index of the first instance.
         * @param aSkinnedVertices Optional pre-skinned vertex buffer produced by
         *        the compute skinning pre-pass; when set it is bound as the
         *        vertex input in place of the mesh's rest-pose vertices.
         * @param aRenderPass Pass this draw feeds; DepthPrePass substitutes the
         *        renderer's cluster-mark pipeline, Shading uses aPipeline.
         */
        virtual void Render ( void* aWindowId,
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
                              RenderPass aRenderPass = RenderPass::Shading ) const = 0;
        /** Issues a single instanced draw for a batch of identical-geometry
         * nodes, each positioned by its own model matrix.
         *
         * The default implementation simply loops Render once per matrix, which
         * is always correct; backends that support a per-instance model-matrix
         * buffer (selected by the shader's INSTANCED variant) override this to
         * collapse the batch into one draw call. Only non-skinned geometry is
         * ever batched, so no skinned vertex buffer is involved.
         * @param aWindowId Platform dependent window handle.
         * @param aModelMatrices Contiguous per-instance model matrices.
         * @param aMesh Mesh shared by every instance.
         * @param aPipeline Pipeline shared by every instance.
         * @param aMaterial Optional material shared by every instance.
         * @param aTopology Primitive topology (default: TRIANGLE_LIST).
         * @param aVertexStart First vertex index.
         * @param aVertexCount Number of vertices to draw (default: all).
         * @param aRenderPass Pass this draw feeds; DepthPrePass substitutes the
         *        renderer's cluster-mark pipeline, Shading uses aPipeline.
         */
        virtual void RenderInstanced ( void* aWindowId,
                                       std::span<const Matrix4x4> aModelMatrices,
                                       const Mesh& aMesh,
                                       const Pipeline& aPipeline,
                                       const Material* aMaterial = nullptr,
                                       Topology aTopology = Topology::TRIANGLE_LIST,
                                       uint32_t aVertexStart = 0,
                                       uint32_t aVertexCount = 0xffffffff,
                                       RenderPass aRenderPass = RenderPass::Shading )
        {
            for ( const Matrix4x4& model_matrix : aModelMatrices )
            {
                Render ( aWindowId, model_matrix, aMesh, aPipeline, aMaterial, aTopology, aVertexStart, aVertexCount, 1, 0, nullptr, aRenderPass );
            }
        }
        /** Dispatches the compute stage of a pipeline.
         * Group counts are measured in workgroups, not invocations. For
         * backends with an explicit render pass (Vulkan), this must be recorded
         * between BeginFrame and BeginRenderPass.
         * @param aWindowId Platform dependent window handle.
         * @param aPipeline Pipeline whose compute stage to dispatch.
         * @param aGroupCountX Number of workgroups in X.
         * @param aGroupCountY Number of workgroups in Y.
         * @param aGroupCountZ Number of workgroups in Z.
         * @param aStorageBuffers Storage buffers (SSBOs) to bind for this
         *        dispatch, each mapped to a shader storage block by the CRC32
         *        of its GLSL block name. Blocks the pipeline does not declare
         *        are silently ignored.
         */
        virtual void Dispatch ( void* aWindowId,
                                const Pipeline& aPipeline,
                                uint32_t aGroupCountX,
                                uint32_t aGroupCountY = 1,
                                uint32_t aGroupCountZ = 1,
                                std::span<const StorageBufferBinding> aStorageBuffers = {},
                                uint32_t aComputeStageIndex = 0 ) const = 0;
        /** Dispatches the compute skinning pre-pass for a single skinned mesh,
         * transforming the mesh's rest-pose vertices by the supplied per-joint
         * skinning matrices into an output vertex buffer. Like Dispatch, for
         * backends with an explicit render pass (Vulkan) this must be recorded
         * between BeginFrame and BeginRenderPass.
         * @param aWindowId Platform dependent window handle.
         * @param aSkinningPipeline Compute pipeline implementing the skinning kernel.
         * @param aMesh Source mesh whose vertex buffer is bound as SourceVertices.
         * @param aSkinningMatrices SSBO of per-joint pose*inverse-bind matrices.
         * @param aSkinnedVertices Output SSBO receiving the skinned vertices,
         *        sized vertexCount * mesh stride.
         */
        virtual void Skin ( void* aWindowId,
                            const Pipeline& aSkinningPipeline,
                            const Mesh& aMesh,
                            const BufferAccessor& aSkinningMatrices,
                            const BufferAccessor& aSkinnedVertices ) const = 0;
        /** Inserts a memory barrier ensuring shader storage-buffer (SSBO)
         * writes from a preceding Dispatch are visible to subsequent shader
         * reads (compute or graphics).
         * @param aWindowId Platform dependent window handle.
         */
        virtual void Barrier ( void* aWindowId ) const = 0;
        /** Returns the view frustum for the given window surface.
         * @param aWindowId Platform dependent window handle.
         * @return A const reference to the current view frustum.
         */
        virtual const Frustum& GetFrustum ( void* aWindowId ) const = 0;
        /** Returns the projection matrix currently uploaded for the given window
         * surface. Intended for debug visualization (e.g. drawing camera
         * frustums by inverting it).
         * @param aWindowId Platform dependent window handle.
         * @return A const reference to the current projection matrix.
         */
        virtual const Matrix4x4& GetProjectionMatrix ( void* aWindowId ) const = 0;
        /** Returns the current frame's per-cluster light grid SSBO for the given
         * window surface. Intended for test/debug introspection of clustering.
         * @param aWindowId Platform dependent window handle.
         * @return Pointer to the light-grid BufferAccessor, or nullptr if the
         *         window is unknown.
         */
        virtual const BufferAccessor* GetFrameLightGrid ( void* aWindowId ) const = 0;
        /** Returns the current frame's per-cluster active-flag SSBO produced by
         * the depth pre-pass mark stage. Intended for test/debug introspection.
         * @param aWindowId Platform dependent window handle.
         * @return Pointer to the cluster-active BufferAccessor, or nullptr if the
         *         window is unknown.
         */
        virtual const BufferAccessor* GetFrameClusterActive ( void* aWindowId ) const = 0;
        /** Allocates uniform buffer memory that is valid for a single frame.
         * @param aWindowId Platform dependent window handle.
         * @param aSize Size in bytes of the requested allocation.
         * @return A BufferAccessor to the allocated memory.
         */
        virtual BufferAccessor AllocateSingleFrameUniformMemory ( void* aWindowId, size_t aSize ) = 0;
        /** Allocates storage (SSBO) buffer memory that is valid for a single frame.
         * @param aWindowId Platform dependent window handle.
         * @param aSize Size in bytes of the requested allocation.
         * @return A BufferAccessor to the allocated memory.
         */
        virtual BufferAccessor AllocateSingleFrameStorageMemory ( void* aWindowId, size_t aSize ) = 0;
        ///@}

        ///@name Overlay rendering
        ///@{
        /** Composites a GUI overlay on top of the current frame.
         * Renders a full-screen quad textured with the overlay's RGBA pixel buffer,
         * using alpha blending. Must be called between BeginRender and EndRender.
         * @param aWindowId Platform dependent window handle.
         * @param aGuiOverlay The GUI overlay whose pixel buffer to composite.
         */
        virtual void RenderOverlay ( void* aWindowId, const GuiOverlay& aGuiOverlay ) = 0;
        ///@}

        ///@name Scene rendering
        ///@{
        /** Renders an entire scene for a window in a single call.
         * Owns the per-frame render protocol as a fixed sequence of steps:
         * brackets the frame with BeginRender/EndRender, builds the scene's
         * render queue from the window frustum, runs the depth pre-pass and
         * light culling when the scene defines a lighting pipeline, submits the
         * shading pass, runs the optional debug-geometry step, and composites
         * the optional GUI overlay. The sequence lives here (not in the
         * backends) so it is defined once; backends only implement the
         * individual step primitives. Callers remain responsible for the
         * per-frame setup that precedes BeginRender (BeginFrame,
         * view/projection matrices, lights and skinning).
         *
         * The debug-geometry step is gated by a bool flag toggled by
         * SetDebugRendering: when disabled the step is skipped entirely; when
         * enabled SubmitDebugGeometry is invoked. The step is placed after the
         * shading submit and before the overlay so debug wireframes share the
         * scene depth buffer and stay underneath the GUI.
         * @param aWindowId Platform dependent window handle.
         * @param aScene The scene to render.
         * @param aGuiOverlay Optional GUI overlay to composite on top of the frame.
         */
        DLL void RenderScene ( void* aWindowId, const Scene& aScene, const GuiOverlay* aGuiOverlay = nullptr );
        /** Enables or disables the debug-geometry render step for RenderScene.
         * Debug geometry is only drawn by backends that override
         * SubmitDebugGeometry.
         * @param aEnabled True to draw debug geometry, false for the production path.
         */
        DLL void SetDebugRendering ( bool aEnabled );
        /** @return True when the debug-geometry render step is currently enabled. */
        DLL bool GetDebugRendering() const;
        /** Replaces the debug-geometry parameters (grid spacing/colors, which
         * features to draw). Takes effect on the next RenderScene.
         * @param aSettings New debug render settings.
         */
        DLL void SetDebugRenderSettings ( const DebugRenderSettings& aSettings );
        /** @return The current debug render settings. */
        DLL const DebugRenderSettings& GetDebugRenderSettings() const;
        /** Enables or disables a whole light type for shading. Disabled types
         * are filtered out of the per-frame light set in SetLights, so the
         * change takes effect on the next frame. Intended as a debugging aid
         * (e.g. isolate the directional light to inspect its shadow).
         * @param aType Light type to toggle.
         * @param aEnabled True to include lights of this type, false to drop them.
         */
        DLL void SetLightTypeEnabled ( LightType aType, bool aEnabled );
        /** @return True when lights of @p aType are currently included. */
        DLL bool GetLightTypeEnabled ( LightType aType ) const;
        /** Flips the enabled state of a whole light type. */
        DLL void ToggleLightType ( LightType aType );
        /** @return The renderer's registered name ("OpenGL"/"Vulkan"), used to
         *  resolve per-renderer pipeline shader variants. */
        virtual std::string_view GetName() const = 0;
        /** @return True when the rendering device has been lost and not yet
         *  recovered. Backends that can lose their device (e.g. Vulkan on a GPU
         *  reset/TDR) override this; while it returns true RenderScene skips the
         *  whole frame so no work is recorded against dead GPU handles. The
         *  default implementation always returns false.
         *
         *  Defined inline on purpose: Renderer has no other non-pure virtual, so
         *  an out-of-line definition here would become the vtable key function
         *  and move the (currently weak, emitted-per-TU) Renderer vtable into a
         *  single object file, breaking the separately-linked renderer DLLs. */
        virtual bool IsDeviceLost() const
        {
            return false;
        }
        ///@}
    protected:
        /** Returns @p aLights filtered to only the currently enabled light
         * types. When every type is enabled the input span is returned
         * unchanged; otherwise the kept lights are copied into a reused member
         * buffer and a span over it is returned (valid until the next call).
         * Backends call this in their SetLights before uploading.
         * @param aLights The frame's full light set.
         */
        DLL std::span<const GpuLight> FilterLightsByType ( std::span<const GpuLight> aLights ) const;
        /** Submits the scene's render queue for one pass, issuing one draw per
         * batch (instanced when a batch holds more than one item). Backends
         * resolve the target window once and walk Scene::ForEachRenderBatch.
         * @param aWindowId Platform dependent window handle.
         * @param aScene Scene whose previously-built render queue to submit.
         * @param aRenderPass Pass to submit (DepthPrePass or Shading).
         */
        virtual void SubmitRenderQueue ( void* aWindowId, const Scene& aScene, RenderPass aRenderPass ) = 0;
        /** @return True when @p aWindowId names a surface known to this renderer. */
        virtual bool IsValidWindow ( void* aWindowId ) const = 0;
    private:
        /** Draws debug geometry (an analytic infinite ground grid, per-node
         * world-space AABB wireframes and the scene octree cell wireframes) on
         * top of the shaded scene, sharing its depth buffer. Implemented in
         * terms of the public Render API and the Scene spatial queries, so it
         * is backend-agnostic and lives here rather than in each backend. The
         * built-in pipelines, meshes and materials are lazily loaded on first
         * use. Which features draw and how the grid looks are controlled by
         * SetDebugRenderSettings. Only invoked when debug rendering is enabled.
         * @param aWindowId Platform dependent window handle.
         * @param aScene Scene whose debug geometry to draw.
         */
        void SubmitDebugGeometry ( void* aWindowId, const Scene& aScene );
        /** Lazily loads the built-in debug assets (idempotent). */
        void EnsureDebugAssets();
        /** When true, RenderScene runs the debug-geometry step after shading.
         * Toggled by SetDebugRendering. */
        bool mDebugRendering{false};
        /** True once the built-in debug assets have been loaded. */
        bool mDebugAssetsLoaded{false};
        /** True when mDebugSettings changed and must be pushed to the grid material. */
        bool mDebugSettingsDirty{true};
        /** Tunable debug-geometry parameters. */
        DebugRenderSettings mDebugSettings{};
        /** Bitmask of enabled light types (bit = 1u << LightType). All types
         * enabled by default; cleared bits drop that type in FilterLightsByType. */
        uint32_t mLightTypeMask{ ( 1u << static_cast<uint32_t> ( LightType::Point ) ) |
            ( 1u << static_cast<uint32_t> ( LightType::Spot ) ) |
            ( 1u << static_cast<uint32_t> ( LightType::Directional ) ) };
        /** Scratch buffer holding the type-filtered light subset, reused across
         * frames so FilterLightsByType allocates only when the set grows. */
        mutable std::vector<GpuLight> mFilteredLights{};
        /** Solid-color line pipeline for debug wireframes. */
        std::unique_ptr<Pipeline> mDebugPipeline{};
        /** Unit (radii 1) wireframe cube; scaled per draw to an AABB. */
        std::unique_ptr<Mesh> mDebugWireMesh{};
        /** Solid-color material for per-node AABB wireframes. */
        std::unique_ptr<Material> mDebugAABBMaterial{};
        /** Solid-color material for octree cell wireframes. */
        std::unique_ptr<Material> mDebugOctreeMaterial{};
        /** Solid-color material for camera frustum wireframes. */
        std::unique_ptr<Material> mDebugFrustumMaterial{};        /** Analytic infinite ground-grid pipeline. */
        std::unique_ptr<Pipeline> mDebugGridPipeline{};
        /** Full-screen triangle the grid pipeline unprojects to the ground plane. */
        std::unique_ptr<Mesh> mDebugGridMesh{};
        /** Grid appearance material (colors, spacing, fade). */
        std::unique_ptr<Material> mDebugGridMaterial{};
        /** Per-window, per-caster point shadow cache. A caster's six-face cube
         *  map is only re-rendered when its light or some shadow-casting
         *  geometry actually changed; otherwise the previous frame's map is
         *  reused (the depth image persists). An entry records the inputs the
         *  last render depended on: the caster's light world position+radius and
         *  the scene's shadow-geometry signature. @c rendered guards the first
         *  frame and any time the map was invalidated. */
        struct PointShadowCacheEntry
        {
            Vector4 light_position_radius{};
            uint64_t geometry_signature{0};
            bool rendered{false};
        };
        std::unordered_map<void*, std::array<PointShadowCacheEntry, MAX_POINT_SHADOW_CASTERS>> mPointShadowCache{};
    };
    /**@name Factory Functions */
    /*@{*/
    /** Constructs a Renderer identified by a numeric identifier.
     * @param aIdentifier Numeric renderer identifier.
     * @param aWindow Platform dependent window handle.
     * @return A unique_ptr to the newly created Renderer.
     */
    DLL std::unique_ptr<Renderer> ConstructRenderer ( uint32_t aIdentifier, void* aWindow );
    /** Constructs a Renderer identified by a string name.
     * @param aIdentifier String renderer identifier.
     * @param aWindow Platform dependent window handle.
     * @return A unique_ptr to the newly created Renderer.
     */
    DLL std::unique_ptr<Renderer> ConstructRenderer ( const std::string& aIdentifier, void* aWindow );
    /** Constructs a Renderer identified by a StringId.
     * @param aIdentifier StringId renderer identifier.
     * @param aWindow Platform dependent window handle.
     * @return A unique_ptr to the newly created Renderer.
     */
    DLL std::unique_ptr<Renderer> ConstructRenderer ( const StringId& aIdentifier, void* aWindow );
    /** Registers a Renderer loader for a specific identifier.*/
    DLL bool RegisterRendererConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<Renderer> ( void* ) >& aConstructor );
    /** Unregisters a Renderer loader for a specific identifier.*/
    DLL bool UnregisterRendererConstructor ( const StringId& aIdentifier );
    /** Enumerates Renderer loader identifiers via an enumerator functor.*/
    DLL void EnumerateRendererConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /** Returns the names of all registered Renderer constructors.
     * @return A vector of registered renderer constructor name strings.
     */
    DLL std::vector<std::string> GetRendererConstructorNames();
    /*@}*/
}
#endif
