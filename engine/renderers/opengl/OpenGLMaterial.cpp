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
#include <fstream>
#include <sstream>
#include <ostream>
#include <regex>
#include <utility>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufUtils.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/ProtoBufUtils.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/AeonEngine.h"
#include "aeongames/Material.h"
#include "aeongames/CRC.h"
#include "OpenGLFunctions.h"
#include "OpenGLMaterial.h"
#include "OpenGLImage.h"

namespace AeonGames
{
    OpenGLMaterial::OpenGLMaterial ( uint32_t aPath ) :
        Material{},
        mUniformBuffer{}
    {
        if ( aPath )
        {
            Material::Load ( aPath );
        }
    }

    OpenGLMaterial::OpenGLMaterial ( std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers ) :
        Material{},
        mUniformBuffer{}
    {
        OpenGLMaterial::Load ( aUniforms, aSamplers );
    }

    void OpenGLMaterial::Load ( std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers )
    {

    }

    OpenGLMaterial::OpenGLMaterial ( const OpenGLMaterial& aMaterial ) :
        Material{aMaterial},
        mUniformBuffer{aMaterial.mUniformBuffer}
    {
    }

    OpenGLMaterial& OpenGLMaterial::operator= ( const OpenGLMaterial& aMaterial )
    {
        this->Material::operator= ( aMaterial );
        mUniformBuffer = aMaterial.mUniformBuffer;
        return *this;
    }

    std::unique_ptr<Material> OpenGLMaterial::Clone() const
    {
        return std::make_unique<OpenGLMaterial> ( *this );
    }

    void OpenGLMaterial::Load ( const MaterialBuffer& aMaterialBuffer )
    {
        size_t size = LoadVariables ( aMaterialBuffer );
        if ( size )
        {
            mUniformBuffer.Initialize ( static_cast<GLsizei> ( size ), GL_DYNAMIC_DRAW );
            for ( auto& i : aMaterialBuffer.property() )
            {
                Set ( PropertyToKeyValue ( i ) );
            }
        }
        LoadSamplers ( aMaterialBuffer );
    }

    void OpenGLMaterial::Unload()
    {
        mUniformBuffer.Finalize();
    }

    void OpenGLMaterial::Set ( size_t aIndex, const UniformValue& aValue )
    {
        mUniformBuffer.WriteMemory ( mVariables.at ( aIndex ).GetOffset(), GetUniformValueSize ( aValue ), GetUniformValuePointer ( aValue ) );
    }

    void OpenGLMaterial::Set ( const UniformKeyValue& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aValue] ( const UniformVariable & variable )
        {
            return variable.GetName() == std::get<std::string> ( aValue );
        } );
        if ( i != mVariables.end() )
        {
            size_t value_size = GetUniformValueSize ( std::get<UniformValue> ( aValue ) );
            // Do some bounds checking
            auto j = i + 1;
            if ( ( ( j != mVariables.end() ) ? ( j->GetOffset() - i->GetOffset() ) : ( mUniformBuffer.GetSize() - i->GetOffset() ) ) < value_size )
            {
                throw std::runtime_error ( "Value type size exceeds original type size." );
            }
            mUniformBuffer.WriteMemory ( i->GetOffset(), value_size, GetUniformValuePointer ( std::get<UniformValue> ( aValue ) ) );
        }
    }

    void OpenGLMaterial::SetSampler ( const std::string& aName, const ResourceId& aValue )
    {
        auto i = std::find_if ( mSamplers.begin(), mSamplers.end(),
                                [&aName] ( const std::tuple<std::string, ResourceId>& aTuple )
        {
            return std::get<0> ( aTuple ) == aName;
        } );
        if ( i != mSamplers.end() )
        {
            std::get<1> ( *i ) = aValue;
        }
    }

    ResourceId OpenGLMaterial::GetSampler ( const std::string& aName )
    {
        auto i = std::find_if ( mSamplers.begin(), mSamplers.end(),
                                [&aName] ( const std::tuple<std::string, ResourceId>& aTuple )
        {
            return std::get<0> ( aTuple ) == aName;
        } );
        if ( i != mSamplers.end() )
        {
            return std::get<1> ( *i );
        }
        return ResourceId{"Image"_crc32, 0};
    }

    OpenGLMaterial::~OpenGLMaterial()
    {
        Unload();
    }

    GLuint OpenGLMaterial::GetPropertiesBufferId() const
    {
        return mUniformBuffer.GetBufferId();
    }

    const std::vector<std::tuple<std::string, ResourceId>>& OpenGLMaterial::GetSamplers() const
    {
        return mSamplers;
    }
}
