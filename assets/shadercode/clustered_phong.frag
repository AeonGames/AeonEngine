#version 450
#ifdef VULKAN
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require
#else
#extension GL_ARB_bindless_texture : require
#endif

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

// Metallic-roughness material inputs. On both backends the surface's textures
// and factors come from one record in the global bindless material storage
// buffer, selected by MATERIAL_INDEX (a per-instance flat varying on Vulkan,
// forwarded by the vertex shader from the InstanceMaterials buffer; a
// default-block uniform on OpenGL). The DiffuseMap..EmissiveMap and
// BaseColorFactor..EmissiveFactor names below are macros into that record, so
// main() is backend-agnostic: Vulkan indexes a global combined-image-sampler
// array, OpenGL constructs a sampler2D from the record's resident ARB_bindless
// texture handle.
struct GpuMaterial
{
      uvec2 texture_refs[6]; // Vulkan: .x = slot into global_textures. OpenGL: resident sampler handle.
      vec4  base_color_factor;
      float metallic_factor;
      float roughness_factor;
      float pad0;
      float pad1;
      vec4  emissive_factor;
};
#ifdef VULKAN
layout(set = 2, binding = 0) uniform sampler2D global_textures[];
// Material records reached by buffer device address (BDA): the push constant
// carries a 64-bit pointer to the global material storage buffer, so the shader
// dereferences it directly instead of binding the buffer as a descriptor.
layout(buffer_reference, std430, buffer_reference_align = 16) readonly buffer MaterialRef
{
      GpuMaterial data[];
};
// The material storage buffer pointer is a push constant (constant per frame,
// shared across an indirect multi-draw); the per-instance material index
// arrives as a flat varying the vertex shader read from the InstanceMaterials
// buffer, so a single indirect draw can shade meshes with different materials.
layout(push_constant) uniform MaterialPushConstant
{
      layout(offset = 72) MaterialRef Materials;
};
layout(location = 5) flat in uint vMaterialIndex;
#define MATERIAL_INDEX  vMaterialIndex
#define MAT_REC         Materials.data[MATERIAL_INDEX]
#define MAT_TEX(i)      global_textures[nonuniformEXT(MAT_REC.texture_refs[i].x)]
#else
// Global material storage buffer (block name "Bindless" -> Mesh::BINDLESS, the
// hash OpenGLMaterial::Bind looks up to detect a bindless pipeline and bind this
// renderer-owned SSBO). texture_refs entries are resident ARB_bindless_texture
// handles the macro below turns straight into a sampler2D.
layout(binding = 4, std430) readonly buffer Bindless
{
      GpuMaterial materials[];
};
layout(location = 0) uniform uint MaterialIndex;
#define MATERIAL_INDEX  MaterialIndex
#define MAT_REC         materials[MATERIAL_INDEX]
#define MAT_TEX(i)      sampler2D(MAT_REC.texture_refs[i])
#endif
#define DiffuseMap      MAT_TEX(0)
#define NormalMap       MAT_TEX(1)
#define MetallicMap     MAT_TEX(2)
#define RoughnessMap    MAT_TEX(3)
#define OcclusionMap    MAT_TEX(4)
#define EmissiveMap     MAT_TEX(5)
#define BaseColorFactor MAT_REC.base_color_factor
#define MetallicFactor  MAT_REC.metallic_factor
#define RoughnessFactor MAT_REC.roughness_factor
#define EmissiveFactor  MAT_REC.emissive_factor.xyz

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

// Material textures (DiffuseMap..EmissiveMap) are macros defined with the
// material record above: on Vulkan they index the global bindless texture array,
// on OpenGL they construct a sampler2D from the record's resident texture handle.

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
uniform samplerCubeArrayShadow PointShadowMap;

// Per-frame scene-wide shading globals, independent of any object, light or
// cluster. ambient.rgb is the ambient color and ambient.a its intensity; the
// fragment shader uses their product as the flat ambient fill. Kept in its own
// small block so future frame-wide values (fog, exposure, ...) can join it.
#ifdef VULKAN
layout(set = 14, binding = 0, std140)
#else
layout(binding = 3, std140)
#endif
uniform Globals
{
      vec4 ambient;
      vec4 sh[9];
};

layout(location = 0) in vec3 tnorm;
layout(location = 1) in vec3 eyeCoords;
layout(location = 2) in vec2 CoordUV;
layout(location = 3) in vec3 ttangent;
layout(location = 4) in vec3 tbitangent;
// Deferred specular G-buffer: FragColor holds scene radiance WITHOUT the
// specular ambient (the composite/tonemap pass adds it), GNormalRough carries
// the view-space normal + roughness, and GSpecWeight the pre-integrated
// specular weight the composite multiplies the reflection by.
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 GNormalRough;
layout(location = 2) out vec4 GSpecWeight;

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

// Sample the point shadow cube map for caster @p aCaster and return a
// visibility factor in [0,1]. The depth pass rendered six cube faces storing
// normalized radial distance from the light, so the hardware picks the face and
// (s,t) from the world-space light-to-fragment direction and we compare against
// the same metric (length(frag - light) / radius). 3x3 PCF perturbs the
// sampling direction in a frame perpendicular to it.
float point_shadow ( int aCaster, vec3 aLightPos )
{
      vec4 world = inverse ( ViewMatrix ) * vec4 ( eyeCoords, 1.0 );
      vec3 dir = world.xyz - aLightPos;
      float radius = point_caster_position_radius[aCaster].w;
      float current_depth = length ( dir ) / radius - point_shadow_params.y;
      // Build an orthonormal frame around the sampling direction for PCF.
      vec3 ref_up = ( abs ( dir.z ) < 0.999 * length ( dir ) ) ? vec3 ( 0.0, 0.0, 1.0 ) : vec3 ( 1.0, 0.0, 0.0 );
      vec3 tangent = normalize ( cross ( ref_up, dir ) );
      vec3 bitangent = cross ( dir, tangent );
      float texel = point_shadow_params.x * point_shadow_params.z * length ( dir );
      float visibility = 0.0;
      for ( int y = -1; y <= 1; ++y )
      {
            for ( int x = -1; x <= 1; ++x )
            {
                  vec3 offset = ( tangent * float ( x ) + bitangent * float ( y ) ) * texel;
                  visibility += texture ( PointShadowMap, vec4 ( dir + offset, float ( aCaster ) ), current_depth );
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

const float PI = 3.14159265359;

// GGX / Trowbridge-Reitz normal distribution: the relative concentration of
// microfacets whose normal is the half-vector H, for the given roughness.
float distribution_ggx ( vec3 N, vec3 H, float roughness )
{
      float a     = roughness * roughness;
      float a2    = a * a;
      float NdotH = max ( dot ( N, H ), 0.0 );
      float d     = ( NdotH * NdotH ) * ( a2 - 1.0 ) + 1.0;
      return a2 / ( PI * d * d );
}

// Smith geometry term via the Schlick-GGX approximation, using the direct-
// lighting roughness remap (r+1)^2 / 8. Models microfacet self-shadowing and
// masking from both the view and light directions.
float geometry_schlick_ggx ( float NdotX, float roughness )
{
      float r = roughness + 1.0;
      float k = ( r * r ) / 8.0;
      return NdotX / ( NdotX * ( 1.0 - k ) + k );
}
float geometry_smith ( vec3 N, vec3 V, vec3 L, float roughness )
{
      return geometry_schlick_ggx ( max ( dot ( N, V ), 0.0 ), roughness ) *
             geometry_schlick_ggx ( max ( dot ( N, L ), 0.0 ), roughness );
}

// Fresnel-Schlick: the fraction of light reflected rather than refracted,
// interpolating from the surface's normal-incidence reflectance F0 up to 1 at
// grazing angles.
vec3 fresnel_schlick ( float cosTheta, vec3 F0 )
{
      return F0 + ( 1.0 - F0 ) * pow ( clamp ( 1.0 - cosTheta, 0.0, 1.0 ), 5.0 );
}

// Fresnel-Schlick with a roughness term (Lagarde): rough surfaces keep a lower
// grazing reflectance, which stops the ambient specular from over-brightening
// the silhouettes of rough materials. Used only by the ambient (IBL) term;
// direct lights use the sharp fresnel_schlick above.
vec3 fresnel_schlick_roughness ( float cosTheta, vec3 F0, float roughness )
{
      vec3 Fr = max ( vec3 ( 1.0 - roughness ), F0 );
      return F0 + ( Fr - F0 ) * pow ( clamp ( 1.0 - cosTheta, 0.0, 1.0 ), 5.0 );
}

// Analytic fit of the split-sum "environment BRDF" (Karis / Lazarov): the
// (scale, bias) applied to F0 for the specular ambient term, avoiding a
// precomputed BRDF LUT. A future real-IBL pass may swap this for a LUT lookup.
vec2 env_brdf_approx ( float NoV, float roughness )
{
      const vec4 c0 = vec4 ( -1.0, -0.0275, -0.572, 0.022 );
      const vec4 c1 = vec4 ( 1.0, 0.0425, 1.04, -0.04 );
      vec4 r = roughness * c0 + c1;
      float a004 = min ( r.x * r.x, exp2 ( -9.28 * NoV ) ) * r.x + r.y;
      return vec2 ( -1.04, 1.04 ) * a004 + r.zw;
}

// Cook-Torrance contribution of a single light, accumulated into the running
// outgoing-radiance sum Lo. aShadow scales the radiance (1.0 = fully lit). The
// light direction and attenuation are computed exactly as before; only the BRDF
// changed from Blinn-Phong to metallic-roughness Cook-Torrance.
void accumulate_light ( GpuLight L, vec3 N, vec3 V, vec3 albedo, float metallic,
                        float roughness, vec3 F0, float aShadow, inout vec3 Lo )
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
      float NdotL = max ( dot ( N, to_light ), 0.0 );
      if ( NdotL <= 0.0 )
      {
            return;
      }
      vec3 radiance = L.color_intensity.rgb * L.color_intensity.a * attenuation * aShadow;
      // Cook-Torrance specular: D * G * F / (4 * NdotV * NdotL).
      vec3  H     = normalize ( to_light + V );
      float NdotV = max ( dot ( N, V ), 0.0 );
      float D     = distribution_ggx ( N, H, roughness );
      float G     = geometry_smith ( N, V, to_light, roughness );
      vec3  F     = fresnel_schlick ( max ( dot ( H, V ), 0.0 ), F0 );
      vec3  specular = ( D * G * F ) / max ( 4.0 * NdotV * NdotL, 1e-4 );
      // Energy conservation: light not reflected specularly (kd) is diffused;
      // metals have no diffuse term.
      vec3 kd = ( vec3 ( 1.0 ) - F ) * ( 1.0 - metallic );
      Lo += ( kd * albedo / PI + specular ) * radiance * NdotL;
}

// --- Colour management -------------------------------------------------------
// The scene is lit in linear space; the base-colour texture is decoded from
// sRGB before shading. Exposure, tone mapping and the final sRGB encode are
// done by the separate fullscreen tonemap pass (tonemap.frag) that resolves the
// off-screen HDR target to the swapchain.

// Approximate sRGB decode (gamma 2.2): brings sRGB base-colour textures into
// linear light for shading.
vec3 srgb_to_linear ( vec3 c )
{
      return pow ( c, vec3 ( 2.2 ) );
}

// Diffuse irradiance E(n) from the environment's order-2 SH coefficients
// (Globals.sh, world space, +Z up). Ramamoorthi's cosine-lobe convolution folds
// the three SH bands by A0 = pi, A1 = 2pi/3, A2 = pi/4. The Lambertian diffuse
// response is albedo / pi * E(n).
vec3 sh_irradiance ( vec3 n )
{
      const float A0 = 3.14159265;   // pi
      const float A1 = 2.09439510;   // 2pi/3
      const float A2 = 0.78539816;   // pi/4
      vec3 E = A0 * 0.282095 * sh[0].rgb;
      E += A1 * 0.488603 * ( sh[1].rgb * n.y + sh[2].rgb * n.z + sh[3].rgb * n.x );
      E += A2 * ( sh[4].rgb * 1.092548 * n.x * n.y
                + sh[5].rgb * 1.092548 * n.y * n.z
                + sh[6].rgb * 0.315392 * ( 3.0 * n.z * n.z - 1.0 )
                + sh[7].rgb * 1.092548 * n.x * n.z
                + sh[8].rgb * 0.546274 * ( n.x * n.x - n.y * n.y ) );
      return max ( E, vec3 ( 0.0 ) );
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
      // Tangent-space normal mapping. NormalMap defaults to a flat (0,0,1)
      // texture for materials with no normal map, and the vertex shader emits a
      // zero tangent basis for meshes with no tangents; either case collapses
      // the result back to the interpolated normal N.
      vec3 tangent_normal = texture ( NormalMap, CoordUV ).xyz * 2.0 - 1.0;
      N = normalize ( mat3 ( ttangent, tbitangent, N ) * tangent_normal );
      // In eye space the camera sits at the origin, so the view direction is
      // just the negated, normalized eye-space position.
      vec3 V = normalize ( -eyeCoords );

      // Metallic-roughness surface inputs. BaseColorFactor tints the base-colour
      // texture; the metallic/roughness maps (white fallback = 1.0) scale their
      // factors. F0 is the normal-incidence reflectance: 0.04 for dielectrics,
      // the base colour for metals. Roughness is clamped away from 0 so the
      // specular highlight stays finite.
      vec3  albedo    = BaseColorFactor.rgb * srgb_to_linear ( tex.rgb );
      float metallic  = MetallicFactor * texture ( MetallicMap, CoordUV ).r;
      float roughness = clamp ( RoughnessFactor * texture ( RoughnessMap, CoordUV ).r, 0.045, 1.0 );
      vec3  F0        = mix ( vec3 ( 0.04 ), albedo, metallic );
      // Ambient occlusion scales the ambient term; emissive is added after
      // lighting. The emissive texture is sRGB; its white fallback times a zero
      // EmissiveFactor means non-emissive materials add nothing.
      float ao        = texture ( OcclusionMap, CoordUV ).r;
      vec3  emissive  = EmissiveFactor * srgb_to_linear ( texture ( EmissiveMap, CoordUV ).rgb );

      vec3 Lo = vec3 ( 0.0 );

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
            accumulate_light ( Lights_data[li], N, V, albedo, metallic, roughness, F0, light_visibility, Lo );
      }

      // Directional lights bypass clustering (they touch every fragment), so
      // the cull stage skips them; shade them here for every fragment. The sun
      // is shadowed by the directional shadow map (computed once per fragment).
      float sun_visibility = directional_shadow();
      for ( uint i = 0u; i < LightCount; ++i )
      {
            if ( Lights_data[i].type == 2u )
            {
                  accumulate_light ( Lights_data[i], N, V, albedo, metallic, roughness, F0, sun_visibility, Lo );
            }
      }

      // Ambient light via the split-sum IBL form, with the specular reflection
      // DEFERRED: the diffuse irradiance (from the environment's SH, or the flat
      // ambient when a scene has none) is added here, while the specular
      // reflection is left to the composite/tonemap pass. That pass samples the
      // prefiltered environment (or screen-space reflections) along the
      // reflection vector and multiplies by the specular weight written below,
      // so reflections can mix cube-map and SSR without double counting.
      float NdotV_ambient    = max ( dot ( N, V ), 0.0 );
      vec3  F_ambient        = fresnel_schlick_roughness ( NdotV_ambient, F0, roughness );
      vec3  kd_ambient       = ( vec3 ( 1.0 ) - F_ambient ) * ( 1.0 - metallic );
      vec2  ambient_brdf     = env_brdf_approx ( NdotV_ambient, roughness );
      // Diffuse IBL from the environment's SH irradiance, evaluated with the
      // world-space normal (albedo / pi * E(n)).
      vec3  world_normal     = normalize ( mat3 ( inverse ( ViewMatrix ) ) * N );
      vec3  diffuse_ambient  = albedo * sh_irradiance ( world_normal ) / PI;
      vec3  color = kd_ambient * diffuse_ambient * ao + Lo + emissive;
      FragColor = vec4 ( color, tex.a * BaseColorFactor.a );
      // G-buffer for the deferred specular composite: the view-space normal and
      // roughness select the reflection, and the pre-integrated specular weight
      // scales it. The weight is attenuated by a specular-occlusion term
      // (Lagarde) rather than raw AO: rough, occluded surfaces should barely
      // reflect the environment, which stops matte interiors looking wet under a
      // bright sky while leaving smooth / open surfaces reflective.
      float spec_occlusion = clamp ( pow ( NdotV_ambient + ao, exp2 ( -16.0 * roughness - 1.0 ) )
                                     - 1.0 + ao, 0.0, 1.0 );
      // Overall strength of the environment specular reflection. Physically 1.0,
      // but a bright HDR sky makes a full-strength reflection read as a wet sheen
      // on floors and other flat surfaces, so it is dialled back a little.
      const float AMBIENT_SPECULAR_INTENSITY = 0.5;
      GNormalRough = vec4 ( N, roughness );
      GSpecWeight  = vec4 ( ( F0 * ambient_brdf.x + ambient_brdf.y ) * spec_occlusion * AMBIENT_SPECULAR_INTENSITY, 1.0 );
}
