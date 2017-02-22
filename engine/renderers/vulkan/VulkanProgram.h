/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_VULKANPROGRAM_H
#define AEONGAMES_VULKANPROGRAM_H
#include <cstdint>
#include <string>
#include "VulkanMaterial.h"
#include "aeongames/Uniform.h"

namespace AeonGames
{
    class Program;
    class VulkanTexture;
    class VulkanProgram
    {
    public:
        VulkanProgram ( const std::shared_ptr<Program> aProgram );
        ~VulkanProgram();
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
        std::vector<std::pair<std::shared_ptr<VulkanTexture>, int>> mTextures;
    };
}
#endif
