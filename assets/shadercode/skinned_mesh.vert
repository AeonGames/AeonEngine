#version 450

// Skinned shading. The model matrix is delivered per-API: Vulkan uses a push
// constant (its 8 descriptor sets are fully taken, leaving no slot for an
// object SSBO), while OpenGL — which has no push constants — sources it from
// the same per-object matrix buffer the static shaders use. Skinned meshes are
// never instanced, so that buffer holds a single matrix here.
#ifdef VULKAN
layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };
#define MODEL_MATRIX ModelMatrix
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

#ifdef VULKAN
layout(set = 3, binding = 0, std140)
#else
layout(binding = 2, std140)
#endif
uniform Skeleton{
      mat4 skeleton[256];
};

#ifndef VULKAN
layout(binding = 3, std430)
readonly buffer InstanceMatrices
{
      mat4 InstanceModelMatrices[];
};
#define MODEL_MATRIX InstanceModelMatrices[gl_InstanceID]
#endif

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
// Eye-space tangent basis for normal mapping; zero-length when the mesh has no
// tangents (the fragment shader then falls back to the interpolated normal).
layout(location = 3) out vec3 ttangent;
layout(location = 4) out vec3 tbitangent;

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
            tnorm = normalize ( mat3(ViewMatrix * MODEL_MATRIX) * vertex_normal );
      }
      else
      {
            tnorm = vec3 ( 0.0 );
      }
      // Skin the tangent basis with the same bone weights as the normal, then
      // rotate into eye space. Zero-length when the mesh has no tangents.
      if ( length ( VertexTangent ) != 0.0 )
      {
            vec3 vertex_tangent =
                  ( mat3(skeleton[VertexWeightIndices[0]]) * (VertexWeights[0] * VertexTangent) ) +
                  ( mat3(skeleton[VertexWeightIndices[1]]) * (VertexWeights[1] * VertexTangent) ) +
                  ( mat3(skeleton[VertexWeightIndices[2]]) * (VertexWeights[2] * VertexTangent) ) +
                  ( mat3(skeleton[VertexWeightIndices[3]]) * (VertexWeights[3] * VertexTangent) );
            vec3 vertex_bitangent =
                  ( mat3(skeleton[VertexWeightIndices[0]]) * (VertexWeights[0] * VertexBitangent) ) +
                  ( mat3(skeleton[VertexWeightIndices[1]]) * (VertexWeights[1] * VertexBitangent) ) +
                  ( mat3(skeleton[VertexWeightIndices[2]]) * (VertexWeights[2] * VertexBitangent) ) +
                  ( mat3(skeleton[VertexWeightIndices[3]]) * (VertexWeights[3] * VertexBitangent) );
            ttangent = normalize ( mat3(ViewMatrix * MODEL_MATRIX) * vertex_tangent );
            tbitangent = normalize ( mat3(ViewMatrix * MODEL_MATRIX) * vertex_bitangent );
      }
      else
      {
            ttangent = vec3 ( 0.0 );
            tbitangent = vec3 ( 0.0 );
      }
      eyeCoords = ( ViewMatrix * MODEL_MATRIX * vec4 ( weighted_position.xyz, 1.0 ) ).xyz;
      gl_Position = ProjectionMatrix * ViewMatrix * MODEL_MATRIX * vec4(weighted_position.xyz, 1.0);
      CoordUV = VertexUV;
}
