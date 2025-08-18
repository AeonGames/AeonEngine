/*
Copyright (C) 2018,2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLBUFFER_H
#define AEONGAMES_OPENGLBUFFER_H
#include <cstddef>
#include "aeongames/Buffer.hpp"
#include "OpenGLFunctions.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLBuffer : public Buffer
    {
    public:
        OpenGLBuffer ();
        OpenGLBuffer ( const GLsizei aSize, const GLenum aUsage, const void *aData = nullptr );
        OpenGLBuffer ( OpenGLBuffer&& aOpenGLBuffer );
        OpenGLBuffer ( const OpenGLBuffer& aOpenGLBuffer ) = delete;
        OpenGLBuffer& operator= ( const OpenGLBuffer& aOpenGLBuffer ) = delete;
        OpenGLBuffer& operator= ( OpenGLBuffer&& aOpenGLBuffer ) = delete;
        ~OpenGLBuffer();

        void Initialize ( const GLsizei aSize, const GLenum aUsage, const void *aData = nullptr );
        void Finalize();
        /// @name Virtual functions
        ///@{
        void WriteMemory ( const size_t aOffset, const size_t aSize, const void *aData = nullptr ) const final;
        void* Map ( const size_t aOffset, size_t aSize ) const final;
        void Unmap() const final;
        size_t GetSize() const final;
        ///@}
        void* Map ( const GLbitfield aAccess ) const;
        void* MapRange ( const GLintptr aOffset, const GLsizeiptr aSize, const GLbitfield aAccess ) const;
        GLuint GetBufferId() const;
    private:
        void Initialize ( const void *aData );
        GLsizei mSize{};
        GLenum mUsage{};
        GLuint mBuffer{};
    };
}
#endif
