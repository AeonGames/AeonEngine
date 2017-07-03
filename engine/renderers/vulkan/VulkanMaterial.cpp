/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.h"
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
#include "aeongames/ResourceCache.h"
#include "VulkanMaterial.h"
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMaterial::VulkanMaterial ( const std::shared_ptr<Material> aMaterial, const VulkanRenderer* aVulkanRenderer ) :
        mVulkanRenderer(),
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

    VulkanMaterial::~VulkanMaterial()
        = default;

    const std::vector<uint8_t>& VulkanMaterial::GetUniformData() const
    {
        return mUniformData;
    }

    const std::vector<std::shared_ptr<VulkanTexture>>& VulkanMaterial::GetTextures() const
    {
        return mTextures;
    }

    void VulkanMaterial::Initialize()
    {
        mUniformData.resize ( mMaterial->GetUniformBlockSize() );
        uint32_t offset = 0;
        for ( auto& i : mMaterial->GetUniformMetaData() )
        {
            uint32_t advance = 0;
            switch ( i.GetType() )
            {
            case Uniform::FLOAT_VEC4:
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 3 ) = i.GetW();
            /* Intentional Pass-Thru */
            case Uniform::FLOAT_VEC3:
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 2 ) = i.GetZ();
                advance += sizeof ( float ) * 2; /* Both VEC3 and VEC4 have a 4 float stride due to std140 padding. */
            /* Intentional Pass-Thru */
            case Uniform::FLOAT_VEC2:
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 1 ) = i.GetY();
                advance += sizeof ( float );
            /* Intentional Pass-Thru */
            case Uniform::FLOAT:
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 0 ) = i.GetX();
                advance += sizeof ( float );
                break;
            case Uniform::SAMPLER_2D:
                mTextures.emplace_back ( Get<VulkanTexture> ( i.GetImage(), mVulkanRenderer ) );
                break;
            default:
                break;
            }
            offset += advance;
        }
    }
    void VulkanMaterial::Finalize()
    {
    }
}
