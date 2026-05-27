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
      uvec4 type_pad;
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
            // Accumulate Lambertian contribution over the per-frame point-light list.
            // Spot/directional types are deferred to Slice 2; treat everything as a
            // point light using position_radius.xyz in world space (transformed into
            // eye space via ViewMatrix).
            vec3 accum = vec3 ( 0.0 );
            for ( int i = 0; i < 64; i++ )
            {
                  if ( i >= int(LightCount) ) { break; }
                  vec3 light_eye = ( ViewMatrix * vec4 ( Lights_data[i].position_radius.xyz, 1.0 ) ).xyz;
                  vec3 s = normalize ( light_eye - eyeCoords.xyz );
                  float lambert = max ( dot ( s, tnorm ), 0.0 );
                  accum += Lights_data[i].color_intensity.rgb * Lights_data[i].color_intensity.a * lambert;
            }
            LightIntensity = Kd * accum;
      }
      else
      {
            LightIntensity = vec3(1,1,1);
      }
      gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(VertexPosition, 1.0);
      CoordUV = VertexUV;
}
