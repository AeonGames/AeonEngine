/*
Copyright (C) 2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.hpp"
#include "OpenGLFrameBuffer.h"

namespace AeonGames
{
    OpenGLFrameBuffer::OpenGLFrameBuffer() = default;
    OpenGLFrameBuffer::~OpenGLFrameBuffer()
    {
        Finalize();
    }

    void OpenGLFrameBuffer::Initialize()
    {
        // Frame Buffer
        glGenFramebuffers ( 1, &mFBO );
        OPENGL_CHECK_ERROR_THROW;
        glBindFramebuffer ( GL_FRAMEBUFFER, mFBO );
        OPENGL_CHECK_ERROR_THROW;

        // Color Buffer
        glGenTextures ( 1, &mColorBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, mColorBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, 0 );
        OPENGL_CHECK_ERROR_THROW;

        glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorBuffer, 0 );
        OPENGL_CHECK_ERROR_THROW;
        // Render Buffer
        glGenRenderbuffers ( 1, &mRBO );
        OPENGL_CHECK_ERROR_THROW;
        glBindRenderbuffer ( GL_RENDERBUFFER, mRBO );
        OPENGL_CHECK_ERROR_THROW;
        glRenderbufferStorage ( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600 );
        OPENGL_CHECK_ERROR_THROW;
        glBindRenderbuffer ( GL_RENDERBUFFER, 0 );
        OPENGL_CHECK_ERROR_THROW;

        glFramebufferRenderbuffer ( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO );
        OPENGL_CHECK_ERROR_THROW;
        if ( glCheckFramebufferStatus ( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        {
            throw std::runtime_error ( "Incomplete Framebuffer." );
        }
        OPENGL_CHECK_ERROR_THROW;

        glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLFrameBuffer::Finalize()
    {
        if ( glIsRenderbuffer ( mRBO ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteRenderbuffers ( 1, &mRBO );
            OPENGL_CHECK_ERROR_NO_THROW;
            mRBO = 0;
        }
        if ( glIsTexture ( mColorBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteTextures ( 1, &mColorBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mColorBuffer = 0;
        }
        if ( glIsFramebuffer ( mFBO ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteFramebuffers ( 1, &mFBO );
            OPENGL_CHECK_ERROR_NO_THROW;
            mFBO = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
    }

    void OpenGLFrameBuffer::Resize ( uint32_t aWidth, uint32_t aHeight )
    {
        glBindTexture ( GL_TEXTURE_2D, mColorBuffer );
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, aWidth, aHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );
        glBindRenderbuffer ( GL_RENDERBUFFER, mRBO );
        glRenderbufferStorage ( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, aWidth, aHeight );
        glBindRenderbuffer ( GL_RENDERBUFFER, 0 );
    }
    void OpenGLFrameBuffer::Bind()
    {
        glBindFramebuffer ( GL_FRAMEBUFFER, mFBO );
        OPENGL_CHECK_ERROR_NO_THROW;
    }
    void OpenGLFrameBuffer::Unbind()
    {
        glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
        OPENGL_CHECK_ERROR_NO_THROW;
    }
    GLuint OpenGLFrameBuffer::GetFBO() const
    {
        return mFBO;
    }

    OpenGLFrameBuffer::OpenGLFrameBuffer ( OpenGLFrameBuffer&& aOpenGLFrameBuffer )
    {
        std::swap ( mFBO, aOpenGLFrameBuffer.mFBO );
        std::swap ( mColorBuffer, aOpenGLFrameBuffer.mColorBuffer );
        std::swap ( mRBO, aOpenGLFrameBuffer.mRBO );
    }
}
