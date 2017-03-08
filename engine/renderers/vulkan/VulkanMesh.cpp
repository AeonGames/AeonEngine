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

#if 0
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#endif

#include "aeongames/Utilities.h"
#include "aeongames/Mesh.h"
#include "VulkanRenderer.h"
#include "VulkanMesh.h"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const std::shared_ptr<Mesh> aMesh, const VulkanRenderer& aVulkanRenderer ) :
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
        }
    }

    void VulkanMesh::Initialize()
    {
        auto& triangle_groups = mMesh->GetTriangleGroups();
        mBuffers.resize ( triangle_groups.size() );
        for ( size_t i = 0; i < triangle_groups.size(); ++i )
        {
            if ( triangle_groups[i].mVertexCount )
            {
                VkBufferCreateInfo buffer_create_info{};
                buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                buffer_create_info.pNext = nullptr;
                buffer_create_info.flags = 0;
                buffer_create_info.size = triangle_groups[i].mVertexBuffer.size();
                buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                buffer_create_info.queueFamilyIndexCount = 0;
                buffer_create_info.pQueueFamilyIndices = nullptr;

                if ( VkResult result = vkCreateBuffer ( mVulkanRenderer.GetDevice(), &buffer_create_info, nullptr, &mBuffers[i].mVertexBuffer ) )
                {
                    throw std::runtime_error ( "vkCreateBuffer failed for vertex buffer" );
                }
            }
            if ( triangle_groups[i].mIndexCount )
            {
                VkBufferCreateInfo buffer_create_info{};
                buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                buffer_create_info.pNext = nullptr;
                buffer_create_info.flags = 0;
                buffer_create_info.size = triangle_groups[i].mIndexBuffer.size();
                buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                buffer_create_info.queueFamilyIndexCount = 0;
                buffer_create_info.pQueueFamilyIndices = nullptr;

                if ( VkResult result = vkCreateBuffer ( mVulkanRenderer.GetDevice(), &buffer_create_info, nullptr, &mBuffers[i].mIndexBuffer ) )
                {
                    throw std::runtime_error ( "vkCreateBuffer failed for index buffer" );
                }
            }
        }
    }

    void VulkanMesh::Finalize()
    {
        for ( auto& i : mBuffers )
        {
            if ( i.mVertexBuffer != VK_NULL_HANDLE )
            {
                vkDestroyBuffer ( mVulkanRenderer.GetDevice(), i.mVertexBuffer, nullptr );
                i.mVertexBuffer = VK_NULL_HANDLE;
            }
            if ( i.mIndexBuffer != VK_NULL_HANDLE )
            {
                vkDestroyBuffer ( mVulkanRenderer.GetDevice(), i.mIndexBuffer, nullptr );
                i.mIndexBuffer = VK_NULL_HANDLE;
            }
        }
    }
}
