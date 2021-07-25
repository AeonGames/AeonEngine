/*
Copyright (C) 2019,2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLMEMORYPOOLBUFFER_H
#define AEONGAMES_OPENGLMEMORYPOOLBUFFER_H
#include <cstdint>
#include <string>
#include <vector>
#include <tuple>
#include <initializer_list>
#include "OpenGLFunctions.h"
#include "OpenGLBuffer.h"
#include "aeongames/MemoryPoolBuffer.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLMemoryPoolBuffer : public MemoryPoolBuffer
    {
    public:
        OpenGLMemoryPoolBuffer ( const OpenGLRenderer&  aOpenGLRenderer, GLsizei aStackSize );
        OpenGLMemoryPoolBuffer ( const OpenGLRenderer& aOpenGLRenderer );
        OpenGLMemoryPoolBuffer ( OpenGLMemoryPoolBuffer&& );
        OpenGLMemoryPoolBuffer ( const OpenGLMemoryPoolBuffer& ) = delete;
        OpenGLMemoryPoolBuffer& operator= ( const OpenGLMemoryPoolBuffer& ) = delete;
        OpenGLMemoryPoolBuffer& operator = ( OpenGLMemoryPoolBuffer&& ) = delete;
        ~OpenGLMemoryPoolBuffer();
        BufferAccessor Allocate ( size_t aSize ) final;
        void Reset() final;
        const Buffer & GetBuffer() const final;
        void Initialize ( GLsizei aStackSize );
        void Finalize();
    private:
        const OpenGLRenderer&  mOpenGLRenderer;
        size_t mOffset{0};
        OpenGLBuffer mUniformBuffer;
    };
}
#endif
