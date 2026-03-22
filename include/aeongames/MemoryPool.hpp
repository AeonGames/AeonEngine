/*
Copyright (C) 2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MEMORYPOOL_H
#define AEONGAMES_MEMORYPOOL_H
#include <cstdint>
#include <vector>
#include "aeongames/Platform.hpp"

namespace AeonGames
{
    /** @brief User-defined literal for kilobytes.
     *  @param aKyloBytes Value in kilobytes.
     *  @return Value in bytes.
     */
    constexpr const std::size_t operator ""_kb ( unsigned long long int aKyloBytes )
    {
        return aKyloBytes * 1024;
    }
    /** @brief User-defined literal for megabytes.
     *  @param aMegaBytes Value in megabytes.
     *  @return Value in bytes.
     */
    constexpr const std::size_t operator ""_mb ( unsigned long long int aMegaBytes )
    {
        return aMegaBytes * 1024_kb;
    }
    /** @brief User-defined literal for gigabytes.
     *  @param aGigaBytes Value in gigabytes.
     *  @return Value in bytes.
     */
    constexpr const std::size_t operator ""_gb ( unsigned long long int aGigaBytes )
    {
        return aGigaBytes * 1024_mb;
    }
    /**
     This is a modification of the implementation in the
     "Fast Efficient Fixed-Size Memory Pool" paper
     by Ben Kenwright
    */
    class MemoryPool
    {
    public:
        /** @brief Construct a memory pool.
         *  @param sizeOfEachBlock Size of each memory block in bytes.
         *  @param numOfBlocks Number of blocks to pre-allocate.
         */
        DLL MemoryPool ( size_t sizeOfEachBlock, size_t numOfBlocks );
        /** @brief Allocate a block from the pool. @return Pointer to the allocated block. */
        DLL void* Allocate();
        /** @brief Return a block to the pool. @param p Pointer to the block to deallocate. */
        DLL void DeAllocate ( void* p );
    private:
        uint8_t * AddrFromIndex ( size_t i ) const;
        size_t IndexFromAddr ( const uint8_t* p ) const;
        size_t mBlockSize;
        size_t mNumOfBlocks;
        size_t mNumOfFreeBlocks;
        size_t mNumOfInitializedBlocks;
        std::vector<uint8_t> mMemory;
        uint8_t* mNext;
    };
}
#endif
