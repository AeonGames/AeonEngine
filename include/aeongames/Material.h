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
#ifndef AEONGAMES_MATERIAL_H
#define AEONGAMES_MATERIAL_H
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <initializer_list>
#include "aeongames/Platform.h"
#include "aeongames/ResourceId.h"
#include "aeongames/Vector2.h"
#include "aeongames/Vector3.h"
#include "aeongames/Vector4.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/Resource.h"

namespace AeonGames
{
    class Image;
    class MaterialMsg;
    class PropertyMsg;
    class Material : public Resource<MaterialMsg, "AEONMTL"_mgk>
    {
    public:
        using UniformValue = std::variant<uint32_t, int32_t, float, Vector2, Vector3, Vector4, Matrix4x4>;
        using UniformKeyValue = std::tuple<std::string, UniformValue>;
        using SamplerKeyValue = std::tuple<std::string, ResourceId>;
        DLL Material();
        /// The Copy Contsructor is used for virtual copying.
        DLL Material ( const Material& aMaterial );
        /// Assignment operator due to rule of zero/three/five.
        DLL Material& operator= ( const Material& aMaterial );
        /// No move assignment allowed
        Material& operator = ( Material&& ) = delete;
        /// No move allowed
        Material ( Material&& ) = delete;
        DLL ~Material() final;
        ///@name Loaders
        ///@{
        DLL void Load ( const MaterialMsg& aMaterialMsg ) final;
        DLL void Unload() final;
        ///@}
        ///@name Property and Sampler Setters
        ///@{
        DLL void Set ( size_t aIndex, const UniformValue& aValue );
        DLL void Set ( const UniformKeyValue& aValue );
        DLL void SetSampler ( const std::string& aName, const ResourceId& aValue );
        ///@}
        ///@name Property and Sampler Getters
        ///@{
        DLL ResourceId GetSampler ( const std::string& aName );
        DLL const std::vector<std::tuple<std::string, ResourceId>>& GetSamplers() const;
        ///@}
        DLL const std::vector<uint8_t>& GetUniformBuffer() const;
    private:
        class UniformVariable
        {
        public:
            UniformVariable ( const std::string& aName, size_t aOffset ) :
                mName{aName},
                mOffset{aOffset} {}
            const std::string& GetName() const
            {
                return mName;
            }
            size_t GetOffset()
            {
                return mOffset;
            }
        private:
            std::string mName{};
            size_t mOffset{};
        };
        DLL size_t LoadVariables ( const MaterialMsg& aMaterialMsg );
        DLL void LoadSamplers ( const MaterialMsg& aMaterialMsg );
        DLL size_t LoadVariables ( std::initializer_list<UniformKeyValue> aUniforms );
        DLL void LoadSamplers ( std::initializer_list<SamplerKeyValue> aSamplers );
        std::vector<UniformVariable> mVariables{};
        std::vector<std::tuple<std::string, ResourceId>> mSamplers{};
        std::vector<uint8_t> mUniformBuffer{};
    };
    DLL size_t GetUniformValueSize ( const Material::UniformValue& aValue );
    DLL const void* GetUniformValuePointer ( const Material::UniformValue& aValue );
}
#endif
