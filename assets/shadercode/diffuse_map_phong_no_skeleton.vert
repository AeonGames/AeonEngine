#version 450

#ifdef VULKAN
layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };
#endif
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
      vec3 LightPosition;
      vec3 Kd;
};

// Per-frame light list, populated by Renderer::SetLights from
// Scene::GetFrameLights. Mirrors the C++ GpuLightsBlock / GpuLight types in
// include/aeongames/GpuLight.hpp.
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
layout(set = 3, binding = 0, std140)
#else
layout(binding = 2, std140)
#endif
uniform Skeleton{
      mat4 skeleton[256];
};

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 VertexTangent;
layout(location = 3) in vec3 VertexBitangent;
layout(location = 4) in vec2 VertexUV;

layout(location = 0) out vec3 LightIntensity;
layout(location = 1) out vec2 CoordUV;

void main()
{
      if ( length ( VertexNormal ) != 0 )
      {
            vec3 tnorm = normalize ( mat3(ViewMatrix * ModelMatrix) * VertexNormal );
            vec4 eyeCoords = ViewMatrix * ModelMatrix * vec4 ( VertexPosition, 1.0 );
            // In eye space the camera sits at the origin, so the view
            // direction is just the negated, normalized eye-space position.
            vec3 V = normalize ( -eyeCoords.xyz );
            // Material constants for the specular term. Hardcoded here until
            // Ks / shininess are pulled from the Material UBO.
            const vec3  Ks        = vec3 ( 0.4 );
            const float Shininess = 32.0;
            // Accumulate diffuse (Lambert) and specular (Blinn-Phong) over the
            // per-frame light list, branching by light type. See
            // include/aeongames/GpuLight.hpp for the per-field interpretation.
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
                        vec3 d = light_eye - eyeCoords.xyz;
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
                  float NdotL = max ( dot ( to_light, tnorm ), 0.0 );
                  diffuse_accum += light_radiance * NdotL;
                  if ( NdotL > 0.0 )
                  {
                        // Blinn-Phong: half-vector between view and light.
                        vec3  H     = normalize ( to_light + V );
                        float NdotH = max ( dot ( tnorm, H ), 0.0 );
                        specular_accum += light_radiance * pow ( NdotH, Shininess );
                  }
            }
            LightIntensity = Kd * diffuse_accum + Ks * specular_accum;
      }
      else
      {
            LightIntensity = vec3(1,1,1);
      }
      gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(VertexPosition, 1.0);
      CoordUV = VertexUV;
}
