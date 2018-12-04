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
#ifndef AEONGAMES_VULKANUNIFORMBUFFER_H
#define AEONGAMES_VULKANUNIFORMBUFFER_H
#include "aeongames/UniformBuffer.h"
#include "VulkanBuffer.h"
namespace AeonGames
{
    class VulkanUniformBuffer : public UniformBuffer
    {
    public:
        VulkanUniformBuffer ( const VulkanRenderer& aVulkanRenderer, const VkDeviceSize aSize, const void *aData = nullptr );
        /// No Copy contsructor.
        VulkanUniformBuffer ( const VulkanUniformBuffer& aBuffer ) = delete;
        /// No move allowed
        VulkanUniformBuffer ( VulkanUniformBuffer&& ) = delete;
        /// No Assignment operator
        VulkanUniformBuffer& operator= ( const VulkanUniformBuffer& aBuffer ) = delete;
        /// No move assignment allowed
        VulkanUniformBuffer& operator = ( VulkanUniformBuffer&& ) = delete;
        const VkBuffer& GetBuffer() const;
        const VkDescriptorSet& GetDescriptorSet() const;
        /**@ name Overriden Functions */
        ///@{
        ~VulkanUniformBuffer() final;
        void WriteMemory ( size_t aOffset, size_t aSize, const void *aData = nullptr ) const final;
        void* Map ( size_t aOffset, size_t aSize ) const final;
        void Unmap() const final;
        size_t GetSize() const final;
        ///@}
    private:
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        void InitializeDescriptorSet ( VkDescriptorSet& aVkDescriptorSet, const VkDescriptorSetLayout& aVkDescriptorSetLayout, const VkDescriptorBufferInfo& aVkDescriptorBufferInfo );
        void FinalizeDescriptorSet ( VkDescriptorSet& aVkDescriptorSet );
        const VulkanRenderer& mVulkanRenderer;
        VulkanBuffer mBuffer;
        VkDescriptorPool mVkDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSet mVkDescriptorSet{ VK_NULL_HANDLE };
    };
}
#endif