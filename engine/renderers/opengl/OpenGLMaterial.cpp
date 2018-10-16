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
#include <fstream>
#include <sstream>
#include <ostream>
#include <regex>
#include <utility>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ResourceCache.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Material.h"
#include "aeongames/CRC.h"
#include "OpenGLFunctions.h"
#include "OpenGLMaterial.h"
#include "OpenGLImage.h"

namespace AeonGames
{
    void OpenGLMaterial::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }
    void OpenGLMaterial::Load ( const uint32_t aId )
    {

    }
    void OpenGLMaterial::Load ( const void* aBuffer, size_t aBufferSize )
    {
    }
    void OpenGLMaterial::Load ( const MaterialBuffer& aMaterialBuffer )
    {
        //mPropertiesBuffer.Initialize ( static_cast<GLsizei> ( GetPropertyBlock().size() ), GL_DYNAMIC_DRAW, GetPropertyBlock().data() );
    }
    void OpenGLMaterial::Unload() {}
    void OpenGLMaterial::AddUint ( const std::string& aName, uint32_t aValue ) {}
    void OpenGLMaterial::AddSint ( const std::string& aName, int32_t aValue ) {}
    void OpenGLMaterial::AddFloat ( const std::string& aName, float aValue ) {}
    void OpenGLMaterial::AddFloatVec2 ( const std::string& aName, const Vector2& aValue ) {}
    void OpenGLMaterial::AddFloatVec3 ( const std::string& aName, const Vector3& aValue ) {}
    void OpenGLMaterial::AddFloatVec4 ( const std::string& aName, const Vector4& aValue ) {}
    void OpenGLMaterial::AddSampler ( const std::string& aName, const std::string& aValue ) {}
    void OpenGLMaterial::Remove ( const std::string& aName ) {}
    void OpenGLMaterial::SetUint ( const std::string& aName, uint32_t aValue ) {}
    void OpenGLMaterial::SetSint ( const std::string& aName, int32_t aValue ) {}
    void OpenGLMaterial::SetFloat ( const std::string& aName, float aValue ) {}
    void OpenGLMaterial::SetFloatVec2 ( const std::string& aName, const Vector2& aValue ) {}
    void OpenGLMaterial::SetFloatVec3 ( const std::string& aName, const Vector3& aValue ) {}
    void OpenGLMaterial::SetFloatVec4 ( const std::string& aName, const Vector4& aValue ) {}
    void OpenGLMaterial::SetSampler ( const std::string& aName, const std::string& aValue ) {}
    uint32_t OpenGLMaterial::GetUint ( const std::string& aName )
    {
        return 0;
    }
    int32_t OpenGLMaterial::GetSint ( const std::string& aName )
    {
        return 0;
    }
    float OpenGLMaterial::GetFloat ( const std::string& aName )
    {
        return 0.0f;
    }
    Vector2 OpenGLMaterial::GetFloatVec2 ( const std::string& aName )
    {
        return Vector2{};
    }
    Vector3 OpenGLMaterial::GetFloatVec3 ( const std::string& aName )
    {
        return Vector3{};
    }
    Vector4 OpenGLMaterial::GetFloatVec4 ( const std::string& aName )
    {
        return Vector4{};
    }
    std::string OpenGLMaterial::GetSampler ( const std::string& aName )
    {
        return std::string{};
    }

    OpenGLMaterial::OpenGLMaterial() :
        Material(), mPropertiesBuffer{}
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    OpenGLMaterial::~OpenGLMaterial()
    {
        Finalize();
    }

    GLuint OpenGLMaterial::GetPropertiesBufferId() const
    {
        return mPropertiesBuffer.GetBufferId();
    }

    void OpenGLMaterial::Initialize()
    {
    }

    void OpenGLMaterial::Finalize()
    {
        mPropertiesBuffer.Finalize();
    }
}
