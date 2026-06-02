/*
Copyright (C) 2017,2018,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANMESH_HPP
#define AEONGAMES_VULKANMESH_HPP

#include <vulkan/vulkan.h>
#include "aeongames/Mesh.hpp"
#include "VulkanBuffer.hpp"

namespace AeonGames
{
    class VulkanRenderer;
    /** @brief Vulkan mesh wrapper managing vertex and index buffer resources. */
    class VulkanMesh
    {
    public:
        /// @brief Construct from a renderer and mesh resource.
        VulkanMesh ( const VulkanRenderer&  aVulkanRenderer, const Mesh& aMesh );
        ~VulkanMesh();
        /// @brief Move constructor.
        VulkanMesh ( VulkanMesh&& aVulkanMesh );
        VulkanMesh ( const VulkanMesh& aVulkanMesh ) = delete;
        VulkanMesh& operator= ( const VulkanMesh& aVulkanMesh ) = delete;
        VulkanMesh& operator= ( VulkanMesh&& aVulkanMesh ) = delete;
        /// @brief Bind the mesh vertex and index buffers to a command buffer.
        void Bind ( VkCommandBuffer aVkCommandBuffer ) const;
        /** @brief Get the descriptor set that exposes the static vertex buffer as
         * a storage buffer (SSBO), for binding as @c SourceVertices in compute
         * skinning. Bound with a zero dynamic offset; range covers the vertex
         * region of the mesh buffer. */
        const VkDescriptorSet& GetSourceVerticesDescriptorSet() const;
    private:
        void InitializeSourceVerticesDescriptor();
        void FinalizeSourceVerticesDescriptor();
        const VulkanRenderer& mVulkanRenderer;
        const Mesh* mMesh{nullptr};
        VulkanBuffer mMeshBuffer;
        VkDescriptorPool mSourceVerticesDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSetLayout mSourceVerticesDescriptorSetLayout{ VK_NULL_HANDLE };
        VkDescriptorSet mSourceVerticesDescriptorSet{ VK_NULL_HANDLE };
    };
}
#endif
