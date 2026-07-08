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
#include "aeongames/ProtoBufClasses.hpp"
#include "OpenGLFrameBuffer.hpp"

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

        // Creates an RGBA16F colour texture and attaches it at aAttachment.
        auto make_color = [] ( GLuint & aTexture, GLenum aAttachment, GLint aFilter )
        {
            glGenTextures ( 1, &aTexture );
            glBindTexture ( GL_TEXTURE_2D, aTexture );
            glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGBA, GL_HALF_FLOAT, nullptr );
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, aFilter );
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aFilter );
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glBindTexture ( GL_TEXTURE_2D, 0 );
            glFramebufferTexture2D ( GL_FRAMEBUFFER, aAttachment, GL_TEXTURE_2D, aTexture, 0 );
        };
        // [0] linear HDR radiance (tonemap samples it, LINEAR); [1] view normal +
        // roughness; [2] specular weight. The G-buffer is point-sampled (NEAREST)
        // so the composite reads exact per-pixel normals and weights.
        make_color ( mColorBuffer, GL_COLOR_ATTACHMENT0, GL_LINEAR );
        make_color ( mNormalRoughBuffer, GL_COLOR_ATTACHMENT1, GL_NEAREST );
        make_color ( mSpecWeightBuffer, GL_COLOR_ATTACHMENT2, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;

        const GLenum draw_buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers ( 3, draw_buffers );
        OPENGL_CHECK_ERROR_THROW;

        // Sampleable depth/stencil texture (renderbuffers cannot be sampled; the
        // composite reconstructs view-space position from this depth).
        glGenTextures ( 1, &mDepthTexture );
        glBindTexture ( GL_TEXTURE_2D, mDepthTexture );
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glBindTexture ( GL_TEXTURE_2D, 0 );
        glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0 );
        OPENGL_CHECK_ERROR_THROW;

        if ( glCheckFramebufferStatus ( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        {
            std::cout << LogLevel::Error << "Incomplete Framebuffer." << std::endl;
            throw std::runtime_error ( "Incomplete Framebuffer." );
        }
        OPENGL_CHECK_ERROR_THROW;

        glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLFrameBuffer::Finalize()
    {
        auto drop = [] ( GLuint & aTexture )
        {
            if ( glIsTexture ( aTexture ) )
            {
                OPENGL_CHECK_ERROR_NO_THROW;
                glDeleteTextures ( 1, &aTexture );
                OPENGL_CHECK_ERROR_NO_THROW;
                aTexture = 0;
            }
        };
        drop ( mDepthTexture );
        drop ( mSpecWeightBuffer );
        drop ( mNormalRoughBuffer );
        drop ( mColorBuffer );
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
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, aWidth, aHeight, 0, GL_RGBA, GL_HALF_FLOAT, nullptr );
        glBindTexture ( GL_TEXTURE_2D, mNormalRoughBuffer );
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, aWidth, aHeight, 0, GL_RGBA, GL_HALF_FLOAT, nullptr );
        glBindTexture ( GL_TEXTURE_2D, mSpecWeightBuffer );
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA16F, aWidth, aHeight, 0, GL_RGBA, GL_HALF_FLOAT, nullptr );
        glBindTexture ( GL_TEXTURE_2D, mDepthTexture );
        glTexImage2D ( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, aWidth, aHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr );
        glBindTexture ( GL_TEXTURE_2D, 0 );
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

    GLuint OpenGLFrameBuffer::GetColorBuffer() const
    {
        return mColorBuffer;
    }

    GLuint OpenGLFrameBuffer::GetNormalRoughBuffer() const
    {
        return mNormalRoughBuffer;
    }

    GLuint OpenGLFrameBuffer::GetSpecWeightBuffer() const
    {
        return mSpecWeightBuffer;
    }

    GLuint OpenGLFrameBuffer::GetDepthBuffer() const
    {
        return mDepthTexture;
    }

    OpenGLFrameBuffer::OpenGLFrameBuffer ( OpenGLFrameBuffer&& aOpenGLFrameBuffer )
    {
        std::swap ( mFBO, aOpenGLFrameBuffer.mFBO );
        std::swap ( mColorBuffer, aOpenGLFrameBuffer.mColorBuffer );
        std::swap ( mNormalRoughBuffer, aOpenGLFrameBuffer.mNormalRoughBuffer );
        std::swap ( mSpecWeightBuffer, aOpenGLFrameBuffer.mSpecWeightBuffer );
        std::swap ( mDepthTexture, aOpenGLFrameBuffer.mDepthTexture );
    }
}
