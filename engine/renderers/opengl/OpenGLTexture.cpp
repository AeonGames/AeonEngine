/*
Copyright (C) 2016-2020 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLTexture.h"

#include <utility>
#include "aeongames/AeonEngine.h"
#include "aeongames/Texture.h"
#include "aeongames/CRC.h"
#include "aeongames/Utilities.h"

namespace AeonGames
{

    OpenGLTexture::OpenGLTexture ( Format aFormat, Type aType, uint32_t aWidth, uint32_t aHeight, const uint8_t* aPixels ) : mFormat{aFormat}, mType{aType}
    {
        Resize ( aWidth, aHeight, aPixels );
    }

    OpenGLTexture::OpenGLTexture ( uint32_t aPath )
    {
        if ( aPath )
        {
            Load ( aPath );
        }
    }

    OpenGLTexture::~OpenGLTexture()
    {
        Finalize();
    }

    void OpenGLTexture::Load ( const std::string& aPath )
    {
        Load ( crc32i ( aPath.data(), aPath.size() ) );
    }

    void OpenGLTexture::Load ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        DecodeImage ( *this, buffer.data(), buffer.size() );
    }

    void OpenGLTexture::Resize ( uint32_t aWidth, uint32_t aHeight, const uint8_t* aPixels, Format aFormat, Type aType )
    {
        aFormat = ( aFormat == Format::Unknown ) ? mFormat : aFormat;
        aType = ( aType == Type::Unknown ) ? mType : aType;
        mFormat = aFormat;
        mType = aType;
        mWidth = aWidth;
        mHeight = aHeight;

        if ( mWidth == 0 || mHeight == 0 )
        {
            Finalize();
            return;
        }
        else if ( glIsTexture ( mTexture ) != GL_TRUE )
        {
            glGenTextures ( 1, &mTexture );
            OPENGL_CHECK_ERROR_THROW;
        }
        glBindTexture ( GL_TEXTURE_2D, mTexture );
        OPENGL_CHECK_ERROR_THROW;
        glTexImage2D ( GL_TEXTURE_2D,
                       0,
                       ( mFormat == Texture::Format::RGB ) ? GL_RGB : GL_RGBA, ///<@todo decide if this should be a separate variable
                       mWidth,
                       mHeight,
                       0,
                       ( mFormat == Texture::Format::RGB ) ? GL_RGB : ( mFormat == Texture::Format::BGRA ) ? GL_BGRA : GL_RGBA,
                       ( mType == Texture::Type::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : ( mType == Texture::Type::UNSIGNED_SHORT ) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT_8_8_8_8_REV,
                       aPixels );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, 0 );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLTexture::WritePixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Format aFormat, Type aType, const uint8_t* aPixels )
    {
        if ( glIsTexture ( mTexture ) != GL_TRUE )
        {
            throw std::runtime_error ( "OpenGLTexture: Trying to write to an uninitialized image." );
        }
        glTextureSubImage2D ( mTexture, 0, aXOffset, aYOffset, aWidth, aHeight,
                              ( aFormat == Texture::Format::RGB ) ? GL_RGB : ( aFormat == Texture::Format::BGRA ) ? GL_BGRA : GL_RGBA,
                              ( aType == Texture::Type::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : ( aType == Texture::Type::UNSIGNED_SHORT ) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT_8_8_8_8_REV,
                              aPixels );
        OPENGL_CHECK_ERROR_THROW;
    }

    uint32_t OpenGLTexture::GetWidth() const
    {
        return mWidth;
    }

    uint32_t OpenGLTexture::GetHeight() const
    {
        return mHeight;
    }

    Texture::Format OpenGLTexture::GetFormat() const
    {
        return mFormat;
    }

    Texture::Type OpenGLTexture::GetType() const
    {
        return mType;
    }

    void OpenGLTexture::Finalize()
    {
        if ( glIsTexture ( mTexture ) == GL_TRUE )
        {
            glDeleteTextures ( 1, &mTexture );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
        mTexture = 0;
    }

    const uint32_t OpenGLTexture::GetTextureId() const
    {
        return mTexture;
    }
}
