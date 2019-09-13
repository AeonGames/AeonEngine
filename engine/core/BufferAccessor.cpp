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

#include "aeongames/BufferAccessor.h"
namespace AeonGames
{
    BufferAccessor::BufferAccessor ( Buffer* aBuffer, size_t aOffset, size_t aSize ) :
        mBuffer{aBuffer}, mOffset{aOffset}, mSize{aSize}
    {}

    BufferAccessor::BufferAccessor ( const BufferAccessor& ) = default;
    BufferAccessor::BufferAccessor ( BufferAccessor&& ) = default;
    BufferAccessor& BufferAccessor::operator = ( const BufferAccessor& ) = default;
    BufferAccessor& BufferAccessor::operator = ( BufferAccessor&& ) = default;

    void BufferAccessor::WriteMemory ( size_t aOffset, size_t aSize, const void *aData ) const
    {
        return mBuffer->WriteMemory ( mOffset + aOffset, aSize, aData );
    }
    void* BufferAccessor::Map ( size_t aOffset, size_t aSize ) const
    {
        return mBuffer->Map ( mOffset + aOffset, aSize );
    }
    void BufferAccessor::Unmap() const
    {
        mBuffer->Unmap();
    }
}
