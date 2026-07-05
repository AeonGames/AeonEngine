/*
Copyright (C) 2014-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_SCENE_H
#define AEONGAMES_SCENE_H
/*! \file
    \brief Header for the Scene class.
    \copyright 2014-2019
    \author Rodrigo Hernandez.
*/
#include "aeongames/Platform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/FrameLightContainer.hpp"
#include "aeongames/GpuShadowParams.hpp"
#include "aeongames/GpuGlobals.hpp"
#include "aeongames/ResourceId.hpp"
#include "aeongames/RenderItem.hpp"
#include "aeongames/Octree.hpp"
#include <memory>
#include <vector>
#include <span>
#include <string>
#include <functional>

namespace AeonGames
{
    class SceneMsg;
    class Node;
    class Renderer;
    class InputSystem;
    class Pipeline;
    class Texture;
    class Frustum;
    /*! \brief Scene class.
      Scene is the container for all elements in a game level,
      takes care of collision, rendering and updates to all elements therein.
    */
    class Scene
    {
    public:
        // Deleted Methods (avoid copy and copy construction)
        Scene& operator= ( const Scene& ) = delete;
        Scene ( const Scene& ) = delete;
        /** Construct an empty scene. */
        DLL explicit Scene(); // "Explicit Scene"... chuckle
        /** Destructor. */
        DLL ~Scene();
        /** Load a scene from a resource identified by id.
            @param aId Resource identifier. */
        DLL void Load ( uint32_t aId );
        /** Load a scene from a file.
            @param aFilename Path to the scene file. */
        DLL void Load ( const std::string& aFilename );
        /** Load a scene from a memory buffer.
            @param aBuffer Pointer to the data buffer.
            @param aBufferSize Size of the buffer in bytes. */
        DLL void Load ( const void* aBuffer, size_t aBufferSize );
        /** Set the scene name.
            @param aName Null-terminated name string. */
        DLL void SetName ( const char* aName );
        /** Get the scene name.
            @return Pointer to the null-terminated name string. */
        DLL const char* const GetName() const;
        /** Add a top-level node to the scene.
            @param aNode Unique pointer to the node.
            @return Raw pointer to the added node. */
        DLL Node* Add ( std::unique_ptr<Node> aNode );
        /** Insert a top-level node at a specific index.
            @param aIndex Position at which to insert.
            @param aNode Unique pointer to the node.
            @return Raw pointer to the inserted node. */
        DLL Node* Insert ( size_t aIndex, std::unique_ptr<Node> aNode );
        /** Remove a top-level node by pointer.
            @param aNode Pointer to the node to remove.
            @return Unique pointer to the removed node, or nullptr if not found. */
        DLL std::unique_ptr<Node> Remove ( Node* aNode );
        /** Remove a top-level node by index.
            @param aIndex Index of the node to remove.
            @return Unique pointer to the removed node. */
        DLL std::unique_ptr<Node> RemoveByIndex ( size_t aIndex );
        /** Get the number of top-level child nodes.
            @return Child count. */
        DLL size_t GetChildrenCount() const;
        /** Get a top-level child node by index.
            @param aIndex Index of the child.
            @return Pointer to the child node. */
        DLL Node* GetChild ( size_t aIndex ) const;
        /** Get the index of a top-level child node.
            @param aNode Pointer to the node to look up.
            @return Index of the node within the top-level children. */
        DLL size_t GetChildIndex ( const Node* aNode ) const;
        /** Access a top-level child node by index (const).
            @param index Index of the child.
            @return Const reference to the child node. */
        DLL const Node& operator[] ( const std::size_t index ) const;
        /** Access a top-level child node by index.
            @param index Index of the child.
            @return Reference to the child node. */
        DLL Node& operator[] ( const std::size_t index );
        /** Update all nodes in the scene.
            @param delta Elapsed time in seconds since the last update. */
        DLL void Update ( const double delta );
        /** Broadcast a message to all nodes in the scene.
            @param aMessageType Type identifier for the message.
            @param aMessageData Pointer to message-specific data. */
        DLL void BroadcastMessage ( uint32_t aMessageType, const void* aMessageData );
        /** Iterative depth-first pre-order traversal of all nodes in the scene.
            @param aAction Callable invoked for each node. */
        DLL void LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction );
        /** Iterative depth-first pre-order traversal with separate preamble and postamble actions.
            @param aPreamble Callable invoked upon entering each node.
            @param aPostamble Callable invoked upon leaving each node. */
        DLL void LoopTraverseDFSPreOrder (
            const std::function<void ( Node& ) >& aPreamble,
            const std::function<void ( Node& ) >& aPostamble );
        /** Iterative depth-first pre-order traversal (const version).
            @param aAction Callable invoked for each node. */
        DLL void LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const;
        /** Iterative depth-first post-order traversal of all nodes in the scene.
            @param aAction Callable invoked for each node. */
        DLL void LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction );
        /** Iterative depth-first post-order traversal (const version).
            @param aAction Callable invoked for each node. */
        DLL void LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const;
        /** Recursive depth-first pre-order traversal of all nodes in the scene.
            @param aAction Callable invoked for each node. */
        DLL void RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction );
        /** Recursive depth-first post-order traversal of all nodes in the scene.
            @param aAction Callable invoked for each node. */
        DLL void RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction );
        /** Find the first node matching a predicate via depth-first search.
            @param aUnaryPredicate Callable returning true for the desired node.
            @return Pointer to the matching node, or nullptr if none found. */
        DLL Node* Find ( const std::function<bool ( const Node& ) >& aUnaryPredicate ) const;
        /** Serialize the scene to a string.
            @param aAsBinary If true, serialize as binary protobuf; otherwise as text.
            @return The serialized scene data. */
        DLL std::string Serialize ( bool aAsBinary = true ) const;
        /** Deserialize a scene from a string.
            @param aSerializedScene The serialized scene data. */
        DLL void Deserialize ( const std::string& aSerializedScene );
        /** @name Camera Data */
        /**@{*/
        /** Set rendering camera
         * @param aNode pointer to the node for which camera location and orientation will be extracted.
         * @todo Need a way to uniquely identify nodes for messaging and camera assignment. */
        DLL void SetCamera ( Node* aNode );
        /** Set rendering camera by node id.
            @param aNodeId Identifier of the camera node. */
        DLL void SetCamera ( uint32_t aNodeId );
        /** Set rendering camera by node name.
            @param aNodeName Name of the camera node. */
        DLL void SetCamera ( const std::string& aNodeName );
        /** Get the current camera node.
            @return Pointer to the camera node, or nullptr if unset. */
        DLL const Node* GetCamera() const;
        /** Set the view matrix directly.
            @param aMatrix The new view matrix. */
        DLL void SetViewMatrix ( const Matrix4x4& aMatrix );
        /** Get the current view matrix.
            @return Const reference to the view matrix. */
        DLL const Matrix4x4& GetViewMatrix() const;
        /**@}*/
        /** Set the vertical field of view.
            @param aFieldOfView Field of view in degrees. */
        DLL void SetFieldOfView ( float aFieldOfView );
        /** Set the near clipping plane distance.
            @param aNear Near plane distance. */
        DLL void SetNear ( float aNear );
        /** Set the far clipping plane distance.
            @param aFar Far plane distance. */
        DLL void SetFar ( float aFar );
        /** Get the vertical field of view.
            @return Field of view in degrees. */
        DLL float GetFieldOfView() const;
        /** Get the near clipping plane distance.
            @return Near plane distance. */
        DLL float GetNear() const;
        /** Get the far clipping plane distance.
            @return Far plane distance. */
        DLL float GetFar() const;
        /** @name Ambient lighting */
        /**@{*/
        /** Set the scene's ambient light: a flat fill added to every lit
            surface (the engine has no global illumination).
            @param aAmbient xyz = ambient color, w = intensity multiplier. */
        DLL void SetAmbient ( const Vector4& aAmbient );
        /** Get the scene's ambient light.
            @return xyz = ambient color, w = intensity multiplier. */
        DLL const Vector4& GetAmbient() const;
        /** Build the per-frame scene-wide shading globals (ambient) to upload
            to the renderer via Renderer::SetGlobals.
            @return The GpuGlobals mirroring this scene's ambient. */
        DLL GpuGlobals GetGlobals() const;
        /**@}*/
        /** @name Input */
        /**@{*/
        /** Set the InputSystem associated with this scene.
         *  The scene does not take ownership; the caller must guarantee
         *  the pointer outlives the scene's usage (or reset it to nullptr).
         *  Components can query the InputSystem during Update via
         *  GetInputSystem() to drive gameplay from raw input. */
        DLL void SetInputSystem ( InputSystem* aInputSystem );
        /** Get the InputSystem associated with this scene, or nullptr. */
        DLL InputSystem* GetInputSystem() const;
        /**@}*/

        /** @name Per-frame light list */
        /**@{*/
        /** @brief Append a light to the scene's per-frame list.
         *  Intended to be called from light components' Update(). The
         *  list is reset at the start of Scene::Update so each frame
         *  starts empty. Calls past MAX_LIGHTS_PER_FRAME are dropped. */
        DLL void AddLight ( const GpuLight& aLight );
        /** @brief Read-only view of the lights submitted this frame. */
        DLL std::span<const GpuLight> GetFrameLights() const;
        /**@}*/

        /** @name Per-frame lighting/clustering compute pipeline */
        /**@{*/
        /** @brief Set the compute pipeline dispatched once per frame (e.g. light
         *  clustering) by resource id. Pass an empty id to disable. */
        DLL void SetLightingPipeline ( const ResourceId& aResourceId );
        /** @brief Get the per-frame compute pipeline, loading it from the cache
         *  if set, or nullptr when none is configured. */
        DLL const Pipeline* GetLightingPipeline() const;
        /** @brief Set the scene environment map (an equirectangular HDR) by
         *  resource id. Drawn as a skybox and, later, the source for image-based
         *  lighting. Pass an empty id to disable. */
        DLL void SetEnvironmentMap ( const ResourceId& aResourceId );
        /** @brief Get the environment map texture, loading it from the cache if
         *  set, or nullptr when none is configured. */
        DLL const Texture* GetEnvironmentMap() const;
        /** @brief Compute the directional shadow caster's world-to-light-clip
         *  matrix for the current frame. Selects the first directional light
         *  submitted this frame and fits an orthographic light view-projection
         *  around the camera's view frustum (truncated to the shadow coverage
         *  distance) so the fixed-resolution shadow map is spent on what the
         *  camera can see; the depth range still spans the whole scene so
         *  casters between the sun and the visible region are included. Writes
         *  the matrix and returns true when a caster exists; returns false
         *  (leaving @p aLightViewProjection untouched) when no directional light
         *  was submitted or the scene has no geometry, so the caller can skip
         *  the shadow pass. Must be called after BuildRenderQueue so the spatial
         *  index is current.
         *  @param[out] aLightViewProjection Receives Ortho * LightView in engine
         *  convention (no per-backend depth flip applied).
         *  @param aCameraProjection The camera projection matrix uploaded this
         *  frame; only its aspect ratio is read (invariant to the backend depth
         *  flip), so either backend's matrix works. */
        DLL bool GetDirectionalShadowMatrix ( Matrix4x4& aLightViewProjection,
                                              const Matrix4x4& aCameraProjection ) const;
        /** @brief Select the spot lights that should cast a shadow this frame
         *  and fill their per-caster shadow data. Picks up to
         *  @ref MAX_SPOT_SHADOW_CASTERS spot lights from the frame light list
         *  (in submission order) and, for each, computes a perspective light
         *  view-projection from the light's position, cone direction, outer cone
         *  angle (field of view) and radius (far plane), recording the casting
         *  light's index so the fragment shader can match a shaded spot light to
         *  its shadow map layer. Slots past the returned count are left with a
         *  light index of -1. Must be called after BuildRenderQueue so the
         *  spatial index is current.
         *  @param[out] aSpotShadowParams Receives the per-caster light
         *  view-projections (engine convention, no per-backend depth flip), the
         *  caster light indices and the filtering params (count in params[3]).
         *  @return The number of spot shadow casters selected (0 when none). */
        DLL uint32_t GetSpotShadowCasters ( GpuSpotShadowParams& aSpotShadowParams ) const;
        /** @brief Select the point lights that should cast a shadow this frame
         *  and fill their per-caster shadow data. Picks up to
         *  @ref MAX_POINT_SHADOW_CASTERS point lights from the frame light list
         *  and, for each, builds six 90-degree perspective light view-projections
         *  (one per cube face, axis order +X,-X,+Y,-Y,+Z,-Z) from the light's
         *  position and radius, recording the light's world position and radius
         *  so the fragment shader can match a shaded point light to its caster
         *  and pick the cube face from the light-to-fragment direction. Must be
         *  called after BuildRenderQueue so the spatial index is current.
         *  @param[out] aPointShadowParams Receives the six-per-caster light
         *  view-projections (engine convention, no per-backend depth flip), the
         *  caster positions/radii and the filtering params (count in params[3]).
         *  @return The number of point shadow casters selected (0 when none). */
        DLL uint32_t GetPointShadowCasters ( GpuPointShadowParams& aPointShadowParams ) const;
        /**@}*/

        /** @name Visibility culling */
        /**@{*/
        /** @brief Invoke a callback for every node whose world-space AABB
         *  intersects the given frustum.
         *
         *  Internally backed by a lazily-built octree (rebuilt on the next call
         *  after the scene changes), falling back to a brute-force traversal when
         *  the spatial index is empty. Each visited node still passes an exact
         *  per-node frustum test, so the visited set is identical to a brute-force
         *  cull, only with whole non-visible subtrees skipped.
         *  @param aFrustum Frustum to test node bounds against.
         *  @param aCallback Invoked once per visible node. */
        DLL void CullVisible ( const Frustum& aFrustum, const std::function<void ( const Node& ) >& aCallback ) const;
        /** @brief Build the per-frame render queue from the frustum-visible nodes.
         *
         *  Traverses the scene, frustum-culls each node individually (via
         *  CullVisible), and asks every visible node's components to append the
         *  draws they contribute (Node::Collect / Component::Collect). The
         *  resulting items are sorted so that draws sharing pipeline, material
         *  and mesh become adjacent, ready for ForEachRenderBatch to merge them
         *  into instanced draws. Components never touch the Renderer; they only
         *  declare what to draw here.
         *
         *  Runs once per frame and performs no per-call heap allocation in
         *  steady state: the queue buffer's capacity persists across calls
         *  (clear() keeps the storage) and std::sort is in place. The queue
         *  stays valid until the next BuildRenderQueue call, so a single build
         *  can feed several submit passes (e.g. depth pre-pass and shading).
         *  @param aFrustum Frustum to cull the scene against. */
        DLL void BuildRenderQueue ( const Frustum& aFrustum ) const;
        /** @brief A hash of all shadow-casting geometry's world poses this frame.
         *
         *  Folds the world transform (and size) of every node that has geometry
         *  (a non-degenerate AABB) into one value. Nodes without geometry, such
         *  as the camera or bare light nodes, contribute nothing, so moving the
         *  camera does NOT change the signature. The renderer uses it to skip
         *  re-rendering a shadow map whose casters and light are unchanged: the
         *  signature only differs when some shadow-casting geometry actually
         *  moved, was resized, or was added/removed. Cheap (one traversal, no
         *  allocation); call once per frame. */
        DLL uint64_t GetShadowGeometrySignature() const;
        /** @brief Read-only view of the queue built by the last BuildRenderQueue. */
        DLL const std::vector<RenderItem>& GetRenderQueue() const;
        /** @brief Walk the built render queue grouping consecutive items that can
         *  be drawn as a single instanced batch.
         *
         *  After BuildRenderQueue's sort, a maximal run of non-skinned items
         *  sharing pipeline, material and mesh forms one batch; skinned items
         *  (which carry a per-node posed vertex buffer) and otherwise unique
         *  items each form a length-1 batch. The spans are slices into the
         *  queue and are only valid for the duration of the callback.
         *  @param aCallback Invoked once per batch with the batch's items. */
        DLL void ForEachRenderBatch ( const std::function<void ( std::span<const RenderItem> ) >& aCallback ) const;
        /** @brief Issue the built render queue to a renderer.
         *
         *  Draws every batch from ForEachRenderBatch: length-1 batches via
         *  Renderer::Render (passing the skinned vertex buffer when present),
         *  larger batches via a single Renderer::RenderInstanced. Safe to call
         *  more than once per BuildRenderQueue (e.g. depth pre-pass then shading)
         *  and performs no per-frame heap allocation in steady state.
         *  @param aRenderer Renderer to submit the draws to.
         *  @param aWindowId Platform-specific window identifier.
         *  @param aRenderPass Pass these draws feed; forwarded to the renderer so
         *         it selects the cluster-mark pipeline for DepthPrePass or each
         *         item's own pipeline for Shading. */
        DLL void SubmitRenderQueue ( Renderer& aRenderer, void* aWindowId, RenderPass aRenderPass ) const;
        /** @brief Invoke a callback for every node whose world-space AABB
         *  intersects the given query box.
         *
         *  Shares the lazily-built octree with CullVisible as a broad-phase
         *  accelerator, falling back to a brute-force traversal when the spatial
         *  index is empty. Each visited node still passes an exact AABB overlap
         *  test, so the visited set is identical to a brute-force scan; intended
         *  as the broad phase for collision queries.
         *  @param aBox World-space box to test node bounds against.
         *  @param aCallback Invoked once per overlapping node. */
        DLL void QueryAABB ( const AABB& aBox, const std::function<void ( const Node& ) >& aCallback ) const;
        /** @brief Invoke a callback for every allocated octree cell, passing its
         *  world-space bounds and subdivision depth (root = 0).
         *
         *  Intended for debug visualization of the scene's spatial subdivision
         *  (drawing the octree grid). Shares the lazily-built octree with
         *  CullVisible/QueryAABB; visits nothing when the index is empty. When a
         *  frustum is supplied, whole subtrees outside it are skipped so only
         *  on-screen cells are visited.
         *  @param aCallback Invoked once per cell with its bounds and depth. */
        DLL void ForEachOctreeCell ( const std::function<void ( const AABB&, uint32_t ) >& aCallback ) const;
        /** @brief Frustum-filtered overload of ForEachOctreeCell.
         *  @param aFrustum Only cells intersecting this frustum are visited.
         *  @param aCallback Invoked once per intersecting cell with its bounds and depth. */
        DLL void ForEachOctreeCell ( const Frustum& aFrustum, const std::function<void ( const AABB&, uint32_t ) >& aCallback ) const;
        /** @brief Mark the spatial index stale so it is rebuilt on the next
         *  CullVisible or QueryAABB call. Called automatically when nodes are
         *  added, removed, or moved; expose publicly so external mutations can
         *  request a rebuild. */
        DLL void InvalidateSpatialIndex();
        /**@}*/
    private:
        friend class Node;
        Matrix4x4 mViewMatrix{};
        float mFieldOfView{60.0f};
        float mNear{1.0f};
        float mFar{1600.0f};
        /// Scene ambient light: xyz = color, w = intensity. Default reproduces
        /// the former constant vec3(0.25) flat ambient fill.
        Vector4 mAmbient{1.0f, 1.0f, 1.0f, 0.25f};
        std::string mName{};
        /// Children Nodes
        std::vector<std::unique_ptr<Node >> mNodes{};
#if 0
        /** Local Node Storage
         * This is a storage space for nodes
         * owned by the scene, such as deserialized
         * nodes as well as any nodes requested from
         * the scene, or moved to the scene.
         * It does not necesarily contains a pointer to all
         * nodes in the tree, nor does a pointer existing
         * here means it exists as part of the tree. */
        std::vector<std::unique_ptr<Node >> mNodeStorage{};
#endif
        Node* mCamera {};
        InputSystem* mInputSystem {};
        /// @brief Rebuild the octree from the current node set. Lazy cache helper.
        void BuildSpatialIndex() const;
        /// @brief Octree over node world-space AABBs, used by CullVisible.
        mutable Octree mSpatialIndex{};
        /// @brief True when mSpatialIndex must be rebuilt before the next query.
        mutable bool mSpatialIndexDirty{true};
        /// @brief Per-frame render queue rebuilt by BuildRenderQueue. Its
        /// capacity persists across frames so steady-state collection performs
        /// no heap allocation; mutable so the build can run on a const scene.
        mutable std::vector<RenderItem> mRenderQueue{};
        /// @brief Reused scratch of per-instance transforms gathered while
        /// submitting an instanced batch, so SubmitRenderQueue allocates only
        /// when a batch grows beyond any previously seen size.
        mutable std::vector<Matrix4x4> mInstanceTransforms{};
        FrameLightContainer mFrameLights{};
        ResourceId mLightingPipeline{};
        ResourceId mEnvironmentMap{};
        /// @brief Hash of all shadow-casting geometry's world poses, recomputed
        /// each frame during Update (folded into its existing traversal). Read
        /// by GetShadowGeometrySignature so the renderer can skip re-rendering
        /// unchanged shadow maps without a second scene traversal.
        uint64_t mShadowGeometrySignature{0};
    };
}
#endif
