/*
Copyright (C) 2017-2016 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLPIPELINE_H
#define AEONGAMES_OPENGLPIPELINE_H
#include <cstdint>
#include <string>
#include "OpenGLFunctions.h"
#include "aeongames/Uniform.h"

namespace AeonGames
{
    class Pipeline;
    class OpenGLRenderer;
    class OpenGLMaterial;
    class OpenGLTexture;
    class OpenGLPipeline
    {
    public:
        OpenGLPipeline ( const std::shared_ptr<Pipeline> aPipeline, const std::shared_ptr<const OpenGLRenderer> aOpenGLRenderer );
        ~OpenGLPipeline();
        void Use ( const std::shared_ptr<OpenGLMaterial>& aMaterial = nullptr ) const;
    private:
        void Initialize();
        void Finalize();
        const std::shared_ptr<Pipeline> mPipeline;
        std::shared_ptr<const OpenGLRenderer> mOpenGLRenderer;
        uint32_t mProgramId;
        uint32_t mPropertiesBuffer = 0;
        const std::shared_ptr<OpenGLMaterial> mDefaultMaterial;
    };
}
#endif
