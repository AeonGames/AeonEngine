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
#ifndef AEONGAMES_FRAMEBUFFER_H
#define AEONGAMES_FRAMEBUFFER_H
#include <cstdint>
#include <string>
#include "aeongames/Platform.h"
namespace AeonGames
{
    class FrameBufferBuffer;
    class FrameBuffer
    {
    public:
        DLL virtual ~FrameBuffer() = 0;
        virtual void Load ( uint32_t aId ) = 0;
        virtual void Load ( const std::string& aFilename ) = 0;
        virtual void Load ( const void* aBuffer, size_t aBufferSize ) = 0;
        virtual void Load ( const FrameBufferBuffer& aFrameBufferBuffer ) = 0;
        virtual void Unload () = 0;
    };

}
#endif
