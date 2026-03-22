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

#ifndef AEONGAMES_BUFFERACCESSOR_H
#define AEONGAMES_BUFFERACCESSOR_H
#include "aeongames/Platform.hpp"
namespace AeonGames
{
    class MemoryPoolBuffer;
    /** @brief Provides access to a region within a memory pool buffer. */
    class BufferAccessor
    {
    public:
        /** @brief Default constructor. Creates an empty accessor. */
        DLL BufferAccessor ();
        /** @brief Construct an accessor for a specific region of a memory pool buffer.
         *  @param aMemoryPoolBuffer Pointer to the memory pool buffer.
         *  @param aOffset Byte offset into the buffer.
         *  @param aSize Size of the accessible region in bytes. */
        DLL BufferAccessor ( MemoryPoolBuffer* aMemoryPoolBuffer, size_t aOffset, size_t aSize );
        /** @brief Copy constructor. */
        DLL BufferAccessor ( const BufferAccessor& );
        /** @brief Move constructor. */
        DLL BufferAccessor ( BufferAccessor&& );
        /** @brief Copy assignment operator. */
        DLL BufferAccessor& operator= ( const BufferAccessor& );
        /** @brief Move assignment operator. */
        DLL BufferAccessor& operator = ( BufferAccessor&& );

        /** @brief Write data into the buffer region.
         *  @param aOffset Byte offset relative to the accessor's region.
         *  @param aSize Size of the data to write in bytes.
         *  @param aData Pointer to the source data, or nullptr to zero-fill. */
        DLL void WriteMemory ( size_t aOffset, size_t aSize, const void *aData = nullptr ) const;
        /** @brief Map the buffer region into host-accessible memory.
         *  @param aOffset Byte offset relative to the accessor's region.
         *  @param aSize Size of the region to map (0 for entire region).
         *  @return Pointer to the mapped memory. */
        DLL void* Map ( size_t aOffset = 0, size_t aSize = 0 ) const;
        /** @brief Unmap a previously mapped buffer region. */
        DLL void Unmap() const;
        /** @brief Get the byte offset of this accessor within the memory pool buffer.
         *  @return Offset in bytes. */
        DLL size_t GetOffset() const;
        /** @brief Get the size of the accessible region.
         *  @return Size in bytes. */
        DLL size_t GetSize() const;
        /** @brief Get the underlying memory pool buffer.
         *  @return Pointer to the memory pool buffer. */
        DLL const MemoryPoolBuffer* GetMemoryPoolBuffer() const;
    private:
        MemoryPoolBuffer* mMemoryPoolBuffer{nullptr};
        size_t mOffset{0};
        size_t mSize{0};
    };
}
#endif
