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

#ifndef AEONGAMES_BUFFERACCESSOR_H
#define AEONGAMES_BUFFERACCESSOR_H
#include "aeongames/Buffer.h"
namespace AeonGames
{
    class BufferAccessor
    {
    public:
        DLL BufferAccessor ( Buffer* mBuffer, size_t aOffset, size_t aSize );
        DLL BufferAccessor ( const BufferAccessor& );
        DLL BufferAccessor ( BufferAccessor&& );
        DLL BufferAccessor& operator= ( const BufferAccessor& );
        DLL BufferAccessor& operator = ( BufferAccessor&& );

        DLL void WriteMemory ( size_t aOffset, size_t aSize, const void *aData = nullptr ) const;
        DLL void* Map ( size_t aOffset, size_t aSize ) const;
        DLL void Unmap() const;
    private:
        Buffer* mBuffer;
        size_t mOffset;
        size_t mSize;
    };
}
#endif
