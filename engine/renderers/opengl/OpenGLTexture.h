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
#ifndef AEONGAMES_OPENGLTEXTURE_H
#define AEONGAMES_OPENGLTEXTURE_H
#include <cstdint>
#include <string>
#include <vector>
#include "aeongames/Uniform.h"
#include "aeongames/Memory.h"

namespace AeonGames
{
    class Image;
    class OpenGLTexture
    {
    public:
        OpenGLTexture ( const std::shared_ptr<Image> aImage );
        ~OpenGLTexture();
        /**@todo Determine if we want to keep the texture id exposed like this.
            Maybe all we need is a Bind function.*/
        const uint32_t GetTexture() const;
    private:
        void Initialize();
        void Finalize();
        const std::shared_ptr<Image> mImage;
        uint32_t mTexture;
    };
}
#endif
