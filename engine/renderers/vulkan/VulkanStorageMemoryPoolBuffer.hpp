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
#include <vector>
#include <unordered_map>
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
        /** @brief Get the descriptor set bound to the allocation at @p aOffset.
         *
         * Each allocation owns a descriptor set whose buffer range is exactly
         * the allocation, so it is bound with a zero dynamic offset. This keeps
         * @c (effectiveOffset + range) within the pool buffer for every
         * allocation, unlike a single shared whole-buffer descriptor. */
        const VkDescriptorSet& GetDescriptorSet ( size_t aOffset ) const;
        BufferAccessor Allocate ( size_t aSize ) final;
        void Reset() final;
        const Buffer& GetBuffer() const final;
    private:
        /// @brief Number of descriptor sets allocated from each backing pool.
        ///        Pools are added on demand, so the live-allocation count per
        ///        frame is not capped by a single pool's capacity.
        static constexpr uint32_t kDescriptorSetsPerPool = 256;
        void InitializeDescriptorPool();
        void FinalizeDescriptorPool();
        /// @brief Append a fresh descriptor pool to @ref mVkDescriptorPools.
        void AddDescriptorPool();
        const VulkanRenderer& mVulkanRenderer;
        size_t mOffset{0};
        std::vector<VkDescriptorPool> mVkDescriptorPools{};
        VkDescriptorSetLayout mVkDescriptorSetLayout{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> mVkDescriptorSets{};
        uint32_t mDescriptorSetIndex{0};
        std::unordered_map<size_t, VkDescriptorSet> mOffsetToDescriptorSet{};
        VulkanBuffer mStorageBuffer;
    };
}
#endif
