/*
Copyright (C) 2018,2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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

#ifndef AEONGAMES_BUFFER_H
#define AEONGAMES_BUFFER_H
#include "aeongames/Platform.hpp"
namespace AeonGames
{
    /** @brief Abstract interface for GPU/memory buffer operations. */
    class Buffer
    {
    public:
        virtual ~Buffer() = default;
        /**
         * @brief Write data into the buffer at a given offset.
         * @param aOffset Byte offset into the buffer to begin writing.
         * @param aSize   Number of bytes to write.
         * @param aData   Pointer to the source data, or nullptr to clear.
         */
        virtual void WriteMemory ( size_t aOffset, size_t aSize, const void *aData = nullptr ) const = 0;
        /**
         * @brief Map a region of the buffer into CPU-accessible memory.
         * @param aOffset Byte offset of the region to map.
         * @param aSize   Number of bytes to map.
         * @return Pointer to the mapped memory region.
         */
        virtual void* Map ( size_t aOffset, size_t aSize ) const = 0;
        /** @brief Unmap a previously mapped buffer region. */
        virtual void Unmap() const = 0;
        /** @brief Get the total size of the buffer in bytes. @return Buffer size in bytes. */
        virtual size_t GetSize() const = 0;
    };
}
#endif
