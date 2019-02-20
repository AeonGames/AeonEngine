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
#include "aeongames/Platform.h"
#include "aeongames/ResourceId.h"

namespace AeonGames
{
    class Image;
    class Vector2;
    class Vector3;
    class Vector4;
    class MaterialBuffer;
    class PropertyBuffer;
    class Material
    {
    public:
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
        virtual void SetUint ( const std::string& aName, uint32_t aValue ) = 0;
        virtual void SetSint ( const std::string& aName, int32_t aValue ) = 0;
        virtual void SetFloat ( const std::string& aName, float aValue ) = 0;
        virtual void SetFloatVec2 ( const std::string& aName, const Vector2& aValue ) = 0;
        virtual void SetFloatVec3 ( const std::string& aName, const Vector3& aValue ) = 0;
        virtual void SetFloatVec4 ( const std::string& aName, const Vector4& aValue ) = 0;
        virtual void SetSampler ( const std::string& aName, const ResourceId& aValue ) = 0;
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
