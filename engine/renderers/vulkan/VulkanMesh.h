/*
Copyright (C) 2017,2018,2021 Rodrigo Jose Hernandez Cordoba

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

#include <vulkan/vulkan.h>
#include "aeongames/Mesh.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanMesh
    {
    public:
        VulkanMesh ( const VulkanRenderer&  aVulkanRenderer, const Mesh& aMesh );
        ~VulkanMesh();
        VulkanMesh ( VulkanMesh&& aVulkanMesh );
        VulkanMesh ( const VulkanMesh& aVulkanMesh ) = delete;
        VulkanMesh& operator= ( const VulkanMesh& aVulkanMesh ) = delete;
        VulkanMesh& operator= ( VulkanMesh&& aVulkanMesh ) = delete;
        void Bind() const;
    private:
        const VulkanRenderer& mVulkanRenderer;
        const Mesh* mMesh{nullptr};
        VulkanBuffer mMeshBuffer;
    };
}
#endif
