/*
Copyright (C) 2016-2021,2025 Rodrigo Jose Hernandez Cordoba

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

#include "OpenGLFunctions.h"
#include "OpenGLRenderer.h"
#include "OpenGLTexture.h"
#include "aeongames/Texture.hpp"

namespace AeonGames
{
    OpenGLTexture::OpenGLTexture ( OpenGLTexture&& aOpenGLTexture ) :
        mOpenGLRenderer{aOpenGLTexture.mOpenGLRenderer}
    {
        std::swap ( mTexture, aOpenGLTexture.mTexture );
        std::swap ( mTextureId, aOpenGLTexture.mTextureId );
    }

    OpenGLTexture::OpenGLTexture ( OpenGLRenderer& aOpenGLRenderer, const Texture& aTexture ) :
        mOpenGLRenderer{aOpenGLRenderer}, mTexture{&aTexture}
    {
        glGenTextures ( 1, &mTextureId );
        OPENGL_CHECK_ERROR_THROW;

        if ( glIsTexture ( mTextureId ) )
        {
            GLint w, h;
            glGetTextureLevelParameteriv ( mTextureId, 0, GL_TEXTURE_WIDTH, &w );
            glGetTextureLevelParameteriv ( mTextureId, 0, GL_TEXTURE_HEIGHT, &h );
            if ( aTexture.GetWidth() == static_cast<uint32_t> ( w ) && aTexture.GetHeight() == static_cast<uint32_t> ( h ) )
            {
                glTextureSubImage2D ( mTextureId, 0, 0, 0, aTexture.GetWidth(), aTexture.GetHeight(),
                                      ( aTexture.GetFormat() == Texture::Format::RGB ) ? GL_RGB : ( aTexture.GetFormat() == Texture::Format::BGRA ) ? GL_BGRA : GL_RGBA,
                                      ( aTexture.GetType() == Texture::Type::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : ( aTexture.GetType() == Texture::Type::UNSIGNED_SHORT ) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT_8_8_8_8_REV,
                                      aTexture.GetPixels().data() );
                return;
            }
        }

        glBindTexture ( GL_TEXTURE_2D, mTextureId );
        OPENGL_CHECK_ERROR_THROW;
        glTexImage2D ( GL_TEXTURE_2D,
                       0,
                       ( aTexture.GetFormat() == Texture::Format::RGB ) ? GL_RGB : GL_RGBA, ///<@todo decide if this should be a separate variable
                       aTexture.GetWidth(),
                       aTexture.GetHeight(),
                       0,
                       ( aTexture.GetFormat() == Texture::Format::RGB ) ? GL_RGB : ( aTexture.GetFormat() == Texture::Format::BGRA ) ? GL_BGRA : GL_RGBA,
                       ( aTexture.GetType() == Texture::Type::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : ( aTexture.GetType() == Texture::Type::UNSIGNED_SHORT ) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT_8_8_8_8_REV,
                       aTexture.GetPixels().data() );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, 0 );
        OPENGL_CHECK_ERROR_THROW;
    }

    OpenGLTexture::~OpenGLTexture()
    {
        if ( glIsTexture ( mTextureId ) == GL_TRUE )
        {
            glDeleteTextures ( 1, &mTextureId );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
    }


    GLuint OpenGLTexture::GetTextureId() const
    {
        return mTextureId;
    }
}
