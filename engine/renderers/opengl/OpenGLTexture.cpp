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
#include "OpenGLTexture.h"

#include <utility>
#include "aeongames/Image.h"
#include "aeongames/Utilities.h"

namespace AeonGames
{
    OpenGLTexture::OpenGLTexture ( const Image& aImage ) :
        mImage ( aImage  )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    OpenGLTexture::~OpenGLTexture()
    {
        Finalize();
    }

    void OpenGLTexture::Initialize()
    {
        glGenTextures ( 1, &mTexture );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, mTexture );
        OPENGL_CHECK_ERROR_THROW;
        glTexImage2D ( GL_TEXTURE_2D,
                       0,
                       ( mImage.Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                       mImage.Width(),
                       mImage.Height(),
                       0,
                       ( mImage.Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                       ( mImage.Type() == Image::ImageType::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
                       mImage.Pixels() );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, 0 );
        OPENGL_CHECK_ERROR_THROW;
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

    void OpenGLTexture::Update()
    {
        if ( glIsTexture ( mTexture ) == GL_FALSE )
        {
            return;
        }
        glTextureSubImage2D ( mTexture,
                              0,
                              ( mImage.Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                              mImage.Width(),
                              mImage.Height(),
                              0,
                              ( mImage.Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                              ( mImage.Type() == Image::ImageType::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
                              mImage.Pixels() );
    }

    const uint32_t OpenGLTexture::GetTexture() const
    {
        return mTexture;
    }
}
