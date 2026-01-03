#version 450

#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices
{
      mat4 ModelMatrix;
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
            vec3 tnorm = normalize ( mat3(ViewMatrix) * vertex_normal );
            vec4 eyeCoords = ViewMatrix /* * ModelMatrix */ * vec4 ( VertexPosition, 1.0 );
            vec3 s = normalize ( LightPosition - eyeCoords.xyz );
            LightIntensity = Kd * max ( dot ( s, tnorm ), 0.0 );
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
