/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#include <sstream>
#include "VulkanUniformBuffer.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanUniformBuffer::VulkanUniformBuffer ( const VulkanRenderer& aVulkanRenderer, const VkDeviceSize aSize, const void *aData ) :
        mVulkanRenderer{aVulkanRenderer},
        mBuffer{aVulkanRenderer, aSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, aData}
    {
        InitializeDescriptorPool();
        InitializeDescriptorSet ( mVkDescriptorSet, mVulkanRenderer.GetUniformBufferDescriptorSetLayout(), VkDescriptorBufferInfo{mBuffer.GetBuffer(), 0, aSize} );
    }

    const VkBuffer& VulkanUniformBuffer::GetBuffer() const
    {
        return mBuffer.GetBuffer();
    }

    const VkDescriptorSet& VulkanUniformBuffer::GetDescriptorSet() const
    {
        return mVkDescriptorSet;
    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        FinalizeDescriptorSet ( mVkDescriptorSet );
        FinalizeDescriptorPool();
    }

    void* VulkanUniformBuffer::Map ( size_t aOffset, size_t aSize ) const
    {
        return mBuffer.Map ( aOffset, aSize );
    }
    void VulkanUniformBuffer::Unmap() const
    {
        return mBuffer.Unmap();
    }
    size_t VulkanUniformBuffer::GetSize() const
    {
        return mBuffer.GetSize();
    }
    void VulkanUniformBuffer::WriteMemory ( size_t aOffset, size_t aSize, const void *aData ) const
    {
        mBuffer.WriteMemory ( aOffset, aSize, aData );
    }

    void VulkanUniformBuffer::InitializeDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 1> descriptor_pool_sizes{};
        descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_pool_sizes[0].descriptorCount = 1;
        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.pNext = nullptr;
        descriptor_pool_create_info.flags = 0;
        descriptor_pool_create_info.maxSets = 1;
        descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t> ( descriptor_pool_sizes.size() );
        descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();

        if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer.GetDevice(), &descriptor_pool_create_info, nullptr, &mVkDescriptorPool ) )
        {
            std::ostringstream stream;
            stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanUniformBuffer::FinalizeDescriptorPool()
    {
        if ( mVkDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, nullptr );
            mVkDescriptorPool = VK_NULL_HANDLE;
        }
    }

    void VulkanUniformBuffer::InitializeDescriptorSet ( VkDescriptorSet& aVkDescriptorSet, const VkDescriptorSetLayout& aVkDescriptorSetLayout, const VkDescriptorBufferInfo& aVkDescriptorBufferInfo )
    {
        std::array<VkDescriptorSetLayout, 1> descriptor_set_layouts{ { aVkDescriptorSetLayout } };
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount = static_cast<uint32_t> ( descriptor_set_layouts.size() );
        descriptor_set_allocate_info.pSetLayouts = descriptor_set_layouts.data();

        if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &aVkDescriptorSet ) )
        {
            std::ostringstream stream;
            stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        std::array<VkWriteDescriptorSet, 1> write_descriptor_sets{};
        write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_sets[0].pNext = nullptr;
        write_descriptor_sets[0].dstSet = aVkDescriptorSet;
        write_descriptor_sets[0].dstBinding = 0;
        write_descriptor_sets[0].dstArrayElement = 0;
        write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_sets[0].descriptorCount = 1;
        write_descriptor_sets[0].pBufferInfo = &aVkDescriptorBufferInfo;
        write_descriptor_sets[0].pImageInfo = nullptr;
        write_descriptor_sets[0].pTexelBufferView = nullptr;
        vkUpdateDescriptorSets (
            mVulkanRenderer.GetDevice(),
            static_cast<uint32_t> ( write_descriptor_sets.size() ),
            write_descriptor_sets.data(), 0, nullptr );
    }

    void VulkanUniformBuffer::FinalizeDescriptorSet ( VkDescriptorSet& aVkDescriptorSet )
    {
        if ( aVkDescriptorSet != VK_NULL_HANDLE )
        {
            vkFreeDescriptorSets ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, 1, &aVkDescriptorSet );
            aVkDescriptorSet = VK_NULL_HANDLE;
        }
    }

}