/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLRenderer.h"
#include "OpenGLBuffer.h"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    OpenGLBuffer::OpenGLBuffer ( const OpenGLRenderer & aOpenGLRenderer ) : mOpenGLRenderer ( aOpenGLRenderer )
    {
    }
    OpenGLBuffer::OpenGLBuffer ( const OpenGLRenderer& aOpenGLRenderer, const GLsizei aSize, const GLenum aUsage, const void *aData ) :
        mOpenGLRenderer ( aOpenGLRenderer ), mSize ( aSize ), mUsage ( aUsage )
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

    void OpenGLBuffer::Initialize ( const GLsizei aSize, const GLenum aUsage, const void * aData )
    {
        Finalize();
        mSize = aSize;
        mUsage = aUsage;
        Initialize ( aData );
    }

    void OpenGLBuffer::WriteMemory ( const GLintptr aOffset, const GLsizeiptr aSize, const void * aData ) const
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
        glGenBuffers ( 1, &mBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glNamedBufferData (  mBuffer,
                             mSize,
                             aData,
                             mUsage );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLBuffer::Finalize()
    {
        if ( glIsBuffer ( mBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteBuffers ( 1, &mBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mBuffer = 0;
        }
    }
}
