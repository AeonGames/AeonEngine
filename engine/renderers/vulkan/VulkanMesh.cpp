/*
Copyright (C) 2017-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/AeonEngine.h"
#include "aeongames/Utilities.h"
#include "aeongames/CRC.h"
#include "VulkanRenderer.h"
#include "VulkanMesh.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const VulkanRenderer&  aVulkanRenderer, uint32_t aId ) :
        mVulkanRenderer ( aVulkanRenderer ), mBuffer ( mVulkanRenderer )
    {
        if ( aId )
        {
            Resource::Load ( aId );
        }
    }

    VulkanMesh::~VulkanMesh() = default;

    VkIndexType VulkanMesh::GetIndexType() const
    {
        switch ( mIndexSize )
        {
        case 1:
        case 2:
            return VK_INDEX_TYPE_UINT16;
        case 4:
            return VK_INDEX_TYPE_UINT32;
        default:
            break;
        };
        throw std::runtime_error ( "Invalid Index Size." );
    }

    const VkBuffer & VulkanMesh::GetBuffer() const
    {
        return mBuffer.GetBuffer();
    }

    void VulkanMesh::Load ( const MeshMsg& aMeshMsg )
    {
        mAABB = AABB
        {
            {
                aMeshMsg.center().x(),
                aMeshMsg.center().y(),
                aMeshMsg.center().z()
            },
            {
                aMeshMsg.radii().x(),
                aMeshMsg.radii().y(),
                aMeshMsg.radii().z()
            }
        };

        mVertexCount = aMeshMsg.vertexcount();
        mIndexCount = aMeshMsg.indexcount();
        mIndexSize = aMeshMsg.indexsize();
        mVertexFlags = aMeshMsg.vertexflags();

        // Vulkan Specific code
        const VkDeviceSize vertex_buffer_size = ( sizeof ( Vertex ) * GetVertexCount() );
        // We need to expand 1 byte indices to 2 since they're not supported on Vulkan
        const VkDeviceSize index_buffer_size = aMeshMsg.indexbuffer().size() * ( ( aMeshMsg.indexsize() == 1 ) ? 2 : 1 );
        VkDeviceSize buffer_size = vertex_buffer_size + index_buffer_size;
        VkBufferUsageFlags buffer_usage = ( ( mVertexCount ) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0 ) | ( ( mIndexCount ) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0 );
        mBuffer.Initialize ( buffer_size, buffer_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        std::vector<uint8_t> buffer ( buffer_size );
        if ( GetVertexCount() )
        {
            auto* vertices = reinterpret_cast<Vertex*> ( buffer.data() );
            memset ( vertices, 0, vertex_buffer_size );
            uintptr_t offset = 0;
            for ( uint32_t j = 0; j < GetVertexCount(); ++j )
            {
                if ( mVertexFlags & Mesh::POSITION_BIT )
                {
                    memcpy ( vertices[j].position, aMeshMsg.vertexbuffer().data() + offset, sizeof ( Vertex::position ) );
                    offset += sizeof ( Vertex::position );
                }

                if ( mVertexFlags & Mesh::NORMAL_BIT )
                {
                    memcpy ( vertices[j].normal, aMeshMsg.vertexbuffer().data() + offset, sizeof ( Vertex::normal ) );
                    offset += sizeof ( Vertex::normal );
                }

                if ( mVertexFlags & Mesh::TANGENT_BIT )
                {
                    memcpy ( vertices[j].tangent, aMeshMsg.vertexbuffer().data() + offset, sizeof ( Vertex::tangent ) );
                    offset += sizeof ( Vertex::tangent );
                }

                if ( mVertexFlags & Mesh::BITANGENT_BIT )
                {
                    memcpy ( vertices[j].bitangent, aMeshMsg.vertexbuffer().data() + offset, sizeof ( Vertex::bitangent ) );
                    offset += sizeof ( Vertex::bitangent );
                }

                if ( mVertexFlags & Mesh::UV_BIT )
                {
                    memcpy ( vertices[j].uv, aMeshMsg.vertexbuffer().data() + offset, sizeof ( Vertex::uv ) );
                    offset += sizeof ( Vertex::uv );
                }

                if ( mVertexFlags & Mesh::WEIGHT_IDX_BIT )
                {
                    memcpy ( vertices[j].weight_indices, aMeshMsg.vertexbuffer().data() + offset, sizeof ( Vertex::weight_indices ) );
                    offset += sizeof ( Vertex::weight_indices );
                }

                if ( mVertexFlags & Mesh::WEIGHT_BIT )
                {
                    memcpy ( vertices[j].weight_influences, aMeshMsg.vertexbuffer().data() + offset, sizeof ( Vertex::weight_influences ) );
                    offset += sizeof ( Vertex::weight_influences );
                }

                if ( mVertexFlags & Mesh::COLOR_BIT )
                {
                    memcpy ( vertices[j].color, aMeshMsg.vertexbuffer().data() + offset, sizeof ( Vertex::color ) );
                    offset += sizeof ( Vertex::color );
                }
            }
        }
        if ( mIndexCount )
        {
            void* data = buffer.data() + vertex_buffer_size;
            if ( aMeshMsg.indexsize() != 1 )
            {
                memcpy ( data, aMeshMsg.indexbuffer().data(), aMeshMsg.indexbuffer().size() );
            }
            else
            {
                /**@note upcast to 16 bit indices.*/
                for ( size_t j = 0; j < mIndexCount; ++j )
                {
                    ( reinterpret_cast<uint16_t*> ( data ) [j] ) = aMeshMsg.indexbuffer() [j];
                }
            }
        }
        mBuffer.WriteMemory ( 0, buffer.size(), buffer.data() );
    }

    void VulkanMesh::Unload ()
    {
        mBuffer.Finalize();
        mAABB = {};
        mVertexFlags = {};
        mVertexCount = {};
        mIndexSize = {};
        mIndexCount = {};
    }

    uint32_t VulkanMesh::GetIndexCount() const
    {
        return mIndexCount;
    }

    uint32_t VulkanMesh::GetVertexCount() const
    {
        return mVertexCount;
    }

    const AABB& VulkanMesh::GetAABB() const
    {
        return mAABB;
    }

    uint32_t VulkanMesh::GetIndexSize () const
    {
        return mIndexSize;
    }
}
