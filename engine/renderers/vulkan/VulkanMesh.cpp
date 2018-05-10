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
    VulkanMesh::VulkanMesh ( const Mesh& aMesh, const std::shared_ptr<const VulkanRenderer>&  aVulkanRenderer ) :
        mMesh ( aMesh ), mVulkanRenderer ( aVulkanRenderer ), mBuffer ( *mVulkanRenderer.get() )
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

    void VulkanMesh::Render ( uint32_t aInstanceCount, uint32_t aFirstInstance ) const
    {
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers ( mVulkanRenderer->GetCommandBuffer(), 0, 1, &mBuffer.GetBuffer(), &offset );
        if ( mMesh.GetIndexCount() )
        {
            vkCmdBindIndexBuffer ( mVulkanRenderer->GetCommandBuffer(),
                                   mBuffer.GetBuffer(), ( sizeof ( Vertex ) * mMesh.GetVertexCount() ),
                                   GetIndexType ( static_cast<AeonGames::Mesh::IndexType> ( mMesh.GetIndexType() ) ) );
            vkCmdDrawIndexed ( mVulkanRenderer->GetCommandBuffer(), mMesh.GetIndexCount(), aInstanceCount, 0, 0, aFirstInstance );
        }
        else
        {
            vkCmdDraw ( mVulkanRenderer->GetCommandBuffer(), mMesh.GetVertexCount(), aInstanceCount, 0, aFirstInstance );
        }
    }

    void VulkanMesh::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        const VkDeviceSize vertex_buffer_size = ( sizeof ( Vertex ) * mMesh.GetVertexCount() );
        const VkDeviceSize index_buffer_size = ( mMesh.GetIndexBuffer().length() *
                                               ( ( mMesh.GetIndexType() == Mesh::BYTE || mMesh.GetIndexType() == Mesh::UNSIGNED_BYTE ) ? 2 : 1 ) );
        VkDeviceSize buffer_size = vertex_buffer_size + index_buffer_size;
        VkBufferUsageFlags buffer_usage = ( ( mMesh.GetVertexCount() ) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0 ) | ( ( mMesh.GetIndexCount() ) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0 );
        mBuffer.Initialize ( buffer_size, buffer_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        std::vector<uint8_t> buffer ( buffer_size );
        if ( mMesh.GetVertexCount() )
        {
            auto* vertices = reinterpret_cast<Vertex*> ( buffer.data() );
            memset ( vertices, 0, vertex_buffer_size );
            uintptr_t offset = 0;
            for ( uint32_t j = 0; j < mMesh.GetVertexCount(); ++j )
            {
                if ( mMesh.GetVertexFlags() & Mesh::POSITION_BIT )
                {
                    memcpy ( vertices[j].position, mMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::position ) );
                    offset += sizeof ( Vertex::position );
                }

                if ( mMesh.GetVertexFlags() & Mesh::NORMAL_BIT )
                {
                    memcpy ( vertices[j].normal, mMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::normal ) );
                    offset += sizeof ( Vertex::normal );
                }

                if ( mMesh.GetVertexFlags() & Mesh::TANGENT_BIT )
                {
                    memcpy ( vertices[j].tangent, mMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::tangent ) );
                    offset += sizeof ( Vertex::tangent );
                }

                if ( mMesh.GetVertexFlags() & Mesh::BITANGENT_BIT )
                {
                    memcpy ( vertices[j].bitangent, mMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::bitangent ) );
                    offset += sizeof ( Vertex::bitangent );
                }

                if ( mMesh.GetVertexFlags() & Mesh::UV_BIT )
                {
                    memcpy ( vertices[j].uv, mMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::uv ) );
                    offset += sizeof ( Vertex::uv );
                }

                if ( mMesh.GetVertexFlags() & Mesh::WEIGHT_BIT )
                {
                    memcpy ( vertices[j].weight_indices, mMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::weight_indices ) );
                    offset += sizeof ( Vertex::weight_indices );
                    memcpy ( vertices[j].weight_influences, mMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::weight_influences ) );
                    offset += sizeof ( Vertex::weight_influences );
                }
            }
        }
        if ( mMesh.GetIndexCount() )
        {
            void* data = buffer.data() + vertex_buffer_size;
            if ( ! ( mMesh.GetIndexType() == Mesh::BYTE || mMesh.GetIndexType() == Mesh::UNSIGNED_BYTE ) )
            {
                memcpy ( data, mMesh.GetIndexBuffer().data(), mMesh.GetIndexBuffer().size() );
            }
            else
            {
                /**@note upcast 16 bit indices.*/
                for ( size_t j = 0; j < mMesh.GetIndexBuffer().size(); ++j )
                {
                    reinterpret_cast<uint16_t*> ( data ) [j] = mMesh.GetIndexBuffer() [j];
                }
            }
        }
        mBuffer.WriteMemory ( 0, buffer.size(), buffer.data() );
    }

    void VulkanMesh::Finalize()
    {
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
