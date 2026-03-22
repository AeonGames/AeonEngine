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
#ifndef AEONGAMES_MATERIAL_H
#define AEONGAMES_MATERIAL_H
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <initializer_list>
#include "aeongames/Platform.hpp"
#include "aeongames/ResourceId.hpp"
#include "aeongames/Vector2.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Vector4.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Resource.hpp"

namespace AeonGames
{
    class Image;
    class MaterialMsg;
    class PropertyMsg;
    /** @brief Represents a surface material with uniform properties and texture samplers. */
    class Material final : public Resource
    {
    public:
        /** @brief Variant type holding any supported uniform value. */
        using UniformValue = std::variant<uint32_t, int32_t, float, Vector2, Vector3, Vector4, Matrix4x4>;
        /** @brief Key-value pair mapping a uniform name to its value. */
        using UniformKeyValue = std::tuple<std::string, UniformValue>;
        /** @brief Key-value pair mapping a sampler binding index to an image resource id. */
        using SamplerKeyValue = std::tuple<uint32_t, ResourceId>;
        /** @brief Default constructor. */
        DLL Material();
        /// The Copy Contsructor is used for virtual copying.
        DLL Material ( const Material& aMaterial );
        /// Assignment operator due to rule of zero/three/five.
        DLL Material& operator= ( const Material& aMaterial );
        /// No move assignment allowed
        Material& operator = ( Material&& ) = delete;
        /// No move allowed
        Material ( Material&& ) = delete;
        /** @brief Destructor. */
        DLL ~Material() final;
        ///@name Loaders
        ///@{
        /** @brief Load material data from a protobuf message.
         *  @param aMaterialMsg The protobuf message to load from.
         */
        DLL void LoadFromPBMsg ( const MaterialMsg& aMaterialMsg );
        /** @brief Load material data from a raw memory buffer.
         *  @param aBuffer Pointer to the buffer.
         *  @param aBufferSize Size of the buffer in bytes.
         */
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        /** @brief Unload material data and release resources. */
        DLL void Unload() final;
        ///@}
        ///@name Property and Sampler Setters
        ///@{
        /** @brief Set a uniform value by index.
         *  @param aIndex Index of the uniform variable.
         *  @param aValue New value to assign.
         */
        DLL void Set ( size_t aIndex, const UniformValue& aValue );
        /** @brief Set a uniform value by name-value pair.
         *  @param aValue Key-value pair with the uniform name and value.
         */
        DLL void Set ( const UniformKeyValue& aValue );
        /** @brief Set a sampler binding by name.
         *  @param aName Sampler name.
         *  @param aValue Resource id of the image to bind.
         */
        DLL void SetSampler ( const std::string& aName, const ResourceId& aValue );
        ///@}
        ///@name Property and Sampler Getters
        ///@{
        /** @brief Get the resource id of a sampler by name.
         *  @param aName Sampler name.
         *  @return Resource id bound to the sampler.
         */
        DLL ResourceId GetSampler ( const std::string& aName );
        /** @brief Get all sampler bindings.
         *  @return Const reference to the vector of sampler key-value pairs.
         */
        DLL const std::vector<std::tuple<uint32_t, ResourceId >> & GetSamplers() const;
        ///@}
        /** @brief Get the raw uniform buffer.
         *  @return Const reference to the uniform data byte vector.
         */
        DLL const std::vector<uint8_t>& GetUniformBuffer() const;
    private:
        /** @brief Internal representation of a named uniform variable with an offset into the uniform buffer. */
        class UniformVariable
        {
        public:
            UniformVariable ( const std::string & aName, size_t aOffset ) :
                mName{aName},
                mOffset{aOffset} {}
            const std::string & GetName() const
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
        std::vector<SamplerKeyValue> mSamplers{};
        std::vector<uint8_t> mUniformBuffer{};
    };
    /** @brief Get the byte size of a uniform value.
     *  @param aValue The uniform value variant.
     *  @return Size in bytes.
     */
    DLL size_t GetUniformValueSize ( const Material::UniformValue& aValue );
    /** @brief Get a pointer to the raw data of a uniform value.
     *  @param aValue The uniform value variant.
     *  @return Pointer to the value's underlying data.
     */
    DLL const void* GetUniformValuePointer ( const Material::UniformValue& aValue );
}
#endif
