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

    /** @brief Maximum number of spot lights that can cast a shadow in one frame.
     *  Spot shadow maps are stored as the layers of a single depth texture
     *  array, so this is the array's layer count. Tied to 4 because the
     *  per-slot caster light indices are packed into a single ivec4 in the
     *  SpotShadowParams block (see @ref GpuSpotShadowParams); bumping it means
     *  widening that packing and the shader to match. */
    constexpr uint32_t MAX_SPOT_SHADOW_CASTERS = 4;

    /** @brief Square resolution of each spot shadow map layer, in texels.
     *  Lower than the directional map: spot shadows cover a smaller cone and
     *  there can be several, so they trade per-caster resolution for count. */
    constexpr uint32_t SPOT_SHADOW_MAP_RESOLUTION = 1024;

    /** @brief Maximum number of point lights that can cast a shadow in one
     *  frame. Each point caster is omnidirectional and is captured as six 90
     *  degree perspective faces (one per +/-X, +/-Y, +/-Z axis), stored as six
     *  consecutive layers of a depth texture array, so the array has
     *  6 * MAX_POINT_SHADOW_CASTERS layers and the shadow pass renders six times
     *  per caster. Kept small because of that 6x render cost; bump it (and the
     *  matrix array size below) to allow more. */
    constexpr uint32_t MAX_POINT_SHADOW_CASTERS = 2;

    /** @brief Number of cube faces captured per point shadow caster. */
    constexpr uint32_t POINT_SHADOW_FACES = 6;

    /** @brief Square resolution of each point shadow map face, in texels. */
    constexpr uint32_t POINT_SHADOW_MAP_RESOLUTION = 1024;

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

    /** @brief CPU-side mirror of the @c SpotShadowParams uniform block.
     *
     *  Carries the per-frame spot shadow casters. Each active caster occupies
     *  one slot: @c spot_light_view_projection[slot] transforms world space into
     *  that caster's clip space, and @c caster_position[slot] is the casting
     *  light's world position. The fragment shader, while shading a spot light,
     *  finds the slot whose @c caster_position matches the light's own position
     *  (the records are bit-identical copies of the same light, so an exact
     *  match is reliable). Matching by position rather than array index keeps
     *  the lookup correct after the renderer frustum-culls the light list, which
     *  shifts indices. Slots past @c count have a zero position that no real
     *  light matches. Matches the std140 block:
     *  @code
     *  layout(std140) uniform SpotShadowParams {
     *      mat4 spot_light_view_projection[MAX_SPOT_SHADOW_CASTERS];
     *      vec4 caster_position[MAX_SPOT_SHADOW_CASTERS]; // xyz world pos, w unused
     *      vec4 spot_shadow_params;                       // (texel_size, depth_bias, pcf_radius, count)
     *  };
     *  @endcode */
    struct GpuSpotShadowParams
    {
        Matrix4x4 spot_light_view_projection[MAX_SPOT_SHADOW_CASTERS] {};
        Vector4   caster_position[MAX_SPOT_SHADOW_CASTERS] {};
        float     params[4] { 1.0f / static_cast<float> ( SPOT_SHADOW_MAP_RESOLUTION ), 0.0015f, 1.0f, 0.0f };
    };
    static_assert ( sizeof ( GpuSpotShadowParams ) == 64 * MAX_SPOT_SHADOW_CASTERS + 16 * MAX_SPOT_SHADOW_CASTERS + 16,
                    "GpuSpotShadowParams layout must match the shader-side std140 SpotShadowParams block." );

    /** @brief CPU-side mirror of the @c PointShadowParams uniform block.
     *
     *  Carries the per-frame point shadow casters. Each caster occupies six
     *  consecutive slots in @c point_light_view_projection, one per cube face
     *  (axis order +X, -X, +Y, -Y, +Z, -Z), each a 90 degree perspective from
     *  the light position looking along that world axis; the matching depth
     *  array layer is @c caster*6 + face. @c caster_position_radius holds the
     *  light's world position (.xyz) and shadow radius / far plane (.w) per
     *  caster. The fragment shader, while shading a point light, finds the slot
     *  whose position matches the light's own (bit-identical copies, so an exact
     *  match is reliable through the renderer's light cull), picks the cube face
     *  from the dominant axis of the light-to-fragment vector, and samples that
     *  layer. Matches the std140 block:
     *  @code
     *  layout(std140) uniform PointShadowParams {
     *      mat4 point_light_view_projection[6 * MAX_POINT_SHADOW_CASTERS];
     *      vec4 point_caster_position_radius[MAX_POINT_SHADOW_CASTERS]; // xyz pos, w radius
     *      vec4 point_shadow_params;  // (texel_size, depth_bias, pcf_radius, count)
     *  };
     *  @endcode */
    struct GpuPointShadowParams
    {
        Matrix4x4 point_light_view_projection[POINT_SHADOW_FACES * MAX_POINT_SHADOW_CASTERS] {};
        Vector4   caster_position_radius[MAX_POINT_SHADOW_CASTERS] {};
        float     params[4] { 1.0f / static_cast<float> ( POINT_SHADOW_MAP_RESOLUTION ), 0.0015f, 1.0f, 0.0f };
    };
    static_assert ( sizeof ( GpuPointShadowParams ) ==
                    64 * POINT_SHADOW_FACES * MAX_POINT_SHADOW_CASTERS + 16 * MAX_POINT_SHADOW_CASTERS + 16,
                    "GpuPointShadowParams layout must match the shader-side std140 PointShadowParams block." );

    /** @brief CPU-side mirror of the point shadow depth pass's @c ShadowParams
     *  block, bound per caster for the single-pass cube render.
     *
     *  All six cube faces of one caster are rendered in a single draw. On Vulkan
     *  this uses multiview: the vertex shader projects by
     *  @c face_view_projection[gl_ViewIndex] into the view selected by the
     *  render pass's view mask. On OpenGL a geometry shader replicates each
     *  primitive across the six cube-map-array layers (gl_Layer = base_layer +
     *  face), projecting by @c face_view_projection[face]. The fragment shader
     *  writes the normalized radial distance from @c light_position_radius.
     *  @c base_layer (face_params.x) is the caster's first cube-array layer:
     *  @c caster*6 on OpenGL (whole array attached, absolute gl_Layer) and 0 on
     *  Vulkan (a per-caster six-layer framebuffer, relative views). Matches:
     *  @code
     *  layout(std140) uniform ShadowParams {
     *      mat4 face_view_projection[POINT_SHADOW_FACES];
     *      vec4 light_position_radius; // xyz world pos, w radius
     *      vec4 face_params;           // x = base_layer
     *  };
     *  @endcode */
    struct GpuPointDepthParams
    {
        Matrix4x4 face_view_projection[POINT_SHADOW_FACES] {};
        Vector4   light_position_radius {};
        Vector4   face_params {};
    };
    static_assert ( sizeof ( GpuPointDepthParams ) == 64 * POINT_SHADOW_FACES + 16 + 16,
                    "GpuPointDepthParams layout must match the shader-side std140 point depth ShadowParams block." );
}
#endif
