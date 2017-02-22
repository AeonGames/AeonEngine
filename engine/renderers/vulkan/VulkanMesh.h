/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANMESH_H
#define AEONGAMES_VULKANMESH_H

#include <exception>
#include <string>
#include "aeongames/Memory.h"

namespace AeonGames
{
    class Mesh;
    class VulkanMesh
    {
    public:
        VulkanMesh ( const std::shared_ptr<Mesh> aMesh );
        ~VulkanMesh();
        void Render() const;
    private:
        void Initialize();
        void Finalize();
        const std::shared_ptr<Mesh> mMesh;
        struct Buffers
        {
            uint32_t mArray = 0;
            uint32_t mVertexBuffer = 0;
            uint32_t mIndexBuffer = 0;
        };
        std::vector<Buffers> mBuffers;
    };
}
#endif
