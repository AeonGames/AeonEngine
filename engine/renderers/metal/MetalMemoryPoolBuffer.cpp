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
#include <stdexcept>
#include "MetalBuffer.h"
#include "MetalMemoryPoolBuffer.h"

namespace AeonGames
{
    namespace
    {
        size_t AlignUp ( size_t aValue, size_t aAlignment )
        {
            return ( aValue + aAlignment - 1 ) & ~ ( aAlignment - 1 );
        }
    }

    MetalMemoryPoolBuffer::MetalMemoryPoolBuffer ( void* aDevice, size_t aCapacity, size_t aAlignment ) :
        mBuffer{std::make_unique<MetalBuffer> ( aDevice, aCapacity ) },
        mAlignment{aAlignment}
    {
        if ( aAlignment == 0 || ( aAlignment & ( aAlignment - 1 ) ) != 0 )
        {
            throw std::invalid_argument ( "Metal memory-pool alignment must be a power of two" );
        }
    }

    MetalMemoryPoolBuffer::~MetalMemoryPoolBuffer() = default;
    MetalMemoryPoolBuffer::MetalMemoryPoolBuffer ( MetalMemoryPoolBuffer&& ) noexcept = default;
    MetalMemoryPoolBuffer& MetalMemoryPoolBuffer::operator= ( MetalMemoryPoolBuffer&& ) noexcept = default;

    BufferAccessor MetalMemoryPoolBuffer::Allocate ( size_t aSize )
    {
        if ( aSize == 0 )
        {
            return {};
        }
        const size_t offset = AlignUp ( mOffset, mAlignment );
        if ( offset > mBuffer->GetSize() || aSize > mBuffer->GetSize() - offset )
        {
            throw std::runtime_error ( "Metal frame memory pool cannot fulfill allocation request" );
        }
        mOffset = offset + aSize;
        return BufferAccessor {this, offset, aSize};
    }

    void MetalMemoryPoolBuffer::Reset()
    {
        mOffset = 0;
    }

    const Buffer& MetalMemoryPoolBuffer::GetBuffer() const
    {
        return *mBuffer;
    }
}