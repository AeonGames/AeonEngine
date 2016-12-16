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
#ifndef AEONGAMES_OPENGLPROGRAM_H
#define AEONGAMES_OPENGLPROGRAM_H
#include <cstdint>
#include <string>
#include "OpenGLFunctions.h"
#include "OpenGLMaterial.h"
#include "aeongames/Uniform.h"

namespace AeonGames
{
    class Program;
    class OpenGLTexture;
    class OpenGLProgram
    {
    public:
        OpenGLProgram ( const std::shared_ptr<Program> aProgram );
        ~OpenGLProgram();
        void Use() const;
    private:
        void Initialize();
        void Finalize();
        const std::shared_ptr<Program> mProgram;
        uint32_t mProgramId;
        uint32_t mMatricesBlockIndex = 0;
        uint32_t mPropertiesBlockIndex = 0;
        uint32_t mPropertiesBuffer = 0;
        std::vector<uint8_t> mUniformData;
        std::vector<std::shared_ptr<OpenGLTexture>> mTextures;
    };
}
#endif
