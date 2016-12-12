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
#include "aeongames/Texture.h"
#include <cstdint>
#include <string>
#include <vector>
#include "aeongames/Uniform.h"

namespace AeonGames
{
    class OpenGLTexture
    {
    public:
        OpenGLTexture ( const std::string& aFilename );
        ~OpenGLTexture();
        /**@todo This is a temporary hack since mHandle is specific of ARB_bindless_texture which is not core in any OpenGL version to date.*/
        const uint64_t& GetHandle() const;
    private:
        std::string mFilename;
        uint32_t mTexture;
        uint64_t mHandle;
    };
}
#endif
