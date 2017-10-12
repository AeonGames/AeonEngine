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

#include <fstream>
#include <sstream>
#include <exception>
#include <vector>
#include <cassert>
#include <cstring>
#include "aeongames/Utilities.h"
#include "aeongames/Mesh.h"
#include "VulkanRenderer.h"
#include "VulkanMesh.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const std::shared_ptr<const Mesh> aMesh, const std::shared_ptr<const VulkanRenderer> aVulkanRenderer ) :
        mMesh ( aMesh ), mVulkanRenderer ( aVulkanRenderer )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }
    VulkanMesh::~VulkanMesh()
    {
        Finalize();
    }

    void VulkanMesh::Render() const
    {
        auto& triangle_groups = mMesh->GetTriangleGroups();
        for ( size_t i = 0; i < triangle_groups.size(); ++i )
        {
            const VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers ( mVulkanRenderer->GetCommandBuffer(), 0, 1, &mBuffers[i].GetBuffer(), &offset );
            if ( triangle_groups[i].mIndexCount )
            {
                vkCmdBindIndexBuffer ( mVulkanRenderer->GetCommandBuffer(),
                                       mBuffers[i].GetBuffer(), ( sizeof ( Vertex ) * triangle_groups[i].mVertexCount ),
                                       GetIndexType ( static_cast<AeonGames::Mesh::IndexType> ( triangle_groups[i].mIndexType ) ) );
                vkCmdDrawIndexed ( mVulkanRenderer->GetCommandBuffer(), triangle_groups[i].mIndexCount, 1, 0, 0, 1 );
            }
            else
            {
                vkCmdDraw ( mVulkanRenderer->GetCommandBuffer(), triangle_groups[i].mVertexCount, 1, 0, 0 );
            }
        }
    }

    void VulkanMesh::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        auto& triangle_groups = mMesh->GetTriangleGroups();
        mBuffers.reserve ( triangle_groups.size() );
        for ( size_t i = 0; i < triangle_groups.size(); ++i )
        {
            const VkDeviceSize vertex_buffer_size = ( sizeof ( Vertex ) * triangle_groups[i].mVertexCount );
            const VkDeviceSize index_buffer_size = ( triangle_groups[i].mIndexBuffer.length() *
                                                   ( ( triangle_groups[i].mIndexType == Mesh::BYTE || triangle_groups[i].mIndexType == Mesh::UNSIGNED_BYTE ) ? 2 : 1 ) );
            VkDeviceSize buffer_size = vertex_buffer_size + index_buffer_size;
            VkBufferUsageFlags buffer_usage = ( ( triangle_groups[i].mVertexCount ) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0 ) | ( ( triangle_groups[i].mIndexCount ) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0 );

            mBuffers.emplace_back ( *mVulkanRenderer.get(), buffer_size, buffer_usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

            if ( triangle_groups[i].mVertexCount )
            {
                Vertex* vertices = static_cast<Vertex*> ( mBuffers.back().Map ( 0, vertex_buffer_size ) );
                memset ( vertices, 0, vertex_buffer_size );
                uintptr_t offset = 0;
                for ( uint32_t j = 0; j < triangle_groups[i].mVertexCount; ++j )
                {
                    if ( triangle_groups[i].mVertexFlags & Mesh::POSITION_BIT )
                    {
                        memcpy ( vertices[j].position, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::position ) );
                        offset += sizeof ( Vertex::position );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::NORMAL_BIT )
                    {
                        memcpy ( vertices[j].normal, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::normal ) );
                        offset += sizeof ( Vertex::normal );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::TANGENT_BIT )
                    {
                        memcpy ( vertices[j].tangent, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::tangent ) );
                        offset += sizeof ( Vertex::tangent );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::BITANGENT_BIT )
                    {
                        memcpy ( vertices[j].bitangent, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::bitangent ) );
                        offset += sizeof ( Vertex::bitangent );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::UV_BIT )
                    {
                        memcpy ( vertices[j].uv, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::uv ) );
                        offset += sizeof ( Vertex::uv );
                    }

                    if ( triangle_groups[i].mVertexFlags & Mesh::WEIGHT_BIT )
                    {
                        memcpy ( vertices[j].weight_indices, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::weight_indices ) );
                        offset += sizeof ( Vertex::weight_indices );
                        memcpy ( vertices[j].weight_influences, triangle_groups[i].mVertexBuffer.data() + offset, sizeof ( Vertex::weight_influences ) );
                        offset += sizeof ( Vertex::weight_influences );
                    }
                }
                mBuffers.back().Unmap();
            }
            if ( triangle_groups[i].mIndexCount )
            {
                void* data = mBuffers.back().Map ( triangle_groups[i].mVertexCount ? vertex_buffer_size : 0, index_buffer_size );
                if ( ! ( triangle_groups[i].mIndexType == Mesh::BYTE || triangle_groups[i].mIndexType == Mesh::UNSIGNED_BYTE ) )
                {
                    memcpy ( data, triangle_groups[i].mIndexBuffer.data(), triangle_groups[i].mIndexBuffer.size() );
                }
                else
                {
                    /**@note upcast 16 bit indices.*/
                    for ( size_t j = 0; j < triangle_groups[i].mIndexBuffer.size(); ++j )
                    {
                        reinterpret_cast<uint16_t*> ( data ) [j] = triangle_groups[i].mIndexBuffer[j];
                    }
                }
                mBuffers.back().Unmap();
            }
        }
    }

    void VulkanMesh::Finalize()
    {
        mBuffers.clear();
    }

    VkIndexType VulkanMesh::GetIndexType ( Mesh::IndexType aIndexType ) const
    {
        switch ( aIndexType )
        {
        /**@note BYTE has been upcasted*/
        case Mesh::UNSIGNED_BYTE:
        case Mesh::UNSIGNED_SHORT:
            return VK_INDEX_TYPE_UINT16;
        case Mesh::UNSIGNED_INT:
            return VK_INDEX_TYPE_UINT32;
        default:
            throw std::runtime_error ( "Invalid Index Type." );
        };
        return VK_INDEX_TYPE_MAX_ENUM;
    }
}
