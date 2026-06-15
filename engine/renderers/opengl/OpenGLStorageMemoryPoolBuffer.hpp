/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLSTORAGEMEMORYPOOLBUFFER_HPP
#define AEONGAMES_OPENGLSTORAGEMEMORYPOOLBUFFER_HPP
#include <cstdint>
#include <array>
#include "OpenGLFunctions.hpp"
#include "OpenGLBuffer.hpp"
#include "aeongames/MemoryPoolBuffer.hpp"

namespace AeonGames
{
    class OpenGLRenderer;
    /** @brief OpenGL memory pool buffer for transient per-frame storage-buffer (SSBO) allocations.
     *
     * Sibling of `OpenGLMemoryPoolBuffer`. GL doesn't bake a target into the
     * buffer object, so the only difference is which alignment query is used
     * (`GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT` instead of
     * `GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT`).
     */
    class OpenGLStorageMemoryPoolBuffer : public MemoryPoolBuffer
    {
    public:
        /// @brief Construct with a renderer and initial pool size.
        OpenGLStorageMemoryPoolBuffer ( const OpenGLRenderer&  aOpenGLRenderer, GLsizei aStackSize );
        /// @brief Construct with a renderer, leaving the pool uninitialized.
        OpenGLStorageMemoryPoolBuffer ( const OpenGLRenderer& aOpenGLRenderer );
        /// @brief Move constructor.
        OpenGLStorageMemoryPoolBuffer ( OpenGLStorageMemoryPoolBuffer&& );
        OpenGLStorageMemoryPoolBuffer ( const OpenGLStorageMemoryPoolBuffer& ) = delete;
        OpenGLStorageMemoryPoolBuffer& operator= ( const OpenGLStorageMemoryPoolBuffer& ) = delete;
        OpenGLStorageMemoryPoolBuffer& operator = ( OpenGLStorageMemoryPoolBuffer&& ) = delete;
        ~OpenGLStorageMemoryPoolBuffer();
        BufferAccessor Allocate ( size_t aSize ) final;
        void Reset() final;
        const Buffer & GetBuffer() const final;
        /// @brief Initialize the pool buffer with the given size.
        void Initialize ( GLsizei aStackSize );
        /// @brief Release pool buffer resources.
        void Finalize();
    private:
        const OpenGLRenderer&  mOpenGLRenderer;
        size_t mOffset{0};
        // Ring of distinct buffer objects, one per in-flight frame. Per-frame
        // draws read their object matrices straight from these buffers; reusing
        // a single buffer every frame lets the next frame's writes race the
        // previous frame's still-in-flight draws (a write-after-read hazard the
        // AMD OpenGL driver does not serialize and that buffer orphaning did not
        // cure). Advancing to a different physical buffer each frame removes the
        // hazard without relying on driver buffer-renaming.
        static constexpr size_t kFramesInFlight = 3;
        std::array<OpenGLBuffer, kFramesInFlight> mStorageBuffers{};
        size_t mCurrent{0};
    };
}
#endif
