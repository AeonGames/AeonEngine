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
#include "aeongames/Memory.h"

namespace AeonGames
{
    class Image;
    class MaterialBuffer;
    class Material
    {
    public:
        class IRenderMaterial
        {
        public:
            virtual void Update ( const uint8_t* aValue, size_t aOffset = 0, size_t aSize = 0 ) = 0;
            virtual ~IRenderMaterial() {};
        };
        class Uniform
        {
        public:
            enum Type
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
            Uniform ( const std::string&  aName, float aX, uint8_t* aData );
            Uniform ( const std::string&  aName, uint32_t aX, uint8_t* aData );
            Uniform ( const std::string&  aName, int32_t aX, uint8_t* aData );
            Uniform ( const std::string&  aName, float aX, float aY, uint8_t* aData );
            Uniform ( const std::string&  aName, float aX, float aY, float aZ, uint8_t* aData );
            Uniform ( const std::string&  aName, float aX, float aY, float aZ, float aW, uint8_t* aData );
            Uniform ( const std::string&  aName, const std::string& aFilename );
            ~Uniform();
            ///@name Getters
            ///@{
            DLL Type GetType() const;
            DLL const std::string GetDeclaration() const;
            DLL const std::string& GetName() const;
            DLL uint32_t GetUInt() const;
            DLL int32_t GetSInt() const;
            DLL float GetX() const;
            DLL float GetY() const;
            DLL float GetZ() const;
            DLL float GetW() const;
            DLL const std::shared_ptr<Image> GetImage() const;
            ///@}
            ///@name Setters
            ///@{
            DLL void SetUInt ( uint32_t aValue );
            DLL void SetSInt ( int32_t aValue );
            DLL void SetX ( float aValue );
            DLL void SetY ( float aValue );
            DLL void SetZ ( float aValue );
            DLL void SetW ( float aValue );
            DLL void Set ( void* aValue );
            ///@}
        private:
            std::string mName{};
            Type mType{ UNKNOWN };
            uint8_t* mData{};
        };

        DLL Material();
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
        DLL const std::vector<Uniform>& GetUniforms() const;
        DLL const std::vector<uint8_t>& GetUniformBlock() const;
        DLL void SetUniform ( const std::string& aName, void* aValue );
        DLL void SetRenderMaterial ( std::unique_ptr<IRenderMaterial> aRenderMaterial ) const;
        DLL const IRenderMaterial* const GetRenderMaterial() const;
    private:
        std::vector<Uniform> mUniforms{};
        std::vector<uint8_t> mUniformBlock{};
        mutable std::unique_ptr<IRenderMaterial> mRenderMaterial{};
    };
}
#endif
