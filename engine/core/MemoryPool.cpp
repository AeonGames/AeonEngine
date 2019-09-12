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
#include "aeongames/MemoryPool.h"
namespace AeonGames
{
    MemoryPool::MemoryPool ( size_t aBlockSize, size_t aNumOfBlocks ) :
        mBlockSize{aBlockSize},
        mNumOfBlocks{aNumOfBlocks},
        mNumOfFreeBlocks{aNumOfBlocks},
        mNumOfInitializedBlocks{0},
        mMemory ( static_cast<std::vector<uint8_t>::size_type> ( mBlockSize * mNumOfBlocks ) ),
        mNext{mMemory.data() }
    {
    }

    uint8_t * MemoryPool::AddrFromIndex ( size_t i ) const
    {
        return const_cast<uint8_t *> ( mMemory.data() ) + ( i * mBlockSize );
    }

    size_t MemoryPool::IndexFromAddr ( const uint8_t* p ) const
    {
        return ( ( ( size_t ) ( p - mMemory.data() ) ) / mBlockSize );
    }

    void* MemoryPool::Allocate()
    {
        if ( mNumOfInitializedBlocks < mNumOfBlocks )
        {
            size_t* p = ( size_t* ) AddrFromIndex ( mNumOfInitializedBlocks );
            *p = mNumOfInitializedBlocks + 1;
            mNumOfInitializedBlocks++;
        }
        void* ret = nullptr;
        if ( mNumOfFreeBlocks > 0 )
        {
            ret = ( void* ) mNext;
            --mNumOfFreeBlocks;
            if ( mNumOfFreeBlocks != 0 )
            {
                mNext = AddrFromIndex ( * ( ( size_t* ) mNext ) );
            }
            else
            {
                mNext = nullptr;
            }
        }
        return ret;
    }

    void MemoryPool::DeAllocate ( void* p )
    {
        if ( mNext != nullptr )
        {
            ( * ( size_t* ) p ) = IndexFromAddr ( mNext );
            mNext = ( uint8_t* ) p;
        }
        else
        {
            * ( ( size_t* ) p ) = mNumOfBlocks;
            mNext = ( uint8_t* ) p;
        }
        ++mNumOfFreeBlocks;
    }
}
