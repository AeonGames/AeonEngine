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
#include "aeongames/ResourceId.hpp"
#include "aeongames/Octree.hpp"
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace AeonGames
{
    class SceneMsg;
    class Node;
    class Renderer;
    class InputSystem;
    class Pipeline;
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
        FrameLightContainer mFrameLights{};
        ResourceId mLightingPipeline{};
    };
}
#endif
