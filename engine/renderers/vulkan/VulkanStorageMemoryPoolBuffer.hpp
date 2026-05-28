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
#ifndef AEONGAMES_VULKANSTORAGEMEMORYPOOLBUFFER_HPP
#define AEONGAMES_VULKANSTORAGEMEMORYPOOLBUFFER_HPP
#include <cstdint>
#include <vulkan/vulkan.h>
#include "VulkanBuffer.hpp"
#include "aeongames/MemoryPoolBuffer.hpp"

namespace AeonGames
{
    class VulkanRenderer;
    /** @brief Vulkan memory pool buffer for transient per-frame storage-buffer (SSBO) allocations.
     *
     * Sibling of `VulkanMemoryPoolBuffer`. Differs in three places:
     *   - buffer usage flag: `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT`
     *   - descriptor type:   `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC`
     *   - alignment query:   `minStorageBufferOffsetAlignment`
     */
    class VulkanStorageMemoryPoolBuffer : public MemoryPoolBuffer
    {
    public:
        /// @brief Construct with a renderer and initial pool size.
        VulkanStorageMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer, size_t aStackSize );
        /// @brief Construct with a renderer, leaving the pool uninitialized.
        VulkanStorageMemoryPoolBuffer ( const VulkanRenderer& );
        /// @brief Move constructor.
        VulkanStorageMemoryPoolBuffer ( VulkanStorageMemoryPoolBuffer&& );
        VulkanStorageMemoryPoolBuffer& operator= ( const VulkanStorageMemoryPoolBuffer& ) = delete;
        VulkanStorageMemoryPoolBuffer& operator = ( VulkanStorageMemoryPoolBuffer&& ) = delete;
        ~VulkanStorageMemoryPoolBuffer();
        /// @brief Initialize the pool buffer with the given size.
        void Initialize ( size_t aStackSize );
        /// @brief Release pool buffer and descriptor resources.
        void Finalize();
        /// @brief Get the Vulkan descriptor set for this pool buffer.
        const VkDescriptorSet& GetDescriptorSet() const;
        BufferAccessor Allocate ( size_t aSize ) final;
        void Reset() final;
        const Buffer& GetBuffer() const final;
    private:
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        void InitializeDescriptorSet();
        const VulkanRenderer& mVulkanRenderer;
        size_t mOffset{0};
        VkDescriptorPool mVkDescriptorPool{ VK_NULL_HANDLE };
        VkDescriptorSet mVkDescriptorSet{ VK_NULL_HANDLE };
        VulkanBuffer mStorageBuffer;
    };
}
#endif
