/*
Copyright (C) 2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MEMORYPOOLBUFFER_H
#define AEONGAMES_MEMORYPOOLBUFFER_H
#include <cstdint>
#include <vector>
#include "aeongames/Platform.hpp"
#include "aeongames/BufferAccessor.hpp"

namespace AeonGames
{
    class Buffer;
    /** @brief Number of frames the CPU may record ahead of the GPU.
     *
     * Shared by both backends so per-frame resources (uniform/storage pools,
     * per-window buffers, sync objects) are ring-buffered a consistent number
     * of ways. Reusing a single physical buffer every frame lets the next
     * frame's CPU writes race the previous frame's still-in-flight GPU reads;
     * advancing through @ref kFramesInFlight distinct copies removes the hazard
     * without relying on driver buffer-renaming. Three keeps the GPU fed while
     * bounding latency and per-window memory. */
    constexpr uint32_t kFramesInFlight = 3;
    /** @brief Abstract interface for a pool-based buffer allocator. */
    class MemoryPoolBuffer
    {
    public:
        /**
         * @brief Allocate a sub-region from the memory pool.
         * @param aSize Number of bytes to allocate.
         * @return A BufferAccessor describing the allocated region.
         */
        virtual BufferAccessor Allocate ( size_t aSize ) = 0;
        /** @brief Reset the pool, freeing all previous allocations. */
        virtual void Reset() = 0;
        virtual ~MemoryPoolBuffer() = default;
        /** @brief Get a reference to the underlying Buffer. @return Const reference to the buffer. */
        virtual const Buffer& GetBuffer() const = 0;
    };
}
#endif
