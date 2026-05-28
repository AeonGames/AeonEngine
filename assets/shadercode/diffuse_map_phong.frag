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
layout(set = 4, binding = 0, std140)
#else
layout(binding = 3, std140)
#endif
uniform Lights{
      uint     LightCount;
      uint     _LightsPad0;
      uint     _LightsPad1;
      uint     _LightsPad2;
      GpuLight Lights_data[64];
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

void main()
{
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
      for ( int i = 0; i < 64; i++ )
      {
            if ( i >= int(LightCount) ) { break; }
            GpuLight L = Lights_data[i];
            vec3 to_light;
            float attenuation = 1.0;
            if ( L.type == 2u )
            {
                  // Directional: direction_cosOuter.xyz is the world-space
                  // to-light direction (i.e. -lightDir).
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
      vec3 LightIntensity = Kd * diffuse_accum + Ks * specular_accum;
      FragColor = tex * vec4 ( LightIntensity, 1.0 );
}
