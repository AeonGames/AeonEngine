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
    OpenGLBuffer::OpenGLBuffer ( const OpenGLRenderer& aOpenGLRenderer, const size_t aSize, const size_t aUsage, const size_t aProperties, const void *aData ) :
        mOpenGLRenderer ( aOpenGLRenderer ), mSize ( aSize ), mUsage ( aUsage ), mProperties ( aProperties )
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

    void OpenGLBuffer::Initialize ( const size_t aSize, const size_t aUsage, const size_t aProperties, const void * aData )
    {
        if ( mBuffer != 0 )
        {
            throw ( std::runtime_error ( "Buffer already initialized." ) );
        }
        mSize = aSize;
        mUsage = aUsage;
        mProperties = aProperties;
        Initialize ( aData );
    }

    void OpenGLBuffer::WriteMemory ( const size_t aOffset, const size_t aSize, const void * aData ) const
    {
        if ( aData )
        {
        }
    }

    void * OpenGLBuffer::Map ( const size_t aOffset, const size_t aSize ) const
    {
        void* data = nullptr;
        return data;
    }

    void OpenGLBuffer::Unmap() const
    {
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
        WriteMemory ( 0, mSize, aData );
    }

    void OpenGLBuffer::Finalize()
    {
        if ( mBuffer != 0 )
        {
            mBuffer = 0;
        }
    }
}
