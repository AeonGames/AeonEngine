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
#include "aeongames/Platform.h"
#include "aeongames/Memory.h"

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
        class IRenderMaterial
        {
        public:
            virtual void Update ( const uint8_t* aValue, size_t aOffset = 0, size_t aSize = 0 ) = 0;
            virtual ~IRenderMaterial() {};
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
        DLL ~Material();
        DLL void Load ( const std::string& aFilename );
        DLL void Load ( const void* aBuffer, size_t aBufferSize );
        DLL void Load ( const MaterialBuffer& aMaterialBuffer );
        DLL void Unload();
        DLL const std::vector<Property>& GetProperties() const;
        DLL const std::vector<uint8_t>& GetPropertyBlock() const;
        DLL void SetRenderMaterial ( std::unique_ptr<IRenderMaterial> aRenderMaterial ) const;
        DLL const IRenderMaterial* const GetRenderMaterial() const;
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
    private:
        std::vector<Property> mProperties{};
        std::vector<uint8_t> mPropertyBlock{};
        mutable std::unique_ptr<IRenderMaterial> mRenderMaterial{};
    };
}
#endif
