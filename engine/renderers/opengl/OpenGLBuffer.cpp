/*
Copyright (C) 2018,2019,2021,2026 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <sstream>
#include <limits>
#include "OpenGLRenderer.hpp"
#include "OpenGLBuffer.hpp"
#include "OpenGLFunctions.hpp"

namespace AeonGames
{
    OpenGLBuffer::OpenGLBuffer (  ) = default;
    OpenGLBuffer::OpenGLBuffer ( const GLsizei aSize, const GLenum aUsage, const void *aData ) :
        mSize{aSize}, mUsage {aUsage}
    {
        try
        {
            Initialize ( aData );
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    OpenGLBuffer::~OpenGLBuffer()
    {
        Finalize();
    }

    OpenGLBuffer::OpenGLBuffer ( OpenGLBuffer&& aBuffer )
    {
        std::swap ( mBuffer, aBuffer.mBuffer );
        std::swap ( mSize, aBuffer.mSize );
        std::swap ( mUsage, aBuffer.mUsage );
    }

    GLuint OpenGLBuffer::GetBufferId() const
    {
        return mBuffer;
    }

    void OpenGLBuffer::Initialize ( const GLsizei aSize, const GLenum aUsage, const void * aData )
    {
        mSize = aSize;
        mUsage = aUsage;
        Initialize ( aData );
    }

    void OpenGLBuffer::WriteMemory ( const size_t aOffset, const size_t aSize, const void * aData ) const
    {
        if ( ( glIsBuffer ( mBuffer ) ) && ( aData ) )
        {
            glNamedBufferSubData ( mBuffer,
                                   aOffset,
                                   aSize,
                                   aData );
            OPENGL_CHECK_ERROR_THROW;
        }
    }

    void * OpenGLBuffer::Map ( const GLbitfield aAccess ) const
    {
        void* data = glMapNamedBuffer ( mBuffer, aAccess );
        OPENGL_CHECK_ERROR_THROW;
        return data;
    }

    void* OpenGLBuffer::Map ( size_t aOffset, size_t aSize ) const
    {
        return MapRange ( aOffset, aSize, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT );
    }
    void * OpenGLBuffer::MapRange ( const GLintptr aOffset, const GLsizeiptr aSize, const GLbitfield aAccess ) const
    {
        void* data = glMapNamedBufferRange ( mBuffer, aOffset, aSize, aAccess );
        OPENGL_CHECK_ERROR_THROW;
        return data;
    }

    void OpenGLBuffer::Unmap() const
    {
        glUnmapNamedBuffer ( mBuffer );
        OPENGL_CHECK_ERROR_THROW;
    }

    size_t OpenGLBuffer::GetSize() const
    {
        return mSize;
    }

    void OpenGLBuffer::Initialize ( const void* aData )
    {
        if ( !mSize )
        {
            return;
        }
        glCreateBuffers ( 1, &mBuffer );
        glNamedBufferData (  mBuffer,
                             mSize,
                             aData,
                             mUsage );
    }

    void OpenGLBuffer::Finalize()
    {
        if ( mBuffer != 0 && glIsBuffer ( mBuffer ) )
        {
            glDeleteBuffers ( 1, &mBuffer );
            mBuffer = 0;
            mSize = 0;
            mUsage = 0;
        }
    }
}
