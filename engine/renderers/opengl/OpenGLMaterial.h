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
#include "aeongames/Material.h"
#include <cstdint>
#include <string>
#include <vector>
#include "aeongames/Memory.h"
#include "aeongames/Material.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLTexture;
    class OpenGLMaterial : public Material::IRenderMaterial
    {
    public:
        OpenGLMaterial ( const Material& aMaterial, const std::shared_ptr<const OpenGLRenderer>& aOpenGLRenderer );
        ~OpenGLMaterial() final;
        const std::vector<uint8_t>& GetUniformData() const;
        const std::vector<std::shared_ptr<OpenGLTexture>>& GetTextures() const;
    private:
        void Initialize();
        void Finalize();
        const Material& mMaterial;
        std::shared_ptr<const OpenGLRenderer> mOpenGLRenderer;
        std::vector<uint8_t> mUniformData;
        std::vector<std::shared_ptr<OpenGLTexture>> mTextures;
    };
}
#endif
