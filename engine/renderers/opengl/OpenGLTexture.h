/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLTEXTURE_H
#define AEONGAMES_OPENGLTEXTURE_H
#include "OpenGLFunctions.h"

namespace AeonGames
{
    class Texture;
    class OpenGLRenderer;
    class OpenGLTexture
    {
    public:
        OpenGLTexture ( OpenGLRenderer& aOpenGLRenderer, const Texture& aTexture );
        OpenGLTexture ( OpenGLTexture&& aOpenGLTexture );
        OpenGLTexture ( const OpenGLTexture& ) = delete;
        OpenGLTexture& operator= ( const OpenGLTexture& ) = delete;
        OpenGLTexture& operator= ( OpenGLTexture&& ) = delete;
        ~OpenGLTexture();
        GLuint GetTextureId() const;
    private:
        OpenGLRenderer& mOpenGLRenderer;
        const Texture* mTexture{};
        GLuint mTextureId{};
    };
}
#endif
