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
#ifndef AEONGAMES_OPENGLMATERIAL_H
#define AEONGAMES_OPENGLMATERIAL_H
#include <cstdint>
#include <string>
#include <vector>
#include "aeongames/Memory.h"
#include "aeongames/Material.h"
#include "OpenGLBuffer.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLTexture;
    class OpenGLMaterial : public Material::IRenderMaterial
    {
    public:
        OpenGLMaterial ( const Material& aMaterial );
        ~OpenGLMaterial() final;
        void Update ( const uint8_t* aValue, size_t aOffset = 0, size_t aSize = 0 ) final;
        GLuint GetPropertiesBufferId() const;
        const Material& GetMaterial() const;
    private:
        void Initialize();
        void Finalize();
        const Material& mMaterial;
        OpenGLBuffer mPropertiesBuffer{};
    };
}
#endif
