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
#include "OpenGLFunctions.h"
#include "OpenGLMaterial.h"
#include "OpenGLTexture.h"

namespace AeonGames
{
    OpenGLMaterial::OpenGLMaterial ( const Material& aMaterial, const std::shared_ptr<const OpenGLRenderer>& aOpenGLRenderer ) :
        mMaterial ( aMaterial )
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

    void OpenGLMaterial::Update ( size_t aOffset, size_t aSize, uint8_t aValue )
    {
    }

    const std::vector<std::shared_ptr<OpenGLTexture>>& OpenGLMaterial::GetTextures() const
    {
        return mTextures;
    }

    GLuint OpenGLMaterial::GetPropertiesBuffer() const
    {
        return mPropertiesBuffer;
    }

    void OpenGLMaterial::Initialize()
    {
#if 0
        uint32_t offset = 0;
        GLint image_unit = 0;
        for ( auto& i : mMaterial.GetUniformMetaData() )
        {
            switch ( i.GetType() )
            {
            case Uniform::FLOAT_VEC4:
                offset += ( offset % 16 ) ? 16 - ( offset % 16 ) : 0;
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 3 ) = i.GetW();
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 2 ) = i.GetZ();
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 1 ) = i.GetY();
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 0 ) = i.GetX();
                offset += sizeof ( float ) * 4;
                break;
            case Uniform::FLOAT_VEC3:
                offset += ( offset % 16 ) ? 16 - ( offset % 16 ) : 0;
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 2 ) = i.GetZ();
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 1 ) = i.GetY();
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 0 ) = i.GetX();
                offset += sizeof ( float ) * 3;
                break;
            case Uniform::FLOAT_VEC2:
                offset += ( offset % 8 ) ? 8 - ( offset % 8 ) : 0;
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 1 ) = i.GetY();
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 0 ) = i.GetX();
                offset += sizeof ( float ) * 2;
                break;
            case Uniform::FLOAT:
                offset += ( offset % 4 ) ? 4 - ( offset % 4 ) : 0;
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 0 ) = i.GetX();
                offset += sizeof ( float );
                break;
            case Uniform::UINT:
                offset += ( offset % 4 ) ? 4 - ( offset % 4 ) : 0;
                * ( reinterpret_cast<uint32_t*> ( mUniformData.data() + offset ) + 0 ) = i.GetUInt();
                offset += sizeof ( float );
                break;
            case Uniform::SAMPLER_2D:
                if ( image_unit >= ( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS - GL_TEXTURE0 ) )
                {
                    throw std::runtime_error ( "OpenGL texture image unit values exausted (Too many samplers in shader)." );
                }
                mTextures.emplace_back ( Get<OpenGLTexture> ( i.GetImage().get(), i.GetImage(), mOpenGLRenderer ) );
                break;
            default:
                break;
            }
        }
#endif
        glGenBuffers ( 1, &mPropertiesBuffer );
        OPENGL_CHECK_ERROR_THROW;
        glNamedBufferData ( mPropertiesBuffer, mMaterial.GetUniformBlock().size(), mMaterial.GetUniformBlock().data(), GL_DYNAMIC_DRAW );
        OPENGL_CHECK_ERROR_THROW;
    }
    void OpenGLMaterial::Finalize()
    {
        if ( glIsBuffer ( mPropertiesBuffer ) )
        {
            OPENGL_CHECK_ERROR_NO_THROW;
            glDeleteBuffers ( 1, &mPropertiesBuffer );
            OPENGL_CHECK_ERROR_NO_THROW;
            mPropertiesBuffer = 0;
        }
        OPENGL_CHECK_ERROR_NO_THROW;
    }
}
