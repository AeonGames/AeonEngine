/*
Copyright (C) 2014-2019,2021 Rodrigo Jose Hernandez Cordoba

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
    \copy 2014-2019
    \author Rodrigo Hernandez.
*/
#include "aeongames/Platform.h"
#include "aeongames/Matrix4x4.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace AeonGames
{
    class SceneMsg;
    class Node;
    class Renderer;
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
        DLL explicit Scene(); // "Explicit Scene"... chuckle
        DLL ~Scene();
        DLL void Load ( uint32_t aId );
        DLL void Load ( const std::string& aFilename );
        DLL void Load ( const void* aBuffer, size_t aBufferSize );
        DLL void SetName ( const char* aName );
        DLL const char* const GetName() const;
        DLL Node* Add ( std::unique_ptr<Node> aNode );
        DLL Node* Insert ( size_t aIndex, std::unique_ptr<Node> aNode );
        DLL std::unique_ptr<Node> Remove ( Node* aNode );
        DLL std::unique_ptr<Node> RemoveByIndex ( size_t aIndex );
        DLL size_t GetChildrenCount() const;
        DLL Node* GetChild ( size_t aIndex ) const;
        DLL size_t GetChildIndex ( const Node* aNode ) const;
        DLL const Node& operator[] ( const std::size_t index ) const;
        DLL Node& operator[] ( const std::size_t index );
        DLL void Update ( const double delta );
        DLL void BroadcastMessage ( uint32_t aMessageType, const void* aMessageData );
        /** @copydoc Node::LoopTraverseDFSPreOrder(std::function<void(Node&) > aAction)*/
        DLL void LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction );
        /** @copydoc Node::LoopTraverseDFSPreOrder(
            std::function<void(Node*) > aPreamble,
            std::function<void(Node*) > aPostamble)*/
        DLL void LoopTraverseDFSPreOrder (
            const std::function<void ( Node& ) >& aPreamble,
            const std::function<void ( Node& ) >& aPostamble );
        /** @copydoc Node::LoopTraverseDFSPreOrder(std::function<void(const std::shared_ptr<const Node>&) > aAction) const */
        DLL void LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const;
        /** @copydoc Node::LoopTraverseDFSPostOrder(std::function<void(const Node*&) > aAction)*/
        DLL void LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction );
        /** @copydoc Node::LoopTraverseDFSPostOrder(std::function<void(const std::shared_ptr<const Node>&) > aAction) const */
        DLL void LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const;
        /** @copydoc Node::RecursiveTraverseDFSPreOrder(std::function<void(const Node*&) > aAction)*/
        DLL void RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction );
        /** @copydoc Node::RecursiveTraverseDFSPostOrder(std::function<void(const Node*&) > aAction)*/
        DLL void RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction );
        DLL Node* Find ( const std::function<bool ( const Node& ) >& aUnaryPredicate ) const;
        DLL std::string Serialize ( bool aAsBinary = true ) const;
        DLL void Deserialize ( const std::string& aSerializedScene );
        /** @name Camera Data */
        /**@{*/
        /** Set rendering camera
         * @param aNode pointer to the node for which camera location and orientation will be extracted.
         * @todo Need a way to uniquely identify nodes for messaging and camera assignment. */
        DLL void SetCamera ( Node* aNode );
        DLL void SetCamera ( uint32_t aNodeId );
        DLL void SetCamera ( const std::string& aNodeName );
        DLL const Node* GetCamera() const;
        DLL void SetViewMatrix ( const Matrix4x4& aMatrix );
        DLL const Matrix4x4& GetViewMatrix() const;
        /**@}*/
        DLL void SetFieldOfView ( float aFieldOfView );
        DLL void SetNear ( float aNear );
        DLL void SetFar ( float aFar );
        DLL float GetFieldOfView() const;
        DLL float GetNear() const;
        DLL float GetFar() const;
    private:
        friend class Node;
        Matrix4x4 mViewMatrix{};
        float mFieldOfView{60.0f};
        float mNear{1.0f};
        float mFar{1600.0f};
        std::string mName{};
        /// Children Nodes
        std::vector<std::unique_ptr<Node>> mNodes{};
#if 0
        /** Local Node Storage
         * This is a storage space for nodes
         * owned by the scene, such as deserialized
         * nodes as well as any nodes requested from
         * the scene, or moved to the scene.
         * It does not necesarily contains a pointer to all
         * nodes in the tree, nor does a pointer existing
         * here means it exists as part of the tree. */
        std::vector<std::unique_ptr<Node>> mNodeStorage{};
#endif
        Node* mCamera {};
    };
}
#endif
