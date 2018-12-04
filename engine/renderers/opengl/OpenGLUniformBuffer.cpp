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
#include "OpenGLUniformBuffer.h"

namespace AeonGames
{
    OpenGLUniformBuffer::OpenGLUniformBuffer() = default;

    OpenGLUniformBuffer::OpenGLUniformBuffer ( const OpenGLUniformBuffer& aBuffer ) : mBuffer{aBuffer.mBuffer}
    {
    }

    OpenGLUniformBuffer::OpenGLUniformBuffer ( const GLsizei aSize, const void *aData ) :
        mBuffer{aSize, GL_DYNAMIC_DRAW, aData}
    {
    }

    OpenGLUniformBuffer& OpenGLUniformBuffer::operator= ( const OpenGLUniformBuffer& aBuffer )
    {
        mBuffer = aBuffer.mBuffer;
        return *this;
    }

    GLuint OpenGLUniformBuffer::GetBufferId() const
    {
        return mBuffer.GetBufferId();
    }

    void* OpenGLUniformBuffer::Map ( size_t aOffset, size_t aSize ) const
    {
        return mBuffer.Map ( aOffset, aSize );
    }
    void OpenGLUniformBuffer::Unmap() const
    {
        return mBuffer.Unmap();
    }
    size_t OpenGLUniformBuffer::GetSize() const
    {
        return mBuffer.GetSize();
    }
    void OpenGLUniformBuffer::WriteMemory ( size_t aOffset, size_t aSize, const void *aData ) const
    {
        mBuffer.WriteMemory ( aOffset, aSize, aData );
    }
}