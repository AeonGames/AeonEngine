/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#if 1
    class Material
    {
    public:
        DLL virtual ~Material() = 0;
        /// Virtual Copy Constructor
        virtual std::unique_ptr<Material> Clone() const = 0;
        ///@name Loaders
        ///@{
        virtual void Load ( const std::string& aFilename ) = 0;
        virtual void Load ( const uint32_t aId ) = 0;
        virtual void Load ( const void* aBuffer, size_t aBufferSize ) = 0;
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
#else
    class Material
    {
    public:
        enum PropertyType
        {
            UNKNOWN = 0,
            UINT,
            FLOAT,
            SINT,
            FLOAT_VEC2,
            FLOAT_VEC3,
            FLOAT_VEC4,
            SAMPLER_2D,
            SAMPLER_CUBE,
        };
    private:
        class Property
        {
        public:
            Property ( Material& aMaterial, const PropertyBuffer& aPropertyBuffer );
            ~Property();
            DLL Property ( const Property& aProperty );
            DLL Property& operator = ( const Property& aProperty );
            ///@name Getters
            ///@{
            DLL PropertyType GetType() const;
            DLL const std::string GetDeclaration() const;
            DLL const std::string& GetName() const;
            DLL uint32_t GetUint() const;
            DLL int32_t GetSint() const;
            DLL float GetFloat() const;
            DLL Vector2 GetVector2() const;
            DLL Vector3 GetVector3() const;
            DLL Vector4 GetVector4() const;
            DLL const Image* GetImage() const;
            ///@}
            ///@name Setters
            ///@{
            DLL void Set ( uint32_t aValue );
            DLL void Set ( int32_t aValue );
            DLL void Set ( float aValue );
            DLL void Set ( const Vector2& aValue );
            DLL void Set ( const Vector3& aValue );
            DLL void Set ( const Vector4& aValue );
            DLL void Set ( const std::string& aFileName );
            ///@}
        private:
            friend class Material;
            Material* mMaterial{};
            std::string mName{};
            PropertyType mType{ UNKNOWN };
            union
            {
                size_t mOffset;
                std::unique_ptr<AeonGames::Image> mImage;
            };
        };
    public:
        DLL Material();
        DLL Material ( uint32_t aId );
        DLL Material ( const std::string& aFilename );
        DLL Material ( const void* aBuffer, size_t aBufferSize );
        DLL Material ( const MaterialBuffer& aMaterialBuffer );
        DLL Material ( const Material& aMaterial );
        DLL Material& operator = ( const Material& aMaterial );
        DLL virtual ~Material();
        DLL void Load ( const std::string& aFilename );
        DLL void Load ( const void* aBuffer, size_t aBufferSize );
        DLL void Load ( const MaterialBuffer& aMaterialBuffer );
        DLL void Unload();
        DLL const std::vector<Property>& GetProperties() const;
        DLL const std::vector<uint8_t>& GetPropertyBlock() const;
        DLL size_t GetSamplerCount() const;
        ///@}
        ///@name Property Setters
        ///@{
        DLL void Set ( const std::string& aName, uint32_t aValue );
        DLL void Set ( const std::string& aName, int32_t aValue );
        DLL void Set ( const std::string& aName, float aValue );
        DLL void Set ( const std::string& aName, const Vector2& aValue );
        DLL void Set ( const std::string& aName, const Vector3& aValue );
        DLL void Set ( const std::string& aName, const Vector4& aValue );
        ///@}
        DLL static const std::shared_ptr<Material> GetMaterial ( uint32_t aId );
        DLL static const std::shared_ptr<Material> GetMaterial ( const std::string& aPath );
    private:
        std::vector<Property> mProperties{};
        std::vector<uint8_t> mPropertyBlock{};
    };
#endif
}
#endif
