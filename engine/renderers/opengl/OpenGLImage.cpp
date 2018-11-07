/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLImage.h"

#include <utility>
#include "aeongames/AeonEngine.h"
#include "aeongames/Image.h"
#include "aeongames/CRC.h"
#include "aeongames/Utilities.h"

namespace AeonGames
{
    OpenGLImage::OpenGLImage ( uint32_t aPath )
    {
        if ( aPath )
        {
            Load ( aPath );
        }
    }

    OpenGLImage::~OpenGLImage()
    {
        Finalize();
    }

    void OpenGLImage::Load ( const std::string& aPath )
    {
        Load ( crc32i ( aPath.data(), aPath.size() ) );
    }

    void OpenGLImage::Load ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        DecodeImage ( *this, buffer.data(), buffer.size() );
    }

    void OpenGLImage::Initialize ( uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels )
    {
        if ( glIsTexture ( mTexture ) == GL_TRUE )
        {
            throw std::runtime_error ( "OpenGLImage: Image already initiaized." );
        }
        glGenTextures ( 1, &mTexture );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, mTexture );
        OPENGL_CHECK_ERROR_THROW;
        glTexImage2D ( GL_TEXTURE_2D,
                       0,
                       ( aFormat == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                       aWidth,
                       aHeight,
                       0,
                       ( aFormat == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                       ( aType == Image::ImageType::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
                       aPixels );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, 0 );
        OPENGL_CHECK_ERROR_THROW;
    }

    void OpenGLImage::BitBlit ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels )
    {
        if ( glIsTexture ( mTexture ) != GL_TRUE )
        {
            throw std::runtime_error ( "OpenGLImage: Trying to bit blit an uninitialized image." );
        }
        glTextureSubImage2D ( mTexture, 0, aXOffset, aYOffset, aWidth, aHeight,
                              ( aFormat == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                              ( aType == Image::ImageType::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT, aPixels );
        OPENGL_CHECK_ERROR_THROW;
    }

    uint32_t OpenGLImage::Width() const
    {
        GLint width{};
        glGetTextureLevelParameteriv ( mTexture, 0, GL_TEXTURE_WIDTH, &width );
        return static_cast<uint32_t> ( width );
    }

    uint32_t OpenGLImage::Height() const
    {
        GLint height{};
        glGetTextureLevelParameteriv ( mTexture, 0, GL_TEXTURE_HEIGHT, &height );
        return static_cast<uint32_t> ( height );
    }

    Image::ImageFormat OpenGLImage::Format() const
    {
        GLint format{};
        glGetTextureLevelParameteriv ( mTexture, 0, GL_TEXTURE_INTERNAL_FORMAT, &format );
        return ( format == GL_RGB ) ? Image::ImageFormat::RGB : Image::ImageFormat::RGBA;
    }

    Image::ImageType OpenGLImage::Type() const
    {
        GLint red{};
        //GLint green{};
        //GLint blue{};
        glGetTextureLevelParameteriv ( mTexture, 0, GL_TEXTURE_RED_TYPE, &red );
        //glGetTextureLevelParameteriv(mTexture,0,GL_TEXTURE_GREEN_TYPE,&green);
        //glGetTextureLevelParameteriv(mTexture,0,GL_TEXTURE_BLUE_TYPE,&blue);
        return ( red == GL_UNSIGNED_BYTE ) ? Image::ImageType::UNSIGNED_BYTE : Image::ImageType::UNSIGNED_SHORT;
    }

    void OpenGLImage::Finalize()
    {
        if ( glIsTexture ( mTexture ) == GL_TRUE )
        {
            glDeleteTextures ( 1, &mTexture );
            OPENGL_CHECK_ERROR_NO_THROW;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
        mTexture = 0;
    }

    const uint32_t OpenGLImage::GetTextureId() const
    {
        return mTexture;
    }
}
