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
#ifndef AEONGAMES_OPENGLFRAMEBUFFER_HPP
#define AEONGAMES_OPENGLFRAMEBUFFER_HPP
#include <cstddef>
#include "OpenGLFunctions.hpp"

namespace AeonGames
{
    class OpenGLRenderer;
    /** @brief OpenGL framebuffer object for off-screen rendering. */
    class OpenGLFrameBuffer
    {
    public:
        OpenGLFrameBuffer();
        /// @brief Move constructor.
        OpenGLFrameBuffer ( OpenGLFrameBuffer&& aOpenGLFrameBuffer );
        OpenGLFrameBuffer ( const OpenGLFrameBuffer& aOpenGLFrameBuffer ) = delete;
        OpenGLFrameBuffer& operator= ( const OpenGLFrameBuffer& aOpenGLFrameBuffer ) = delete;
        OpenGLFrameBuffer& operator= ( OpenGLFrameBuffer&& aOpenGLFrameBuffer ) = delete;
        ~OpenGLFrameBuffer();
        /// @brief Resize the framebuffer attachments.
        void Resize ( uint32_t aWidth, uint32_t aHeight );
        /// @brief Bind the framebuffer as the active render target.
        void Bind();
        /// @brief Unbind the framebuffer, restoring the default render target.
        void Unbind();
        /// @brief Create and initialize framebuffer resources.
        void Initialize();
        /// @brief Release framebuffer resources.
        void Finalize();
        /// @brief Get the OpenGL framebuffer object identifier.
        GLuint GetFBO() const; /// This is temporary
    private:
        GLuint mFBO {};
        GLuint mColorBuffer {};
        GLuint mRBO {};
    };
}
#endif
