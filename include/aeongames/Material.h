/*
Copyright (C) 2016-2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MATERIAL_H
#define AEONGAMES_MATERIAL_H
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include "aeongames/Platform.h"
#include "aeongames/ResourceId.h"
#include "aeongames/Vector2.h"
#include "aeongames/Vector3.h"
#include "aeongames/Vector4.h"
#include "aeongames/Matrix4x4.h"

namespace AeonGames
{
    class Image;
    class MaterialBuffer;
    class PropertyBuffer;
    class Material
    {
    public:
        using UniformValue = std::variant<uint32_t, int32_t, float, Vector2, Vector3, Vector4, Matrix4x4, ResourceId>;
        using UniformKeyValue = std::tuple<std::string, UniformValue>;
        DLL virtual ~Material() = 0;
        /// Virtual Copy Constructor
        virtual std::unique_ptr<Material> Clone() const = 0;
        ///@name Loaders
        ///@{
        DLL void Load ( const std::string& aFilename );
        DLL void Load ( const uint32_t aId );
        DLL void Load ( const void* aBuffer, size_t aBufferSize );
        virtual void Load ( const MaterialBuffer& aMaterialBuffer ) = 0;
        virtual void Unload() = 0;
        ///@}
        ///@name Property and Sampler Setters
        ///@{
        virtual void Set ( const std::string& aName, const UniformValue& aValue ) = 0;
        ///@}
        ///@name Property and Sampler Getters
        ///@{
        virtual uint32_t GetUint ( const std::string& aName ) = 0;
        virtual int32_t GetSint ( const std::string& aName ) = 0;
        virtual float GetFloat ( const std::string& aName ) = 0;
        virtual Vector2 GetFloatVec2 ( const std::string& aName ) = 0;
        virtual Vector3 GetFloatVec3 ( const std::string& aName ) = 0;
        virtual Vector4 GetFloatVec4 ( const std::string& aName ) = 0;
        virtual ResourceId GetSampler ( const std::string& aName ) = 0;
        virtual const std::vector<std::tuple<std::string, ResourceId>>& GetSamplers() const = 0;
        ///@}
    };
}
#endif
