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
#include "aeongames/Image.h"
#include "aeongames/Utilities.h"

namespace AeonGames
{
    OpenGLImage::OpenGLImage() :
        Image()
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

    OpenGLImage::~OpenGLImage()
    {
        Finalize();
    }

    void OpenGLImage::Initialize()
    {
        glGenTextures ( 1, &mTexture );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, mTexture );
        OPENGL_CHECK_ERROR_THROW;
        glTexImage2D ( GL_TEXTURE_2D,
                       0,
                       ( Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                       Width(),
                       Height(),
                       0,
                       ( Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                       ( Type() == Image::ImageType::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
                       Pixels() );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, 0 );
        OPENGL_CHECK_ERROR_THROW;
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
#if 0
    void OpenGLImage::Update()
    {
        if ( glIsImage ( mTexture ) == GL_FALSE )
        {
            return;
        }
        glImageSubImage2D ( mTexture,
                            0,
                            ( Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                            Width(),
                            Height(),
                            0,
                            ( Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                            ( Type() == Image::ImageType::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
                            Pixels() );
    }
#endif
    const uint32_t OpenGLImage::GetTextureId() const
    {
        return mTexture;
    }
}
