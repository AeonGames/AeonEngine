/*
Copyright (C) 2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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

#include "aeongames/BufferAccessor.hpp"
#include "aeongames/Buffer.hpp"
#include "aeongames/MemoryPoolBuffer.hpp"

namespace AeonGames
{
    BufferAccessor::BufferAccessor ( MemoryPoolBuffer* aMemoryPoolBuffer, size_t aOffset, size_t aSize ) :
        mMemoryPoolBuffer{aMemoryPoolBuffer}, mOffset{aOffset}, mSize{aSize}
    {}

    BufferAccessor::BufferAccessor () = default;
    BufferAccessor::BufferAccessor ( const BufferAccessor& ) = default;
    BufferAccessor::BufferAccessor ( BufferAccessor&& ) = default;
    BufferAccessor& BufferAccessor::operator = ( const BufferAccessor& ) = default;
    BufferAccessor& BufferAccessor::operator = ( BufferAccessor&& ) = default;

    void BufferAccessor::WriteMemory ( size_t aOffset, size_t aSize, const void *aData ) const
    {
        if ( mMemoryPoolBuffer != nullptr )
        {
            mMemoryPoolBuffer->GetBuffer().WriteMemory ( mOffset + aOffset, aSize, aData );
        }
    }
    void* BufferAccessor::Map ( size_t aOffset, size_t aSize ) const
    {
        aSize = ( aSize != 0 ) ? aSize : mSize;
        return ( mMemoryPoolBuffer != nullptr ) ? mMemoryPoolBuffer->GetBuffer().Map ( mOffset + aOffset, aSize ) : nullptr;
    }
    void BufferAccessor::Unmap() const
    {
        if ( mMemoryPoolBuffer != nullptr )
        {
            mMemoryPoolBuffer->GetBuffer().Unmap();
        }
    }
    size_t BufferAccessor::GetOffset() const
    {
        return mOffset;
    }
    size_t BufferAccessor::GetSize() const
    {
        return mSize;
    }
    const MemoryPoolBuffer* BufferAccessor::GetMemoryPoolBuffer() const
    {
        return mMemoryPoolBuffer;
    }
}
