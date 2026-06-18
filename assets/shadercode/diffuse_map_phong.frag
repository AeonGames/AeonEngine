#version 450

#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices
{
      mat4 ProjectionMatrix;
      mat4 ViewMatrix;
};

#ifdef VULKAN
layout(set = 1, binding = 0, std140)
#else
layout(binding = 1, std140)
#endif
uniform Material{
      vec3  Kd;
      vec3  Ks;
      float Shininess;
};

// Per-frame light list, mirrors include/aeongames/GpuLight.hpp.
struct GpuLight {
      vec4  position_radius;
      vec4  color_intensity;
      vec4  direction_cosOuter;
      uint  type;
      float cos_inner;
      uint  _pad0;
      uint  _pad1;
};
#ifdef VULKAN
layout(set = 4, binding = 0, std430)
#else
layout(binding = 2, std430)
#endif
readonly buffer Lights{
      uint     LightCount;
      uint     _LightsPad0;
      uint     _LightsPad1;
      uint     _LightsPad2;
      GpuLight Lights_data[];
};

// Clustered-shading parameters, shared with the lighting compute pipeline.
// Depth axis is view-space Y in this engine (+X right, +Y forward, +Z up).
#ifdef VULKAN
layout(set = 5, binding = 0, std140)
#else
layout(binding = 4, std140)
#endif
uniform ClusterParams
{
      mat4  inverse_projection;
      uvec4 cluster_grid;   // x,y,z cluster counts; w = total cluster count
      vec4  cluster_screen; // viewport width, height, unused, unused
};

// Per-cluster (offset, count) into LightIndexList, written by the light-cull
// compute stage of the lighting pipeline.
#ifdef VULKAN
layout(set = 6, binding = 0, std430)
#else
layout(binding = 0, std430)
#endif
readonly buffer LightGrid
{
      uvec2 light_grid[];
};

// Flat per-cluster light-index slots (fixed cap per cluster).
#ifdef VULKAN
layout(set = 7, binding = 0, std430)
#else
layout(binding = 1, std430)
#endif
readonly buffer LightIndexList
{
      uint light_index_list[];
};

#ifdef VULKAN
layout(set = 2, binding = 0)
#else
layout(binding = 0)
#endif
uniform sampler2D DiffuseMap;

// Directional shadow mapping. ShadowParams carries the sun's world-space
// view-projection and filtering parameters; ShadowMap is the depth map sampled
// with hardware comparison (sampler2DShadow). shadow_params.w > 0.5 marks a
// directional caster active this frame; otherwise geometry is left fully lit.
#ifdef VULKAN
layout(set = 8, binding = 0, std140)
#else
layout(binding = 5, std140)
#endif
uniform ShadowParams
{
      mat4 light_view_projection; // world -> light clip space
      vec4 shadow_params;         // (texel_size, depth_bias, pcf_radius, enabled)
};

#ifdef VULKAN
layout(set = 9, binding = 0)
#else
layout(binding = 8)
#endif
uniform sampler2DShadow ShadowMap;

// Spot shadow mapping. SpotShadowParams carries one world-space view-projection
// and one world-space position per active spot caster; SpotShadowMap is a depth
// texture array (one layer per caster) sampled with hardware comparison. While
// shading a spot light the fragment shader finds the slot whose caster_position
// matches the light's own position and samples that array layer.
#ifdef VULKAN
layout(set = 10, binding = 0, std140)
#else
layout(binding = 6, std140)
#endif
uniform SpotShadowParams
{
      mat4 spot_light_view_projection[4]; // world -> caster clip space, per slot
      vec4 spot_caster_position[4];        // caster world position, per slot (.xyz)
      vec4 spot_shadow_params;             // (texel_size, depth_bias, pcf_radius, count)
};

#ifdef VULKAN
layout(set = 11, binding = 0)
#else
layout(binding = 9)
#endif
uniform sampler2DArrayShadow SpotShadowMap;

// Point shadow mapping. Each point caster is captured as six 90-degree faces
// (one per +/-X,+/-Y,+/-Z axis) into six consecutive layers of a depth array
// (layer = caster*6 + face). PointShadowParams holds those six view-projections
// per caster plus the caster world position and radius; PointShadowMap is the
// depth array sampled with comparison. The fragment shader picks the face from
// the dominant axis of the light-to-fragment vector and samples that layer.
// Array sizes are 6 * MAX_POINT_SHADOW_CASTERS and MAX_POINT_SHADOW_CASTERS.
#ifdef VULKAN
layout(set = 12, binding = 0, std140)
#else
layout(binding = 7, std140)
#endif
uniform PointShadowParams
{
      mat4 point_light_view_projection[12]; // 6 faces * MAX_POINT_SHADOW_CASTERS
      vec4 point_caster_position_radius[2]; // MAX_POINT_SHADOW_CASTERS (.xyz pos, .w radius)
      vec4 point_shadow_params;             // (texel_size, depth_bias, pcf_radius, count)
};

#ifdef VULKAN
layout(set = 13, binding = 0)
#else
layout(binding = 10)
#endif
uniform sampler2DArrayShadow PointShadowMap;

layout(location = 0) in vec3 tnorm;
layout(location = 1) in vec3 eyeCoords;
layout(location = 2) in vec2 CoordUV;
layout(location = 0) out vec4 FragColor;

// Map a view-space fragment to its cluster index, matching the tiling used by
// the cluster-build compute stage: NDC.xy select the screen tile, view-space Y
// (depth) selects a logarithmic depth slice.
uint fragment_cluster_index()
{
      // NDC.xy from the same projection used to rasterize this fragment.
      vec4 clip = ProjectionMatrix * vec4 ( eyeCoords, 1.0 );
      vec2 ndc = clip.xy / clip.w;

      uint gx = cluster_grid.x;
      uint gy = cluster_grid.y;
      uint gz = cluster_grid.z;
      uint ix = uint ( clamp ( ( ndc.x * 0.5 + 0.5 ) * float ( gx ), 0.0, float ( gx - 1u ) ) );
      uint iy = uint ( clamp ( ( ndc.y * 0.5 + 0.5 ) * float ( gy ), 0.0, float ( gy - 1u ) ) );

      // View-space near/far depths (view Y) from the inverse projection, the
      // same way the cluster-build stage derives them.
      vec4 n = inverse_projection * vec4 ( 0.0, 0.0, -1.0, 1.0 );
      vec4 f = inverse_projection * vec4 ( 0.0, 0.0,  1.0, 1.0 );
      float near_depth = n.y / n.w;
      float far_depth  = f.y / f.w;
      float depth = max ( eyeCoords.y, near_depth );

      // Logarithmic depth slicing (Olsson et al.).
      float slice = log ( depth / near_depth ) / log ( far_depth / near_depth );
      uint iz = uint ( clamp ( slice * float ( gz ), 0.0, float ( gz - 1u ) ) );

      return ix + iy * gx + iz * gx * gy;
}

// Sample the directional shadow map for the current fragment and return a
// visibility factor in [0,1] (1 = fully lit, 0 = fully shadowed). Reconstructs
// the fragment's world position from its view-space position, projects it into
// the light's clip space and does a 3x3 PCF comparison against the stored
// depth. Fragments outside the light frustum, or frames without a caster, are
// treated as fully lit.
float directional_shadow()
{
      if ( shadow_params.w < 0.5 )
      {
            return 1.0;
      }
      // eyeCoords is the view-space fragment position; the view matrix has no
      // projection Z-flip baked in on either backend, so its inverse recovers
      // the world position the light matrix expects.
      vec4 world = inverse ( ViewMatrix ) * vec4 ( eyeCoords, 1.0 );
      vec4 light_clip = light_view_projection * world;
#ifdef VULKAN
      // Match the GL[-1,1] -> VK[0,1] depth remap the shadow_depth pass applied.
      light_clip.z = ( light_clip.z + light_clip.w ) * 0.5;
#endif
      vec3 proj = light_clip.xyz / light_clip.w;
      vec2 uv = proj.xy * 0.5 + 0.5;
      if ( uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 )
      {
            return 1.0;
      }
#ifdef VULKAN
      float current_depth = proj.z;            // already [0,1] after the remap
#else
      float current_depth = proj.z * 0.5 + 0.5; // NDC[-1,1] -> window depth[0,1]
#endif
      current_depth -= shadow_params.y;        // constant bias to fight acne
      // 3x3 percentage-closer filtering. shadow_params.x is one texel in UV
      // space; shadow_params.z scales the kernel footprint.
      float visibility = 0.0;
      float step = shadow_params.x * shadow_params.z;
      for ( int y = -1; y <= 1; ++y )
      {
            for ( int x = -1; x <= 1; ++x )
            {
                  vec2 offset = vec2 ( float ( x ), float ( y ) ) * step;
                  visibility += texture ( ShadowMap, vec3 ( uv + offset, current_depth ) );
            }
      }
      return visibility / 9.0;
}

// Sample the spot shadow map layer for caster slot @p aSlot and return a
// visibility factor in [0,1] (1 = fully lit, 0 = fully shadowed). Mirrors
// directional_shadow() but uses the caster's perspective light view-projection
// and samples the matching layer of the sampler2DArrayShadow.
float spot_shadow ( int aSlot )
{
      vec4 world = inverse ( ViewMatrix ) * vec4 ( eyeCoords, 1.0 );
      vec4 light_clip = spot_light_view_projection[aSlot] * world;
#ifdef VULKAN
      // Match the GL[-1,1] -> VK[0,1] depth remap the shadow_depth pass applied.
      light_clip.z = ( light_clip.z + light_clip.w ) * 0.5;
#endif
      // Behind the light (w <= 0) projects nowhere valid: treat as lit.
      if ( light_clip.w <= 0.0 )
      {
            return 1.0;
      }
      vec3 proj = light_clip.xyz / light_clip.w;
      vec2 uv = proj.xy * 0.5 + 0.5;
      if ( uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 )
      {
            return 1.0;
      }
#ifdef VULKAN
      float current_depth = proj.z;            // already [0,1] after the remap
#else
      float current_depth = proj.z * 0.5 + 0.5; // NDC[-1,1] -> window depth[0,1]
#endif
      current_depth -= spot_shadow_params.y;   // constant bias to fight acne
      // 3x3 PCF on this caster's array layer (slot index as the third coord).
      float visibility = 0.0;
      float step = spot_shadow_params.x * spot_shadow_params.z;
      for ( int y = -1; y <= 1; ++y )
      {
            for ( int x = -1; x <= 1; ++x )
            {
                  vec2 offset = vec2 ( float ( x ), float ( y ) ) * step;
                  visibility += texture ( SpotShadowMap, vec4 ( uv + offset, float ( aSlot ), current_depth ) );
            }
      }
      return visibility / 9.0;
}

// Find the spot shadow caster slot whose recorded world position matches the
// given spot light's position, or -1 when this light is not a shadow caster.
// The shadow records are bit-identical copies of the same light, so an exact
// (small-epsilon) position match reliably identifies the casting light even
// after the renderer's frustum cull shifts light indices.
int spot_shadow_slot ( vec3 aLightPos )
{
      int count = int ( spot_shadow_params.w );
      for ( int i = 0; i < count; ++i )
      {
            if ( distance ( spot_caster_position[i].xyz, aLightPos ) < 0.01 )
            {
                  return i;
            }
      }
      return -1;
}

// Sample the point shadow map for caster @p aCaster and return a visibility
// factor in [0,1]. Reconstructs the fragment's world position, picks the cube
// face from the dominant axis of the light-to-fragment vector to locate the
// array layer and its texel, then compares the fragment's normalized radial
// distance from the light against the stored distance. The depth pass writes
// length(frag - light) / radius, so the comparison metric is the same on both
// faces and both backends (no projected-depth NDC remap needed here).
float point_shadow ( int aCaster, vec3 aLightPos )
{
      vec4 world = inverse ( ViewMatrix ) * vec4 ( eyeCoords, 1.0 );
      vec3 L = world.xyz - aLightPos;
      vec3 aL = abs ( L );
      int face;
      if ( aL.x >= aL.y && aL.x >= aL.z )
      {
            face = ( L.x > 0.0 ) ? 0 : 1;
      }
      else if ( aL.y >= aL.z )
      {
            face = ( L.y > 0.0 ) ? 2 : 3;
      }
      else
      {
            face = ( L.z > 0.0 ) ? 4 : 5;
      }
      int layer = aCaster * 6 + face;
      // Project only to find the texel (uv); the compared depth is radial, not
      // this projection's z. uv uses xy/w, which is identical on GL and Vulkan.
      vec4 light_clip = point_light_view_projection[layer] * world;
      if ( light_clip.w <= 0.0 )
      {
            return 1.0;
      }
      vec2 uv = ( light_clip.xy / light_clip.w ) * 0.5 + 0.5;
      if ( uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 )
      {
            return 1.0;
      }
      // Normalized radial distance: the exact metric the depth pass stored.
      float radius = point_caster_position_radius[aCaster].w;
      float current_depth = length ( L ) / radius - point_shadow_params.y;
      float visibility = 0.0;
      float step = point_shadow_params.x * point_shadow_params.z;
      for ( int y = -1; y <= 1; ++y )
      {
            for ( int x = -1; x <= 1; ++x )
            {
                  vec2 offset = vec2 ( float ( x ), float ( y ) ) * step;
                  visibility += texture ( PointShadowMap, vec4 ( uv + offset, float ( layer ), current_depth ) );
            }
      }
      return visibility / 9.0;
}

// Find the point shadow caster whose recorded world position matches the given
// point light's position, or -1 when this light is not a shadow caster.
int point_shadow_caster ( vec3 aLightPos )
{
      int count = int ( point_shadow_params.w );
      for ( int i = 0; i < count; ++i )
      {
            if ( distance ( point_caster_position_radius[i].xyz, aLightPos ) < 0.01 )
            {
                  return i;
            }
      }
      return -1;
}

// Blinn-Phong contribution of a single light, accumulated into the running
// diffuse/specular sums. aShadow scales the radiance (1.0 = fully lit); only
// the directional sun passes a value below 1.0, from directional_shadow().
void accumulate_light ( GpuLight L, vec3 N, vec3 V, float aShadow,
                        inout vec3 diffuse_accum, inout vec3 specular_accum )
{
      vec3 to_light;
      float attenuation = 1.0;
      if ( L.type == 2u )
      {
            // Directional: direction_cosOuter.xyz is the world-space to-light
            // direction (i.e. -lightDir).
            to_light = normalize ( ( ViewMatrix * vec4 ( L.direction_cosOuter.xyz, 0.0 ) ).xyz );
      }
      else
      {
            vec3 light_eye = ( ViewMatrix * vec4 ( L.position_radius.xyz, 1.0 ) ).xyz;
            vec3 d = light_eye - eyeCoords;
            float dist = length ( d );
            to_light = d / max ( dist, 1e-5 );
            float r = max ( L.position_radius.w, 1e-5 );
            float falloff = clamp ( 1.0 - ( dist * dist ) / ( r * r ), 0.0, 1.0 );
            attenuation = falloff * falloff;
            if ( L.type == 1u )
            {
                  vec3 spot_dir = normalize ( ( ViewMatrix * vec4 ( L.direction_cosOuter.xyz, 0.0 ) ).xyz );
                  float cos_a = dot ( to_light, spot_dir );
                  attenuation *= smoothstep ( L.direction_cosOuter.w, L.cos_inner, cos_a );
            }
      }
      vec3  light_radiance = L.color_intensity.rgb * L.color_intensity.a * attenuation * aShadow;
      float NdotL = max ( dot ( to_light, N ), 0.0 );
      diffuse_accum += light_radiance * NdotL;
      if ( NdotL > 0.0 )
      {
            // Blinn-Phong: half-vector between view and light.
            vec3  H     = normalize ( to_light + V );
            float NdotH = max ( dot ( N, H ), 0.0 );
            specular_accum += light_radiance * pow ( NdotH, Shininess );
      }
}

// Fixed per-cluster cap (mirrors MAX_LIGHTS_PER_CLUSTER in GpuClusterParams.hpp).
// A cluster reaching this count has overflowed and dropped lights.
const uint HEATMAP_OVERFLOW = 128u;
// Light count mapped to the top of the heat ramp; small so typical scenes show
// useful gradients rather than a flat blue.
const float HEATMAP_REFERENCE = 16.0;

// Flat ambient fill. The engine has no global illumination, so surfaces that
// face away from every light would otherwise render pure black, crushing the
// scene and hiding all texture color. This constant lifts the shadows just
// enough to reveal the diffuse maps, approximating the indirect bounce that
// the Blender reference render gets from its world lighting.
const vec3 AMBIENT = vec3 ( 0.25 );

// Map t in [0,1] to a blue -> cyan -> green -> yellow -> red heat ramp.
vec3 heat_color ( float t )
{
      t = clamp ( t, 0.0, 1.0 );
      float r = smoothstep ( 0.5, 0.9, t );
      float g = ( t < 0.5 ) ? smoothstep ( 0.0, 0.5, t ) : smoothstep ( 1.0, 0.6, t );
      float b = smoothstep ( 0.5, 0.1, t );
      return clamp ( vec3 ( r, g, b ), 0.0, 1.0 );
}

void main()
{
      // Debug: cluster light-count heatmap. cluster_screen.z is the runtime
      // toggle (set from the AEON_CLUSTER_HEATMAP environment variable). Black =
      // empty cluster, ramp blue->red by light count, white = overflow.
      if ( cluster_screen.z > 0.5 )
      {
            uint heat_cluster = fragment_cluster_index();
            uint heat_count = light_grid[heat_cluster].y;
            if ( heat_count == 0u )
            {
                  FragColor = vec4 ( 0.0, 0.0, 0.0, 1.0 );
            }
            else if ( heat_count >= HEATMAP_OVERFLOW )
            {
                  FragColor = vec4 ( 1.0, 1.0, 1.0, 1.0 );
            }
            else
            {
                  FragColor = vec4 ( heat_color ( float ( heat_count ) / HEATMAP_REFERENCE ), 1.0 );
            }
            return;
      }
      if ( !gl_FrontFacing )
      {
            FragColor = vec4 ( 1.0, 0.5, 0.5, 1.0 );
            return;
      }
      vec4 tex = texture ( DiffuseMap, CoordUV );
      // Unlit sentinel: vertex shader emits tnorm = 0 when the mesh has no
      // normals. Pass the texture through at full brightness.
      if ( dot ( tnorm, tnorm ) < 1e-6 )
      {
            FragColor = tex;
            return;
      }
      vec3 N = normalize ( tnorm );
      // In eye space the camera sits at the origin, so the view direction is
      // just the negated, normalized eye-space position.
      vec3 V = normalize ( -eyeCoords );

      vec3 diffuse_accum  = vec3 ( 0.0 );
      vec3 specular_accum = vec3 ( 0.0 );

      // Clustered point/spot lights: shade only the lights the light-cull stage
      // assigned to this fragment's cluster.
      uint cluster = fragment_cluster_index();
      uvec2 cell = light_grid[cluster];
      uint offset = cell.x;
      uint count  = cell.y;
      for ( uint n = 0u; n < count; ++n )
      {
            uint li = light_index_list[offset + n];
            if ( li >= LightCount )
            {
                  continue;
            }
            // Spot lights may cast a shadow: find this light's caster slot (by
            // matching its world position) and sample that layer. Point lights
            // sample their six-face cube map; non-casting lights stay fully lit.
            float light_visibility = 1.0;
            if ( Lights_data[li].type == 1u )
            {
                  int slot = spot_shadow_slot ( Lights_data[li].position_radius.xyz );
                  if ( slot >= 0 )
                  {
                        light_visibility = spot_shadow ( slot );
                  }
            }
            else if ( Lights_data[li].type == 0u )
            {
                  int caster = point_shadow_caster ( Lights_data[li].position_radius.xyz );
                  if ( caster >= 0 )
                  {
                        light_visibility = point_shadow ( caster, Lights_data[li].position_radius.xyz );
                  }
            }
            accumulate_light ( Lights_data[li], N, V, light_visibility, diffuse_accum, specular_accum );
      }

      // Directional lights bypass clustering (they touch every fragment), so
      // the cull stage skips them; shade them here for every fragment. The sun
      // is shadowed by the directional shadow map (computed once per fragment).
      float sun_visibility = directional_shadow();
      for ( uint i = 0u; i < LightCount; ++i )
      {
            if ( Lights_data[i].type == 2u )
            {
                  accumulate_light ( Lights_data[i], N, V, sun_visibility, diffuse_accum, specular_accum );
            }
      }

      vec3 LightIntensity = Kd * ( AMBIENT + diffuse_accum ) + Ks * specular_accum;
      FragColor = tex * vec4 ( LightIntensity, 1.0 );
}
