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
#ifndef AEONGAMES_METALMATERIAL_H
#define AEONGAMES_METALMATERIAL_H

#include <array>
#include <cstdint>
#include <memory>
#include "aeongames/MaterialSamplers.hpp"

namespace AeonGames
{
    class Material;
    class MetalBindlessResources;
    class MetalTexture;
    /** @brief Bindless material record and owned texture/sampler-pair slots. */
    class MetalMaterial
    {
    public:
        MetalMaterial ( MetalBindlessResources& aBindless, const Material& aMaterial,
                        const std::array<const MetalTexture*, kMaterialSamplerSlots.size() >& aTextures );
        ~MetalMaterial();
        MetalMaterial ( const MetalMaterial& ) = delete;
        MetalMaterial& operator= ( const MetalMaterial& ) = delete;

        uint32_t GetBindlessIndex() const;

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
#endif