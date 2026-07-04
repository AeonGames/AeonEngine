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
#ifndef AEONGAMES_MATERIALSAMPLERS_HPP
#define AEONGAMES_MATERIALSAMPLERS_HPP
#include <array>
#include <cstddef>

namespace AeonGames
{
    /** @brief One canonical material texture-sampler slot. */
    struct MaterialSamplerSlot
    {
        const char* name;          ///< GLSL sampler name; @c crc32i(name) keys Material::GetSamplers().
        const char* fallback_path; ///< Engine fallback texture bound when a material omits this sampler.
    };

    /** @brief Canonical, engine-wide material sampler layout.
     *
     *  The array index is the descriptor binding slot that every material-
     *  sampling shader (e.g. clustered_phong) declares in the material sampler
     *  set, in this exact order. A material only needs to supply the samplers it
     *  actually uses; the renderers pad every omitted slot with the listed
     *  fallback texture so the material's descriptor set layout always matches
     *  the pipeline (Vulkan binds the whole set by position) and every sampled
     *  texture unit is populated (OpenGL binds by name).
     *
     *  The @c NormalMap fallback is a flat (0,0,1) tangent-space normal, so
     *  sampling it is a no-op after the TBN transform: materials without a
     *  normal map keep their interpolated vertex normal. The @c MetallicMap,
     *  @c RoughnessMap and @c OcclusionMap fallbacks are white (value 1.0), and
     *  the @c EmissiveMap fallback is white too but is scaled by an EmissiveFactor
     *  that defaults to zero; a metallic-roughness shader multiplies each sampled
     *  value by the matching material factor, so a material with no such map falls
     *  back to its scalar factor (or, for occlusion, to no occlusion). This lets
     *  the full PBR texture set be added to the shared shader without requiring
     *  every material to supply every texture.
     *
     *  When extending this list, append new slots and update every material-
     *  sampling shader to declare the full set in the same order. */
    inline constexpr std::array<MaterialSamplerSlot, 6> kMaterialSamplerSlots
    {
        {
            { "DiffuseMap",   "textures/default.png" },
            { "NormalMap",    "textures/flat_normal.png" },
            { "MetallicMap",  "textures/default.png" },
            { "RoughnessMap", "textures/default.png" },
            { "OcclusionMap", "textures/default.png" },
            { "EmissiveMap",  "textures/default.png" },
        }};
}
#endif
