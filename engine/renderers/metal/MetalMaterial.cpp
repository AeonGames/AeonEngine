/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include "aeongames/CRC.hpp"
#include "aeongames/GpuMaterial.hpp"
#include "aeongames/Material.hpp"
#include "MetalBindlessResources.h"
#include "MetalMaterial.h"
#include "MetalTexture.h"

namespace AeonGames
{
    class MetalMaterial::Impl
    {
    public:
        Impl ( MetalBindlessResources& aBindless, const Material& aMaterial,
               const std::array<const MetalTexture*, kMaterialSamplerSlots.size() >& aTextures ) :
            bindless{aBindless}
        {
            pair_slots.fill ( std::numeric_limits<uint32_t>::max() );
            GpuMaterial record{};
            try
            {
                for ( uint32_t slot = 0; slot < kMaterialSamplerSlots.size(); ++slot )
                {
                    Material::SamplerState state{};
                    const uint32_t slot_hash = crc32i ( kMaterialSamplerSlots[slot].name,
                                                        std::strlen ( kMaterialSamplerSlots[slot].name ) );
                    for ( const Material::SamplerKeyValue& sampler : aMaterial.GetSamplers() )
                    {
                        if ( std::get<0> ( sampler ) == slot_hash )
                        {
                            state = std::get<2> ( sampler );
                            break;
                        }
                    }
                    pair_slots[slot] = bindless.AcquirePair ( *aTextures[slot], state );
                    record.texture_refs[slot][0] = pair_slots[slot];
                }
                if ( !aMaterial.GetUniformBuffer().empty() )
                {
                    const size_t factor_offset = sizeof ( record.texture_refs );
                    const size_t factor_size = sizeof ( GpuMaterial ) - factor_offset;
                    std::memcpy ( reinterpret_cast<uint8_t*> ( &record ) + factor_offset,
                                  aMaterial.GetUniformBuffer().data(),
                                  std::min ( factor_size, aMaterial.GetUniformBuffer().size() ) );
                }
                material_index = bindless.RegisterMaterial ( record );
            }
            catch ( ... )
            {
                for ( uint32_t pair_slot : pair_slots )
                {
                    if ( pair_slot != std::numeric_limits<uint32_t>::max() )
                    {
                        bindless.ReleasePair ( pair_slot );
                    }
                }
                throw;
            }
        }

        ~Impl()
        {
            bindless.UnregisterMaterial ( material_index );
            for ( uint32_t slot : pair_slots )
            {
                bindless.ReleasePair ( slot );
            }
        }

        MetalBindlessResources& bindless;
        std::array<uint32_t, kMaterialSamplerSlots.size() > pair_slots{};
        uint32_t material_index{std::numeric_limits<uint32_t>::max() };
    };

    MetalMaterial::MetalMaterial ( MetalBindlessResources& aBindless, const Material& aMaterial,
                                   const std::array<const MetalTexture*, kMaterialSamplerSlots.size() >& aTextures ) :
        mImpl{std::make_unique<Impl> ( aBindless, aMaterial, aTextures ) }
    {
    }

    MetalMaterial::~MetalMaterial() = default;

    uint32_t MetalMaterial::GetBindlessIndex() const
    {
        return mImpl->material_index;
    }
}