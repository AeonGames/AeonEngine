/*
Copyright (C) 2016-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_OPENGLMATERIAL_HPP
#define AEONGAMES_OPENGLMATERIAL_HPP
#include <cstdint>
#include "OpenGLBuffer.hpp"

namespace AeonGames
{
    class Material;
    class OpenGLRenderer;
    class OpenGLPipeline;
    /** @brief OpenGL material binding handler for shader uniforms and textures. */
    class OpenGLMaterial
    {
    public:
        /// @brief Construct from a renderer and material resource.
        OpenGLMaterial ( OpenGLRenderer& aOpenGLRenderer, const Material& aMaterial );
        /// @brief Move constructor.
        OpenGLMaterial ( OpenGLMaterial&& aOpenGLMaterial );
        OpenGLMaterial ( const OpenGLMaterial& ) = delete;
        OpenGLMaterial& operator= ( const OpenGLMaterial& ) = delete;
        OpenGLMaterial& operator= ( OpenGLMaterial&& ) = delete;
        ~OpenGLMaterial();
        /// @brief  Binds the material to the specified pipeline.
        /// @param aPipeline The pipeline to use for binding.
        /// @note Not super happy about this API, might change it later.
        void Bind ( const OpenGLPipeline& aPipeline ) const;
        /// @brief Get this material's index into the renderer's global bindless
        ///        material storage buffer, or UINT32_MAX when the bindless path
        ///        is unavailable.
        uint32_t GetBindlessMaterialIndex() const;
    private:
        OpenGLRenderer& mOpenGLRenderer;
        const Material* mMaterial;
        OpenGLBuffer mUniformBuffer{};
        uint32_t mBindlessMaterialIndex{UINT32_MAX};
    };
}
#endif
