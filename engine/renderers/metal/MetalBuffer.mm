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
#import <Metal/Metal.h>

#include <cstring>
#include <stdexcept>
#include "MetalBuffer.h"

namespace AeonGames
{
    class MetalBuffer::Impl
    {
    public:
        Impl ( id<MTLDevice> aDevice, size_t aSize ) : mSize{aSize}
        {
            if ( aSize == 0 )
            {
                throw std::runtime_error ( "MetalBuffer size must be greater than zero" );
            }
            mBuffer = [aDevice newBufferWithLength:aSize options:MTLResourceStorageModeShared];
            if ( mBuffer == nil )
            {
                throw std::runtime_error ( "MetalBuffer allocation failed" );
            }
        }

        id<MTLBuffer> mBuffer{nil};
        size_t mSize{0};
    };

    MetalBuffer::MetalBuffer ( void* aDevice, size_t aSize ) :
        mImpl{std::make_unique<Impl> ( ( __bridge id<MTLDevice> ) aDevice, aSize ) }
    {
    }

    MetalBuffer::~MetalBuffer() = default;
    MetalBuffer::MetalBuffer ( MetalBuffer&& ) noexcept = default;
    MetalBuffer& MetalBuffer::operator= ( MetalBuffer&& ) noexcept = default;

    void MetalBuffer::WriteMemory ( size_t aOffset, size_t aSize, const void* aData ) const
    {
        if ( aOffset > mImpl->mSize || aSize > mImpl->mSize - aOffset )
    {
        throw std::out_of_range ( "MetalBuffer write exceeds its allocation" );
        }
        void* destination = static_cast<uint8_t*> ( mImpl->mBuffer.contents ) + aOffset;
        if ( aData != nullptr )
    {
        std::memcpy ( destination, aData, aSize );
        }
        else
        {
            std::memset ( destination, 0, aSize );
        }
    }

    void* MetalBuffer::Map ( size_t aOffset, size_t aSize ) const
    {
        const size_t mapped_size = aSize == 0 ? mImpl->mSize - aOffset : aSize;
        if ( aOffset > mImpl->mSize || mapped_size > mImpl->mSize - aOffset )
        {
            return nullptr;
        }
        return static_cast<uint8_t*> ( mImpl->mBuffer.contents ) + aOffset;
    }

    void MetalBuffer::Unmap() const
    {
        // Shared unified-memory buffers stay persistently mapped.
    }

    size_t MetalBuffer::GetSize() const
    {
        return mImpl->mSize;
    }

    void* MetalBuffer::GetNativeBuffer() const
    {
        return ( __bridge void* ) mImpl->mBuffer;
    }
}