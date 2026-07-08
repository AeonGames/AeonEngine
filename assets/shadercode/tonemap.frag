#version 450

// Deferred specular composite + tone map. The geometry pass wrote the scene
// radiance WITHOUT the specular ambient (HdrColor), plus a small G-buffer: the
// view-space normal + roughness (GNormalRough) and the pre-integrated specular
// weight (GSpecWeight). This pass reflects the environment along the per-pixel
// view ray, scales by that weight and adds it back, then exposes, tone maps and
// sRGB-encodes the result. Splitting the specular out this way lets the
// reflection source mix the prefiltered cube with screen-space reflections
// (added later) without double counting.
#ifdef VULKAN
layout(set = 0, binding = 0)
#else
layout(binding = 0)
#endif
uniform sampler2D HdrColor;
#ifdef VULKAN
layout(set = 0, binding = 1)
#else
layout(binding = 1)
#endif
uniform sampler2D GNormalRough;
#ifdef VULKAN
layout(set = 0, binding = 2)
#else
layout(binding = 2)
#endif
uniform sampler2D GSpecWeight;

#ifdef VULKAN
layout(set = 1, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices
{
      mat4 ProjectionMatrix;
      mat4 ViewMatrix;
};

#ifdef VULKAN
layout(set = 2, binding = 0, std140)
#else
layout(binding = 3, std140)
#endif
uniform Globals
{
      vec4 ambient;
      vec4 sh[9];
};

// GGX-prefiltered specular environment cube (see PrefilterEnvironmentCube). A
// 1x1 map means the scene has no environment, so the composite falls back to
// the flat ambient radiance to match the geometry pass's ambient-only look.
#ifdef VULKAN
layout(set = 3, binding = 0)
#else
layout(binding = 11)
#endif
uniform samplerCube PrefilteredEnvironment;

// Scene depth (the main pass's depth buffer, left readable). Screen-space
// reflections reconstruct view-space positions from it and march reflection
// rays against it.
#ifdef VULKAN
layout(set = 0, binding = 3)
#else
layout(binding = 3)
#endif
uniform sampler2D SceneDepth;

layout(location = 0) in vec2 CoordUV;
layout(location = 1) in vec3 WorldRayDir;
layout(location = 0) out vec4 FragColor;

// Overall exposure applied before tone mapping. 1.0 is neutral; raise to
// brighten, lower to darken. A future scene/camera value can drive this.
const float EXPOSURE = 1.0;

// Narkowicz's cheap ACES filmic curve: maps unbounded linear HDR radiance into
// a pleasing [0,1] range, rolling off bright highlights instead of clipping.
vec3 aces_tonemap ( vec3 x )
{
      const float a = 2.51;
      const float b = 0.03;
      const float c = 2.43;
      const float d = 0.59;
      const float e = 0.14;
      return clamp ( ( x * ( a * x + b ) ) / ( x * ( c * x + d ) + e ), 0.0, 1.0 );
}

// Encode linear colour to sRGB (approximate gamma 2.2) for the UNORM swapchain,
// which applies no hardware sRGB conversion.
vec3 linear_to_srgb ( vec3 c )
{
      return pow ( c, vec3 ( 1.0 / 2.2 ) );
}

// Convert a sampled depth-buffer value [0,1] to clip-space NDC z. Vulkan clip z
// is already [0,1]; OpenGL clip z is [-1,1].
float depth_to_ndc_z ( float d )
{
#ifdef VULKAN
      return d;
#else
      return d * 2.0 - 1.0;
#endif
}

// Reconstruct a view-space position from a screen UV + depth using the inverse
// projection. View space here is +Y forward, +Z up (matches the engine).
vec3 reconstruct_view_pos ( vec2 uv, float depth, mat4 inv_proj )
{
      vec4 clip = vec4 ( uv * 2.0 - 1.0, depth_to_ndc_z ( depth ), 1.0 );
      vec4 view = inv_proj * clip;
      return view.xyz / view.w;
}

// Screen-space reflection: march the reflection ray (view space) against the
// scene depth. On a hit within a thickness window, binary-refine and return the
// hit UV with confidence 1; otherwise confidence 0. Fades near screen edges.
float ssr_trace ( vec3 view_pos, vec3 reflect_dir, mat4 inv_proj, out vec2 hit_uv )
{
      const int   STEPS     = 32;
      const int   REFINE    = 5;
      const float MAX_DIST  = 12.0; // view-space ray length
      const float THICKNESS = 0.6;  // view-space depth window for a valid hit
      const float step_len  = MAX_DIST / float ( STEPS );
      hit_uv = vec2 ( 0.0 );
      for ( int i = 1; i <= STEPS; ++i )
      {
            vec3 sample_pos = view_pos + reflect_dir * ( step_len * float ( i ) );
            vec4 clip = ProjectionMatrix * vec4 ( sample_pos, 1.0 );
            if ( clip.w <= 0.0 )
            {
                  break;
            }
            vec2 uv = ( clip.xy / clip.w ) * 0.5 + 0.5;
            if ( uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 )
            {
                  break;
            }
            vec3 scene_pos = reconstruct_view_pos ( uv, texture ( SceneDepth, uv ).r, inv_proj );
            float delta = sample_pos.y - scene_pos.y; // +Y forward: >0 means behind
            if ( delta > 0.0 && delta < THICKNESS )
            {
                  // Binary-refine between the last in-front step and this one.
                  vec3 lo = view_pos + reflect_dir * ( step_len * float ( i - 1 ) );
                  vec3 hi = sample_pos;
                  for ( int r = 0; r < REFINE; ++r )
                  {
                        vec3 mid = ( lo + hi ) * 0.5;
                        vec4 mclip = ProjectionMatrix * vec4 ( mid, 1.0 );
                        hit_uv = ( mclip.xy / mclip.w ) * 0.5 + 0.5;
                        vec3 mscene = reconstruct_view_pos ( hit_uv, texture ( SceneDepth, hit_uv ).r, inv_proj );
                        if ( mid.y - mscene.y > 0.0 )
                        {
                              hi = mid;
                        }
                        else
                        {
                              lo = mid;
                        }
                  }
                  // Fade near the screen edges (hide the march running off the
                  // side) and toward the end of the ray (increasingly uncertain).
                  vec2 edge = smoothstep ( vec2 ( 0.0 ), vec2 ( 0.1 ), hit_uv ) *
                              ( 1.0 - smoothstep ( vec2 ( 0.9 ), vec2 ( 1.0 ), hit_uv ) );
                  float dist_fade = 1.0 - smoothstep ( 0.6, 1.0, float ( i ) / float ( STEPS ) );
                  return edge.x * edge.y * dist_fade;
            }
      }
      return 0.0;
}

// Blur the reflected colour to approximate a rougher surface's wider specular
// lobe: a small hexagonal disk of taps whose radius grows with roughness. A
// cheap stand-in for a proper cone trace; only runs on SSR-hit pixels.
vec3 sample_reflection_blur ( vec2 uv, float radius )
{
      vec3 sum = texture ( HdrColor, uv ).rgb;
      if ( radius <= 0.001 )
      {
            return sum;
      }
      vec2 taps[6] = vec2[6] (
                         vec2 (  1.0,  0.0 ),   vec2 (  0.5,  0.866 ), vec2 ( -0.5,  0.866 ),
                         vec2 ( -1.0,  0.0 ),   vec2 ( -0.5, -0.866 ), vec2 (  0.5, -0.866 ) );
      for ( int i = 0; i < 6; ++i )
      {
            sum += texture ( HdrColor, uv + taps[i] * radius ).rgb;
      }
      return sum / 7.0;
}

void main()
{
      vec3 hdr = texture ( HdrColor, CoordUV ).rgb;

      // Deferred specular ambient. The reflection source is the prefiltered
      // environment (or flat ambient), with screen-space reflections blended in
      // where a sharp surface's ray hits on-screen geometry. Skipped where the
      // specular weight is zero (the sky and non-reflective pixels).
      vec3 spec_weight = texture ( GSpecWeight, CoordUV ).rgb;
      vec3 reflection = vec3 ( 0.0 );
      if ( any ( greaterThan ( spec_weight, vec3 ( 0.0 ) ) ) )
      {
            vec4  normal_rough = texture ( GNormalRough, CoordUV );
            float roughness    = normal_rough.w;
            vec3  world_normal = normalize ( mat3 ( inverse ( ViewMatrix ) ) * normal_rough.xyz );
            vec3  world_refl   = reflect ( normalize ( WorldRayDir ), world_normal );

            // Environment reflection: the always-available fallback that SSR
            // blends over where it finds a hit.
            vec3 env_reflection;
            if ( textureSize ( PrefilteredEnvironment, 0 ).x > 1 )
            {
                  float max_lod = float ( textureQueryLevels ( PrefilteredEnvironment ) - 1 );
                  env_reflection = textureLod ( PrefilteredEnvironment, world_refl, roughness * max_lod ).rgb;
            }
            else
            {
                  env_reflection = ambient.rgb * ambient.a;
            }
            reflection = env_reflection;

            // Screen-space reflections for surfaces glossy enough to benefit
            // (the prefiltered cube handles rough ones, so matte surfaces do not
            // pick up a wet-looking sheen), blurred by roughness to match the
            // surface, and only where this pixel has real geometry (depth < 1).
            float depth = texture ( SceneDepth, CoordUV ).r;
            float ssr_fade = 1.0 - smoothstep ( 0.3, 0.5, roughness );
            if ( depth < 1.0 && ssr_fade > 0.0 )
            {
                  mat4 inv_proj  = inverse ( ProjectionMatrix );
                  vec3 view_pos  = reconstruct_view_pos ( CoordUV, depth, inv_proj );
                  vec3 view_refl = reflect ( normalize ( view_pos ), normalize ( normal_rough.xyz ) );
                  vec2 hit_uv;
                  float conf = ssr_trace ( view_pos, view_refl, inv_proj, hit_uv ) * ssr_fade;
                  if ( conf > 0.0 )
                  {
                        vec3 ssr_color = sample_reflection_blur ( hit_uv, roughness * 0.025 );
                        reflection = mix ( env_reflection, ssr_color, conf );
                  }
            }
      }

      vec3 scene = hdr + reflection * spec_weight;
      vec3 color = aces_tonemap ( scene * EXPOSURE );
      color = linear_to_srgb ( color );
      FragColor = vec4 ( color, 1.0 );
}
