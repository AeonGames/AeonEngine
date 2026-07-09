/*
Copyright (C) 2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLMemoryPoolBuffer.hpp"
#include "OpenGLRenderer.hpp"

namespace AeonGames
{
    static GLint GetUniformBufferOffsetAlignment()
    {
        GLint alignment{};
        glGetIntegerv ( GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment );
        return alignment;
    }

    OpenGLMemoryPoolBuffer::OpenGLMemoryPoolBuffer ( const OpenGLRenderer&  aOpenGLRenderer, GLsizei aStackSize ) :
        mOpenGLRenderer { aOpenGLRenderer }
    {
        Initialize ( aStackSize );
    }

    OpenGLMemoryPoolBuffer::OpenGLMemoryPoolBuffer ( const OpenGLRenderer& aOpenGLRenderer ) :
        mOpenGLRenderer { aOpenGLRenderer }
    {
    }

    OpenGLMemoryPoolBuffer::OpenGLMemoryPoolBuffer ( OpenGLMemoryPoolBuffer&& aOpenGLMemoryPoolBuffer ) :
        mOpenGLRenderer { aOpenGLMemoryPoolBuffer.mOpenGLRenderer },
        mUniformBuffers { std::move ( aOpenGLMemoryPoolBuffer.mUniformBuffers ) }
    {
        std::swap ( mOffset, aOpenGLMemoryPoolBuffer.mOffset );
        std::swap ( mCurrent, aOpenGLMemoryPoolBuffer.mCurrent );
    }

    OpenGLMemoryPoolBuffer::~OpenGLMemoryPoolBuffer()
    {
        Finalize();
    }

    BufferAccessor OpenGLMemoryPoolBuffer::Allocate ( size_t aSize )
    {
        size_t offset = mOffset;
        mOffset += ( (  aSize - 1 ) | ( GetUniformBufferOffsetAlignment() - 1 ) ) + 1;
        if ( mOffset > mUniformBuffers[mCurrent].GetSize() )
        {
            mOffset = offset;
            std::cout << LogLevel::Error << "Memory Pool Buffer cannot fulfill allocation request." << std::endl;
            throw std::runtime_error ( "Memory Pool Buffer cannot fulfill allocation request." );
        }
        return BufferAccessor{this, offset, aSize};
    }

    void OpenGLMemoryPoolBuffer::Reset()
    {
        // Advance to the next physical buffer so the frame about to be recorded
        // writes into uniforms no longer read by the previous frames' in-flight
        // draws.
        mCurrent = ( mCurrent + 1 ) % kFramesInFlight;
        mOffset = 0;
    }
    const Buffer& OpenGLMemoryPoolBuffer::GetBuffer() const
    {
        return mUniformBuffers[mCurrent];
    }

    void OpenGLMemoryPoolBuffer::Initialize ( GLsizei aStackSize )
    {
        const GLsizei aligned = ( ( aStackSize - 1 ) | ( GetUniformBufferOffsetAlignment() - 1 ) ) + 1;
        for ( OpenGLBuffer& buffer : mUniformBuffers )
        {
            buffer.Initialize ( aligned, GL_DYNAMIC_DRAW );
        }
    }
    void OpenGLMemoryPoolBuffer::Finalize()
    {
        for ( OpenGLBuffer& buffer : mUniformBuffers )
        {
            buffer.Finalize();
        }
    }
}
