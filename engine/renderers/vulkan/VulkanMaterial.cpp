/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#include <array>
#include <utility>
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
    VulkanMaterial::VulkanMaterial ( const Material& aMaterial, const std::shared_ptr<const VulkanRenderer>&  aVulkanRenderer ) :
        mVulkanRenderer ( aVulkanRenderer ),
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

    void VulkanMaterial::Update ( const uint8_t* aValue, size_t aOffset, size_t aSize )
    {
    }

    const std::vector<std::shared_ptr<VulkanTexture>>& VulkanMaterial::GetTextures() const
    {
        return mTextures;
    }

    void VulkanMaterial::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        for ( auto& i : mMaterial.GetProperties() )
        {
            switch ( i.GetType() )
            {
            case Material::SAMPLER_2D:
                ///@todo to be reworked.
                //mTextures.emplace_back ( Get<VulkanTexture> ( i.GetImage().get(), i.GetImage(), mVulkanRenderer ) );
                break;
            default:
                break;
            }
        }
    }
    void VulkanMaterial::Finalize()
    {
    }
}
