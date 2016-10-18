/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Image.h"
#include "aeongames/Utilities.h"

namespace AeonGames
{
    OpenGLTexture::OpenGLTexture ( const std::string& aFilename ) :
        mFilename ( aFilename )
    {
        auto image = GetImage ( GetFileExtension ( mFilename ), mFilename );
        glGenTextures ( 1, &mTexture );
        OPENGL_CHECK_ERROR_THROW;
        glBindTexture ( GL_TEXTURE_2D, mTexture );
        OPENGL_CHECK_ERROR_THROW;
        /**@todo Write a format/type dictionary?
            These guesses only work for PNG at the moment.*/
        glTexImage2D ( GL_TEXTURE_2D,
                       0,
                       ( image->Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                       image->Width(),
                       image->Height(),
                       0,
                       ( image->Format() == Image::ImageFormat::RGB ) ? GL_RGB : GL_RGBA,
                       ( image->Type() == Image::ImageType::UNSIGNED_BYTE ) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
                       image->Data() );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        OPENGL_CHECK_ERROR_THROW;
    }

    OpenGLTexture::~OpenGLTexture()
    {
        if ( glIsTexture ( mTexture ) == GL_TRUE )
        {
            glDeleteTextures ( 1, &mTexture );
        }
        mTexture = 0;
    }
}
