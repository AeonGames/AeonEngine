/*
Copyright 2014-2016 Rodrigo Jose Hernandez Cordoba

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
    \copy 2014-2016
    \author Rodrigo Hernandez.
*/
#include "aeongames/Platform.h"
#include <vector>
#include <string>
#include <functional>

namespace AeonGames
{
    class Node;
    /*! \brief Scene class.
      Scene is the container for all elements in a game level,
      takes care of collision, rendering and updates to all elements therein.
    */
    class Scene
    {
    public:
        DLL explicit Scene(); // "Explicit Scene"... chuckle
        DLL ~Scene();
        DLL void SetName ( const char* aName );
        DLL const char* const GetName() const;
        DLL bool AddNode ( Node* aNode );
        DLL bool InsertNode ( size_t aIndex, Node* aNode );
        DLL bool RemoveNode ( Node* aNode );
        DLL size_t GetNodeCount() const;
        DLL size_t GetChildrenCount() const;
        DLL Node* GetChild ( size_t aIndex ) const;
        DLL void Update ( double delta );
        /** @copydoc Node::LoopTraverseDFSPreOrder(std::function<void(Node*) > aAction)*/
        DLL void LoopTraverseDFSPreOrder ( std::function<void ( Node* ) > aAction );
        /** @copydoc Node::LoopTraverseDFSPreOrder(
            std::function<void(Node*) > aPreamble,
            std::function<void(Node*) > aPostamble)*/
        DLL void LoopTraverseDFSPreOrder (
            std::function<void ( Node* ) > aPreamble,
            std::function<void ( Node* ) > aPostamble );
        /** @copydoc Node::LoopTraverseDFSPreOrder(std::function<void(const Node*) > aAction) const */
        DLL void LoopTraverseDFSPreOrder ( std::function<void ( const Node* ) > aAction ) const;
        /** @copydoc Node::LoopTraverseDFSPostOrder(std::function<void(Node*) > aAction)*/
        DLL void LoopTraverseDFSPostOrder ( std::function<void ( Node* ) > aAction );
        /** @copydoc Node::LoopTraverseDFSPostOrder(std::function<void(const Node*) > aAction) const */
        DLL void LoopTraverseDFSPostOrder ( std::function<void ( const Node* ) > aAction ) const;
        /** @copydoc Node::RecursiveTraverseDFSPreOrder(std::function<void(Node*) > aAction)*/
        DLL void RecursiveTraverseDFSPreOrder ( std::function<void ( Node* ) > aAction );
        /** @copydoc Node::RecursiveTraverseDFSPostOrder(std::function<void(Node*) > aAction)*/
        DLL void RecursiveTraverseDFSPostOrder ( std::function<void ( Node* ) > aAction );

        // Deleted Methods (avoid copy and copy construction)
        Scene& operator= ( const Scene& ) = delete;
        Scene ( const Scene& ) = delete;
    private:
        friend class Node;
        std::string mName;
        std::vector<Node*> mRootNodes;
        std::vector<Node*> mAllNodes;
    };
}
#endif
