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
#ifndef AEONGAMES_METALBINDLESSRESOURCES_H
#define AEONGAMES_METALBINDLESSRESOURCES_H

#include <cstdint>
#include <memory>
#include "aeongames/Material.hpp"
#include "aeongames/Renderer.hpp"

namespace AeonGames
{
    struct GpuMaterial;
    class MetalTexture;
    /** @brief Renderer-owned bindless texture/sampler pairs and material records. */
    class MetalBindlessResources
    {
    public:
        static constexpr uint32_t kShaderTextureCapacity = 16384;

        MetalBindlessResources ( void* aDevice, const RendererSettings& aSettings );
        ~MetalBindlessResources();
        MetalBindlessResources ( const MetalBindlessResources& ) = delete;
        MetalBindlessResources& operator= ( const MetalBindlessResources& ) = delete;

        uint32_t AcquirePair ( const MetalTexture& aTexture, const Material::SamplerState& aState );
        void ReleasePair ( uint32_t aSlot );
        uint32_t RegisterMaterial ( const GpuMaterial& aMaterial );
        void UnregisterMaterial ( uint32_t aIndex );

        uint64_t GetGeneration() const;
        /** Create and populate set 2 for the supplied fragment function. */
        void* CreateTextureArgumentBuffer ( void* aFragmentFunction ) const;
        /** Create and populate set 7 with the material-record buffer. */
        void* CreateMaterialArgumentBuffer ( void* aFragmentFunction ) const;
        void UseResources ( void* aRenderEncoder ) const;

    private:
        class Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
#endif