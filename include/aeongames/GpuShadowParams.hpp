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
#ifndef AEONGAMES_GPUSHADOWPARAMS_HPP
#define AEONGAMES_GPUSHADOWPARAMS_HPP

#include <cstdint>
#include "aeongames/Vector4.hpp"
#include "aeongames/Matrix4x4.hpp"

namespace AeonGames
{
    /** @brief Square resolution of the directional shadow map, in texels.
     *  Single full-scene shadow map (no cascades yet). Lives here as one
     *  constant so both backends size their depth target identically and it is
     *  trivial to bump later. */
    constexpr uint32_t SHADOW_MAP_RESOLUTION = 2048;

    /** @brief Depth of the directional shadow map's coverage, as a multiple of
     *  the scene's bounding-sphere radius. The shadow pass fits the map to the
     *  camera's view frustum truncated to this distance (clamped to the camera
     *  far plane) instead of the whole scene, trading shadows on very distant
     *  geometry for frame time and shadow sharpness on what the camera actually
     *  sees. Lower values fit tighter: faster, sharper, shorter shadow range. */
    constexpr float SHADOW_COVERAGE_FRACTION = 1.0f;

    /** @brief CPU-side mirror of the @c ShadowParams uniform block.
     *
     *  Consumed by the shadow-depth pipeline (to transform geometry into the
     *  light's clip space) and by the clustered fragment shader (to project a
     *  shaded fragment into the shadow map and compare depths). Laid out to
     *  match the std140 block:
     *  @code
     *  layout(std140) uniform ShadowParams {
     *      mat4 light_view_projection; // world -> light clip space
     *      vec4 params;                // (texel_size, depth_bias, pcf_radius, enabled)
     *  };
     *  @endcode
     *
     *  @c params.x is 1/SHADOW_MAP_RESOLUTION (one texel in shadow UV space),
     *  @c params.y is a constant depth bias added in the comparison to fight
     *  shadow acne, @c params.z is the PCF kernel radius in texels, and
     *  @c params.w is > 0.5 when a directional shadow caster is active this
     *  frame (the fragment shader leaves geometry fully lit otherwise). */
    struct GpuShadowParams
    {
        Matrix4x4 light_view_projection {};
        float     params[4] { 1.0f / static_cast<float> ( SHADOW_MAP_RESOLUTION ), 0.0015f, 1.0f, 0.0f };
    };
    static_assert ( sizeof ( Vector4 ) == 16,
                    "Vector4 must be a tight 4xfloat for GPU layout compatibility." );
    static_assert ( sizeof ( GpuShadowParams ) == 64 + 16,
                    "GpuShadowParams layout must match the shader-side std140 ShadowParams block." );
}
#endif
