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
#ifndef AEONGAMES_OPENGLBUFFER_H
#define AEONGAMES_OPENGLBUFFER_H
#include <cstddef>
#include "OpenGLFunctions.h"
#include "aeongames/RenderBuffer.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLBuffer : public RenderBuffer
    {
    public:
        OpenGLBuffer ();
        /// Copy contsructor.
        OpenGLBuffer ( const OpenGLBuffer& aBuffer );
        /// No move allowed
        OpenGLBuffer ( OpenGLBuffer&& );
        /// Assignment operator due to rule of zero/three/five.
        OpenGLBuffer& operator= ( const OpenGLBuffer& aBuffer );
        /// No move assignment allowed
        OpenGLBuffer& operator = ( OpenGLBuffer&& ) = delete;

        OpenGLBuffer ( const GLsizei aSize, const GLenum aUsage, const void *aData = nullptr );
        ~OpenGLBuffer();
        void Initialize ( const GLsizei aSize, const GLenum aUsage, const void *aData = nullptr );
        void Finalize();
        void WriteMemory ( const GLintptr aOffset, const GLsizeiptr aSize, const void *aData = nullptr ) const ;
        void* Map ( const GLbitfield aAccess ) const;
        void* MapRange ( const GLintptr aOffset, const GLsizeiptr aSize, const GLbitfield aAccess ) const;
        GLuint GetBufferId() const;
        /**@ name Overriden Functions */
        ///@{
        void* Map ( const size_t aOffset, size_t aSize ) const final;
        void Unmap() const final;
        size_t GetSize() const final;
        ///@}
    private:
        void Initialize ( const void *aData );
        GLsizei mSize{};
        GLenum mUsage{};
        GLuint mBuffer{};
    };
}
#endif
