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
        VkDescriptorPoolSize descriptor_pool_size{};
        descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_pool_size.descriptorCount = 1;
        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.pNext = nullptr;
        descriptor_pool_create_info.flags = 0;
        descriptor_pool_create_info.maxSets = 1;
        descriptor_pool_create_info.poolSizeCount = 1;
        descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;

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
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount = 1;
        descriptor_set_allocate_info.pSetLayouts = &aVkDescriptorSetLayout;

        if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &aVkDescriptorSet ) )
        {
            std::ostringstream stream;
            stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.pNext = nullptr;
        write_descriptor_set.dstSet = aVkDescriptorSet;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pBufferInfo = &aVkDescriptorBufferInfo;
        write_descriptor_set.pImageInfo = nullptr;
        write_descriptor_set.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets (
            mVulkanRenderer.GetDevice(),
            1,
            &write_descriptor_set, 0, nullptr );
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