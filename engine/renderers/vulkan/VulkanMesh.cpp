/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Utilities.h"
#include "aeongames/Mesh.h"
#include "VulkanRenderer.h"
#include "VulkanMesh.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const VulkanRenderer&  aVulkanRenderer ) :
        Mesh(), mVulkanRenderer ( aVulkanRenderer ), mBuffer ( mVulkanRenderer )
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

    const VkBuffer & VulkanMesh::GetBuffer() const
    {
        return mBuffer.GetBuffer();
    }

    void VulkanMesh::Initialize()
    {
        const VkDeviceSize vertex_buffer_size = ( sizeof ( Vertex ) * GetVertexCount() );
        const VkDeviceSize index_buffer_size = ( GetIndexBuffer().length() *
                                               ( ( GetIndexType() == Mesh::BYTE || GetIndexType() == Mesh::UNSIGNED_BYTE ) ? 2 : 1 ) );
        VkDeviceSize buffer_size = vertex_buffer_size + index_buffer_size;
        VkBufferUsageFlags buffer_usage = ( ( GetVertexCount() ) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0 ) | ( ( GetIndexCount() ) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0 );
        mBuffer.Initialize ( buffer_size, buffer_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        std::vector<uint8_t> buffer ( buffer_size );
        if ( GetVertexCount() )
        {
            auto* vertices = reinterpret_cast<Vertex*> ( buffer.data() );
            memset ( vertices, 0, vertex_buffer_size );
            uintptr_t offset = 0;
            for ( uint32_t j = 0; j < GetVertexCount(); ++j )
            {
                if ( GetVertexFlags() & Mesh::POSITION_BIT )
                {
                    memcpy ( vertices[j].position, GetVertexBuffer().data() + offset, sizeof ( Vertex::position ) );
                    offset += sizeof ( Vertex::position );
                }

                if ( GetVertexFlags() & Mesh::NORMAL_BIT )
                {
                    memcpy ( vertices[j].normal, GetVertexBuffer().data() + offset, sizeof ( Vertex::normal ) );
                    offset += sizeof ( Vertex::normal );
                }

                if ( GetVertexFlags() & Mesh::TANGENT_BIT )
                {
                    memcpy ( vertices[j].tangent, GetVertexBuffer().data() + offset, sizeof ( Vertex::tangent ) );
                    offset += sizeof ( Vertex::tangent );
                }

                if ( GetVertexFlags() & Mesh::BITANGENT_BIT )
                {
                    memcpy ( vertices[j].bitangent, GetVertexBuffer().data() + offset, sizeof ( Vertex::bitangent ) );
                    offset += sizeof ( Vertex::bitangent );
                }

                if ( GetVertexFlags() & Mesh::UV_BIT )
                {
                    memcpy ( vertices[j].uv, GetVertexBuffer().data() + offset, sizeof ( Vertex::uv ) );
                    offset += sizeof ( Vertex::uv );
                }

                if ( GetVertexFlags() & Mesh::WEIGHT_BIT )
                {
                    memcpy ( vertices[j].weight_indices, GetVertexBuffer().data() + offset, sizeof ( Vertex::weight_indices ) );
                    offset += sizeof ( Vertex::weight_indices );
                    memcpy ( vertices[j].weight_influences, GetVertexBuffer().data() + offset, sizeof ( Vertex::weight_influences ) );
                    offset += sizeof ( Vertex::weight_influences );
                }
            }
        }
        if ( GetIndexCount() )
        {
            void* data = buffer.data() + vertex_buffer_size;
            if ( ! ( GetIndexType() == Mesh::BYTE || GetIndexType() == Mesh::UNSIGNED_BYTE ) )
            {
                memcpy ( data, GetIndexBuffer().data(), GetIndexBuffer().size() );
            }
            else
            {
                /**@note upcast 16 bit indices.*/
                for ( size_t j = 0; j < GetIndexBuffer().size(); ++j )
                {
                    reinterpret_cast<uint16_t*> ( data ) [j] = GetIndexBuffer() [j];
                }
            }
        }
        mBuffer.WriteMemory ( 0, buffer.size(), buffer.data() );
    }

    void VulkanMesh::Finalize()
    {
    }

    VkIndexType GetVulkanIndexType ( Mesh::IndexType aIndexType )
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
