/*
Copyright (C) 2016-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include <tuple>
#include <memory>
#include "aeongames/Material.h"
#include "aeongames/Vector2.h"
#include "aeongames/Vector3.h"
#include "aeongames/Vector4.h"
#include "OpenGLBuffer.h"

namespace AeonGames
{
    class OpenGLRenderer;
    class OpenGLTexture;
    class OpenGLMaterial final : public Material
    {
    public:
        OpenGLMaterial ( uint32_t aPath = 0 );
        OpenGLMaterial ( std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers );
        /// The Copy Contsructor is used for virtual copying.
        OpenGLMaterial ( const OpenGLMaterial& aMaterial );
        /// Assignment operator due to rule of zero/three/five.
        OpenGLMaterial& operator= ( const OpenGLMaterial& aMaterial );
        /// No move assignment allowed
        OpenGLMaterial& operator = ( OpenGLMaterial&& ) = delete;
        /// No move allowed
        OpenGLMaterial ( OpenGLMaterial&& ) = delete;
        ~OpenGLMaterial() final;
        /// @copydoc Material::Clone()
        std::unique_ptr<Material> Clone() const final;
        ///@name Loaders
        ///@{
        void Load ( const MaterialMsg& aMaterialMsg ) final;
        void Load ( std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers ) final;
        void Unload() final;
        ///@}
        ///@name Property and Sampler Setters
        ///@{
        void Set ( size_t aIndex, const UniformValue& aValue ) final;
        void Set ( const UniformKeyValue& aValue ) final;
        void SetSampler ( const std::string& aName, const ResourceId& aValue ) final;
        ///@}
        ///@name Property and Sampler Getters
        ///@{
        ResourceId GetSampler ( const std::string& aName ) final;
        const std::vector<std::tuple<std::string, ResourceId>>& GetSamplers() const final;
        ///@}
        GLuint GetPropertiesBufferId() const;
    private:
        OpenGLBuffer mUniformBuffer{};
    };
}
#endif
