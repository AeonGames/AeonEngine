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
#include <iostream>
#include <stdexcept>
#include <utility>
#include "aeongames/LogLevel.hpp"
#include "OpenGLStorageMemoryPoolBuffer.hpp"
#include "OpenGLRenderer.hpp"

namespace AeonGames
{
    static GLint GetStorageBufferOffsetAlignment()
    {
        GLint alignment{};
        glGetIntegerv ( GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &alignment );
        return alignment;
    }

    OpenGLStorageMemoryPoolBuffer::OpenGLStorageMemoryPoolBuffer ( const OpenGLRenderer&  aOpenGLRenderer, GLsizei aStackSize ) :
        mOpenGLRenderer { aOpenGLRenderer },
        mStorageBuffer { ( ( aStackSize - 1 ) | ( GetStorageBufferOffsetAlignment() - 1 ) ) + 1, GL_DYNAMIC_DRAW }
    {
    }

    OpenGLStorageMemoryPoolBuffer::OpenGLStorageMemoryPoolBuffer ( const OpenGLRenderer& aOpenGLRenderer ) :
        mOpenGLRenderer { aOpenGLRenderer },
        mStorageBuffer {}
    {
    }

    OpenGLStorageMemoryPoolBuffer::OpenGLStorageMemoryPoolBuffer ( OpenGLStorageMemoryPoolBuffer&& aOpenGLStorageMemoryPoolBuffer ) :
        mOpenGLRenderer { aOpenGLStorageMemoryPoolBuffer.mOpenGLRenderer },
        mStorageBuffer { std::move ( aOpenGLStorageMemoryPoolBuffer.mStorageBuffer ) }
    {
        std::swap ( mOffset, aOpenGLStorageMemoryPoolBuffer.mOffset );
    }

    OpenGLStorageMemoryPoolBuffer::~OpenGLStorageMemoryPoolBuffer()
    {
        Finalize();
    }

    BufferAccessor OpenGLStorageMemoryPoolBuffer::Allocate ( size_t aSize )
    {
        size_t offset = mOffset;
        mOffset += ( ( aSize - 1 ) | ( GetStorageBufferOffsetAlignment() - 1 ) ) + 1;
        if ( mOffset > mStorageBuffer.GetSize() )
        {
            mOffset = offset;
            std::cout << LogLevel::Error << "Storage Memory Pool Buffer cannot fulfill allocation request." << std::endl;
            throw std::runtime_error ( "Storage Memory Pool Buffer cannot fulfill allocation request." );
        }
        return BufferAccessor{this, offset, aSize};
    }

    void OpenGLStorageMemoryPoolBuffer::Reset()
    {
        mOffset = 0;
    }
    const Buffer& OpenGLStorageMemoryPoolBuffer::GetBuffer() const
    {
        return mStorageBuffer;
    }

    void OpenGLStorageMemoryPoolBuffer::Initialize ( GLsizei aStackSize )
    {
        mStorageBuffer.Initialize ( ( ( aStackSize - 1 ) | ( GetStorageBufferOffsetAlignment() - 1 ) ) + 1, GL_DYNAMIC_DRAW );
    }
    void OpenGLStorageMemoryPoolBuffer::Finalize()
    {
        mStorageBuffer.Finalize();
    }
}
