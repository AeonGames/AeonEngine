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
#include <vulkan/vulkan.h>
#include "aeongames/Memory.h"
#include "aeongames/Mesh.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanMesh
    {
    public:
        VulkanMesh ( const std::shared_ptr<const Mesh> aMesh, const std::shared_ptr<const VulkanRenderer> aVulkanRenderer );
        ~VulkanMesh();
        void Render() const;
    private:
        void Initialize();
        void Finalize();
        VkIndexType GetIndexType ( Mesh::IndexType aIndexType ) const;
        std::shared_ptr<const Mesh> mMesh;
        std::shared_ptr<const VulkanRenderer> mVulkanRenderer;
        std::vector<VulkanBuffer> mBuffers;
    };
}
#endif
