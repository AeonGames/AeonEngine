/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Platform.h"

namespace AeonGames
{
    constexpr const std::size_t operator "" _kb ( std::size_t aKyloBytes )
    {
        return aKyloBytes * 1024;
    }
    constexpr const std::size_t operator "" _mb ( std::size_t aMegaBytes )
    {
        return aMegaBytes * 1024_kb;
    }
    constexpr const std::size_t operator "" _gb ( std::size_t aGigaBytes )
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
        DLL MemoryPool ( size_t sizeOfEachBlock, size_t numOfBlocks );
        DLL void* Allocate();
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
