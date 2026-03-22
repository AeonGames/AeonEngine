/*
Copyright (C) 2018,2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLBUFFER_HPP
#define AEONGAMES_OPENGLBUFFER_HPP
#include <cstddef>
#include "aeongames/Buffer.hpp"
#include "OpenGLFunctions.hpp"

namespace AeonGames
{
    class OpenGLRenderer;
    /** @brief OpenGL GPU buffer wrapper implementing the Buffer interface. */
    class OpenGLBuffer : public Buffer
    {
    public:
        OpenGLBuffer ();
        /// @brief Construct from size, usage hint, and optional initial data.
        OpenGLBuffer ( const GLsizei aSize, const GLenum aUsage, const void *aData = nullptr );
        /// @brief Move constructor.
        OpenGLBuffer ( OpenGLBuffer&& aOpenGLBuffer );
        OpenGLBuffer ( const OpenGLBuffer& aOpenGLBuffer ) = delete;
        OpenGLBuffer& operator= ( const OpenGLBuffer& aOpenGLBuffer ) = delete;
        OpenGLBuffer& operator= ( OpenGLBuffer&& aOpenGLBuffer ) = delete;
        ~OpenGLBuffer();

        /// @brief Initialize the buffer with the given size, usage, and optional data.
        void Initialize ( const GLsizei aSize, const GLenum aUsage, const void *aData = nullptr );
        /// @brief Release the buffer resources.
        void Finalize();
        /// @name Virtual functions
        ///@{
        void WriteMemory ( const size_t aOffset, const size_t aSize, const void *aData = nullptr ) const final;
        void* Map ( const size_t aOffset, size_t aSize ) const final;
        void Unmap() const final;
        size_t GetSize() const final;
        ///@}
        /// @brief Map the entire buffer with the specified access flags.
        void* Map ( const GLbitfield aAccess ) const;
        /// @brief Map a sub-range of the buffer.
        void* MapRange ( const GLintptr aOffset, const GLsizeiptr aSize, const GLbitfield aAccess ) const;
        /// @brief Get the OpenGL buffer object identifier.
        GLuint GetBufferId() const;
    private:
        void Initialize ( const void *aData );
        GLsizei mSize{};
        GLenum mUsage{};
        GLuint mBuffer{};
    };
}
#endif
