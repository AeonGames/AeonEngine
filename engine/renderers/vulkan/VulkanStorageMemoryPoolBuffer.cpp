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
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT }
    {
        InitializeDescriptorPool();
    }

    VulkanStorageMemoryPoolBuffer::VulkanStorageMemoryPoolBuffer ( VulkanStorageMemoryPoolBuffer&& aVulkanStorageMemoryPoolBuffer ) :
        mVulkanRenderer { aVulkanStorageMemoryPoolBuffer.mVulkanRenderer },
        mStorageBuffer { std::move ( aVulkanStorageMemoryPoolBuffer.mStorageBuffer ) }
    {
        std::swap ( mOffset, aVulkanStorageMemoryPoolBuffer.mOffset );
        std::swap ( mVkDescriptorPool, aVulkanStorageMemoryPoolBuffer.mVkDescriptorPool );
        std::swap ( mVkDescriptorSetLayout, aVulkanStorageMemoryPoolBuffer.mVkDescriptorSetLayout );
        std::swap ( mWholeBufferDescriptorSet, aVulkanStorageMemoryPoolBuffer.mWholeBufferDescriptorSet );
        std::swap ( mVkDescriptorSets, aVulkanStorageMemoryPoolBuffer.mVkDescriptorSets );
        std::swap ( mDescriptorSetIndex, aVulkanStorageMemoryPoolBuffer.mDescriptorSetIndex );
        std::swap ( mOffsetToDescriptorSet, aVulkanStorageMemoryPoolBuffer.mOffsetToDescriptorSet );
    }

    VulkanStorageMemoryPoolBuffer::VulkanStorageMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer ) :
        mVulkanRenderer { aVulkanRenderer }, mStorageBuffer{aVulkanRenderer} {}

    void VulkanStorageMemoryPoolBuffer::Initialize ( size_t aStackSize )
    {
        mStorageBuffer.Initialize ( ( ( aStackSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment - 1 ) ) + 1,
                                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
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

    void VulkanStorageMemoryPoolBuffer::InitializeDescriptorPool()
    {
        VkDescriptorPoolSize pool_size{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, kMaxDescriptorSets };
        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.maxSets = kMaxDescriptorSets;
        descriptor_pool_create_info.poolSizeCount = 1;
        descriptor_pool_create_info.pPoolSizes = &pool_size;
        if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer.GetDevice(), &descriptor_pool_create_info, nullptr, &mVkDescriptorPool ) )
        {
            std::ostringstream stream;
            stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

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

        // A single descriptor set spanning the whole pool buffer. Per-draw
        // object-matrix allocations bind this set with a dynamic offset, so
        // they never consume a per-offset descriptor set from the pool.
        mWholeBufferDescriptorSet = CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, mVkDescriptorSetLayout );
        VkDescriptorBufferInfo whole_buffer_info = { mStorageBuffer.GetBuffer(), 0, VK_WHOLE_SIZE };
        VkWriteDescriptorSet whole_buffer_write{};
        whole_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        whole_buffer_write.dstSet = mWholeBufferDescriptorSet;
        whole_buffer_write.dstBinding = 0;
        whole_buffer_write.dstArrayElement = 0;
        whole_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        whole_buffer_write.descriptorCount = 1;
        whole_buffer_write.pBufferInfo = &whole_buffer_info;
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), 1, &whole_buffer_write, 0, nullptr );
    }

    void VulkanStorageMemoryPoolBuffer::FinalizeDescriptorPool()
    {
        if ( mVkDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, nullptr );
            mVkDescriptorPool = VK_NULL_HANDLE;
        }
        mWholeBufferDescriptorSet = VK_NULL_HANDLE;
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
        if ( mDescriptorSetIndex >= kMaxDescriptorSets )
        {
            mOffset = offset;
            std::cout << LogLevel::Error << "Storage Memory Pool Buffer ran out of descriptor sets." << std::endl;
            throw std::runtime_error ( "Storage Memory Pool Buffer ran out of descriptor sets." );
        }

        // Grow the descriptor-set cache lazily; sets are reused across frames.
        if ( mDescriptorSetIndex >= mVkDescriptorSets.size() )
        {
            mVkDescriptorSets.push_back (
                CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, mVkDescriptorSetLayout ) );
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

    BufferAccessor VulkanStorageMemoryPoolBuffer::AllocateWithoutDescriptor ( size_t aSize )
    {
        size_t offset = mOffset;
        mOffset += ( ( aSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment - 1 ) ) + 1;
        if ( mOffset > mStorageBuffer.GetSize() )
        {
            mOffset = offset;
            std::cout << LogLevel::Error << "Storage Memory Pool Buffer cannot fulfill allocation request." << std::endl;
            throw std::runtime_error ( "Storage Memory Pool Buffer cannot fulfill allocation request." );
        }
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
    const VkDescriptorSet& VulkanStorageMemoryPoolBuffer::GetWholeBufferDescriptorSet() const
    {
        return mWholeBufferDescriptorSet;
    }
    const Buffer& VulkanStorageMemoryPoolBuffer::GetBuffer() const
    {
        return mStorageBuffer;
    }
}
