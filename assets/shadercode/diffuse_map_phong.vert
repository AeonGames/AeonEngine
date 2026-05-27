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
layout(location = 5) in uvec4 VertexWeightIndices;
layout(location = 6) in vec4 VertexWeights;

layout(location = 0) out vec3 LightIntensity;
layout(location = 1) out vec2 CoordUV;

void main()
{
      if ( length ( VertexNormal ) != 0 )
      {
            vec3 vertex_normal =
                  normalize(( mat3(skeleton[VertexWeightIndices[0]]) * (VertexWeights[0] * VertexNormal) ) +
                        ( mat3(skeleton[VertexWeightIndices[1]]) * (VertexWeights[1] * VertexNormal) ) +
                        ( mat3(skeleton[VertexWeightIndices[2]]) * (VertexWeights[2] * VertexNormal) ) +
                        ( mat3(skeleton[VertexWeightIndices[3]]) * (VertexWeights[3] * VertexNormal) ));
            vec3 tnorm = normalize ( mat3(ViewMatrix * ModelMatrix) * vertex_normal );
            vec4 weighted_eye_position =
                  ( skeleton[VertexWeightIndices[0]] * VertexWeights[0] * vec4 ( VertexPosition, 1 ) ) +
                  ( skeleton[VertexWeightIndices[1]] * VertexWeights[1] * vec4 ( VertexPosition, 1 ) ) +
                  ( skeleton[VertexWeightIndices[2]] * VertexWeights[2] * vec4 ( VertexPosition, 1 ) ) +
                  ( skeleton[VertexWeightIndices[3]] * VertexWeights[3] * vec4 ( VertexPosition, 1 ) );
            vec4 eyeCoords = ViewMatrix * ModelMatrix * vec4 ( weighted_eye_position.xyz, 1.0 );
            vec3 accum = vec3 ( 0.0 );
            for ( int i = 0; i < 64; i++ )
            {
                  if ( i >= int(LightCount) ) { break; }
                  GpuLight L = Lights_data[i];
                  vec3 to_light;
                  float attenuation = 1.0;
                  if ( L.type == 2u )
                  {
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
                  float lambert = max ( dot ( to_light, tnorm ), 0.0 );
                  accum += L.color_intensity.rgb * L.color_intensity.a * lambert * attenuation;
            }
            LightIntensity = Kd * accum;
      }
      else
      {
            LightIntensity = vec3(1,1,1);
      }
      vec4 weighted_position =
            ( skeleton[VertexWeightIndices[0]] * VertexWeights[0] * vec4 ( VertexPosition, 1 ) ) +
            ( skeleton[VertexWeightIndices[1]] * VertexWeights[1] * vec4 ( VertexPosition, 1 ) ) +
            ( skeleton[VertexWeightIndices[2]] * VertexWeights[2] * vec4 ( VertexPosition, 1 ) ) +
            ( skeleton[VertexWeightIndices[3]] * VertexWeights[3] * vec4 ( VertexPosition, 1 ) );
      gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(weighted_position.xyz, 1.0);
      CoordUV = VertexUV;
}
