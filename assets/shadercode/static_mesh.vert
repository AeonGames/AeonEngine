#version 450

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

// Per-object model matrices (Track E uber-pipeline). Every draw — single or
// instanced — sources its model matrix from this buffer, indexed by the
// instance id, so there is no separate instanced shader variant.
#ifdef VULKAN
layout(set = 3, binding = 0, std430)
#else
layout(binding = 3, std430)
#endif
readonly buffer InstanceMatrices
{
      mat4 InstanceModelMatrices[];
};
#ifdef VULKAN
#define MODEL_MATRIX InstanceModelMatrices[gl_InstanceIndex]
#else
#define MODEL_MATRIX InstanceModelMatrices[gl_InstanceID]
#endif

// Per-instance bindless material index (Vulkan only): written parallel to the
// model matrices and forwarded to the fragment shader as a flat varying, so a
// single indirect multi-draw can shade meshes with different materials. OpenGL
// still delivers the material index through a default-block uniform.
#ifdef VULKAN
layout(set = 1, binding = 0, std430) readonly buffer InstanceMaterials
{
      uint InstanceMaterialIndices[];
};
layout(location = 5) flat out uint vMaterialIndex;
#endif

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec3 VertexTangent;
layout(location = 3) in vec3 VertexBitangent;
layout(location = 4) in vec2 VertexUV;

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
      mat3 normal_matrix = mat3 ( ViewMatrix * MODEL_MATRIX );
      if ( length ( VertexNormal ) != 0 )
      {
            tnorm = normalize ( normal_matrix * VertexNormal );
      }
      else
      {
            tnorm = vec3 ( 0.0 );
      }
      if ( length ( VertexTangent ) != 0.0 )
      {
            ttangent = normalize ( normal_matrix * VertexTangent );
            tbitangent = normalize ( normal_matrix * VertexBitangent );
      }
      else
      {
            ttangent = vec3 ( 0.0 );
            tbitangent = vec3 ( 0.0 );
      }
      eyeCoords = ( ViewMatrix * MODEL_MATRIX * vec4 ( VertexPosition, 1.0 ) ).xyz;
      gl_Position = ProjectionMatrix * ViewMatrix * MODEL_MATRIX * vec4(VertexPosition, 1.0);
      CoordUV = VertexUV;
#ifdef VULKAN
      vMaterialIndex = InstanceMaterialIndices[gl_InstanceIndex];
#endif
}
