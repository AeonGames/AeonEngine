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

#include <cstddef>
#include <cstdint>
#include "aeongames/Vector4.hpp"

namespace AeonGames
{
    /** @brief Maximum number of lights uploaded to the GPU per frame.
     *  Backed by a shader storage buffer (SSBO) so the cap can be large:
     *  Clustered Forward+ culls this set down per cluster, so the shading
     *  cost stays bounded by @ref MAX_LIGHTS_PER_CLUSTER regardless. */
    constexpr uint32_t MAX_LIGHTS_PER_FRAME = 4096;

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

    /** @brief CPU-side mirror of the @c Lights storage block header.
     *
     *  The per-frame Lights data is a shader storage buffer (SSBO) laid out
     *  as a 16-byte header followed by a tightly packed @ref GpuLight array:
     *  @code
     *  layout(std430) readonly buffer Lights {
     *      uint     LightCount;
     *      // 3 x uint padding so the array starts on a 16-byte boundary.
     *      GpuLight Lights_data[];
     *  };
     *  @endcode
     *
     *  Only this header is ever materialized CPU-side; the light records are
     *  streamed straight from the submitting container, so nothing allocates
     *  the full @ref GpuLightsBufferSize on the stack. */
    struct GpuLightsHeader
    {
        uint32_t count   { 0 };
        uint32_t _pad[3] { 0, 0, 0 };
    };
    static_assert ( sizeof ( GpuLightsHeader ) == 16,
                    "GpuLightsHeader must stay 16 bytes to match the std430 Lights block header." );

    /** @brief Total byte size of the per-frame Lights storage buffer:
     *  16-byte header + @ref MAX_LIGHTS_PER_FRAME tightly packed records.
     *  At 4096 lights this is ~256 KB, well within the storage-buffer range
     *  guaranteed by GL 4.5+ and Vulkan (maxStorageBufferRange). */
    constexpr std::size_t GpuLightsBufferSize =
        sizeof ( GpuLightsHeader ) + sizeof ( GpuLight ) * MAX_LIGHTS_PER_FRAME;
}
#endif
