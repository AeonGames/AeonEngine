/*
Copyright (C) 2016-2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include "OpenGLBuffer.h"

namespace AeonGames
{
    class Material;
    class OpenGLRenderer;
    class OpenGLPipeline;
    class OpenGLMaterial
    {
    public:
        OpenGLMaterial ( OpenGLRenderer& aOpenGLRenderer, const Material& aMaterial );
        OpenGLMaterial ( OpenGLMaterial&& aOpenGLMaterial );
        OpenGLMaterial ( const OpenGLMaterial& ) = delete;
        OpenGLMaterial& operator= ( const OpenGLMaterial& ) = delete;
        OpenGLMaterial& operator= ( OpenGLMaterial&& ) = delete;
        ~OpenGLMaterial();
        /// @brief  Binds the material to the specified binding index.
        /// @param aBindingIndex The binding index to use.
        /// @note Not super happy about this API, might change it later.
        void Bind ( const OpenGLPipeline& aPipeline ) const;
    private:
        OpenGLRenderer& mOpenGLRenderer;
        const Material* mMaterial;
        OpenGLBuffer mUniformBuffer{};
    };
}
#endif
