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
#ifndef AEONGAMES_GPUMATERIAL_HPP
#define AEONGAMES_GPUMATERIAL_HPP

#include <cstdint>
#include "aeongames/Vector4.hpp"
#include "aeongames/MaterialSamplers.hpp"

namespace AeonGames
{
    /** @brief CPU-side mirror of one bindless material record in the global
     *  material storage buffer.
     *
     *  Bindless rendering replaces the per-material sampler descriptor set (and,
     *  on OpenGL, the per-draw texture-unit binds) with a single global material
     *  SSBO indexed per draw by a material index (pushed as a constant on
     *  Vulkan, or via gl_DrawID/uniform on OpenGL). Each record carries the six
     *  canonical texture references plus the metallic-roughness factors that used
     *  to live in the @c Material uniform block, so a shading shader reads
     *  everything it needs for a surface from one indexed struct.
     *
     *  @c texture_refs holds one @c uvec2 per @ref kMaterialSamplerSlots entry
     *  (DiffuseMap, NormalMap, MetallicMap, RoughnessMap, OcclusionMap,
     *  EmissiveMap, in that order):
     *   - Vulkan: @c .x is the slot index into the renderer's global
     *     combined-image-sampler array (@c textures[nonuniformEXT(idx)]);
     *     @c .y is unused (0).
     *   - OpenGL: the whole @c uvec2 is a resident @c GL_ARB_bindless_texture
     *     sampler handle (low 32 bits in @c .x, high 32 bits in @c .y), used
     *     directly as @c sampler2D(handle).
     *
     *  Matches the std430 shader block:
     *  @code
     *  struct GpuMaterial {
     *      uvec2 texture_refs[6];
     *      vec4  base_color_factor;
     *      float metallic_factor;
     *      float roughness_factor;
     *      float pad0;
     *      float pad1;
     *      vec4  emissive_factor; // xyz used
     *  };
     *  @endcode */
    struct GpuMaterial
    {
        uint32_t texture_refs[kMaterialSamplerSlots.size()][2] {};
        Vector4  base_color_factor { 1.0f, 1.0f, 1.0f, 1.0f };
        float    metallic_factor { 1.0f };
        float    roughness_factor { 1.0f };
        float    pad0 { 0.0f };
        float    pad1 { 0.0f };
        Vector4  emissive_factor { 0.0f, 0.0f, 0.0f, 0.0f };
    };

    static_assert ( kMaterialSamplerSlots.size() == 6,
                    "GpuMaterial assumes the six canonical metallic-roughness sampler slots." );
    static_assert ( sizeof ( Vector4 ) == 16,
                    "Vector4 must be a tight 4xfloat for GPU layout compatibility." );
    static_assert ( sizeof ( GpuMaterial ) == 6 * 8 + 16 + 16 + 16,
                    "GpuMaterial layout must match the shader-side std430 material record (96 bytes)." );
}
#endif
