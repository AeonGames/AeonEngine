#version 450

#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices
{
#ifndef VULKAN
      mat4 ModelMatrix;
#endif
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

// Blinn-Phong contribution of a single light, accumulated into the running
// diffuse/specular sums.
void accumulate_light ( GpuLight L, vec3 N, vec3 V,
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
      vec3  light_radiance = L.color_intensity.rgb * L.color_intensity.a * attenuation;
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
            accumulate_light ( Lights_data[li], N, V, diffuse_accum, specular_accum );
      }

      // Directional lights bypass clustering (they touch every fragment), so
      // the cull stage skips them; shade them here for every fragment.
      for ( uint i = 0u; i < LightCount; ++i )
      {
            if ( Lights_data[i].type == 2u )
            {
                  accumulate_light ( Lights_data[i], N, V, diffuse_accum, specular_accum );
            }
      }

      vec3 LightIntensity = Kd * diffuse_accum + Ks * specular_accum;
      FragColor = tex * vec4 ( LightIntensity, 1.0 );
}
