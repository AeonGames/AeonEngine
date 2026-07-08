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

void main()
{
      vec3 hdr = texture ( HdrColor, CoordUV ).rgb;

      // Deferred specular ambient: reflect the view ray about the surface normal
      // and sample the prefiltered environment (or the flat ambient when a scene
      // has none), scaled by the pre-integrated specular weight. Skipped where
      // the weight is zero (the sky and non-reflective pixels).
      vec3 spec_weight = texture ( GSpecWeight, CoordUV ).rgb;
      vec3 reflection = vec3 ( 0.0 );
      if ( any ( greaterThan ( spec_weight, vec3 ( 0.0 ) ) ) )
      {
            vec4 normal_rough = texture ( GNormalRough, CoordUV );
            vec3 world_normal = normalize ( mat3 ( inverse ( ViewMatrix ) ) * normal_rough.xyz );
            vec3 world_refl = reflect ( normalize ( WorldRayDir ), world_normal );
            if ( textureSize ( PrefilteredEnvironment, 0 ).x > 1 )
            {
                  float max_lod = float ( textureQueryLevels ( PrefilteredEnvironment ) - 1 );
                  reflection = textureLod ( PrefilteredEnvironment, world_refl,
                                            normal_rough.w * max_lod ).rgb;
            }
            else
            {
                  reflection = ambient.rgb * ambient.a;
            }
      }

      vec3 scene = hdr + reflection * spec_weight;
      vec3 color = aces_tonemap ( scene * EXPOSURE );
      color = linear_to_srgb ( color );
      FragColor = vec4 ( color, 1.0 );
}
