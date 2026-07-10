/*
Copyright (C) 2017-2019,2021,2023,2025,2026 Rodrigo Jose Hernandez Cordoba

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

#include <fstream>
#include <sstream>
#include <exception>
#include <utility>
#include <vector>
#include <cassert>
#include <cstring>
#include "aeongames/ProtoBufClasses.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/LogLevel.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanMesh.hpp"
#include "VulkanUtilities.hpp"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const VulkanRenderer& aVulkanRenderer, const Mesh& aMesh ) :
        mVulkanRenderer {aVulkanRenderer}, mMesh{&aMesh}, mMeshBuffer { mVulkanRenderer }
    {
        // A mesh carrying per-vertex weights may be re-posed by the compute
        // skinning pass, which reads its rest-pose vertices as an SSBO
        // (SourceVertices) and writes a separate output buffer the draw binds.
        // Such meshes keep their own buffer. Weightless (static) meshes never
        // skin, so they join the renderer's shared geometry pool -- the
        // foundation for GPU-driven indirect drawing.
        bool has_weights = false;
        for ( const auto& attribute : aMesh.GetAttributes() )
        {
            if ( std::get<0> ( attribute ) == Mesh::WEIGHT_INDEX || std::get<0> ( attribute ) == Mesh::WEIGHT_VALUE )
            {
                has_weights = true;
                break;
            }
        }
        mStride = static_cast<uint32_t> ( aMesh.GetStride() );
        mPooled = !has_weights;

        if ( mPooled )
        {
            // Normalise indices to uint32 for the single shared index pool.
            std::vector<uint32_t> indices{};
            const uint32_t index_count = aMesh.GetIndexCount();
            if ( index_count )
            {
                indices.resize ( index_count );
                const uint8_t* source = aMesh.GetIndexBuffer().data();
                switch ( aMesh.GetIndexSize() )
                {
                case 1:
                    for ( uint32_t i = 0; i < index_count; ++i )
                    {
                        indices[i] = source[i];
                    }
                    break;
                case 2:
                    for ( uint32_t i = 0; i < index_count; ++i )
                    {
                        indices[i] = reinterpret_cast<const uint16_t*> ( source ) [i];
                    }
                    break;
                case 4:
                    std::memcpy ( indices.data(), source, static_cast<size_t> ( index_count ) * sizeof ( uint32_t ) );
                    break;
                default:
                    break;
                }
            }
            const VulkanRenderer::GeometryAllocation allocation = mVulkanRenderer.RegisterMeshGeometry (
                    mStride,
                    aMesh.GetVertexCount() ? aMesh.GetVertexBuffer().data() : nullptr,
                    aMesh.GetVertexBuffer().size(),
                    index_count ? indices.data() : nullptr,
                    index_count );
            mBaseVertex = allocation.mBaseVertex;
            mFirstIndex = allocation.mFirstIndex;
            return;
        }

        // Skinned (weighted) mesh: private buffer holding rest-pose vertices then
        // indices, unchanged from before the geometry pool.
        const VkDeviceSize buffer_size{ aMesh.GetVertexBuffer().size() + ( aMesh.GetIndexBuffer().size() * ( aMesh.GetIndexSize() == 1 ? 2 : 1 ) ) };
        const VkBufferUsageFlags buffer_usage {static_cast<VkBufferUsageFlags> ( ( aMesh.GetVertexCount() ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0 ) | ( aMesh.GetIndexCount() ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0 ) ) };

        mMeshBuffer.Initialize ( buffer_size, buffer_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        if ( aMesh.GetVertexCount() )
        {
            mMeshBuffer.WriteMemory ( 0, aMesh.GetVertexBuffer().size(), aMesh.GetVertexBuffer().data() );
        }
        if ( aMesh.GetIndexCount() )
        {
            if ( aMesh.GetIndexSize() != 1 )
            {
                mMeshBuffer.WriteMemory ( aMesh.GetVertexBuffer().size(), aMesh.GetIndexBuffer().size(), aMesh.GetIndexBuffer().data() );
            }
            else
            {
                std::vector<uint16_t> buffer{};
                buffer.resize ( aMesh.GetIndexCount() );
                for ( size_t i = 0; i < aMesh.GetIndexCount(); ++i )
                {
                    buffer[i] = aMesh.GetIndexBuffer() [i];
                }
                mMeshBuffer.WriteMemory ( aMesh.GetVertexBuffer().size(), buffer.size() * sizeof ( uint16_t ), buffer.data() );
            }
        }

        InitializeSourceVerticesDescriptor();
    }

    void VulkanMesh::InitializeSourceVerticesDescriptor()
    {
        if ( !mMesh->GetVertexCount() )
        {
            return;
        }

        VkDescriptorPoolSize pool_size{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 };
        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.maxSets = 1;
        descriptor_pool_create_info.poolSizeCount = 1;
        descriptor_pool_create_info.pPoolSizes = &pool_size;
        if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer.GetDevice(), &descriptor_pool_create_info, nullptr, &mSourceVerticesDescriptorPool ) )
        {
            std::ostringstream stream;
            stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
        descriptor_set_layout_binding.binding = 0;
        descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        descriptor_set_layout_binding.descriptorCount = 1;
        descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        descriptor_set_layout_binding.pImmutableSamplers = nullptr;
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.bindingCount = 1;
        descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
        mSourceVerticesDescriptorSetLayout = mVulkanRenderer.GetDescriptorSetLayout ( descriptor_set_layout_create_info );

        mSourceVerticesDescriptorSet = CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mSourceVerticesDescriptorPool, mSourceVerticesDescriptorSetLayout );

        // The vertex region occupies [0, GetVertexBuffer().size()) of the mesh
        // buffer; bind exactly that range with a zero dynamic offset.
        VkDescriptorBufferInfo descriptor_buffer_info{ mMeshBuffer.GetBuffer(), 0, mMesh->GetVertexBuffer().size() };
        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstSet = mSourceVerticesDescriptorSet;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), 1, &write_descriptor_set, 0, nullptr );
    }

    void VulkanMesh::FinalizeSourceVerticesDescriptor()
    {
        if ( mSourceVerticesDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mSourceVerticesDescriptorPool, nullptr );
            mSourceVerticesDescriptorPool = VK_NULL_HANDLE;
        }
        mSourceVerticesDescriptorSet = VK_NULL_HANDLE;
    }

    const VkDescriptorSet& VulkanMesh::GetSourceVerticesDescriptorSet() const
    {
        return mSourceVerticesDescriptorSet;
    }

    VulkanMesh::~VulkanMesh()
    {
        FinalizeSourceVerticesDescriptor();
        mMeshBuffer.Finalize();
    }

    VulkanMesh::VulkanMesh ( VulkanMesh&& aVulkanMesh ) :
        mVulkanRenderer{aVulkanMesh.mVulkanRenderer},
        mMesh{aVulkanMesh.mMesh},
        mPooled{aVulkanMesh.mPooled},
        mStride{aVulkanMesh.mStride},
        mBaseVertex{aVulkanMesh.mBaseVertex},
        mFirstIndex{aVulkanMesh.mFirstIndex},
        mMeshBuffer{std::move ( aVulkanMesh.mMeshBuffer ) }
    {
        std::swap ( mSourceVerticesDescriptorPool, aVulkanMesh.mSourceVerticesDescriptorPool );
        std::swap ( mSourceVerticesDescriptorSetLayout, aVulkanMesh.mSourceVerticesDescriptorSetLayout );
        std::swap ( mSourceVerticesDescriptorSet, aVulkanMesh.mSourceVerticesDescriptorSet );
    }

    static VkIndexType GetIndexType ( const Mesh* aMesh )
    {
        switch ( aMesh->GetIndexSize() )
        {
        case 1:
        case 2:
            return VK_INDEX_TYPE_UINT16;
        case 4:
            return VK_INDEX_TYPE_UINT32;
        default:
            break;
        };
        std::cout << LogLevel::Error << "Invalid Index Size." << std::endl;
        throw std::runtime_error ( "Invalid Index Size." );
    }

    void VulkanMesh::Bind ( VkCommandBuffer aVkCommandBuffer, VkBuffer aSkinnedVertexBuffer, VkDeviceSize aSkinnedVertexOffset ) const
    {
        const VkDeviceSize zero_offset = 0;
        if ( aSkinnedVertexBuffer != VK_NULL_HANDLE )
        {
            // Pre-skinned vertices produced by the compute pass replace the
            // rest-pose vertex input; the index buffer still comes from this
            // (weighted, private) mesh's own buffer.
            vkCmdBindVertexBuffers ( aVkCommandBuffer, 0, 1, &aSkinnedVertexBuffer, &aSkinnedVertexOffset );
            if ( mMesh->GetIndexCount() )
            {
                vkCmdBindIndexBuffer ( aVkCommandBuffer,
                                       mMeshBuffer.GetBuffer(), mMesh->GetVertexBuffer().size(),
                                       GetIndexType ( mMesh ) );
            }
        }
        else if ( mPooled )
        {
            // Static mesh drawn from the renderer's shared geometry pool; the
            // draw's base-vertex / first-index select this mesh's region.
            vkCmdBindVertexBuffers ( aVkCommandBuffer, 0, 1, &mVulkanRenderer.GetGeometryVertexBuffer ( mStride ), &zero_offset );
            if ( mMesh->GetIndexCount() )
            {
                vkCmdBindIndexBuffer ( aVkCommandBuffer,
                                       mVulkanRenderer.GetGeometryIndexBuffer(), 0,
                                       VK_INDEX_TYPE_UINT32 );
            }
        }
        else
        {
            // Weighted mesh drawn at rest pose (e.g. a shadow pass) from its own
            // buffer.
            vkCmdBindVertexBuffers ( aVkCommandBuffer, 0, 1, &mMeshBuffer.GetBuffer(), &zero_offset );
            if ( mMesh->GetIndexCount() )
            {
                vkCmdBindIndexBuffer ( aVkCommandBuffer,
                                       mMeshBuffer.GetBuffer(), mMesh->GetVertexBuffer().size(),
                                       GetIndexType ( mMesh ) );
            }
        }
    }

    uint32_t VulkanMesh::GetBaseVertex() const
    {
        return mBaseVertex;
    }

    uint32_t VulkanMesh::GetFirstIndex() const
    {
        return mFirstIndex;
    }
}
