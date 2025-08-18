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
#include <fstream>
#include <ostream>
#include <regex>
#include <array>
#include <utility>
#include <cassert>
#include "aeongames/AeonEngine.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Vector2.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Vector4.hpp"
#include "OpenGLMemoryPoolBuffer.h"
#include "OpenGLRenderer.h"

namespace AeonGames
{
    static GLint GetUniformBufferOffsetAlignment()
    {
        GLint alignment{};
        glGetIntegerv ( GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment );
        return alignment;
    }

    OpenGLMemoryPoolBuffer::OpenGLMemoryPoolBuffer ( const OpenGLRenderer&  aOpenGLRenderer, GLsizei aStackSize ) :
        mOpenGLRenderer { aOpenGLRenderer },
        mUniformBuffer { ( ( aStackSize - 1 ) | ( GetUniformBufferOffsetAlignment() - 1 ) ) + 1, GL_DYNAMIC_DRAW }
    {
    }

    OpenGLMemoryPoolBuffer::OpenGLMemoryPoolBuffer ( const OpenGLRenderer& aOpenGLRenderer ) :
        mOpenGLRenderer { aOpenGLRenderer },
        mUniformBuffer {}
    {
    }

    OpenGLMemoryPoolBuffer::OpenGLMemoryPoolBuffer ( OpenGLMemoryPoolBuffer&& aOpenGLMemoryPoolBuffer ) :
        mOpenGLRenderer { aOpenGLMemoryPoolBuffer.mOpenGLRenderer },
        mUniformBuffer { std::move ( aOpenGLMemoryPoolBuffer.mUniformBuffer ) }
    {
        std::swap ( mOffset, aOpenGLMemoryPoolBuffer.mOffset );
    }

    OpenGLMemoryPoolBuffer::~OpenGLMemoryPoolBuffer()
    {
        Finalize();
    }

    BufferAccessor OpenGLMemoryPoolBuffer::Allocate ( size_t aSize )
    {
        size_t offset = mOffset;
        mOffset += ( (  aSize - 1 ) | ( GetUniformBufferOffsetAlignment() - 1 ) ) + 1;
        if ( mOffset > mUniformBuffer.GetSize() )
        {
            mOffset = offset;
            throw std::runtime_error ( "Memory Pool Buffer cannot fulfill allocation request." );
        }
        return BufferAccessor{this, offset, aSize};
    }

    void OpenGLMemoryPoolBuffer::Reset()
    {
        mOffset = 0;
    }
    const Buffer& OpenGLMemoryPoolBuffer::GetBuffer() const
    {
        return mUniformBuffer;
    }

    void OpenGLMemoryPoolBuffer::Initialize ( GLsizei aStackSize )
    {
        mUniformBuffer.Initialize ( ( ( aStackSize - 1 ) | ( GetUniformBufferOffsetAlignment() - 1 ) ) + 1, GL_DYNAMIC_DRAW );
    }
    void OpenGLMemoryPoolBuffer::Finalize()
    {
        mUniformBuffer.Finalize();
    }
}
