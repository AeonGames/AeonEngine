/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#include <iostream>
#include <stdexcept>
#include <utility>
#include "aeongames/LogLevel.hpp"
#include "VulkanStorageMemoryPoolBuffer.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanUtilities.hpp"

namespace AeonGames
{
    VulkanStorageMemoryPoolBuffer::VulkanStorageMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer, size_t aStackSize ) :
        mVulkanRenderer { aVulkanRenderer },
        mStorageBuffer { mVulkanRenderer,
                         ( ( aStackSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment - 1 ) ) + 1,
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT }
    {
        InitializeDescriptorPool();
    }

    VulkanStorageMemoryPoolBuffer::VulkanStorageMemoryPoolBuffer ( VulkanStorageMemoryPoolBuffer&& aVulkanStorageMemoryPoolBuffer ) :
        mVulkanRenderer { aVulkanStorageMemoryPoolBuffer.mVulkanRenderer },
        mStorageBuffer { std::move ( aVulkanStorageMemoryPoolBuffer.mStorageBuffer ) }
    {
        std::swap ( mOffset, aVulkanStorageMemoryPoolBuffer.mOffset );
        std::swap ( mVkDescriptorPools, aVulkanStorageMemoryPoolBuffer.mVkDescriptorPools );
        std::swap ( mVkDescriptorSetLayout, aVulkanStorageMemoryPoolBuffer.mVkDescriptorSetLayout );
        std::swap ( mVkDescriptorSets, aVulkanStorageMemoryPoolBuffer.mVkDescriptorSets );
        std::swap ( mDescriptorSetIndex, aVulkanStorageMemoryPoolBuffer.mDescriptorSetIndex );
        std::swap ( mOffsetToDescriptorSet, aVulkanStorageMemoryPoolBuffer.mOffsetToDescriptorSet );
    }

    VulkanStorageMemoryPoolBuffer::VulkanStorageMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer ) :
        mVulkanRenderer { aVulkanRenderer }, mStorageBuffer{aVulkanRenderer} {}

    void VulkanStorageMemoryPoolBuffer::Initialize ( size_t aStackSize )
    {
        mStorageBuffer.Initialize ( ( ( aStackSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment - 1 ) ) + 1,
                                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
        InitializeDescriptorPool();
    }

    VulkanStorageMemoryPoolBuffer::~VulkanStorageMemoryPoolBuffer()
    {
        Finalize();
    }

    void VulkanStorageMemoryPoolBuffer::Finalize()
    {
        FinalizeDescriptorPool();
        mStorageBuffer.Finalize();
    }

    void VulkanStorageMemoryPoolBuffer::AddDescriptorPool()
    {
        VkDescriptorPoolSize pool_size{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, kDescriptorSetsPerPool };
        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.maxSets = kDescriptorSetsPerPool;
        descriptor_pool_create_info.poolSizeCount = 1;
        descriptor_pool_create_info.pPoolSizes = &pool_size;
        VkDescriptorPool descriptor_pool{ VK_NULL_HANDLE };
        if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer.GetDevice(), &descriptor_pool_create_info, nullptr, &descriptor_pool ) )
        {
            std::ostringstream stream;
            stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        mVkDescriptorPools.push_back ( descriptor_pool );
    }

    void VulkanStorageMemoryPoolBuffer::InitializeDescriptorPool()
    {
        AddDescriptorPool();

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
        descriptor_set_layout_binding.binding = 0;
        descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        descriptor_set_layout_binding.descriptorCount = 1;
        descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        descriptor_set_layout_binding.pImmutableSamplers = nullptr;
        descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
        mVkDescriptorSetLayout = mVulkanRenderer.GetDescriptorSetLayout ( descriptor_set_layout_create_info );
    }

    void VulkanStorageMemoryPoolBuffer::FinalizeDescriptorPool()
    {
        for ( VkDescriptorPool& descriptor_pool : mVkDescriptorPools )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), descriptor_pool, nullptr );
        }
        mVkDescriptorPools.clear();
        mVkDescriptorSets.clear();
        mOffsetToDescriptorSet.clear();
        mDescriptorSetIndex = 0;
    }

    BufferAccessor VulkanStorageMemoryPoolBuffer::Allocate ( size_t aSize )
    {
        size_t offset = mOffset;
        mOffset += ( ( aSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment - 1 ) ) + 1;
        if ( mOffset > mStorageBuffer.GetSize() )
        {
            mOffset = offset;
            std::cout << LogLevel::Error << "Storage Memory Pool Buffer cannot fulfill allocation request." << std::endl;
            throw std::runtime_error ( "Storage Memory Pool Buffer cannot fulfill allocation request." );
        }

        // Grow the descriptor-set cache lazily; sets are reused across frames.
        // Each backing pool holds kDescriptorSetsPerPool sets, so add a new
        // pool whenever the cache crosses a pool boundary.
        if ( mDescriptorSetIndex >= mVkDescriptorSets.size() )
        {
            const uint32_t pool_index = mDescriptorSetIndex / kDescriptorSetsPerPool;
            if ( pool_index >= mVkDescriptorPools.size() )
            {
                AddDescriptorPool();
            }
            mVkDescriptorSets.push_back (
                CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mVkDescriptorPools[pool_index], mVkDescriptorSetLayout ) );
        }
        VkDescriptorSet descriptor_set = mVkDescriptorSets[mDescriptorSetIndex++];

        // Bind exactly this allocation's window: the dynamic offset is zero, so
        // (descriptor offset + range) == (offset + aSize) stays within the pool
        // buffer for every allocation.
        VkDescriptorBufferInfo descriptor_buffer_info = { mStorageBuffer.GetBuffer(), offset, aSize };
        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.pNext = nullptr;
        write_descriptor_set.dstSet = descriptor_set;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
        write_descriptor_set.pImageInfo = nullptr;
        write_descriptor_set.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), 1, &write_descriptor_set, 0, nullptr );

        mOffsetToDescriptorSet[offset] = descriptor_set;
        return BufferAccessor{this, offset, aSize};
    }

    void VulkanStorageMemoryPoolBuffer::Reset()
    {
        mOffset = 0;
        mDescriptorSetIndex = 0;
        mOffsetToDescriptorSet.clear();
    }

    const VkDescriptorSet& VulkanStorageMemoryPoolBuffer::GetDescriptorSet ( size_t aOffset ) const
    {
        return mOffsetToDescriptorSet.at ( aOffset );
    }
    const Buffer& VulkanStorageMemoryPoolBuffer::GetBuffer() const
    {
        return mStorageBuffer;
    }
}
