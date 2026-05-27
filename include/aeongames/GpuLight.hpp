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
#ifndef AEONGAMES_GPULIGHT_HPP
#define AEONGAMES_GPULIGHT_HPP

#include <cstdint>
#include "aeongames/Vector4.hpp"

namespace AeonGames
{
    /** @brief Maximum number of lights uploaded to the GPU per frame.
     *  Sized to keep the per-frame Lights uniform buffer small and to
     *  bound the per-cluster index list once Clustered Forward+ is in. */
    constexpr uint32_t MAX_LIGHTS_PER_FRAME = 64;

    /** @brief Light type tag for the shared GPU representation. */
    enum class LightType : uint32_t
    {
        Point       = 0,
        Spot        = 1,
        Directional = 2,
    };

    /** @brief Packed, std140/std430-friendly light record consumed by shaders.
     *
     *  All vectors are 4-component on purpose so the layout matches what GLSL
     *  sees with no padding surprises. Per-type interpretation:
     *  - @b Point:       position_radius = (worldPos.xyz, radius),
     *                    direction_cosOuter = unused (set to (0,0,-1, -1)),
     *                    cos_inner = unused.
     *  - @b Spot:        position_radius = (worldPos.xyz, radius),
     *                    direction_cosOuter = (-lightDir.xyz, cos(outer)),
     *                    cos_inner = cos(inner) with inner < outer (cos_inner > cos_outer).
     *  - @b Directional: position_radius = (0,0,0, 0) (no falloff),
     *                    direction_cosOuter = (-lightDir.xyz, -1),
     *                    cos_inner = unused.
     *  color_intensity packs the linear RGB color in .rgb and a scalar
     *  intensity multiplier in .a so shaders can do a single mad. */
    struct GpuLight
    {
        Vector4  position_radius    { 0.0f, 0.0f,  0.0f,  0.0f };
        Vector4  color_intensity    { 1.0f, 1.0f,  1.0f,  1.0f };
        Vector4  direction_cosOuter { 0.0f, 0.0f, -1.0f, -1.0f };
        uint32_t type               { static_cast<uint32_t> ( LightType::Point ) };
        float    cos_inner          { 1.0f };
        uint32_t _pad[2]            { 0, 0 };
    };
    static_assert ( sizeof ( Vector4 ) == 16,
                    "Vector4 must be a tight 4xfloat for GPU layout compatibility." );
    static_assert ( sizeof ( GpuLight ) == 64,
                    "GpuLight must stay 16-byte aligned at 64 bytes for std140/std430." );

    /** @brief CPU-side mirror of the @c Lights uniform block layout.
     *
     *  Matches the std140 layout expected by shaders:
     *  @code
     *  layout(std140) uniform Lights {
     *      uint     count;
     *      // 3 x uint padding so the array starts on a 16-byte boundary.
     *      GpuLight lights[MAX_LIGHTS_PER_FRAME];
     *  };
     *  @endcode
     *
     *  Total size is 16 B header + 64 B * MAX_LIGHTS_PER_FRAME records, e.g.
     *  4112 B at MAX_LIGHTS_PER_FRAME == 64. This fits comfortably in the
     *  uniform-block size guaranteed by GL 4.5+ (16 KB) and Vulkan
     *  (maxUniformBufferRange, typically 16 KB or 64 KB). */
    struct GpuLightsBlock
    {
        uint32_t count            { 0 };
        uint32_t _pad[3]          { 0, 0, 0 };
        GpuLight lights[MAX_LIGHTS_PER_FRAME] {};
    };
    static_assert ( sizeof ( GpuLightsBlock ) == 16 + 64 * MAX_LIGHTS_PER_FRAME,
                    "GpuLightsBlock layout must match the shader-side std140 Lights block." );
}
#endif
