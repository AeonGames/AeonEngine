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

#include <exception>
#include <string>
#include <vulkan/vulkan.h>
#include <memory>
#include "aeongames/Mesh.h"
#include "VulkanBuffer.h"

namespace AeonGames
{
    class VulkanRenderer;
    class VulkanMesh final : public Mesh
    {
    public:
        VulkanMesh ( const VulkanRenderer&  aVulkanRenderer, uint32_t aId = 0 );
        ~VulkanMesh() final;
        void Load ( const MeshMsg& aMeshMsg ) final;
        void Unload () final;
        uint32_t GetIndexSize () const final;
        uint32_t GetIndexCount() const final;
        uint32_t GetVertexCount() const final;
        const AABB& GetAABB() const final;

        VkIndexType GetIndexType() const;
        const VkBuffer& GetBuffer() const;
    private:
        const VulkanRenderer& mVulkanRenderer;
        VulkanBuffer mBuffer;
        AABB mAABB{};
        uint32_t mVertexFlags{};
        uint32_t mVertexCount{};
        uint32_t mIndexSize{};
        uint32_t mIndexCount{};
    };
}
#endif
