/*
Copyright (C) 2018,2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Platform.h"
namespace AeonGames
{
    class Buffer
    {
    public:
        virtual ~Buffer() = default;
        virtual void WriteMemory ( size_t aOffset, size_t aSize, const void *aData = nullptr ) const = 0;
        virtual void* Map ( size_t aOffset, size_t aSize ) const = 0;
        virtual void Unmap() const = 0;
        virtual size_t GetSize() const = 0;
    };
}
#endif
