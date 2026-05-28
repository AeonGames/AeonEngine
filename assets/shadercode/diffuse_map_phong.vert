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

// Eye-space attributes consumed by the per-pixel Phong fragment shader.
// A zero-length tnorm acts as an "unlit" sentinel (mesh has no normals).
layout(location = 0) out vec3 tnorm;
layout(location = 1) out vec3 eyeCoords;
layout(location = 2) out vec2 CoordUV;

void main()
{
      vec4 weighted_position =
            ( skeleton[VertexWeightIndices[0]] * VertexWeights[0] * vec4 ( VertexPosition, 1 ) ) +
            ( skeleton[VertexWeightIndices[1]] * VertexWeights[1] * vec4 ( VertexPosition, 1 ) ) +
            ( skeleton[VertexWeightIndices[2]] * VertexWeights[2] * vec4 ( VertexPosition, 1 ) ) +
            ( skeleton[VertexWeightIndices[3]] * VertexWeights[3] * vec4 ( VertexPosition, 1 ) );
      if ( length ( VertexNormal ) != 0 )
      {
            vec3 vertex_normal =
                  normalize(( mat3(skeleton[VertexWeightIndices[0]]) * (VertexWeights[0] * VertexNormal) ) +
                        ( mat3(skeleton[VertexWeightIndices[1]]) * (VertexWeights[1] * VertexNormal) ) +
                        ( mat3(skeleton[VertexWeightIndices[2]]) * (VertexWeights[2] * VertexNormal) ) +
                        ( mat3(skeleton[VertexWeightIndices[3]]) * (VertexWeights[3] * VertexNormal) ));
            tnorm = normalize ( mat3(ViewMatrix * ModelMatrix) * vertex_normal );
      }
      else
      {
            tnorm = vec3 ( 0.0 );
      }
      eyeCoords = ( ViewMatrix * ModelMatrix * vec4 ( weighted_position.xyz, 1.0 ) ).xyz;
      gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(weighted_position.xyz, 1.0);
      CoordUV = VertexUV;
}
