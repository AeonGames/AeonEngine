/*
Copyright (C) 2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OCTREE_H
#define AEONGAMES_OCTREE_H
#include "aeongames/Platform.hpp"
#include <cstdint>
#include <vector>
#include <tuple>
namespace AeonGames
{
    class Node;
    /** @brief Spatial partitioning data structure that organizes nodes in 3D space. */
    class Octree
    {
    public:
        DLL Octree();
        DLL ~Octree();
        /** @brief Insert a node into the octree.
         *  @param aNode Pointer to the node to add. */
        DLL void AddNode ( Node* aNode );
        /** @brief Remove a node from the octree.
         *  @param aNode Pointer to the node to remove. */
        DLL void RemoveNode ( Node* aNode );
    private:
        size_t mSize{0};
        std::vector<std::tuple<uint64_t, std::vector<Node* >>> mNodes{};
    };
}
#endif
