/*
Copyright (C) 2018,2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLUNIFORMBUFFER_H
#define AEONGAMES_OPENGLUNIFORMBUFFER_H
#include "aeongames/UniformBuffer.h"
#include "OpenGLBuffer.h"
namespace AeonGames
{
    class OpenGLUniformBuffer : public UniformBuffer
    {
    public:
        OpenGLUniformBuffer ();
        /// Copy contsructor.
        OpenGLUniformBuffer ( const OpenGLUniformBuffer& aBuffer );
        /// No move allowed
        OpenGLUniformBuffer ( OpenGLBuffer&& ) = delete;
        /// Assignment operator due to rule of zero/three/five.
        OpenGLUniformBuffer& operator= ( const OpenGLUniformBuffer& aBuffer );
        /// No move assignment allowed
        OpenGLUniformBuffer& operator = ( OpenGLUniformBuffer&& ) = delete;
        OpenGLUniformBuffer ( const GLsizei aSize, const void *aData = nullptr );

        GLuint GetBufferId() const;

        /**@ name Overriden Functions */
        ///@{
        void WriteMemory ( size_t aOffset, size_t aSize, const void *aData = nullptr ) const final;
        void* Map ( size_t aOffset, size_t aSize ) const final;
        void Unmap() const final;
        size_t GetSize() const final;
        ///@}
    private:
        OpenGLBuffer mBuffer{};
    };
}
#endif